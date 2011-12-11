/* ana modules for decoding compressed TRACE images, this is separate from the
   earlier test modules which are not needed for most distributions. Therefore
   some duplication of tables and such. The name of this file is  trace_decoder
   to avoid (too much) confusion with the standalone program trace_decode */
 /* R. Shine 2/16/98 */
 /* the decoding steps are:

 load relevant Huffman tables, one for dc and one for ac terms

 convert these Huffman tables into a usable form for decoding

 load Q table

 scan for messages and handle pads (note that TRACE messages are a bit
 different from standard JPEG) while Huffman decoing the stream

 perform the IDCT to restore the image
 */
#include  <stdio.h>
#include  <sys/stat.h>
#include <strings.h>		/* for bzero */
#include <string.h>		/* for memmove */
#include "ana_structures.h"
 /* for SGI only (?) */
#if __sgi
#include  <sys/types.h>
#include  <malloc.h>
#endif
#ifdef __alpha
#include  <stdlib.h>
#endif
#define huff_EXTEND(x,s)  ((x) < extend_test[s] ? (x) + extend_offset[s] : (x))
#define MAX_GAPS  50
 extern	struct sym_desc sym[];
 extern	struct sym_list		*subr_sym_list[];
 extern	int	temp_base;
 extern	int	edb_context;
 extern	int	ana_float();
 extern	char *find_sym_name();
 extern	int	ana_type_size[];
 extern	int	vfix_top, num_ana_subr, next_user_subr_num;
 extern	float	float_arg();
 extern	double	double_arg();
 static const int extend_test[16] =   /* entry n is 2**(n-1) */
  { 0, 0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
    0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000 };

 static const int extend_offset[16] = /* entry n is (-1 << n) + 1 */
  { 0, ((-1)<<1) + 1, ((-1)<<2) + 1, ((-1)<<3) + 1, ((-1)<<4) + 1,
    ((-1)<<5) + 1, ((-1)<<6) + 1, ((-1)<<7) + 1, ((-1)<<8) + 1,
    ((-1)<<9) + 1, ((-1)<<10) + 1, ((-1)<<11) + 1, ((-1)<<12) + 1,
    ((-1)<<13) + 1, ((-1)<<14) + 1, ((-1)<<15) + 1 };

 FILE	*fopen(), *fin, *fout;
 char	*default_huffman = {"/hosts/shimmer/usr/people/shine/trace/hufpack1"};
 char	*default_qt = {"/hosts/shimmer/usr/people/shine/trace/qt1"};
 /* global tables for Huffman decoding*/
 byte	dc_look_nbits[256],dc_look_sym[256],ac_look_nbits[256],ac_look_sym[256];
 byte	dc_bits[16], dc_huffval[16], ac_bits[16], ac_huffval[256];
 int	dc_mincode[16], dc_maxcode[16], dc_valptr[16];
 int	ac_mincode[16], ac_maxcode[16], ac_valptr[16];

 /* image size and # of blocks */
 static	int	nblocks, nx, ny, qfactor;
 static	int	n_restarts_found = 0, n_unexpected = 0;

 /* zag[i] is the natural-order position of the i'th element of zigzag order. */
 
 static const int zag[64] = {
   0,  1,  8, 16,  9,  2,  3, 10,
  17, 24, 32, 25, 18, 11,  4,  5,
  12, 19, 26, 33, 40, 48, 41, 34,
  27, 20, 13,  6,  7, 14, 21, 28,
  35, 42, 49, 56, 57, 50, 43, 36,
  29, 22, 15, 23, 30, 37, 44, 51,
  58, 59, 52, 45, 38, 31, 39, 46,
  53, 60, 61, 54, 47, 55, 62, 63
 };

 static float aansf[8] = { 1.0, 1.387039845, 1.306562965, 1.175875602,
	   1.0, 0.785694958, 0.541196100, 0.275899379};
 static float ws[64], fqtbl[64], bias = 2048.5;
/* short	*dct = NULL; */
 char	dct_area[2400000];
 int	del_indices[2048];
 byte	*start_indices[2048];
 char	*dct_head = dct_area;
 short	*dct = (short *) (dct_area + sizeof( struct ahead ));
 static	short	*qt;
 static	short	qt_all[64][64];
 int	bigendian;
 short	bcorrect;

 /* ----------------------------------------------------*/
void prefill(dct_ptr, nb)
 /* used for areas where data may not be present, results in zero after
 DCT operation */
 short	*dct_ptr;
 int	nb;
 {
 short * pq = dct_ptr;
 bzero(dct_ptr, nb*64*sizeof(short));
 /* pre-load the dc values (everybody already 0) with bias */
 while (nb--) { *pq = bcorrect;  pq += 64;}
 }
 /* ----------------------------------------------------*/
int huff_setups( packed_huffman)
 byte *packed_huffman;
 {
 unsigned char huffsize[256], *p, *bits, *huffval;
 unsigned short	huffcode[256], *pc;
 int	*mincode, *maxcode, *valptr;
 byte	*look_nbits, *look_sym;
 int	i, j, code, k, iq, n;
 int	jq, ntable, lookbits;
 /* unpack the Huffman tables read from file, make our own copy
 format for Huffman files:

 304 bytes consisting of
 
 dcbits		0:15
 dcvalues	16:31
 acbits		32:47
 acvalues	48:303
 
 first 3 are 16 byte arrays, last is 156 byte array
 */
  p = dc_bits;
  n = 16;	while (n--) *p++ = *packed_huffman++;
  p = dc_huffval;
  n = 16;	while (n--) *p++ = *packed_huffman++;
  p = ac_bits;
  n = 16;	while (n--) *p++ = *packed_huffman++;
  p = ac_huffval;
  n = 256;	while (n--) *p++ = *packed_huffman++;

 /* use bits to generate the Huffman codes and sizes in code-length order */
 /* for both dc and ac tables */
 ntable = 2;
 bits = dc_bits;	valptr = dc_valptr;	mincode = dc_mincode;
 maxcode = dc_maxcode;	look_nbits = dc_look_nbits;
 look_sym = dc_look_sym;	huffval = dc_huffval;
 while (ntable--) {
 p = huffsize;
 pc = huffcode;
 code = 0;
 for (k = 0; k < 16; k++) {
   j = k + 1;	n = (int) bits[k];	/* n is the number of this length */
   while (n--)  {			/* the codes of a common length */
     /* *p++ = (unsigned char) (j); */	/* just increase by 1 wrt previous */
	*pc++ = code++;	  }
  code <<= 1;				/* for new code size, left shift */
  }
 /* *p = 0; */		/* 9/1/98 - this can point out of range !! */
 /* lastp = p - huffsize; */

 /* Figure F.15: generate decoding tables for bit-sequential decoding */
 iq = 0;
 for (k = 0; k < 16; k++) {
   if (bits[k]) {
     valptr[k] = iq; 	/* huffval[] index of 1st symbol of code length k+1 */
     mincode[k] = huffcode[iq]; /* minimum code of length k+1 */
     iq += bits[k];
     maxcode[k] = huffcode[iq-1]; /* maximum code of length k+1 */
   } else {
     maxcode[k] = -1;	/* -1 if no codes of this length */
   }
 }

 /* generate lookup tables to speed up the process for Huffman codes of
 length 8 or less, one set for dc and ac, follows technique in jpeg-5a but
 not the same code so some variables may look similar but beware ! */
 
 bzero(look_nbits, 256);
 iq = 0;
 for (k = 0; k < 8; k++) {
  for (i = 0; i < (int) bits[k]; i++, iq++) {
  /* k+1 = current code's length, iq = its index in huffcode[] & huffval[]. */
  /* Generate left-justified code followed by all possible bit sequences */
    lookbits = huffcode[iq] << (7 - k);
    jq = k+1;
    for (j= 1 << (7 - k);j > 0;j--) {
      look_nbits[lookbits] = jq;
      look_sym[lookbits] = huffval[iq];
      lookbits++;
    }
  }
 } 
 bits = ac_bits;	valptr = ac_valptr;	mincode = ac_mincode;
 maxcode = ac_maxcode;	look_nbits = ac_look_nbits;
 look_sym = ac_look_sym;	huffval = ac_huffval;
 }
 return 1;
 }
 /*------------------------------------------------------------------------- */
