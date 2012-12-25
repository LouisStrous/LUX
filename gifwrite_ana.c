/* experimental gif writer, learning .... */
#if HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <strings.h>		/* for bzero */
#include "ana_structures.h"
 extern struct sym_desc sym[];
 struct GIFScreen {
        char id[6];
        unsigned char width_lsb;
        unsigned char width_msb;
        unsigned char height_lsb;
        unsigned char height_msb;
        unsigned char mask;
        unsigned char background;
        unsigned char extra;
 } ;
 struct GIFImage {
        char sep;
        unsigned char left_lsb;
        unsigned char left_msb;
        unsigned char top_lsb;
        unsigned char top_msb;
        unsigned char width_lsb;
        unsigned char width_msb;
        unsigned char height_lsb;
        unsigned char height_msb;
        unsigned char mask;
 } ;
 static void compress(Int, FILE *, Byte *, Int), output(Int),
   cl_block(void), cl_hash(register Int) /*, char_init()*/;
 static void char_out(Int), flush_char(void);

Int	ana_gifwrite(Int, Int []);
 /*------------------------------------------------------------------------- */
Int ana_gifwrite_f(narg, ps)
 /* a function version that returns 1 if read OK */
 Int	narg, ps[];
 {
 if ( ana_gifwrite(narg, ps) == 1 ) return 1; else return 4;
 }
 /*------------------------------------------------------------------------- */
Int ana_gifwrite(narg,ps)      /* gifwrite subroutine */
 /* write a very plain gif file, 8 bit deep */
 /* call is gifwrite,array,file,[map] where map is the color map 
 and must be (3,256) I*1 in rgb triplets */
 Int    narg, ps[];
 {
 FILE	*fout;
 Int    iq, nd, type, i;
 Int    nx, ny, ncolmap;
 Byte   codesize=8;
 char   *p, *data, *colormap, *name;
 struct ahead   *h, *h2;
 struct GIFScreen gh = {"GIF87a", 0,0,0,0,247,0,0};
 struct GIFImage gimage = {',', 0,0,0,0,0,0,0,0,0};

                                         /* first arg. must be an array */
 iq = ps[0];
 if ( sym[iq].class != 4 ) return execute_error(66);
 type = sym[iq].type;
 h = (struct ahead *) sym[iq].spec.array.ptr;
 data = ((char *)h + sizeof(struct ahead));
 nd = h->ndim;
 if ( nd != 2 || type != 0 )
  { printf("GIFWRITE only supports 2-D Byte arrays\n");
  return -1; }
                        /* second argument must be a string, file name */
 if ( sym[ ps[1] ].class != 2 ) return execute_error(70);
 name = (char *) sym[ps[1] ].spec.array.ptr;
                        /* third arg. must be an array, color map */
 ncolmap = 256*3;       /* length in bytes */
 if (narg > 2) {
 iq = ps[2];
 if ( sym[iq].class != 4 ) return execute_error(66);
 type = sym[iq].type;
 h2 = (struct ahead *) sym[iq].spec.array.ptr;
 colormap = ((char *)h2 + sizeof(struct ahead));
 /* we are fussy about the organization of the color map, must be 3x256 */
 if (h2->ndim != 2 || h2->dims[0] != 3 || h2->dims[1] != 256 || type != 0)
  { printf("GIFWRITE requires a 3 by 256 I*1 color map\n");
  return -1; }
 } else {
 /* if no colormap passed, we provide a b/w one */
 colormap = malloc(ncolmap);
 if (colormap == NULL) { printf("malloc error in gifwrite\n"); return -1;}
 p = colormap;
 for (i=0; i < 256; i++) { *p++ = i; *p++ = i; *p++ = i; }
 }
 nx = h->dims[0];       ny = h->dims[1];
                                                 /* try to open the file */
 if ((fout=fopen(name,"w")) == NULL) return file_open_error();

 gh.width_lsb = nx & 0xff;
 gh.width_msb = nx >> 8;
 gh.height_lsb = ny & 0xff;
 gh.height_msb = ny >> 8;
 /* note that we use 8 bit color map and 8 bit pixels */
 
 /* stream out the GIF id and screen descriptor */
 if (fwrite(&gh, 1, sizeof(gh), fout) != sizeof(gh) )
         { execute_error(90);   fclose(fout);   return -1; }
 /* now the color map */
 if (fwrite(colormap, 1, ncolmap, fout) != ncolmap) {
         execute_error(90);     fclose(fout);   return -1; }
 /* the image descriptor, some things already as they should be */
 /* image width and height same as screen */
 gimage.width_lsb = nx & 0xff;
 gimage.width_msb = nx >> 8;
 gimage.height_lsb = ny & 0xff;
 gimage.height_msb = ny >> 8;
 if (fwrite(&gimage, 1, sizeof(gimage), fout) != sizeof(gimage) )
         { execute_error(90);   fclose(fout);   return -1; }
 /* our code size is 8 */
 putc(codesize, fout);
 /* could probably use anybody's lzw compressor here */
 compress(codesize+1, fout, (Byte *) data, nx*ny);
 
 putc(0, fout);
 putc(';', fout);
 fclose(fout);
 return 1;
 }
 /*------------------------------------------------------------------------- */
 static unsigned long cur_accum = 0;
 static Int           cur_bits = 0;

