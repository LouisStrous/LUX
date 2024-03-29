/* This is file gifread_lux.cc.

Copyright 2013 Louis Strous, Richard Shine
Copyright 2014 Louis Strous

This file is part of LUX.

LUX is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your
option) any later version.

LUX is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with LUX.  If not, see <http://www.gnu.org/licenses/>.
*/
// experimental gif reader, learning ....
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include "lux_structures.hh"
 extern SymbolImpl sym[];

/// Attributes of a GIF screen
struct GIFScreen {
  char id[6];
  unsigned char width_lsb;
  unsigned char width_msb;
  unsigned char height_lsb;
  unsigned char height_msb;
  unsigned char mask;
  unsigned char background;
  unsigned char extra;
};

/// Attributes of a GIF image
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
#define min(x,y) ((x) < (y) ? (x) : (y))
#define FALSE 0
#define TRUE 1

/// LZW compression code element
struct codetype {
  codetype *prefix;
  unsigned char first,suffix;
};
 codetype *codetable;                // LZW compression code data
 int32_t datasize,codesize,codemask;     // Decoder working variables
 int32_t clear,eoi;                      // Special code values

 void readextension(FILE *), readimage(FILE *, int32_t, char *),
   loadcolortable(FILE *, int32_t, int32_t), readraster(int32_t, FILE *, unsigned char *);
 static        int32_t        quit = 0, status = 1, gcmflag;
 static int32_t        nxs, nys;

int32_t        lux_gifread(int32_t, int32_t []);
void        process(int32_t, unsigned char **);

 //-------------------------------------------------------------------------
int32_t lux_gifread_f(ArgumentCount narg, Symbol ps[])
 // a function version that returns 1 if read OK
 {
 if ( lux_gifread(narg, ps) == 1 ) return 1; else return 4;
 }
 //-------------------------------------------------------------------------
int32_t lux_gifread(ArgumentCount narg, Symbol ps[])       // gifread subroutine
 // read a "simple" gif file, 8 bit deep
 /* call is gifread,array,file,map where map is the color map, if
 map not in argument list, you don't get it! */
 {
 FILE        *fin;
 int32_t    iq, n, cr, pixel;
 int32_t    sep, cmsym, dim[8];
 char   *p, *name, *data;
 struct ahead   *h;
 struct GIFScreen gh;

                 // first arg is the variable to load, second is name of file
 if (!symbolIsStringScalar(ps[1]))
   return cerror(NEED_STR, ps[1]);
 name = expand_name(string_value(ps[1]), NULL);
 // try to open the file
 if ((fin=fopen(name,"r")) == NULL) return file_open_error();

 // ck if output colormap wanted, set cmsym = 0 if not
 if (narg > 2) { cmsym = ps[2]; } else cmsym = 0;

 /* gif files must have a 6 Byte signature followed by a screen descriptor and
 normally followed by a global color map, the first 2 of these total 15 bytes*/

 if (fread(&gh,1,13,fin) != 13) { perror("gifread in header");
                 fclose(fin); return -1; }
 if (strncmp((gh.id),"GIF",3) != 0) {
         printf("not a GIF file\n"); return -1; }
 if (strncmp(((gh.id))+3,"87a",3) != 0) {
         if (strncmp(((gh.id))+3,"89a",3) != 0) {
         printf("invalid GIF version #\n"); return -1; } else {
        printf("version 89a, warning, not all options supported\n"); }
        }
 // yank out the screen size
 nxs = ( (gh.width_msb << 8) | gh.width_lsb );
 nys = ( (gh.height_msb << 8) | gh.height_lsb );
 // printf("screen size %d %d\n", nxs, nys);

 // define the output array as a Byte of the screen size
 iq = ps[0];        dim[0] = nxs;        dim[1] = nys;
 if ( redef_array(iq, LUX_INT8, 2, dim) != 1) { fclose(fin); return -1; }
 h = (struct ahead *) sym[iq].spec.array.ptr;
 data = ((char *)h + sizeof(struct ahead));

 // global color map stuff
 gcmflag = gh.mask >> 7;        // top bit in mask
 if (gcmflag) {
 cr = (gh.mask >> 4) & 0x7;        cr += 1;        cr = 1 << cr;
 pixel = gh.mask & 0x7;        pixel += 1;        pixel = 1 << pixel;
 // set the data array to the background color
 p = data; n = nxs*nys; while (n--) *p++ = gh.background;
 /* still in global color table exist conditional, read the color table
 and load into 3rd arg if it exists */
 loadcolortable(fin, pixel, cmsym);
 }

 quit = 0;        status = 1;
 //  read the next separator and try to process

 do {
  sep = getc(fin);
  //  printf("separator %#x\n", sep);
  switch (sep) {
   case EOF: perror("gifread, separator");
         quit = 1;
           status = -1;
           break;
   case 0x3b:
        // must be the end
     //        printf("end of image\n");
        quit = 1;
        break;
   case 0x21:
        // an extension of some sort
        readextension(fin);        // currently nothing is done
        break;
   case 0x2c:
        // normal, read image descriptor and image
        readimage(fin, cmsym, data);
         quit = 1;
         break;
   default:
         quit = 1;
        printf("illegal GIF block type\n");
           status = -1;
        break;
 }
 } while (!quit);

 fclose(fin);
 return status;                // normally a 1
 }
 //-------------------------------------------------------------------------