static int huff_decode_err(n)
 int	n;
 {
 printf("Huffman decoding error # %d\n", n);
 return -1; 
 }
 /*------------------------------------------------------------------------- */
static int dct_buffer_decode_err(limit)
 int limit;
 {
 printf("Exceeded data size during Huffman decompression , limit =%d\n", limit);
 return -1;
 }
 /*------------------------------------------------------------------------- */
static int huff_decode_dct(dct, nblocks, x, limit)
 /* returns decoded dct, the coded data is in x, huffman tables as in
 huff_encode_dct */
 /* assumes messages and pads already removed by a pre-scan */
 short	dct[];
 byte	x[];
 int	limit, nblocks;
 {
 int	i, j, k, idct, r1, last_dc=0;
 int	jq, look, nb, lbits, ks;
 unsigned int	temp, temp2, nbits, rs;
 unsigned int mask[] = { 0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff,
  0x1ff,0x3ff,0x7ff,0xfff,0x1fff,0x3fff,0x7fff,0xffff};
 byte *px;

 px = x;	/* where we get the Huffman coded version */
 r1 = 0;	/* a bit count */

 /* decode the dc and ac components of a block */
 /* limit is the max number of bytes in the stream */
 /* parts modified from the jpeg-5a code */

 while (nblocks--) {
 bzero(dct, 128);	/* pre-zero this block */
 /* start dc decode, grab 8 bits and use lookup shortcut to see if we
 got a short Huffman code */
 
 i=r1>>3;	j=r1%8;		/* our byte and bit addresses */
 if (i > limit) return dct_buffer_decode_err(limit);
 px = x + i;
 if (j == 0) {	/* aligned on byte, lucky 1/8 */
  look = *px;
 } else {	/* need part of next byte as well */
  jq = 8 - j;	/* available bits in px */
  look = *px++ << j;	/* msb, make room for lsb */
  look |= ((int) *px) >> jq;
  look = look & 0xff;
 }
 if ((nb = dc_look_nbits[look]) != 0) {
     nbits = dc_look_sym[look];
     lbits = 8 -nb;
     r1 += nb;
 } else {
     /* get here if the code is longer than 8 bits or some kind of disaster */
     r1 += 8;	/* point after look */
     i=r1>>3;	j=r1%8;		px = x + i;
     /* get 16 bits in temp */
     temp = ((int) look) << 8;	/* look will be top 8 */
     if (j == 0) { temp = temp | ( (int) *px );
     } else {
     /* need 2 transfers, 8 bits total */
     jq = 8 - j;	/* available bits in px */
     temp |= ( *px++ << j );
     temp |= ( ((int) *px) >> jq);
     }
     k = 7;	nb = 8;		/* here nb is code size - 1 */
     while ( (ks =(temp >> k)) > dc_maxcode[nb] ) { k--; nb++;
     	if (k < 0)  return huff_decode_err(0); }
     /* note error return if we couldn't find a code */
     nbits = dc_huffval[ dc_valptr[nb] + ks - dc_mincode[nb] ];
     nb++;	/* actual size of code */
     r1 += (nb -8);
     lbits = 0;
 }
 /* that defines the dc range interval, now get the rest of the bits */
 /* if nbits is 0, don't need any */
 if (nbits) {
 /* we have some still in look, lbits */
 /* if so, nb is valid, so use it to shift old look */
 if (lbits >= nbits) {  temp = (look>> (lbits-nbits)) & mask[nbits];
 } else {
 /* harder, need "nbits" # of bits */
 i=r1>>3;	j=r1%8;		px = x + i;
 jq = 8 - j;
 temp = ( *px  & mask[jq] );	/* gets us jq ls bits */
 ks = nbits - jq;		/* how much more needed (could be < 0) */
 if (ks>0) { temp = temp << ks;	/* shift over to make room */
  ks -=8;	temp2 = ((int) *++px);
  if (ks>0) { temp |= ( temp2 << ks );	/* need a third ? */
    ks -=8;	temp2 = ((int) *++px);
    }
 temp |= ( temp2 >> (-ks) );		/* last, #2 or #3 */
 } else { temp = temp >> (-ks); }
 }
 r1 += nbits;	/* count after this is done */
 /* now extend the sign, uses the jpeg-5a macro defined above */
 /* we use the lookup table version */
 nbits = huff_EXTEND(temp, nbits);
 }
 dct[0] = last_dc = nbits + last_dc;
 /* wraps the dc term, start the ac */

 for (idct = 1; idct < 64; idct++) {
 i=r1>>3;	j=r1%8;		px = x + i;
 if (i > limit) return dct_buffer_decode_err(limit);
 /* decode the Huffman symbol, use ac tables */
 if (j == 0) {	/* aligned on byte, lucky 1/8 */
  look = *px;
 } else {	/* need part of next byte as well */
  jq = 8 - j;	/* available bits in px */
  look = *px++ << j;	/* msb, make room for lsb */
  look |= ((int) *px) >> jq;
  look = look & 0xff;
 }
 if ((nb = ac_look_nbits[look]) != 0) {
     nbits = ac_look_sym[look];
     lbits = 8 -nb;
     r1 += nb;
 } else {
     /* get here if the code is longer than 8 bits or some kind of disaster */
     r1 += 8;	/* point after look */
     i=r1>>3;	j=r1%8;		px = x + i;
     /* get 16 bits in temp */
     temp = ((int) look) << 8;	/* look will be top 8 */
     if (j == 0) { temp = temp | ( (int) *px );
     } else {
     /* need 2 transfers, 8 bits total */
     jq = 8 - j;	/* available bits in px */
     temp |= ( *px++ << j );
     temp |= ( ((int) *px) >> jq);
     }
     /*printf("16 bit candidate in slow decode = %#x\n", temp);*/
     k = 7;	nb = 8;		/* here nb is code size - 1 */
     while ( (ks =(temp >> k)) > ac_maxcode[nb] ) { k--; nb++;
     	if (k < 0)  {
	printf("error at i,j,r1,idct = %d,%d,%d,%d\n",i,j,r1,idct);
	printf("temp, ks, k, nb =  %d,%d,%d,%d\n",temp, ks, k, nb);
	return huff_decode_err(1);
	}
	}
     /* note error return if we couldn't find a code */
     nbits = ac_huffval[ ac_valptr[nb] + ks - ac_mincode[nb] ];
     nb++;	/* actual size of code */
     r1 += (nb -8);
     lbits = 0;
 }
 /* that defines the ac symbol, contains a zero run length and bits in
 next non-zero coeff. */
 rs = nbits >> 4;	/* run length */
 nbits = nbits & 0xf;	/* bits in next one, unless 0, then a special code */
 if (nbits == 0) {
   if (rs != 15) break;		/* hit the end, rest are all zips */
   idct += 15;
 } else {
 idct += rs;			/* skip over the zeroes */
 /* need the next nbits bits */
 /* we have some still in look, lbits */
 /* if so, nb is valid, so use it to shift old look */
 if (lbits >= nbits) {  temp = (look>> (lbits-nbits)) & mask[nbits];
 } else {
 /* harder, need "nbits" # of bits */
 i=r1>>3;	j=r1%8;		px = x + i;
 jq = 8 - j;
 temp = ( *px  & mask[jq] );	/* gets us jq ls bits */
 ks = nbits - jq;		/* how much more needed (could be < 0) */
 if (ks>0) { temp = temp << ks;	/* shift over to make room */
  ks -=8;	temp2 = ((int) *++px);
  if (ks>0) { temp |= ( temp2 << ks );	/* need a third ? */
    ks -=8;	temp2 = ((int) *++px);
    }
 temp |= ( temp2 >> (-ks) );		/* last, #2 or #3 */
 } else { temp = temp >> (-ks); }
 }
 r1 += nbits;	/* count after this is done */
 /* now extend the sign, uses the jpeg-5a macro defined above */
 /* we use the lookup table version */
 nbits = huff_EXTEND(temp, nbits);
 /* this nbits is now the ac term */
 dct[idct] = (short) nbits;
 }
 }	/* end of idct loop for ac terms */
 dct += 64;
 }	/* end of nblocks loop */
 return 1;
 }
 /*------------------------------------------------------------------------- */