#define min(a,b)        ((a>b) ? b : a)
#define XV_BITS 12    /* BITS was already defined on some systems */
#define MSDOS   1
#define HSIZE  5003            /* 80% occupancy */

 typedef unsigned char   char_type;
 
 static Int n_bits;                    /* number of bits/code */
 static Int maxbits = XV_BITS;         /* user settable max # bits/code */
 static Int maxcode;                   /* maximum code, given n_bits */
 static Int maxmaxcode = 1 << XV_BITS; /* NEVER generate this */

#define MAXCODE(n_bits)     ( (1 << (n_bits)) - 1)

 static  Int      htab [HSIZE];
 static  unsigned short codetab [HSIZE];
#define HashTabOf(i)   htab[i]
#define CodeTabOf(i)   codetab[i]

 static Int hsize = HSIZE;            /* for dynamic table sizing */

 /*
 * To save much memory, we overlay the table used by compress() with those
 * used by decompress().  The tab_prefix table is the same size and type
 * as the codetab.  The tab_suffix table needs 2**BITS characters.  We
 * get this from the beginning of htab.  The output stack uses the rest
 * of htab, and contains characters.  There is plenty of room for any
 * possible stack (stack used to be 8000 characters).
 */

#define tab_prefixof(i) CodeTabOf(i)
#define tab_suffixof(i)        ((char_type *)(htab))[i]
#define de_stack               ((char_type *)&tab_suffixof(1<<XV_BITS))

static Int free_ent = 0;       /* first unused entry */

 /*
 * block compression parameters -- after all codes are used up,
 * and compression rate changes, start over.
 */
 static Int clear_flg = 0;
 static int64_t in_count = 1;            /* length of input */
 static int64_t out_count = 0;           /* # of codes output (for debugging) */

 /*
 * compress stdin to stdout
 *
 * Algorithm:  use open addressing Double hashing (no chaining) on the 
 * prefix code / next character combination.  We do a variant of Knuth's
 * algorithm D (vol. 3, sec. 6.4) along with G. Knott's relatively-prime
 * secondary probe.  Here, the modular division first probe is gives way
 * to a faster exclusive-or manipulation.  Also do block compression with
 * an adaptive reset, whereby the code table is cleared when the compression
 * ratio decreases, but after the table fills.  The variable-length output
 * codes are re-sized at this point, and a special CLEAR code is generated
 * for the decompressor.  Late addition:  construct the table according to
 * file size for noticeable speed improvement on small files.  Please direct
 * questions about this implementation to ames!jaw.
 */

 static Int g_init_bits;
 static FILE *g_outfile;
 
 static Int ClearCode;
 static Int EOFCode;

 /********************************************************/
static void compress(Int init_bits, FILE *outfile, Byte *data, Int len)
 {
  register long fcode;
  register Int i = 0;
  register Int c;
  register Int ent;
  register Int disp;
  register Int hsize_reg;
  register Int hshift;

  /*
   * Set up the globals:  g_init_bits - initial number of bits
   *                      g_outfile   - pointer to output file
   */
  g_init_bits = init_bits;
  g_outfile   = outfile;

  /* initialize 'compress' globals */
  maxbits = XV_BITS;
  maxmaxcode = 1<<XV_BITS;
  bzero((char *) htab,    sizeof(htab));
  bzero((char *) codetab, sizeof(codetab));
  hsize = HSIZE;
  free_ent = 0;
  clear_flg = 0;
  in_count = 1;
  out_count = 0;
  cur_accum = 0;
  cur_bits = 0;

  /*
   * Set up the necessary values
   */
  out_count = 0;
  clear_flg = 0;
  in_count = 1;
  maxcode = MAXCODE(n_bits = g_init_bits);

  ClearCode = (1 << (init_bits - 1));
  EOFCode = ClearCode + 1;
  free_ent = ClearCode + 2;

  ent = *data++;  len--;

  hshift = 0;
  for ( fcode = (long) hsize;  fcode < 65536L; fcode *= 2L )
    hshift++;
  hshift = 8 - hshift;                /* set hash code range bound */

  hsize_reg = hsize;
  cl_hash( (Int) hsize_reg);            /* clear hash table */

  output(ClearCode);
    
  while (len) {
    c = *data++;  len--;
    in_count++;

    fcode = (long) ( ( (long) c << maxbits) + ent);
    i = (((Int) c << hshift) ^ ent);    /* xor hashing */

    if ( HashTabOf (i) == fcode ) {
      ent = CodeTabOf (i);
      continue;
    }

    else if ( (long)HashTabOf (i) < 0 )      /* empty slot */
      goto nomatch;

    disp = hsize_reg - i;           /* secondary hash (after G. Knott) */
    if ( i == 0 )
      disp = 1;

 probe:
    if ( (i -= disp) < 0 )
      i += hsize_reg;

    if ( HashTabOf (i) == fcode ) {
      ent = CodeTabOf (i);
      continue;
    }

    if ( (long)HashTabOf (i) >= 0 ) 
      goto probe;

 nomatch:
    output(ent);
    out_count++;
    ent = c;

    if ( free_ent < maxmaxcode ) {
      CodeTabOf (i) = free_ent++; /* code -> hashtable */
      HashTabOf (i) = fcode;
    }
    else
      cl_block();
  }

  /* Put out the final code */
  output(ent);
  out_count++;
  output(EOFCode);
 }
 /*****************************************************************
 * TAG( output )
 *
 * Output the given code.
 * Inputs:
 *      code:   A n_bits-bit integer.  If == -1, then EOF.  This assumes
 *              that n_bits =< (long)wordsize - 1.
 * Outputs:
 *      Outputs code to the file.
 * Assumptions:
 *      Chars are 8 bits long.
 * Algorithm:
 *      Maintain a BITS character long buffer (so that 8 codes will
 * fit in it exactly).  Use the VAX insv instruction to insert each
 * code in turn.  When the buffer fills up empty it and start over.
 */

 static
 unsigned long masks[] = { 0x0000, 0x0001, 0x0003, 0x0007, 0x000F,
                                  0x001F, 0x003F, 0x007F, 0x00FF,
                                  0x01FF, 0x03FF, 0x07FF, 0x0FFF,
                                  0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF };

static void output(Int code)
 {
  cur_accum &= masks[cur_bits];

  if (cur_bits > 0)
    cur_accum |= ((long)code << cur_bits);
  else
    cur_accum = code;
        
  cur_bits += n_bits;

  while( cur_bits >= 8 ) {
    char_out( (uint32_t) (cur_accum & 0xff) );
    cur_accum >>= 8;
    cur_bits -= 8;
  }

  /*
   * If the next entry is going to be too big for the code size,
   * then increase it, if possible.
   */

  if (free_ent > maxcode || clear_flg) {

    if( clear_flg ) {
      maxcode = MAXCODE (n_bits = g_init_bits);
      clear_flg = 0;
    }
    else {
      n_bits++;
      if ( n_bits == maxbits )
        maxcode = maxmaxcode;
      else
        maxcode = MAXCODE(n_bits);
    }
  }
        
  if( code == EOFCode ) {
    /* At EOF, write the rest of the buffer */
    while( cur_bits > 0 ) {
      char_out( (uint32_t)(cur_accum & 0xff) );
      cur_accum >>= 8;
      cur_bits -= 8;
    }

    flush_char();
        
    fflush( g_outfile );

#ifdef FOO
    if( ferror( g_outfile ) ) 
      FatalError("unable to write GIF file");
#endif
  }
 }
 /********************************/
static void cl_block(void)	/* table clear for block compress */
 {
  /* Clear out the hash table */

  cl_hash ( (Int) hsize );
  free_ent = ClearCode + 2;
  clear_flg = 1;

  output(ClearCode);
 }
 /********************************/
static void cl_hash(register Int hsize)          /* reset code table */
 {
  register Int *htab_p = htab+hsize;
  register long i;
  register long m1 = -1;

  i = hsize - 16;
  do {                            /* might use Sys V memset(3) here */
    *(htab_p-16) = m1;
    *(htab_p-15) = m1;
    *(htab_p-14) = m1;
    *(htab_p-13) = m1;
    *(htab_p-12) = m1;
    *(htab_p-11) = m1;
    *(htab_p-10) = m1;
    *(htab_p-9) = m1;
    *(htab_p-8) = m1;
    *(htab_p-7) = m1;
    *(htab_p-6) = m1;
    *(htab_p-5) = m1;
    *(htab_p-4) = m1;
    *(htab_p-3) = m1;
    *(htab_p-2) = m1;
    *(htab_p-1) = m1;
    htab_p -= 16;
  } while ((i -= 16) >= 0);

  for ( i += 16; i > 0; i-- )
    *--htab_p = m1;
 }
 /******************************************************************************
 *
 * GIF Specific routines
 *
 ******************************************************************************/
 /*
 * Number of characters so far in this 'packet'
 */
static Int a_count;

 /*
 * Set up the 'Byte output' routine
static void char_init()
 {
        a_count = 0;
 }
 */

 /*
 * Define the storage for the packet accumulator
 */
 static char accum[ 256 ];

 /*
 * Add a character to the end of the current packet, and if it is 254
 * characters, flush the packet to disk.
 */
static void char_out(Int c)
 {
  accum[ a_count++ ] = c;
  if( a_count >= 254 ) 
    flush_char();
 }

 /*
 * Flush the packet to disk, and reset the accumulator
 */
static void flush_char(void)
 {
  if( a_count > 0 ) {
    fputc( a_count, g_outfile );
    fwrite( accum, 1, a_count, g_outfile );
    a_count = 0;
  }
 }       