void readextension(FILE *fin)
 // Read a GIF extension block (and do nothing with it).
 {
 unsigned char count;
 char buf[255];
 getc(fin);
 while ((count = getc(fin))) fread(buf, 1, count, fin);
 // printf("note , extension with code = %c ignored\n");
 }
 //-------------------------------------------------------------------------
void readimage(FILE *fin, int32_t cmsym, char *data)
 {
 struct GIFImage gimage;
 int32_t        nx, ny, ix, iy, local, localbits, nc, fflag;
 int32_t        n, m, stride;
 char        *image, *p, *p2;
 if (fread(&gimage.left_lsb,1,9,fin) != 9) {
 perror("gifread in image descriptor");
 quit = 1;        status = -1;        return; }
 // get offsets, etc
 ix = ( (gimage.left_msb << 8) | gimage.left_lsb );
 iy = ( (gimage.top_msb << 8) | gimage.top_lsb );
 nx = ( (gimage.width_msb << 8) | gimage.width_lsb );
 ny = ( (gimage.height_msb << 8) | gimage.height_lsb );
 local = gimage.mask & 0x80;
 // printf("ix, iy, nx, ny = %d %d %d %d\n", ix, iy, nx, ny);
 // printf("mask = %#x\n", gimage.mask);
 if (local) {
 /* a local color table, not too common, we just replace the global if
 there was one */
 localbits = (gimage.mask & 0x7) + 1;
 nc = 1 << localbits;
 if (gcmflag) printf("global color table superseded in GIF file\n");
 loadcolortable(fin, nc, cmsym);
 }
 /* normally the image is the same size as screen, if not we assume there
 might be several images and load each into the screen array, this requires
 scratch storage for each image; if the same size, we just use the already
 allocated screen array */

 if (ix && iy) fflag = 0; else { if (nx != nxs || ny != nys) fflag = 0;
         else fflag =1; }
 if (fflag) image = data; else { image = (char*) malloc(nx * ny);
        if (!image) { printf("malloc error in GIFREAD\n"); quit=1; status=-1;
           return; }
 }
 // now read in
 readraster(nx * ny, fin, (unsigned char *) image);
 if (status != 1) { quit = 1; return; }
 // handle interleaf/interlace, make a note of it
 // if not the screen size, load into screen image and free temp
 // printf("fflag = %d\n", fflag);
 if (!fflag) {
 p2 = image;        p = data + ix + iy*nxs;
 m = ny;        stride = nxs - nx;
 while (m--) { n = nx;  while (n--) {*p++ = *p2++; }  p += stride; }
 free(image);
 }
 }
 //-------------------------------------------------------------------------