static int dct_err(n)
 int	n;
 {
 printf("DCT error # %d\n", n);
 return -1; 
 }
 /*------------------------------------------------------------------------- */
static int rdct(image, nx, ny, nblocks, qtable, dct_array)
 /* reverse dct, quant, and re-order included, result is I*2 */
 /* nx and ny must be multiples of 8 (check in calling routine) */
 /* qtable is not modified */
 short	*dct_array;
 short	*qtable, *image;
 int	nx, ny, nblocks;
 {
   /* void rdct_cell(); */
 int	n, i, ncx, iq, ystride, icx, icy, row, col;
 short	*dctstart;
 /* condition the q table for the dct's we created, must be same input qtable */
 i = 0;
 for (i = 0; i < 64; i++) {
   row = zag[i] >> 3;
   col = zag[i] & 7;
   fqtbl[i] = (float) qtable[i] * aansf[row] * aansf[col] * 0.125;
   }
 n = nx*ny/64;	/* number of cells */
 /* check consistency of nx, ny, and nblocks */
 if (nblocks != n) return dct_err(0);
 ncx = nx/8;	ystride = nx;
 dctstart = dct_array;
 
 for (i=0; i < n; i++) {
 short	*start;
 /* note that we access image by 8x8 blocks but dct is stored seq. */
 icx = i%ncx;		icy = i/ncx;	iq = icx*8 + icy*8*ystride;
 start = image + iq;
 /* make a fp copy of this cell */
 /* pf = dctfp;	pff = dctstart; */
 /* for (j=0;j < 64; j++)  *pf++ = (float) *pff++; */

 {
 int i;
 float tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
 float tmp10, tmp11, tmp12, tmp13;
 float z5, z11, z13, z10, z12;

 /* first de-zag and de-quantize */
 { register float *wsptr;
  short	 *dctptr;
  wsptr = ws;	dctptr = dctstart;
  for (i=0; i < 64; i++) {
  wsptr[zag[i]] = (float) *dctptr++ * fqtbl[i];
  }
 }
  /* Pass 1: process columns. */
  /* we don't check for columns of zeroes since this usually uses full
  precision */
  {
  register float *wsptr = ws;
  /* register float *fqtptr = fqtbl; */
  int	nq = 8;
  while (nq--) {
    tmp0 = wsptr[0];
    tmp1 = wsptr[8*2];
    tmp2 = wsptr[8*4];
    tmp3 = wsptr[8*6];

    tmp10 = tmp0 + tmp2;	/* phase 3 */
    tmp11 = tmp0 - tmp2;

    tmp13 = tmp1 + tmp3;	/* phases 5-3 */
    tmp12 = (tmp1 - tmp3) *  1.414213562 - tmp13; /* 2*c4 */

    tmp0 = tmp10 + tmp13;	/* phase 2 */
    tmp3 = tmp10 - tmp13;
    tmp1 = tmp11 + tmp12;
    tmp2 = tmp11 - tmp12;
    
    /* Odd part */

    tmp4 = wsptr[8];
    tmp5 = wsptr[8*3];
    tmp6 = wsptr[8*5];
    tmp7 = wsptr[8*7];

    z13 = tmp6 + tmp5;		/* phase 6 */
    z10 = tmp6 - tmp5;
    z11 = tmp4 + tmp7;
    z12 = tmp4 - tmp7;

    tmp7 = z11 + z13;		/* phase 5 */
    tmp11 = (z11 - z13) * ( 1.414213562); /* 2*c4 */

    z5 = (z10 + z12) * ( 1.847759065); /* 2*c2 */
    tmp10 = ( 1.082392200) * z12 - z5; /* 2*(c2-c6) */
    tmp12 = ( -2.613125930) * z10 + z5; /* -2*(c2+c6) */

    tmp6 = tmp12 - tmp7;	/* phase 2 */
    tmp5 = tmp11 - tmp6;
    tmp4 = tmp10 + tmp5;

    wsptr[0]   = tmp0 + tmp7;
    wsptr[8*7] = tmp0 - tmp7;
    wsptr[8]   = tmp1 + tmp6;
    wsptr[8*6] = tmp1 - tmp6;
    wsptr[8*2] = tmp2 + tmp5;
    wsptr[8*5] = tmp2 - tmp5;
    wsptr[8*4] = tmp3 + tmp4;
    wsptr[8*3] = tmp3 - tmp4;

    /* fqtptr++; */
    wsptr++;		/* advance pointers to next column */
  } }

  /* Pass 2: process rows. */
  {
  register float *wsptr;
  register short *elemptr;
  int	nq = 8;
  wsptr = ws;	elemptr = start;
  while (nq--) {
      /* Even part */

    tmp10 = wsptr[0] + wsptr[4] + bias;
    tmp11 = wsptr[0] - wsptr[4] + bias;

    tmp13 = wsptr[2] + wsptr[6];
    tmp12 = (wsptr[2] - wsptr[6]) * ( 1.414213562) - tmp13;

    tmp0 = tmp10 + tmp13;
    tmp3 = tmp10 - tmp13;
    tmp1 = tmp11 + tmp12;
    tmp2 = tmp11 - tmp12;

    /* Odd part */

    z13 = wsptr[5] + wsptr[3];
    z10 = wsptr[5] - wsptr[3];
    z11 = wsptr[1] + wsptr[7];
    z12 = wsptr[1] - wsptr[7];

    tmp7 = z11 + z13;
    tmp11 = (z11 - z13) * ( 1.414213562);

    z5 = (z10 + z12) * ( 1.847759065); /* 2*c2 */
    tmp10 = ( 1.082392200) * z12 - z5; /* 2*(c2-c6) */
    tmp12 = ( -2.613125930) * z10 + z5; /* -2*(c2+c6) */

    tmp6 = tmp12 - tmp7;
    tmp5 = tmp11 - tmp6;
    tmp4 = tmp10 + tmp5;

    /* Final output stage, note bias was added in above */
    /* we don't range limit since results should be near exact */
    elemptr[0] = (short) (tmp0 + tmp7);
    elemptr[7] = (short) (tmp0 - tmp7);
    elemptr[1] = (short) (tmp1 + tmp6);
    elemptr[6] = (short) (tmp1 - tmp6);
    elemptr[2] = (short) (tmp2 + tmp5);
    elemptr[5] = (short) (tmp2 - tmp5);
    elemptr[4] = (short) (tmp3 + tmp4);
    elemptr[3] = (short) (tmp3 - tmp4);

    wsptr += 8;
    elemptr += ystride;		/* to next row */
  } }
 }

  /* rdct_cell(&image[iq], ystride, dctfp); */
 dctstart += 64;
 }
 return 1;	/* success return */
 }
 /*---------------------------------------------------------------------------*/
int scan_err(n)
 int	n;
 {
 printf("scan error # %d\n", n);
 return -1; 
 }
 /*------------------------------------------------------------------------- */
int trace_scan(x, limit)
 /* scans the contents of x for a TRACE image, decodes cleaned up blocks */
 byte	x[];
 int	limit;
 {
 byte	*px, *pt, *pb;
 short	*dct_ptr;
 int	i, j, k, n, code, iblock = 0, restart_interval = 8, stat;
 int    RestartNumber, PrevRestartNumber, bcount, knowngaps, expected_restarts;
 int    GapCount, missed, lq, dcount, gcount, next_gap, dq, count_prior;
 int    gapsize[MAX_GAPS], gaprollover[MAX_GAPS];
 int    gapblock[MAX_GAPS], gaprs[MAX_GAPS], gapdel[MAX_GAPS];
 float	gap_rollf[MAX_GAPS];
 float	del_mean, gapdel_mean, fq, gsum, cfac, delq, dismerit1, dismerit2;
 int	del_total, gapdel_total, gap_guess, del_prior, missed_total;
 n_restarts_found = n_unexpected = 0;
 /* for TRACE the restart_interval is always 8 but in general it could be
 specified by a DRI message */

 px = x;
 /* look for a comment, it will have the image size and Q */
 /* i = 0; */
 for (;;) {
 if ( (limit = limit - 2) < 0) { return scan_err(0); }
 /* printf("i, *px, px = %d %#x %#x\n", i, *px, px); */
 /* i++; */
 if (*px++  == 0xff) {
  if (*px++ == 0xfe) { /*printf("got the comment\n");*/ break; } else {
  	px--;
	printf("unexpected code before comment = %#x\n", *px );
	/* this means a data gap between the SOI and the comment, fatal
	for now but we could save part of the data and we'll try eventually */
	return scan_err(0);
	}
  }
 px++;   /* skip next byte since only evens can be messages for TRACE */
 if ( (limit = limit - 2) <= 0) return scan_err(0);
 }
 /* the comment length should be 20, in the next 2 bytes */
 n = px[0] * 256 + px[1];
 if (n != 20) return scan_err(3);
 nx = px[12]*256 + px[13];
 ny = px[14]*256 + px[15];
 qfactor = px[18]*256 + px[19];
 nblocks = nx*ny/64;
 expected_restarts = nblocks/8;
 /* printf("nx, ny, q = %d %d %d\n", nx, ny, qfactor); */
 /* a sanity check, limit the size */
 if ( (nx*ny) > 1048576) {
   printf("data size too large, must stop\n");
   return ANA_ZERO;
   }

 /* now get memory for the DCT result */
 /* printf("nblocks = %d\n", nblocks); */
 /* dct = (short *) malloc(nblocks*64*sizeof(short)); */
 /* if (dct == NULL) { printf("malloc failed for DCT\n"); return 0;} */

 px += 20;
 limit = limit - 20;
 /* look for a SOI */
 /* i = 0; */
 for (;;) {
 /* if we hit the limit, it is likely that the EOI is missing because of a data
 gap, fatal for now but eventually ... */
 if ( (limit = limit - 2) < 0) { return scan_err(0); }
 /* printf("i, *px, px = %d %#x %#x\n", i, *px, px); */
 /* i++; */
 if (*px++  == 0xff) {
  if (*px++ == 0xd8) { /* printf("got the SOI\n"); */ break; } else {
  	px--;
	printf("unexpected code before SOI = %#x\n", *px ); }
  }
 px++;   /* skip next byte since only evens can be messages for TRACE */
 }

 /* in the TRACE world, the data starts after the SOI rather than a SOS */
 /* we should be pointing right after the SOI now, we now transfer the
 data while removing pads and checking for messages, the re-formatted
 data will always be shorter so use the same buffer, start at beginning
 which overwrites the SOI and anything before it */
 pt = pb = x;
 n = 0;		/* n will be the bytes found between restarts */
 PrevRestartNumber = 0;
 GapCount = knowngaps = 0;
 for (;;) {	/* scan until we hit limit or decode nblocks or error */
 if ( (limit = limit - 2) < 0) { return scan_err(0); }
 if ( (*pt++ = *px++) == 0xff) {	/* a message or pad? */
  code = *px++ & 0x00ff;
  if (code == 0)
    /* just a padded ff, ff in stream already, just bump n */
     n++; else {
   if (code == 0xff) {
     /* this is 2 ff's */
     *pt++ = 0xff;	n +=2;	} else {
     /* look for restarts and EOI (which also marks the end of a
     restart block) */
     if ( ((code & 0xf8) == 0xd0) || (code == 0xd9) )
     {
     --pt;
     start_indices[n_restarts_found] = pb;  /* this is beginning of this region */
     /* printf("%d  %d", n_restarts_found, start_indices[n_restarts_found]-x);
	if (n_restarts_found) printf(" %d\n", start_indices[n_restarts_found]-start_indices[n_restarts_found-1]); */
     n_restarts_found += 1;
     /* printf("message code = %#x\n", code); */
     /* either the end of image or a restart, process what we have */
     /* note that normal
     TRACE images will always have an integer number of restart
     intervals */
     /* nb = restart_interval;
       if ( (nblocks - iblock) < nb ) nb = nblocks - iblock;
       if (nb <= 0) { return scan_err(2); } */
     /* printf("decoding, n, nb = %d, %d\n", n, nb);*/
     dct_ptr = dct + iblock*64;

     /* check for gap, even if the code is EOI */
     if (code == 0xd9) RestartNumber = 7; else  RestartNumber = code & 0x07;
     if ( RestartNumber != PrevRestartNumber) {
      bcount = ( (RestartNumber - PrevRestartNumber) & 7);
      printf("found a gap, size = %d\n", bcount);
      /* following line is for debug only */
      if (code == 0xd9) printf("gap was at end\n");
      knowngaps += bcount;	/* the excess, actual gap could be larger */
      gapsize[GapCount] = bcount;
      bcount++;			/* excess + 1 is the # of intervals */
      if (GapCount == 0) {
        /* for the first gap, pre-fill the rest of the array, if there are no
	gaps this isn't necessary, note that we also need to check for a hidden
	gap(s) and if we find any, make any fills as needed */
       prefill(dct_ptr, (nblocks-iblock));
      }
      iblock +=  restart_interval*bcount;
      gapblock[GapCount] = iblock;	/* iblock for next data */
      gaprs[GapCount] = n_restarts_found;
      GapCount++;		/* the number of gaps */
      if (GapCount >= MAX_GAPS) {
      	printf("excessive # of gaps! Giving up.\n"); return -1; }
     } else iblock +=  restart_interval;
     /* but that was all for the next one, only reason we do it above is to
     make sure the pre-fill gets done for the first gap */
     
     stat = huff_decode_dct(dct_ptr, restart_interval, pb, n);
     if (stat != 1) printf("error at iblock = %d\n", iblock);
 
     /* if the code was the end of image, we should be done */
     if (code == 0xd9) {
       /* printf("got the EOI\n");
	  printf("end position %d, %d\n", pt - x, pt - start_indices[n_restarts_found-1]); */
       /* check if we are square */
       if (expected_restarts != n_restarts_found) {
	 /* printf("deficit in restarts, expected = %d, found = %d, missing =%d\n",
	//	expected_restarts, n_restarts_found,
	//		expected_restarts - n_restarts_found);
	//printf("# of gaps = %d, known missing blocks = %d\n",
	//	GapCount, knowngaps); */
       /* check if known gaps can handle it */
       missed_total = expected_restarts - n_restarts_found;
       missed =  missed_total - knowngaps;
        if (missed) {
	  /* printf("known gaps insufficient, still missing %d\n", missed); */
	/* this means we have to try to adjust things */
	/* compute the dels */
	dcount = 0;
	if (GapCount) next_gap = gaprs[0]; else next_gap = -1;
	gcount =0; del_total = 0.0;
	gapdel_total = 0;	del_prior = 0;	count_prior = 0;

	for (i=0;i<(n_restarts_found-1);i++) {
	 dq = del_indices[i] = start_indices[i+1] - start_indices[i];
	/* also compute mean del, ignoring known gaps */
	 if (next_gap == (i+1)) {
	   /* this is a known gap */
	   gapdel_total += dq;
	   /* gapdel[gcount] = dq;
	   //if (count_prior) gap_del_prior[gcount] = (float) del_prior/count_prior;
	   //   else gap_del_prior[gcount] = 0.0;
	   //del_prior = 0;	count_prior = 0;
	   //printf("del at gap %d = %d, mean del prior = %6.1f\n", gcount, dq,
	   //	gap_del_prior[gcount]); */
	   gcount++;
	   if (gcount < GapCount)  next_gap = gaprs[gcount]; else next_gap = -1;
	   /* printf("next gap at %d\n", next_gap); */
	 } else { del_total += dq; dcount++; del_prior += dq; count_prior++; }
	}
	/* 3/9/99, also have to provide a final del_indices based on the last
	start_indices and the final position (in pt) if we have a gap at the very end */
	/* printf("next_gap = %d, n_restarts_found = %d\n", next_gap, n_restarts_found); */
	if (next_gap == n_restarts_found) {
	 dq = del_indices[n_restarts_found-1] = pt - start_indices[n_restarts_found-1];
	 gapdel_total =+ dq;
	} else del_indices[n_restarts_found-1] = 0;
	del_mean = (float) del_total/dcount;
	/* printf("del_mean = %6.1f, intervals used = %d\n", del_mean, dcount); */
	/* compute mean del for the missing areas */
	gapdel_mean = (float) gapdel_total/(missed_total);
	/* printf("gapdel_mean = %6.1f\n", gapdel_mean); */

	/* scan for large values in del_indices that might be hidden gaps */
	if (GapCount) next_gap = gaprs[0]; else next_gap = -1;
	gcount =0;
	k = 0;	/* use to reconstruct iblock value */
	for (i=0;i<(n_restarts_found);i++) {
	 dq = del_indices[i];
	 if (next_gap == (i+1)) {
	   k +=  restart_interval*(gapsize[gcount]+1);
	   /* this is a known gap, mark with a negative del */
	   del_indices[i] = -del_indices[i];
	   /* printf("large del = %d at %d, factor = %6.1f", dq, i,
	//	(float) dq/del_mean);
	//printf("   known gap # %d\n", gcount); */
	   gcount++;
	   if (gcount < GapCount)  next_gap = gaprs[gcount]; else next_gap = -1;
	   /* printf("next gap = %d\n", next_gap); */
	 } else {
	 k +=  restart_interval;
	 if (dq > 1010 && dq > 5.*del_mean) {
	   /* printf("large del = %d at %d, factor = %6.1f", dq, i,
	   //	(float) dq/del_mean );
	   //printf("   potential hidden gap\n"); */
	   del_indices[i] = -del_indices[i]; /* mark it */
	   /* insert this into the list of gaps, gapcount is the next gap and
	   we take its place */
	   gapdel_total += dq;	/* add to gap total */
	   GapCount++;
	   if (GapCount >= MAX_GAPS) {
	     printf("excessive # of gaps! Giving up.\n"); return -1; }
	   for (j=GapCount-1;j>gcount;j--) {
	    gapblock[j] = gapblock[j-1];
	    gaprs[j] = gaprs[j-1];
	    gapsize[j] = gapsize[j-1];
	    }
	   gapblock[gcount] = k;
	   gaprs[gcount] = i+1;
	   gapsize[gcount] = 0;
	   gcount++;
	   if (gcount < GapCount)  next_gap = gaprs[gcount]; else next_gap = -1;
	   /* printf("next gap = %d\n", next_gap); */
	   }
	 }
	}
	gapdel_mean = (float) gapdel_total/(missed_total);
	/* printf("gapdel_mean = %6.1f\n", gapdel_mean); */
	
	printf("found %d gaps\n", GapCount);
	/* predict gap rollovers */
	gsum = 0.0;
	for (i=0; i<GapCount; i++) {
	  /* printf("gap # %d, gaprs = %d, gapblock = %d\n", i, gaprs[i], gapblock[i]); */
	 j = gaprs[i]-1;  dq = abs(del_indices[j]); /* that is gapdel */
	 /* printf("i, j, del_indices[j] = %d, %d, %d\n", i, j, del_indices[j]); */
	 /* look before and after for local del values, limit to 2 each */
	 del_prior = count_prior = 0;
	 k = j - 1;
	 /* printf("i,j,k,dq = %d %d %d %d\n", i,j,k,dq); */
	 while (k > 0) {
	 	if (del_indices[k] > 0) { del_prior += del_indices[k];
		/* printf("before: del_prior, count_prior %d %d\n",del_prior, count_prior); */
			count_prior++;  if (count_prior >=2) break; }
		k--;
	 }
	 /* use same variables and accumulate some after if available */
	 k = j + 1;
	 while (k < n_restarts_found) {
	 	if (del_indices[k] > 0) { del_prior += del_indices[k];
		/* printf("after : del_prior, count_prior %d %d\n",del_prior, count_prior); */
			count_prior++;  if (count_prior >=4) break; }
		k++;
	 }
	 /* if we have something, use it, else use gapdel_mean */
	 /* printf("final: del_prior, count_prior %d %d\n",del_prior, count_prior); */
	 if (count_prior>0) delq = (float) del_prior/count_prior;
	 	else delq = gapdel_mean;
	 /* printf("delq = %6.1f , gapdel_mean = %6.1f\n", delq, gapdel_mean);
	 //if (gap_del_prior[i] > 0.0) delq=gap_del_prior[i]; else delq=gapdel_mean;
	 //fq = gapdel[i]/delq - gapsize[i];
	 //printf("dq, delq, gapsize[i] = %d, %6.1f, %d\n", dq, delq, gapsize[i]); */
	 fq = (float) dq/delq - (float) gapsize[i];
	 /* printf("fq = %6.1f\n", fq); */
	 if (fq >= 0) {
	 gap_rollf[i] = fq;	/* actually 8 times the rollover */
	 gsum += fq; } else {
	 /* negatives are bad here */
	 /* if this is the end and it is weird (fq < 0) then give it all
	 remaining missing restarts */
	 fq = gap_rollf[i] = missed - gsum;
	 gsum = missed;
	 }
	 /* printf("gsum = %6.1f\n", gsum);
	    //printf("rollover predict = %6.1f for gap %d\n", 0.125*gap_rollf[i],i); */
	}

	cfac = (float) missed/gsum;
	/* printf("cfac = %7.3f\n", cfac); */
	gap_guess = 0;

	for (i=0; i<GapCount; i++) {
	 gap_rollf[i] = fq = 0.125*(cfac*gap_rollf[i]);
	 /* note that we save fp value in gap_rollf for future use */
	 dq = (int) (fq + 0.5);
	 gaprollover[i] = dq;
	 gap_guess += dq;
	 /* printf("rollover predict = %6.1f -> %d for gap %d\n",fq,gaprollover[i],i); */
	}

	if (missed % 8) { printf("missed not a multiple of 8? %d\n", missed); }
	/* printf("total gap_guess = %d, missed = %d\n", gap_guess*8, missed); */
	/* check if these match */
	if (missed != (gap_guess*8) ) {
	 dq = missed/8 - gap_guess;
	 /* printf("mismatch, try to adjust last rollover by %d\n", dq); */
	 i = GapCount-1;
	 if (dq > 0) gaprollover[i] += dq; else {
	  if (gaprollover[i] >= dq) gaprollover[i] -= dq; else {
	   /* getting too weird, give up */
	   printf("giving up, can't make sense of gap sizes\n");
	   return scan_err(0);
	  }
	 }
	}
	
	/* make accumulative sum, this will be amount to shift each section,
	also compute size of each section, we don't have to move any pre-gap
	sections, so the # of sections to move is the # of gaps unless the
	last one is empty */
	if (iblock == gapblock[GapCount-1]) GapCount--;
	/* line above drops the last gap if it was between last data and EOI,
	shouldn't happen too often but is possible */
	dq = 0;
	for (i=0;i<GapCount;i++) {
	  /* we re-use some arrays here so be careful with changes */
	  dq = gaprollover[i] += dq;
	}
	for (i=0;i<GapCount-1;i++) {
	  gapdel[i] = gapblock[i+1] - gapblock[i] - gapsize[i+1]; }
	gapdel[GapCount-1] = iblock - gapblock[GapCount-1];
	/* for (i=0;i<GapCount;i++) {
	// printf("%d  source block = %6d, dest = %6d, length = %6d\n", i, gapblock[i],
	// 	gapblock[i] + gaprollover[i]*64, gapdel[i]);
	//} */

	/* if we have a 1024 wide image, check edges for isolated sections to
	avoid half width errors, the quantity we adjust will be gaprollover */
	
	if (nx == 1024 && GapCount > 1) {
	 for (i=0;i<(GapCount-1);i++) {
	  short *source;
	  int	mode, istart, icq;
	  int	i1,i2,i3,i4,iq,j1;
	  float	s1, s2;
	  short	v1,v2,v3,v4;
	  istart = gapblock[i];
	  /* printf("initial istart = %d, istart*64 = %d\n", istart, istart*64); */
	  iq = (istart%64);
	  if (iq != 0) istart = (iq+1)*64;  /* next higher if not on boundary */
	  source = dct + istart*64;	/* a 4096 boundary in pixels */
	  mode = (istart/64+gaprollover[i])%2;	/* this is 0 if section starts on edge, 1 if
	  			it starts in the middle */
	  iq = istart*64;
	  /* printf("%d = gap, mode = %d, istart = %d\n", i, mode, istart); */
	  lq = (gapblock[i] - istart + gapdel[i])*64;
	  i1 = 63*64;	i2 = 64*64;
	  i3 = i1+4096;	i4 = i2+4096;
	  /* d1 = d2 = s1 = s2 = r1 = r2 = 0.0; */
	  s1 = s2 = 0.0;
	  icq = 0;
	  while (i4 < lq) {
	  /* we could still get into a rtash of zips at the end, so check last one
	  also and scrub if it is zero */
	  v4 = *(source + i4);
	  /* if (v4 == 0) continue;
	     //if (v4 == 0) { printf("hit a v4 = 0 case, i4 =%d, lq =%d\n", i4, lq); continue;} */
	  if (v4 != 0) {
	  v1 = *(source + i1);
	  v2 = *(source + i2);
	  v3 = *(source + i3);
	  /* d1 += (float) ABS(v1 - v2);
	     //d2 += (float) ABS(v3 - v4); */
	   s1 += v1 + v2;
	   s2 += v3 + v4;
	   /* r1 += ((float) ABS(v1 - v2))/(float)(v1 + v2);
	   //r2 += ((float) ABS(v3 - v4))/(float)(v3 + v4);
	   //printf("i1, i2 = %#x, %#x, values=%d, %d, dif=%d, sum=%d\n",i1+iq,i2+iq,v1,v2, v1-v2,v1+v2);
	   //printf("i3, i4 = %#x, %#x, values=%d, %d, dif=%d, sum=%d\n",i3+iq,i4+iq,v3,v4, v3-v4,v3+v4); */
	   }
	   icq++;
	   i1 +=8192;  i2 +=8192;  i3 +=8192;  i4 +=8192;
	  }
	  /* printf("results for gap %d, d1=%6.1f, s1=%6.1f, d2=%6.1f, s2=%6.1f, count = %d\n", i,d1, s1,d2,s2,icq);
	  //printf("results for gap %d, s1=%6.1f, s2=%6.1f, count = %d\n", i,s1,s2,icq);
	  //printf("r1, r2 = %6.1f, %6.1f\n", r1,r2); */
	  /* of possible discriminators, the edge vs the mid sum looks most
	  reliable, derivatives and various ways of adding them fail in my test
	  cases */
	  i1 = (int) (gap_rollf[i] + 0.5);
	  i2 = (int) (gap_rollf[i+1] + 0.5);
	  if ( (i1 > 0) || (i2 > 0) ) {	/* note we can't make a change if
	  				both rollovers are 0 */
	  if (mode) {
	   /* the mode = 1 case, here we want s2 > s1 or we need a change */
	   if (s2 < s1) {
	    if (i1 == 0) { j1 = 1;}
	    else if (i2 == 0) { j1 = -1;}
	    else {	/* normal case, consider merit of two possibilities */
	    dismerit1 = ABS((float) i1 + 1.0 - gap_rollf[i])+
	    	ABS((float) i2 - 1.0 - gap_rollf[i+1]);
	    dismerit2 = ABS((float) i1 - 1.0 - gap_rollf[i])+
	    	ABS((float) i2 + 1.0 - gap_rollf[i+1]);
		if (dismerit1 < dismerit2) { j1 = 1; } else { j1 = -1; }
	   }
	    /* printf("re-align, j1 = %d\n", j1); */
	   /* remember that gaprollover is now the accumulated rollover, so only change
	   the value prior to the gap here, leave later positions alone */
	   gaprollover[i] += j1;
	   }
	  } else {
	   /* the mode = 0 case, here we want s1 > s2 or we need a change */
	   if (s1 < s2) {
	    if (i1 == 0) { j1 = 1;}
	    else if (i2 == 0) { j1 = -1;}
	    else {	/* normal case, consider merit of two possibilities */
	    dismerit1 = ABS((float) i1 + 1.0 - gap_rollf[i])+
	    	ABS((float) i2 - 1.0 - gap_rollf[i+1]);
	    dismerit2 = ABS((float) i1 - 1.0 - gap_rollf[i])+
	    	ABS((float) i2 + 1.0 - gap_rollf[i+1]);
		if (dismerit1 < dismerit2) { j1 = 1; } else { j1 = -1; }
	   }
	    /* printf("re-align, j1 = %d\n", j1); */
	   gaprollover[i] += j1;
	   }
	  }
	 }
	} }
	
	/* move em out, backwards */
	for (i=GapCount-1;i>=0;i--) {
	 short *dest, *source;
	 if (gaprollover[i]) {
	  source = dct + gapblock[i]*64;
	  dest = source + gaprollover[i]*4096;
	  lq = gapdel[i]*64*sizeof(short); /* length must be in bytes */
	  /* printf("dest, source = %#x, %#x, wrt start = %#x, %#x\n", dest, source, dest-dct,source-dct);
	  //printf("size = %d, size+dest-dct = %d, size+source-dct =%d\n", lq,dest+lq-dct,source+lq-dct);
	  //printf("memmove\n"); */
	  memmove(dest, source, lq);
	  lq = MIN(gapdel[i], gaprollover[i]*64);
	  if (lq > 0) prefill(source, lq);
	 }
	}
	
	/* not too bad if there is one gap */
	/* if (GapCount == 1) { */
	  /* check if the missing gaps are a multiple of 8 */
	/* if (missed % 8) { printf("not a multiple of 8? %d\n", missed); } */
	  /* try in any case */
	/* lq = nblocks - gapblock[0] - missed*restart_interval;
	   //printf("block after gap was %d, length was %d\n", gapblock[0], lq); */
	  /* overlap so use memmove (or we could use pointers and work backwards) */
	/* printf("start offset (bytes) = %d\n", gapblock[0]*64*sizeof(short));
	  //printf("dest  offset (bytes) = %d\n", (gapblock[0]+missed*restart_interval)*64*sizeof(short));
	  //printf("length (bytes) = %d\n", lq*64*sizeof(short));
	  //memmove( dct+(gapblock[0]+missed*restart_interval)*64, dct+gapblock[0]*64, lq*64*sizeof(short) ); */
	 /* also default fill the area uncovered */
	/* prefill(dct+gapblock[0]*64, missed*restart_interval);
	   //} */
	}
       }
       break;
     }
     PrevRestartNumber = (RestartNumber+1)& 0x07;
     n = 0;	pb = pt = px; 
    
     } else {
     /* check ahead to see if there is a restart message next */
     if ( *px != 0xff || ( (*(px+1) & 0xf8) != 0xd0) ) {
     printf("unexpected message %#x at iblock = %d\n", code, iblock);
     n_unexpected += 1; }
     /* printf("%#x  %#x  %#x  %#x\n", *px, *(px+1), *(px+2), *(px+3) ); */
     /* treat as data and see if we choke */
     *pt++ = code;  n += 2;
     /*return scan_err(1);*/
     }
     } }
  } else { *pt++ = *px++;  n += 2; } /* if not a ff, get next byte also*/

  }
     /* printf("final limit value = %d\n", limit);
	//printf("end of scan, iblock = %d\n", iblock); */
 return 1;
 }
 /*------------------------------------------------------------------------- */