void loadcolortable(FILE *fin, int32_t nc, int32_t cmsym)
 {
 int32_t        dim[8], ncolmap;
 struct ahead   *h;
 char        *colormap;
 /* we either load the color table into lux symbol cmsym or we just
 skip over the area in the file */
 ncolmap = 3*nc;
 if (cmsym) {
 dim[0] = 3;        dim[1] = nc;
 if ( redef_array(cmsym, LUX_INT8, 2, dim) != 1) { status = -1;  quit = 1; return; }
 h = (struct ahead *) sym[cmsym].spec.array.ptr;
 colormap = ((char *)h + sizeof(struct ahead));
 if (fread(colormap, 1, ncolmap,fin) != ncolmap)
         { perror("gifread, colormap"); status = -1;  quit = 1; return; }
 } else if (fseek(fin, ncolmap, SEEK_CUR) == -1)
         { perror("gifread, colormap"); status = -1;  quit = 1; return; }
 return;
 }
 //-------------------------------------------------------------------------
void fatal(char const* s)
 // well, not really, in LUX we like to stick around
 {
        fprintf(stderr,"giftops: %s\n",s);
        quit = 1;        status = -1;
 }


 //-------------------------------------------------------------------------
 // Output the bytes associated with a code to the raster array

void outcode(codetype *p, unsigned char **fill)
 {
        if (p->prefix) outcode(p->prefix,fill);
        *(*fill)++ = p->suffix;
 }

 /* Process a compression code.  "clear" resets the code table.  Otherwise
   make a new code table entry, and output the bytes associated with the
   code. */

 //-------------------------------------------------------------------------
void process(int32_t code, unsigned char **fill)
 {
        static int32_t avail,oldcode;
        codetype *p;

        if (code == clear) {
            codesize = datasize + 1;
            codemask = (1 << codesize) - 1;
            avail = clear + 2;
            oldcode = -1;
        } else if (code < avail) {
            outcode(&codetable[code],fill);
            if (oldcode != -1) {
                p = &codetable[avail++];
                p->prefix = &codetable[oldcode];
                p->first = p->prefix->first;
                p->suffix = codetable[code].first;
                if ((avail & codemask) == 0 && avail < 4096) {
                    codesize++;
                    codemask += avail;
                }
            }
            oldcode = code;
        } else if (code == avail && oldcode != -1) {
            p = &codetable[avail++];
            p->prefix = &codetable[oldcode];
            p->first = p->prefix->first;
            p->suffix = p->first;
            outcode(p,fill);
            if ((avail & codemask) == 0 && avail < 4096) {
                codesize++;
                codemask += avail;
            }
            oldcode = code;
        } else {
            fatal("illegal code in raster data");
        }
}
 //-------------------------------------------------------------------------
void readraster(int32_t nsize, FILE *fin, unsigned char *raster)
 {
        unsigned char *fill;
        unsigned char buf[255];
        int32_t bits=0;
        unsigned count,datum=0;
        unsigned char *ch;
        int32_t code;

        fill = raster;
        datasize = getc(fin);
        clear = 1 << datasize;
        eoi = clear+1;
        codesize = datasize + 1;
        codemask = (1 << codesize) - 1;
        codetable = (codetype*)malloc(4096*sizeof(codetype));
        if (!codetable) fatal("not enough memory for code table");
        for (code = 0; code < clear; code++) {
            codetable[code].prefix = (codetype*)0;
            codetable[code].first = code;
            codetable[code].suffix = code;
        }
        for (count = getc(fin); count > 0; count = getc(fin)) {
            fread(buf,1,count,fin);
            for (ch=buf; count-- > 0; ch++) {
                datum += *ch << bits;
                bits += 8;
                while (bits >= codesize) {
                    code = datum & codemask;
                    datum >>= codesize;
                    bits -= codesize;
                    if (code == eoi) goto exitloop;  /* This kludge put in
                                                        because some GIF files
                                                        aren't standard */
                    process(code,&fill);
                }
            }
        }
 exitloop:
        if (fill != raster +nsize) fatal("raster has the wrong size");
        free(codetable);
 }
 //-------------------------------------------------------------------------