int preload_q()
 {
 int	iq;
 char	*fname, *pname;
 /* get path name */
 pname = (char *) getenv("QTAB");
 if (!pname) pname = "/hosts/shimmer/usr/people/shine/trace/qtables/";
 fname = (char *) malloc(strlen(pname) + 10);

 /* there are 64 Q tables for TRACE */
 for (iq=0;iq<64;iq++) {
 sprintf(fname, "%s/qt%01d", pname, iq);
 fin=fopen(fname,"r");
  if (fin == NULL) { perror("QT file open error");
	  free(fname);    return -1; }
  if ( fseek(fin, 512, 0) == -1) { perror("QT file position error");
	  free(fname);    return -1; }
  if ( fread(qt_all[iq], 2, 64, fin) != 64) { perror("QT file read error");
	  free(fname);    return -1; }
  int swapb(char *, int);
 if (!bigendian) swapb( (char *) qt_all[iq], 128);
 fclose(fin);
 }
 free(fname);
 return 1;
 }
 /*------------------------------------------------------------------------- */
int ana_trace_decoder(narg,ps)	/* initial TRACE decompresser */
 /* image = trace_decoder(qt, ht, buf) */
 int	narg, ps[];
 {
 /* presently we read each huffman and q result from files, plan to save the
 entire set in memory for faster results eventually */
 static	int last_htin = -1, qs_loaded = 0;
 int	htin, qtin, iq, nd, limit, j, dim[8], result_sym, stat;
 int	i;
 static	char	*dctarray = {"$DCT_ARRAY"};
 char	*fname, *pname;
 byte	packed[304], *buf;
 short	*image;
 struct	ahead	*h;
 /* determine our endian */
 { int one = 1; bigendian = (*(char *)&one == 0); }
 /* get the h and q table entries */
 if (int_arg_stat(ps[1], &htin) != 1) return -1;
 if (int_arg_stat(ps[0], &qtin) != 1) return -1;
 /* get address of input buffer */
 iq = ps[2];
 if ( sym[iq].class != 4 ) return execute_error(66);
 h = (struct ahead *) sym[iq].spec.array.ptr;
 buf = (byte *) ((char *)h + sizeof(struct ahead));
 nd = h->ndim;
 limit = ana_type_size[sym[iq].type];
 for (j=0;j<nd;j++) limit *= h->dims[j]; 		/* # of bytes */
 /* printf("limit = %d\n", limit); */

 /* huffman table */
 /* there are only 4 Huffman tables for TRACE */
 if (htin <0 || htin >3) {
 	printf("trace_decoder: illegal Huffman table %d\n", htin);
	return -1; }
 if (htin != last_htin) {
 /* get file name */
 pname = (char *) getenv("HTAB");
 if (!pname) pname = "/hosts/shimmer/usr/people/shine/trace/htables/";
 fname = (char *) malloc(strlen(pname) + 10);
 sprintf(fname,"%s/ht%01d",pname,htin);
 /* try to open the Huffman file */
 fin=fopen(fname,"r");
  if (fin == NULL) {
  	printf("file: %s\n", fname);
  	perror("Huffman file open error");
 	free(fname);
	return -1; }
 if ( fseek(fin, 512, 0) == -1) { perror("Huffman file position error");
 	free(fname);
 	return -1; }
 if ( fread(packed, 1, 304, fin) != 304) { perror("Huffman file read error");
 	free(fname);
 	return -1; }
 /* now make the various Huffman tables */
 if ( huff_setups(packed) != 1 )
 	{ printf("error converting the Huffman tables\n");
 	free(fname);
	return -1; }
 fclose(fin);
 last_htin = htin;
 }
 
 /* q table */
 if (!qs_loaded) { if (preload_q() <0 ) {
 	printf("trace_decoder: error loading the Q tables\n");  return -1; }
	else qs_loaded = 1; }
 	
 qt = qt_all[qtin];

 bcorrect = -16388/qt[0];
 /* printf("qt[0] = %d, bcorrect = %d\n",qt[0], bcorrect); */
 /* and scan and decode */
 stat = trace_scan(buf, limit); /* dct setup in trace_scan */
 if (stat != 1) { printf("problem with trace_scan\n"); return ANA_ZERO; }
 if (n_unexpected)
   printf("restarts = %d, unexpected = %d\n", n_restarts_found, n_unexpected);
 
 /* allocate for the output array */ 
 dim[0] = nx;	dim[1] = ny;
 result_sym = array_scratch(1, 2, dim);
 h = (struct ahead *) sym[result_sym].spec.array.ptr;
 image = (short *) ((char *)h + sizeof(struct ahead));

 /* also define a special global array with the dct's */
 i = find_sym(dctarray);
 /* printf("i = %d\n", i); */
 sym[i].class=4;	sym[i].type=1;
 /* printf("dct_head = %#x\n", dct_head);
 //printf("dct = %#x\n", dct);
 //printf("*dct = %d\n", *dct); */
 sym[i].spec.general.ptr = dct_head;
 h = (struct ahead *) sym[i].spec.array.ptr;
 h->ndim = 2;
 h->c1 = 0; h->c2 = 0;
 h->dims[0] = 64;
 h->dims[1] = nblocks;
 h->facts = NULL;			/*no known facts */

 sym[i].spec.array.bstore = 128*nblocks + sizeof( struct ahead );
 stat = rdct(image, nx, ny, nblocks, qt, dct);
 if (stat != 1) { printf("problem with rdct\n"); return ANA_ZERO; }

 /* free(dct); */
 
 return result_sym;;
 }
