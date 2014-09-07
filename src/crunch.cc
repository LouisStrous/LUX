/* This is file crunch.cc.

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
/* crunch.c, begun 12/6/91 using fragments from earlier modules */
#if HAVE_CONFIG_H
#include "config.h"
#endif
#include "lux_structures.hh"
 /* 2/4/96, finished adding a 32 bit crunch/decrunch which is especially
 useful for recording darks and flats at La Palma since these are sums of
 50 or more 10 bit images and can easily exceed the positive range of
 a short, these often use slice sizes of 9 or so
 While doing these, I discovered a difference between the sunbow and
 umbra version for the 16 bit case with the sunbow version in error, this
 caused rare errors in output that might have gone unnoticed for a while,
 not sure how long the sunbow vesrion had this bug, the umbra version was
 OK, bug marked in crunch32 below */
/* union types_ptr { uint8_t *b; short *w; int32_t *l; float *f; double *d;}; */
/* commented out because already defined in .h file - LS 30apr97 */
uint8_t bits[8]={1,2,4,8,16,32,64,128};
int32_t	crunch_bits;
float	crunch_bpp;
int32_t	crunch_slice = 5;
static int32_t	crunch_run_flag=0;

int32_t	docrunch(int32_t narg, int32_t ps[], int32_t showerror),
   anacrunch(uint8_t *, short [], int32_t, int32_t, int32_t, int32_t),
   anacrunch8(uint8_t *, uint8_t [], int32_t, int32_t, int32_t, int32_t),
   anacrunchrun(uint8_t *, short [], int32_t, int32_t, int32_t, int32_t),
   anacrunchrun8(uint8_t *, uint8_t [], int32_t, int32_t, int32_t, int32_t),
   anadecrunch(uint8_t *, short [], int32_t, int32_t, int32_t),
   anadecrunch8(uint8_t *, uint8_t [], int32_t, int32_t, int32_t),
   anadecrunchrun(uint8_t *, short [], int32_t, int32_t, int32_t),
   anadecrunchrun8(uint8_t *, uint8_t [], int32_t, int32_t, int32_t);
#if SIZEOF_LONG_LONG_INT == 8	/* 64 bit integers */
int32_t anacrunch32(uint8_t *, int32_t *, int32_t, int32_t, int32_t, int32_t),
  anadecrunch32(uint8_t *, int32_t *, int32_t, int32_t, int32_t);
#endif
void	swapl(void *, int32_t);

 /*--------------------------------------------------------------------------*/
int32_t lux_crunchrun(int32_t narg, int32_t ps[])		/* crunch subroutine */
 /* compress an array with imbedded run length encoding plus variable
 bit encoding */		
 /*  lux call is: crunchrun, IN, b ,OUT */
 /* note that OUT must be predefined and limit is used to make certain we
 don't run out of space */
{
  int32_t iq;
  crunch_run_flag = 1;	/* normally 0, used below */
  iq = docrunch(narg,ps,1);
  crunch_run_flag = 0;	/* restore default */
  return iq;
}
/*--------------------------------------------------------------------------*/
int32_t lux_crunch(int32_t narg, int32_t ps[])
{
  return docrunch(narg, ps, 1);
}
/*--------------------------------------------------------------------------*/
int32_t lux_crunch_f(int32_t narg, int32_t ps[])
{
  return docrunch(narg, ps, 0);
}
/*--------------------------------------------------------------------------*/
int32_t docrunch(int32_t narg, int32_t ps[], int32_t showerror)/* crunch subroutine */
 /* compress an array */		
 /*  lux call is: crunch, IN, b ,OUT */
 /* note that OUT must be predefined and limit is used to make certain we
    don't run out of space */
{
  /* works only for I*1, I*2, and I*4 arrays */
  int32_t	iq, slice, limit, nx, outer, n, type, ctype;
  union	types_ptr q1, q2;

  iq = ps[0];
  if (!symbolIsNumericalArray(iq))
    return cerror(NEED_ARR, iq);
  type = array_type(iq);
  if (type > LUX_INT32)
    return luxerror("crunch only accepts I*1,2,4 arrays", iq);
  q1.l = (int32_t*) array_data(iq);
  nx = array_dims(iq)[0];
  outer = array_size(iq)/nx;
  /* get the fixed slice width */
  crunch_slice = slice = int_arg(ps[1]);
  /* the result array */
  iq = ps[2];
  if (!symbolIsNumericalArray(iq))
    return cerror(NEED_ARR, iq);
  q2.l = (int32_t*) array_data(iq);
  n = array_size(iq);
  limit = n*lux_type_size[array_type(iq)]; /* use to avoid overflows */
  /* the types in the compress structure are non-inuitive for historical
     reasons:	ctype 0		I*2 without run length
     ctype 1		I*1 without run length
     ctype 2		I*2 with run length imbedded
     ctype 3		I*1 with run length imbedded
  */
  ctype = (type == 0);
  if (type == 2)
    ctype = 4;
  if (crunch_run_flag == 1)
    ctype += 2;
  switch (ctype) {
  case 0:
    iq = anacrunch(q2.b, q1.w, slice, nx, outer, limit);
    break;
  case 1:
    iq = anacrunch8(q2.b, q1.b, slice, nx, outer, limit);
    break;
  case 2:
    iq = anacrunchrun(q2.b, q1.w, slice, nx, outer, limit);
    break;
  case 3:
    iq = anacrunchrun8(q2.b, q1.b, slice, nx, outer, limit);
    break;
  case 4:
#if SIZEOF_LONG_LONG_INT == 8	/* 64 bit integers */
    iq = anacrunch32(q2.b, q1.l, slice, nx, outer, limit);
    break;
#else
    puts("32-bit compression was not compiled into your version of LUX.");
    iq = LUX_ERROR;
#endif
  default:
    return luxerror("CRUNCH - Impossible error", 0);
  }
  if (iq < 0) {
    crunch_bpp = 0.0;
    crunch_bits = 0;
    return showerror?
      luxerror("not enough space allocated (%d bytes) for compressed array",
	    ps[2]): LUX_ZERO;
  }
  /* printf("crunch used %d bytes out of %d available\n",iq, limit);*/
  crunch_bpp = 8.0 * iq / (nx * outer);
  crunch_bits = 8 * iq;
  /* printf("bits/pixel = %g\n", crunch_bpp);*/
  return LUX_OK;
}
 /*--------------------------------------------------------------------------*/
int32_t lux_decrunch(int32_t narg, int32_t ps[])		/* decrunch subroutine */
 /* decompress an array */		
 /*  decrunch, IN, OUT */
{
  /* works only for I*2 and I*1 arrays */
  int32_t	iq, slice, nx, outer, nd, ctype, dim[2];
  Symboltype type;
#if WORDS_BIGENDIAN
  int32_t bsize;
#endif
  struct	ahead	*h;
  union	types_ptr q1, q2;

  iq = ps[0];
  if ( symbol_class(iq) != 4 ) return cerror(NEED_ARR, iq);
  /* decrunch doesn't care about input array type since it looks at bit stream*/
  h = (struct ahead *) sym[iq].spec.array.ptr;
  q1.l = (int32_t *) ((char *)h + sizeof(struct ahead));
#if WORDS_BIGENDIAN
  bsize =  *q1.l++;
#else
  q1.l++;
#endif
  outer = *q1.l++; nx = *q1.l++;
  /* get the fixed slice width */
  slice = *q1.b++;	ctype = *q1.b++;
  /*printf("ctype = %d\n", ctype);*/
#if WORDS_BIGENDIAN
  swapl(&bsize,1); swapl(&nx,1); swapl(&outer,1); 
#endif
  /* the result array */
  iq = ps[1];
  /* the types in the compress structure are non-inuitive for historical
     reasons:	ctype 0		I*2 without run length
     ctype 1		I*1 without run length
     ctype 2		I*2 with run length imbedded
     ctype 3		I*1 with run length imbedded
     ctype 4		I*4
  */
  if (ctype%2 == 0) type = LUX_INT16; else type = LUX_INT8; /*I*2 or I*1 */
  if (ctype ==4) type = LUX_INT32;
  nd = 1; if (outer > 1) nd = 2;  dim[0] = nx;   dim[1] = outer;
  if ( redef_array( iq, type, nd, dim) != 1 ) {
    printf("problem setting up result for DECRUNCH, blocksize, numblocks = %d %d",
	   nx, outer);
    return -1;}
  h = (struct ahead *) sym[iq].spec.array.ptr;
  q2.l = (int32_t *) ((char *)h + sizeof(struct ahead));
  switch (ctype) {
  case 0:
    iq = anadecrunch(q1.b, q2.w, slice, nx, outer);
    break;
  case 1:
    iq = anadecrunch8(q1.b, q2.b, slice, nx, outer);
    break;
  case 2:
    iq = anadecrunchrun(q1.b, q2.w, slice, nx, outer);
    break;
  case 3:
    iq = anadecrunchrun8(q1.b, q2.b, slice, nx, outer);
    break;
  case 4:
#if SIZEOF_LONG_LONG_INT == 8	/* 64-bit integers */
    iq = anadecrunch32(q1.b, q2.l, slice, nx, outer);
    break;
#else
    puts("32-bit decompression was not compiled into your version of LUX.");
    iq = LUX_ERROR;
#endif
  default:
    printf("DECRUNCH: illegal compression type %d\n", ctype);
    iq = LUX_ERROR;
    break;
  }
  return iq;
}
 /*--------------------------------------------------------------------------*/
void bitprint(short z)
{
  int32_t mask={0xffff},yq;
  int32_t i;
  yq=z & mask;
  for (i=0;i<16;i++) {printf("%1d",yq%2); yq=yq/2; }
  printf(" ");
  return;
}       /* end of bitprint */
 /*--------------------------------------------------------------------------*/
#if SIZEOF_LONG_LONG_INT == 8	/* 64-bit integers */
int32_t anacrunch32(uint8_t *x, int32_t array[], int32_t slice, int32_t nx, int32_t ny, int32_t limit)
 /* compress 32 bit array into x (a uint8_t array) using ny blocks each of size
 nx, bit slice size slice, returns # of bytes in x */
{
  struct compresshead {
    int32_t     tsize,nblocks,bsize;
    uint8_t    slice_size,type; } *ch;
  uint32_t nb,ixa,ixb,big=0;
  unsigned register i,j,r1,in;
  int32_t r0;
  long long	r3, mask, y64;
  int32_t i2,k,iy;
  union { int32_t i; short w; unsigned char b[4]; } y;
  union { long long l64; unsigned char b[8];  } yy, c;

  /* We need value 0x1ffffffff but it's too wide for an int32_t using gcc
     on an i586-pc-linux-gnulibc1 system.  The GNU C manual says that
     one can specify that a constant is long int64_t by appending LL to
     it, but at least for our constant in hexadecimal notation the compiler
     regards the LL suffix as a parser error.  We construct the constant
     uint8_t by uint8_t instead.  LS 13mar99 */
  c.b[0] = c.b[1] = c.b[2] = c.b[3] = 0xff;
  c.b[4] = 0x1f;
  c.b[5] = c.b[6] = c.b[7] = 0;
  /* printf("%x\n", c.l64); */

  /* c.l64 = 0x1ffffffff; */

  /* begin execution */
  if (limit < 25)
    return luxerror("limit (%d) too small in crunch32", 0, limit);

  limit = limit - 24;	/* need 14 for header and some margin since
			   we don't check all times */
  mask=1; for (i=0;i<slice;i++) mask=2*mask;
  mask=mask-1; /* no inline expon. in C */
  /* determine the # of bytes to transfer to 32 bit int32_t for fixed portion */
  nb = (slice + 14)/8;	/* range 1 to 5 */
  if (slice == 0) nb=0;	/* but slice = 0 a special case */

  y.i=0;
  /* do the compression header */
  ch = (struct compresshead *) x;
  /* important note - can't use the sizeof(struct compresshead) because it
     is 14 on some machines and rounded up to 16 on others */
  /*x = x + sizeof(struct compresshead);*/
  x = x + 14;
  ch->bsize = nx;  ch->nblocks = ny;  ch->slice_size = slice;  ch->type = 4;
  i=0;    r1=0;   in=0;
  for (iy=0;iy<ny;iy++) {                 /* start of iy (outer) loop */
    /* load the first value, reverse bytes (VAX style)*/
#if WORDS_BIGENDIAN
    y.i=array[in]; x[i]=y.b[3]; x[i+1]=y.b[2]; x[i+2]=y.b[1]; x[i+3]=y.b[0];
#else
    y.i=array[in]; x[i]=y.b[0]; x[i+1]=y.b[1]; x[i+2]=y.b[2]; x[i+3]=y.b[3];
#endif 
    r1=r1+32;
    ixa=1+iy*nx;    ixb=(iy+1)*nx;
    for (in=ixa; in<ixb; in++)      {               /* start of ix (inner) loop */
      /* first the fixed slice portion */
      y64 = (long long) array[in] - (long long) array[in-1];
      r3 = (y64>>slice);
      i=r1>>3;
      j=r1%8;
      if ( i > limit ) return -1;		/* bad news, went too far */
      /* now load nb bytes into x */
      /*low order uint8_t of y.i is first in stream */
      if (j == 0) {
	y64=(y64 & mask);	x[i]= (uint8_t) y64;
	/* since we started at bit 0, spillover to the next uint8_t is
	   determined as follows (and is unlikely since slice gt 8 is unusual */
	if (slice > 8) { x[i+1]= (uint8_t) (y64 >> 8);
	if (slice > 16) { x[i+2]= (uint8_t) (y64 >> 16);
	if (slice > 24) { x[i+3]= (uint8_t) (y64 >> 24);}}}
      } else {
	y64=(y64 & mask)<<j;	x[i]=x[i] | (uint8_t) y64;
	/* spillover more likely here */
	if (nb>1) { x[i+1] = (uint8_t) (y64 >> 8);
	if (nb>2) { x[i+2] = (uint8_t) (y64 >> 16);
	if (nb>3) { x[i+3] = (uint8_t) (y64 >> 24);
	if (nb>4) { x[i+4] = (uint8_t) (y64 >> 32);}}}}
      }

      r1=r1+slice;       /* bump r1 pass the fixed part */
      i=r1>>3;                j=r1%8;
      /* note that r3 is the # of bits required minus 1 */
      if (r3==0) { if (j ==0 ) {x[i]= *bits;} else {x[i]=x[i]| bits[j];}
      r1++;} else    {
	r3=2*r3;        if (r3<0) r3 = -r3-1;
	if (r3<31)  {
	  r0=j+r3;        /* this is the bit that needs setting offset from x[i] */
	  if (r0 < 8) { if (j == 0) x[i]=bits[r0]; else x[i]=x[i]|bits[r0];}
	  else  {
	    if (j == 0) x[i]=0;
	    j=r0%8;
	    if (r0 < 16) x[i+1]=bits[j]; else {
	      i2=i+r0/8;
	      for (k=i+1;k<i2;k++) x[k]=0;
	      x[i2]=bits[j]; }
	  }
	  r1+=1+r3;
	} else {        /* big one exception, should be rare */
	  /* does not need to be efficient, used rarely */
	  /* printf("big one for I*4\n"); */
	  big++;
	  if (j == 0) x[i]=0;     /* gotta zero the virgins */
	  r0=j+31;        j=r0%8;         i2=i+r0/8;
	  for (k=i+1;k<i2;k++) x[k]=0;    x[i2]=bits[j];
	  /* recompute the difference and load 33 bits (always 5 bytes) */
	  r1=r1+32;
	  i=r1/8;
	  j=r1%8;
	  if (j == 0) x[i]=0;     /* gotta zero the virgins */
	  yy.l64=(((long long) array[in]- (long long) array[in-1])& c.l64) << j;
#if WORDS_BIGENDIAN
	  x[i]=x[i] | yy.b[7]; x[i+1]=yy.b[6]; x[i+2]=yy.b[5];
	  x[i+3]=yy.b[4]; x[i+4]=yy.b[3];
#else
	  x[i]=x[i] | yy.b[0]; x[i+1]=yy.b[1]; x[i+2]=yy.b[2];
	  x[i+3]=yy.b[3]; x[i+4]=yy.b[4];
#endif
	  r1=r1+33;       } /* end of big one exception */
      }       /* end of (r3==0) conditional */
      /*in=in+1; */                   }       /* end of ix loop */
    /* some checks here */
    /* bump to next uint8_t boundary */
    i=(r1+7)/8;     r1=8*i;                 }       /* end of iy loop */
  ch->tsize = i = i + 14;
  /* we have to put these in a form readable by the Vax (these may be used
     by fcwrite) */
#if WORDS_BIGENDIAN
  swapl(&(ch->tsize),1); swapl(&(ch->bsize),1); swapl(&(ch->nblocks),1); 
#endif
  /* printf("number of big ones for this I*4 = %d\n", big); */
  return  i;      /*return # of bytes used */
}       /* end of routine */
#endif
 /*--------------------------------------------------------------------------*/
int32_t anacrunch(uint8_t *x, short array[], int32_t slice, int32_t nx, int32_t ny, int32_t limit)
 /* compress 16 bit array into x (a uint8_t array) using ny blocks each of size
 nx, bit slice size slice, returns # of bytes in x */
{
  struct compresshead {
    int32_t     tsize,nblocks,bsize;
    uint8_t    slice_size,type; } *ch;
  unsigned nb,ixa,ixb;
  unsigned register i,j,r1,in;
  int32_t r0,r3,mask;
  int32_t i2,k,iy;
  union { int32_t i; short w; unsigned char b[4]; } y;
  /* begin execution */
  if (limit < 25)
    return luxerror("limit (%d) too small in crunch", 0, limit);
  limit = limit - 24;	/* need 14 for header and some margin since
			   we don't check all times */
  mask=1; for (i=0;i<slice;i++) mask=2*mask;
  mask=mask-1; /* no inline expon. in C */
  /* determine the # of bytes to transfer to 32 bit int32_t for fixed portion */
  if (slice == 0) nb=0; else { if (slice < 2 ) nb=1;
  else { if (slice < 10) nb=2; else nb=3;    }};
  y.i=0;
  /* do the compression header */
  ch = (struct compresshead *) x;
  /* important note - can't use the sizeof(struct compresshead) because it
     is 14 on some machines and rounded up to 16 on others */
  /*x = x + sizeof(struct compresshead);*/
  x = x + 14;
  ch->bsize = nx;  ch->nblocks = ny;  ch->slice_size = slice;  ch->type = 0;
  i=0;    r1=0;   in=0;
  for (iy=0;iy<ny;iy++) {                 /* start of iy (outer) loop */
    /* load the first value, reverse bytes (VAX style)*/
#if WORDS_BIGENDIAN
    y.w=array[in]   ;x[i]=y.b[1]    ;x[i+1]=y.b[0];
#else
    y.w=array[in]   ;x[i]=y.b[0]    ;x[i+1]=y.b[1];
#endif 
    r1=r1+16;
    ixa=1+iy*nx;    ixb=(iy+1)*nx;
    for (in=ixa; in<ixb; in++)      {               /* start of ix (inner) loop */
      /* first the fixed slice portion */
      y.i=array[in]-array[in-1];
      r3=(y.i>>slice);
      i=r1>>3;
      j=r1%8;
      if ( i > limit ) return -1;		/* bad news, went too far */
      /* now load nb bytes into x */
      /*low order uint8_t of y.i is first in stream */
      if (j == 0) {
	y.i=(y.i & mask);	x[i]= (uint8_t) y.i;
	/* since we started at bit 0, spillover to the next uint8_t is
	   determined as follows (and is unlikely since slice gt 8 is unusual */
	if (slice > 8) x[i+1]= (uint8_t) (y.i >> 8);
      } else {
	y.i=(y.i & mask)<<j;	x[i]=x[i] | (uint8_t) y.i;
	/* spillover more likely here */
	if (nb>1) { x[i+1]= (uint8_t) (y.i >> 8);
	if (nb>2) x[i+2]=(uint8_t) (y.i >> 16); }
      }

      r1=r1+slice;       /* bump r1 pass the fixed part */
      i=r1>>3;                j=r1%8;
      /* note that r3 is the # of bits required minus 1 */
      if (r3==0) { if (j ==0 ) {x[i]= *bits;} else {x[i]=x[i]| bits[j];}
      r1++;} else    {
	r3=2*r3;        if (r3<0) r3 = -r3-1;
	if (r3<31)  {
	  r0=j+r3;        /* this is the bit that needs setting offset from x[i] */
	  if (r0 < 8) { if (j == 0) x[i]=bits[r0]; else x[i]=x[i]|bits[r0];}
	  /* note, discovered on 2/3/96 that the j==0 case not done above for the
	     sunbow version, was OK in the umbra version, may have happened while cleaning
	     up code?, caused extra bits to be set if x[i] wasn't zero */
	  else  {
	    if (j == 0) x[i]=0;
	    j=r0%8;
	    if (r0 < 16) x[i+1]=bits[j]; else {
	      i2=i+r0/8;
	      for (k=i+1;k<i2;k++) x[k]=0;
	      x[i2]=bits[j]; }
	  }
	  r1+=1+r3;
	} else {        /* big one exception, should be rare */
	  /* does not need to be efficient, used rarely */
	  /*printf("big one \n");*/
	  if (j == 0) x[i]=0;     /* gotta zero the virgins */
	  r0=j+31;        j=r0%8;         i2=i+r0/8;
	  for (k=i+1;k<i2;k++) x[k]=0;    x[i2]=bits[j];
	  /* recompute the difference and load 17 bits (always 3 bytes) */
	  r1=r1+32;
	  i=r1/8;
	  j=r1%8;
	  if (j == 0) x[i]=0;     /* gotta zero the virgins */
	  y.i=((array[in]-array[in-1])& 0x1ffff) << j;
#if WORDS_BIGENDIAN
	  x[i]=x[i] | y.b[3]; x[i+1]=y.b[2];      x[i+2]=y.b[1];
#else
	  x[i]=x[i] | y.b[0]; x[i+1]=y.b[1];      x[i+2]=y.b[2];
#endif
	  r1=r1+17;       } /* end of big one exception */
      }       /* end of (r3==0) conditional */
      /*in=in+1; */                   }       /* end of ix loop */
    /* some checks here */
    /* bump to next uint8_t boundary */
    i=(r1+7)/8;     r1=8*i;                 }       /* end of iy loop */
  ch->tsize = i = i + 14;
  /* we have to put these in a form readable by the Vax (these may be used
     by fcwrite) */
#if WORDS_BIGENDIAN
  swapl(&(ch->tsize),1); swapl(&(ch->bsize),1); swapl(&(ch->nblocks),1); 
#endif
  return  i;      /*return # of bytes used */
}       /* end of routine */
 /*--------------------------------------------------------------------------*/
int32_t anacrunch8(uint8_t *x, uint8_t array[], int32_t slice, int32_t nx, int32_t ny, int32_t limit)
 /* compress 8 bit array into x (a uint8_t array) using ny blocks each of size
 nx, bit slice size slice, returns # of bytes in x */
{
  struct compresshead {
    int32_t     tsize,nblocks,bsize;
    uint8_t    slice_size,type; } *ch;
  unsigned nb,ixa,ixb;
  unsigned register i,j,r1,in;
  int32_t r0,r3,mask;
  int32_t i2,k,iy;
  union { int32_t i; short w; unsigned char b[4]; } y;
  /* begin execution */
  if (limit < 25)
    return luxerror("limit (%d) too small in crunch8", 0, limit);
  limit = limit - 24;	/* need 14 for header and some margin since
			   we don't check all times */
  mask=1; for (i=0;i<slice;i++) mask=2*mask;
  mask=mask-1; /* no inline expon. in C */
  /* determine the # of bytes to transfer to 32 bit int32_t for fixed portion */
  if (slice > 8) slice = 8;
  if (slice == 0) nb=0; else { if (slice < 2 ) nb=1;
  else { if (slice < 10) nb=2; else nb=3;    }};
  y.i=0;
  /* do the compression header */
  ch = (struct compresshead *) x;
  /* important note - can't use the sizeof(struct compresshead) because it
     is 14 on some machines and rounded up to 16 on others */
  /*x = x + sizeof(struct compresshead);*/
  x = x + 14;
  ch->bsize = nx;  ch->nblocks = ny;  ch->slice_size = slice;  ch->type = 1;
  i=0;    r1=0;   in=0;
  for (iy=0;iy<ny;iy++) {                 /* start of iy (outer) loop */
    /* load the first value */
    x[i] = array[in];
    r1=r1+8;
    ixa=1+iy*nx;    ixb=(iy+1)*nx;
    for (in=ixa; in<ixb; in++)      {               /* start of ix (inner) loop */
      /* first the fixed slice portion */
      y.i= (int32_t) array[in]- (int32_t) array[in-1];
      r3=(y.i>>slice);
      i=r1>>3;
      j=r1%8;
      if ( i > limit ) return -1;		/* bad news, went too far */
      /* now load nb bytes into x */
      /*low order uint8_t of y.i is first in stream */
#if WORDS_BIGENDIAN
      if (j == 0) {y.i=(y.i & mask); x[i]=y.b[3];}
      else { y.i=(y.i & mask)<<j; x[i]=x[i] | y.b[3];}
      if (nb>1) { x[i+1]=y.b[2]; }
#else 
      if (j == 0) {y.i=(y.i & mask); x[i]=y.b[0];}
      else { y.i=(y.i & mask)<<j; x[i]=x[i] | y.b[0];}
      if (nb>1) { x[i+1]=y.b[1]; }
#endif 
      r1=r1+slice;       /* bump r1 pass the fixed part */
      i=r1>>3;                j=r1%8;
      /* note that r3 is the # of bits required minus 1 */
      if (r3==0) { if (j ==0 ) {x[i]=bits[j];} else {x[i]=x[i]|bits[j];}
      r1+=1;} else    {
	r3=2*r3;        if (r3<0) r3 = -r3-1;
	if (r3<31)  {
	  r0=j+r3;        /* this is the bit that needs setting offset from x[i] */
	  if (r0 < 8) { if (j == 0) x[i]=bits[r0]; else x[i]=x[i]|bits[r0];}
	  else {if (j == 0) x[i]=0;  j=r0%8; if (r0 < 16) x[i+1]=bits[j];
	  else { i2=i+r0/8; for (k=i+1;k<i2;k++) x[k]=0;  x[i2]=bits[j]; }
	  }
	  r1+=1+r3;
	} else {        /* big one exception, should be rare */
	  /* does not need to be efficient, used rarely */
	  /* printf("big one \n"); */
	  if (j == 0) x[i]=0;     /* gotta zero the virgins */
	  r0=j+31;        j=r0%8;         i2=i+r0/8;
	  for (k=i+1;k<i2;k++) x[k]=0;    x[i2]=bits[j];
	  /* recompute the difference and load 9 bits (always 2 bytes) */
	  r1=r1+32;
	  i=r1/8;
	  j=r1%8;
	  if (j == 0) x[i]=0;     /* gotta zero the virgins */
	  y.i=((array[in]-array[in-1])& 0x1ff) << j;
#if WORDS_BIGENDIAN
	  x[i]=x[i] | y.b[3]; x[i+1]=y.b[2];
#else
	  x[i]=x[i] | y.b[0]; x[i+1]=y.b[1];
#endif
	  r1=r1+9;       } /* end of big one exception */
      }       /* end of (r3==0) conditional */
    }       /* end of ix loop */
    /* some checks here */
    /* bump to next uint8_t boundary */
    i=(r1+7)/8;     r1=8*i;                 }       /* end of iy loop */
  ch->tsize = i = i + 14;
  /* we have to put these in a form readable by the Vax (these may be used
     by fcwrite) */
#if WORDS_BIGENDIAN
  swapl(&(ch->tsize),1); swapl(&(ch->bsize),1); swapl(&(ch->nblocks),1); 
#endif
  return  i;      /*return # of bytes used */
}       /* end of routine */
 /*--------------------------------------------------------------------------*/
void swapl(void *vx, int32_t n)
/* reverse n longs (4 bytes each) */
{
  int32_t	i1, i2, i3, i4, i;
  char	xq, *x;

  x = (char *) vx;
  i1 = 0;
  i2 = 1;
  i3 = 2;
  i4 = 3;
  for (i = 0; i < n; i++) {
    xq = x[i1];
    x[i1] = x[i4];
    x[i4] = xq;
    xq = x[i2];
    x[i2] = x[i3];
    x[i3] = xq;
    i1 += 4;
    i2 += 4;
    i3 += 4;
    i4 += 4;
  }
}
 /*--------------------------------------------------------------------------*/
void swapd(char x[], int32_t n)
     /* n; the number of F*8 (8 bytes each) to reverse */
{
  int32_t   i1,i2,i3,i4,i5,i6,i7,i8,i;
  char  xq;
  i1=0; i2=1; i3=2; i4=3; i5=5; i6=5; i7=6; i8=7;
  for (i=0;i<n;i++) { xq=x[i1];
  x[i1]=x[i8];    x[i8]=xq;       xq=x[i2];
  x[i2]=x[i7];    x[i7]=xq;	 xq=x[i3];
  x[i3]=x[i6];	 x[i6]=xq;	 xq=x[i4];
  x[i4]=x[i5];	 x[i5]=xq;
  i1+=8;  i2+=8;  i3+=8;  i4+=8; i5+=8; i6+=8; i7+=8; i8+=8;  }
}
 /*--------------------------------------------------------------------------*/
int32_t swapb(char x[], int32_t n)
{
  int32_t   i;
  char  xq;
  if (n < 2 ) { printf("error in swapb count, n = %d\n",n);  return -1; }
  if (n%2 != 0 ) n--;
  for (i=0;i<n;i += 2) { xq=x[i];  x[i] = x[i+1];  x[i+1] = xq; }
  return 1;
}
 /*--------------------------------------------------------------------------*/
#if SIZEOF_LONG_LONG_INT == 8	/* 64-bit integers */
int32_t anadecrunch32(uint8_t *x, int32_t array[], int32_t r9, int32_t nx, int32_t ny)
     /* decompress a bit stream in x; result is n I*4 elements, put in array;
	 bit slice size r9 */
{
  int32_t	iq;
  int32_t	r0, r1, r2, nb;
  int32_t	j, in, i, k, ix, iy, mask;
  long long	y64;
  unsigned char xq;
  union { int32_t i; short w; unsigned char b[4]; } y;
  union { long long l64; unsigned char b[8];  } yy, c1, c2, c3;

  /* We cannot specify long int64_t hexadecimal constants on all
     platforms, so we construct those we need on the fly here.
     LS 13mar99 */
#if WORDS_BIGENDIAN
  c1.b[7] = c1.b[6] = c1.b[5] = c1.b[4] = 0xff;
  c1.b[3] = 0x1f;
  c1.b[2] = c1.b[1] = c1.b[0] = 0;
  c2.b[7] = c2.b[6] = c2.b[5] = c2.b[4] = c2.b[2] = c2.b[1] = c2.b[0] = 0;
  c2.b[3] = 0x1;
  c3.b[7] = c3.b[6] = c3.b[5] = c3.b[4] = 0;
  c3.b[3] = c3.b[2] = c3.b[1] = c3.b[0] = 0xff;
#else
  c1.b[0] = c1.b[1] = c1.b[2] = c1.b[3] = 0xff;
  c1.b[4] = 0x1f;
  c1.b[5] = c1.b[6] = c1.b[7] = 0;
  c2.b[0] = c2.b[1] = c2.b[2] = c2.b[3] = c2.b[5] = c2.b[6] = c2.b[7] = 0;
  c2.b[4] = 0x1;
  c3.b[0] = c3.b[1] = c3.b[2] = c3.b[3] = 0;
  c3.b[4] = c3.b[5] = c3.b[6] = c3.b[7] = 0xff;
#endif

  /* begin execution */
  mask = 1;
  for (i = 0; i < r9; i++)
    mask = 2*mask;
  mask = mask - 1;
  /* printf("slice width = %d\n",r9); */
  /* printf ("mask = %x, %d\n",mask,mask); */
  /* determine the # of bytes to transfer to 32 bit int32_t for fixed portion */
  nb = (r9 + 14)/8;	/* range 1 to 5 */
  if (r9 == 0)
    nb = 0;	/* but slice = 0 a special case */
  /* printf("nb = %d\n", nb); */
  y.i = 0;
  i = 0;
  r1 = 0;
  in = 0;
  for (iy = 0; iy < ny; iy++) {	/* start of iy (outer) loop */
    /* get the first value, 4 bytes */
#if WORDS_BIGENDIAN
    y.b[0] = x[i + 3];
    y.b[1] = x[i + 2];
    y.b[2]=x[i+1];  y.b[3]=x[i];
    iq = array[in++]=y.i;
#else
    y.b[0] = x[i];
    y.b[1] = x[i + 1];
    y.b[2] = x[i + 2];
    y.b[3] = x[i + 3];
    iq = array[in++] = y.i;
#endif
    /*printf("first value = %d 0x%x\n",iq, iq);*/
    r1=r1+32;
    r2=0;
    for (ix=1; ix<nx; ix++) {
      /* first the fixed slice portion */
      i=r1/8;         j=r1%8;
#if WORDS_BIGENDIAN
      yy.b[7]=x[i];
      if (nb>1) {
	yy.b[6]=x[i+1];
	if (nb>2) {
	  yy.b[5]=x[i+2];
	  if (nb>3) {
	    yy.b[4]=x[i+3];
	    if (nb>4) {
	      yy.b[3]=x[i+4];
	    }
	  }
	}
      }
#else
      yy.b[0]=x[i];
      if (nb>1) {
	yy.b[1]=x[i+1];
	if (nb>2) {
	  yy.b[2]=x[i+2];
	  if (nb>3) {
	    yy.b[3]=x[i+3];
	    if (nb>4) {
	      yy.b[4]=x[i+4];
	    }
	  }
	}
      }
#endif
      /* shift and mask out the bit slice */
      r2= (int32_t) ((yy.l64>>j) & mask);
      /*printf("r2 = %x, %d\n",r2,r2);*/
      /* the variable bit portion, find the first set bit */
      r1=r1+r9;       /* bump r1 pass the fixed part */
      i=r1/8;         j=r1%8;
      if ((xq= (x[i]>>j) ) != 0) {
	/* caught it on first uint8_t, find the bit */
	if ((xq&1) != 0) r0=1; else {
	  if ((xq&2) != 0) r0=2; else {
	    if ((xq&4) != 0) r0=3; else {
	      if ((xq&8) != 0) r0=4; else {
		if ((xq&16) != 0) r0=5; else {
		  if ((xq&32) != 0) r0=6; else {
		    if ((xq&64) != 0) r0=7; else {
		      if ((xq&128) != 0) r0=8; }}}}}}}}       else {
			/* not in first uint8_t (or part of one) checked, carry on, first count bits in
			   that first uint8_t */
			r0=8-j;
			/* check up to 4 more bytes, if not found than an error */
			for (k=i+1;k<i+5;k++) { if ( (xq=x[k]) != 0 ) {
			  /* caught it here, find the bit and then jump from loop */
			  if ((xq&1) != 0) r0+=1; else {
			    if ((xq&2) != 0) r0+=2; else {
			      if ((xq&4) != 0) r0+=3; else {
				if ((xq&8) != 0) r0+=4; else {
				  if ((xq&16) != 0) r0+=5; else {
				    if ((xq&32) != 0) r0+=6; else {
				      if ((xq&64) != 0) r0+=7; else {
					if ((xq&128) != 0) r0+=8; }}}}}}} break; } else { r0=r0+8; 
					/* add 8 bits for each all zero uint8_t */
					if (r0 > 32) { printf("DECRUNCH -- bad bit sequence, cannot continue\n");
					printf("i = %d, r1 = %d, ix= %d, iy = %d\n",i,r1,ix,iy);
					return -1; }       }       }       }
      r1=r1+r0;       /* update pointer */
      /* r0 even or odd determines sign of difference */
      /*printf("r0 = %d\n", r0);*/
      if ((r0&1) != 0) { 
	/* positive case */
	/*printf("plus case, r0, r2, iq = %d %d %d\n", r0, r2, iq);*/
	r0=(r0/2)<<r9;  iq=iq+r2;       iq=iq+r0;       array[in]=iq;
	/*printf("r0 now = %d\n", r0);*/
      } else
	{ if (r0 == 32) { 
	  /* a long one, yank out the next 33 bits and use as difference */
	  i=r1/8;         j=r1%8;
#if WORDS_BIGENDIAN
	  yy.b[7]=x[i];
	  yy.b[6]=x[i+1]; yy.b[5]=x[i+2]; yy.b[4]=x[i+3]; yy.b[3]=x[i+4];
#else
	  yy.b[0]=x[i];
	  yy.b[1]=x[i+1]; yy.b[2]=x[i+2]; yy.b[3]=x[i+3]; yy.b[4]=x[i+4];
#endif
	  /* shift and mask out the 33 bit slice */
	  y64=(yy.l64>>j) & c1.l64;
	  r1=r1+33;
	  /* if the top bit was set, do a sign extend, note that 64 bit arithmetic used*/
	  if ( (y64 & c2.l64) != 0 ) y64 = y64 | c3.l64;
	  y64 = y64 + (long long) array[in-1];
	  iq = array[in]= (long) y64;
	} else {
	  /* minus case (normal) */
	  /*printf("minus case, r0, r2, iq = %d %d %d\n", r0, r2, iq);*/
	  r0=(-r0/2)<<r9; iq=iq+r2;       iq=iq+r0;       array[in]=iq;
	}}
      in=in+1;                                }   	    /* end of ix loop */
    i=(r1+7)/8;     r1=8*i;                 }   	    /* end of iy loop */
  return 1;
}  						     /* end of routine */
#endif
/*--------------------------------------------------------------------------*/
int32_t anadecrunch(uint8_t *x, short array[], int32_t r9, int32_t nx, int32_t ny)
 /* decompress a bit stream in x; result is n I*2 elements, put in array;
	 bit slice size r9 */
{
  short iq;
  int32_t r0,r1,r2,r4,nb,mask;
  int32_t j,in,i,k,ix,iy;
  unsigned char xq;
  union { int32_t i; short w; unsigned char b[4]; } y;
  /* begin execution */
  mask=1; for (i=0;i<r9;i++) mask=2*mask; mask=mask-1;
  /*printf("slice width = %d\n",r9);*/
  /*printf ("mask = %x, %d\n",mask,mask);*/
  /* determine the # of bytes to transfer to 32 bit int32_t for fixed portion */
  if (r9 == 0) nb=0; else { if (r9 < 2 ) nb=1;
  else { if (r9 < 10) nb=2; else nb=3;    }};
  y.i=0;
  i=0;    r1=0;   in=0;
  for (iy=0;iy<ny;iy++) {                 /* start of iy (outer) loop */
    /* get the first value */
#if WORDS_BIGENDIAN
    y.b[0]=x[i+1];  y.b[1]=x[i];    iq=y.w; array[in++]=iq;
#else
    y.b[0]=x[i];  y.b[1]=x[i+1];    iq=y.w; array[in++]=iq;
#endif
    /*printf("first value = %d 0x%x\n",iq, iq);*/
    r1=r1+16;
    r2=0;
    for (ix=1; ix<nx; ix++) {
      /* first the fixed slice portion */
      i=r1/8;         j=r1%8;
#if WORDS_BIGENDIAN
      y.b[3]=x[i];
#else
      y.b[0]=x[i];
#endif
      /* test effect on timing */
#if WORDS_BIGENDIAN
      if (nb>1) { y.b[2]=x[i+1]; if (nb>2) y.b[1]=x[i+2]; }
#else
      if (nb>1) { y.b[1]=x[i+1]; if (nb>2) y.b[2]=x[i+2]; }
#endif
      /* shift and mask out the bit slice */
      r2=(y.i>>j) & mask;
      /*printf("r2 = %x, %d\n",r2,r2);*/
      /* the variable bit portion, find the first set bit */
      r1=r1+r9;       /* bump r1 pass the fixed part */
      i=r1/8;         j=r1%8;
      if ((xq=x[i]>>j) != 0) {
	/* caught it on first uint8_t, find the bit */
	if ((xq&1) != 0) r0=1; else {
	  if ((xq&2) != 0) r0=2; else {
	    if ((xq&4) != 0) r0=3; else {
	      if ((xq&8) != 0) r0=4; else {
		if ((xq&16) != 0) r0=5; else {
		  if ((xq&32) != 0) r0=6; else {
		    if ((xq&64) != 0) r0=7; else {
		      if ((xq&128) != 0) r0=8; }}}}}}}}       else {
			/* not in first uint8_t (or part of one) checked, carry on, first count bits in
			   that first uint8_t */
			r0=8-j;
			/* check up to 4 more bytes, if not found than an error */
			for (k=i+1;k<i+5;k++) { if ( (xq=x[k]) != 0 ) {
			  /* caught it here, find the bit and then jump from loop */
			  if ((xq&1) != 0) r0+=1; else {
			    if ((xq&2) != 0) r0+=2; else {
			      if ((xq&4) != 0) r0+=3; else {
				if ((xq&8) != 0) r0+=4; else {
				  if ((xq&16) != 0) r0+=5; else {
				    if ((xq&32) != 0) r0+=6; else {
				      if ((xq&64) != 0) r0+=7; else {
					if ((xq&128) != 0) r0+=8; }}}}}}} break; } else { r0=r0+8; 
					/* add 8 bits for each all zero uint8_t */
					if (r0 > 32) { printf("DECRUNCH -- bad bit sequence, cannot continue\n");
					printf("i = %d, r1 = %d, ix= %d, iy = %d\n",i,r1,ix,iy);
					return -1; }       }       }       }
      r1=r1+r0;       /* update pointer */
      /* r0 even or odd determines sign of difference */
      if ((r0&1) != 0) { 
	/* positive case */
	r0=(r0/2)<<r9;  iq=iq+r2;       iq=iq+r0;       array[in]=iq;
      } else
	{ if (r0 == 32) { 
	  /* a long one, yank out the next 17 bits and use as difference */
	  i=r1/8;         j=r1%8;
#if WORDS_BIGENDIAN
	  y.b[3]=x[i];
	  y.b[2]=x[i+1]; y.b[1]=x[i+2];
#else
	  y.b[0]=x[i];
	  y.b[1]=x[i+1]; y.b[2]=x[i+2];
#endif
	  /* shift and mask out the 17 bit slice */
	  r2=(y.i>>j) & 0x1ffff;
	  r1=r1+17;
	  /* if the top bit was set, do a sign extend, note that 32 bit arithmetic used*/
	  if ( (r2& 0x10000) != 0 ) r2=r2 | 0xffff0000;
	  r4=array[in-1]; r4=r4+r2; array[in]=r4; iq=r4;
	} else {
	  /* minus case (normal) */
	  r0=(-r0/2)<<r9; iq=iq+r2;       iq=iq+r0;       array[in]=iq;
	}}
      in=in+1;                                }   	    /* end of ix loop */
    i=(r1+7)/8;     r1=8*i;                 }   	    /* end of iy loop */
  return 1;
}  						     /* end of routine */
 /*--------------------------------------------------------------------------*/
int32_t anadecrunch8(uint8_t *x, uint8_t array[], int32_t r9, int32_t nx, int32_t ny)
 /* decompress a bit stream in x; result is n I*1 elements, put in array;
	  bit slice size r9 */
				 /* uint8_t version, modified from I*2 version
				 r. shine 6/5/91 */
{
  uint8_t iq;
  int32_t r0,r1,r2,r4,nb,mask;
  int32_t j,in,i,k,ix,iy;
  uint8_t xq;
  union { int32_t i; short w; unsigned char b[4]; } y;
  /* begin execution */
  mask=1; for (i=0;i<r9;i++) mask=2*mask; mask=mask-1;
  /* determine the # of bytes to transfer to 32 bit int32_t for fixed portion */
  if (r9 == 0) nb=0; else { if (r9 < 2 ) nb=1;
  else { if (r9 < 10) nb=2; else nb=3;    }};
  y.i=0;
  i=0;    r1=0;   in=0;
  for (iy=0;iy<ny;iy++) {                 /* start of iy (outer) loop */
    /* get the first value */
    iq=x[i];        array[in++]=iq;
    r1=r1+8;
    r2=0;
    for (ix=1; ix<nx; ix++) {
      /* first the fixed slice portion */
      i=r1/8;         j=r1%8;
#if WORDS_BIGENDIAN
      y.b[3]=x[i];
      if (nb>1) { y.b[2]=x[i+1]; if (nb>2) y.b[1]=x[i+2]; }
#else
      y.b[0]=x[i];
      if (nb>1) { y.b[1]=x[i+1]; if (nb>2) y.b[2]=x[i+2]; }
#endif
      /* shift and mask out the bit slice */
      r2=(y.i>>j) & mask;
      /* the variable bit portion, find the first set bit */
      r1=r1+r9;     				  /* bump r1 pass the fixed part */
      i=r1/8;         j=r1%8;
      if ((xq=x[i]>>j) != 0) {
	/* caught it on first uint8_t, find the bit */
	if ((xq&1) != 0) r0=1; else {
	  if ((xq&2) != 0) r0=2; else {
	    if ((xq&4) != 0) r0=3; else {
	      if ((xq&8) != 0) r0=4; else {
		if ((xq&16) != 0) r0=5; else {
		  if ((xq&32) != 0) r0=6; else {
		    if ((xq&64) != 0) r0=7; else {
		      if ((xq&128) != 0) r0=8; }}}}}}}}       else {
			/* not in first uint8_t (or part of one) checked, carry on, first count bits in
			   that first uint8_t */
			r0=8-j;
			/* check up to 4 more bytes, if not found than an error */
			for (k=i+1;k<i+5;k++) { if ( (xq=x[k]) != 0 ) {
			  /* caught it here, find the bit and then jump from loop */
			  if ((xq&1) != 0) r0+=1; else {
			    if ((xq&2) != 0) r0+=2; else {
			      if ((xq&4) != 0) r0+=3; else {
				if ((xq&8) != 0) r0+=4; else {
				  if ((xq&16) != 0) r0+=5; else {
				    if ((xq&32) != 0) r0+=6; else {
				      if ((xq&64) != 0) r0+=7; else {
					if ((xq&128) != 0) r0+=8; }}}}}}} break; } else { r0=r0+8; 
					/* add 8 bits for each all zero uint8_t */
					if (r0 > 32) { printf("DECRUNCH -- bad bit sequence, cannot continue");
					return -1; }       }       }       }
      r1=r1+r0;       /* update pointer */
      /* r0 even or odd determines sign of difference */
      if ((r0&1) != 0) { 
	/* positive case */
	r0=(r0/2)<<r9;  iq=iq+r2;       iq=iq+r0;       array[in]=iq;
      } else
	{ if (r0 == 32) { 
	  /* a long one, yank out the next 9 bits and use as difference */
	  i=r1/8;         j=r1%8;
#if WORDS_BIGENDIAN
	  y.b[3]=x[i];
	  y.b[2]=x[i+1];
#else
	  y.b[0]=x[i];
	  y.b[1]=x[i+1];
#endif
	  /* shift and mask out the 9 bit slice */
	  r2=(y.i>>j) & 0x1ff;
	  r1=r1+9;
	  /* if the top bit was set, do a sign extend, note that 32 bit arithmetic used*/
	  if ( (r2& 0x100) != 0 ) r2=r2 | 0xffffff00;
	  r4=array[in-1]; r4=r4+r2; array[in]=r4; iq=r4;
	} else {
	  /* minus case (normal) */
	  r0=(-r0/2)<<r9; iq=iq+r2;       iq=iq+r0;       array[in]=iq;
	} }
      in=in+1;                                }       /* end of ix loop */
    i=(r1+7)/8;     r1=8*i;                 }       /* end of iy loop */
  return 1;
}       /* end of routine */
 /*--------------------------------------------------------------------------*/
int32_t anacrunchrun(uint8_t *x, short array[], int32_t slice, int32_t nx, int32_t ny, int32_t limit)
 /* compress 16 bit array into x (a uint8_t array) using ny blocks each of size
 nx, bit slice size slice, returns # of bytes in x */
{
  struct compresshead {
    int32_t     tsize,nblocks,bsize;
    uint8_t    slice_size,type; } *ch;
  short	*p;
  unsigned nb;
  unsigned register i,j,r1;
  int32_t	r0,r3,mask, nrun, lrun, ic;
  int32_t	*dif, *d, nc, zq, yq, *dd;
  /* enum	state { RUN, LITERAL }; */
  int32_t	i2,k,iy;
  union { int32_t i; short w; unsigned char b[4]; } y;
  /* begin execution */
  if (limit < 25)
    return luxerror("limit (%d) too small in crunchrun", 0, limit);
  limit = limit - 24;	/* some margin since we don't check all times */
  mask=1; for (i=0;i<slice;i++) mask=2*mask;
  mask=mask-1; /* no inline expon. in C */
  /* determine the # of bytes to transfer to 32 bit int32_t for fixed portion */
  if (slice == 0) nb=0; else { if (slice < 2 ) nb=1;
  else { if (slice < 10) nb=2; else nb=3;    }};
  y.i=0;
  /* do the compression header */
  ch = (struct compresshead *) x;
  /* important note - can't use the sizeof(struct compresshead) because it
     is 14 on some machines and rounded up to 16 on others */
  x = x + 14;
  ch->bsize = nx;  ch->nblocks = ny;  ch->slice_size = slice;  ch->type = 2;
  i=0;    r1=0;
  dif = (int32_t *) malloc(nx*4);			/* line buffer */
  for (iy=0;iy<ny;iy++) {                 	/* start of iy (outer) loop */
    /* load the first value, reverse bytes (VAX style)*/
#if WORDS_BIGENDIAN
    y.w=array[iy*nx]   ;x[i++]=y.b[1]    ;x[i++]=y.b[0];
#else
    y.w=array[iy*nx]   ;x[i++]=y.b[0]    ;x[i++]=y.b[1];
#endif 
    /* compute and store the first differences for this line */
    p = (array+nx*iy);	nc=nx-1;
    d=dif; yq=(int32_t) *p++;	zq=(int32_t) *p++;
    while (nc--) { *d++ = zq - yq; yq = zq; zq = (int32_t) *p++; }
    r1=r1+16;
    p = (array+nx*iy);	nc=nx-1;
    d=dif;
    ic = i++;			/* count position */
    r1=r1+8;		/* to cover first count */
    lrun = 0;		/* literal count) */
    while (1) {
      /* look for a run */
      /* printf("*d, *(d+1) = %d %d, nc = %d\n",*d, *(d+1), nc );*/
      y.i = *d++;
      if (nc > 1) {
	while ( y.i == *d ) {	/* at least a run of 2 */
	  dd = d+1;	nrun = 2;
	  while ( nc-- > 2 && y.i == *dd) {
	    /* printf("run!, y.i, *dd = %d %d, nc = %d\n", y.i, *dd, nc ); */
	    nrun++;  dd++; }
	  /* short runs are not worth it, make the legal limit 4 */
	  /* printf("nrun = %d, nc = %d\n", nrun,nc);*/
	  if ( nrun >= 4 ) {	/* code the run */
	    /* a previous literal ? */
	    if (lrun != 0) {
	      /* printf("previous was literal, ic, i = %d %d\n", ic,i);*/
	      x[ic] = lrun;	i = (r1+7)/8;	lrun = 0;
	      /* printf("now, i = %d\n",i );*/
	    } else i=ic;
	    while (nrun  > 128 )	{ /* a big one, multiple runs */
	      /* need only 2 bytes to represent run, runs can't be 17 bits */
	      if (nrun == 129)	/* beware the dreaded 129 */
		{ x[i++] = 0x82; nrun -= 127;} else { x[i++] = 0x81;	nrun -= 128; }
#if WORDS_BIGENDIAN
	      x[i++]=y.b[3];	x[i++]=y.b[2];
#else
	      x[i++]=y.b[0];	x[i++]=y.b[1];
#endif
	    }
	    /* printf("encoding run, nrun = %d, i=%d, iy = %d\n",nrun,i,iy); */
#if WORDS_BIGENDIAN
	    x[i++] = -(nrun-1);	x[i++]=y.b[3];	x[i++]=y.b[2];
#else
	    x[i++] = -(nrun-1);	x[i++]=y.b[0];	x[i++]=y.b[1];
#endif
	    /* prepare for a literal and for next run check */
	    nc--;
	    if (nc <= 0) goto ended_on_run;
	    lrun = 0;	ic = i++;	r1 = 8*i;	d = dd;
	    y.i = *d++;
	    /* printf("after a run, new y.i = %d, new *d = %d\n", y.i, *d);*/
	  } else { nc = nc + nrun -1;	break; }
	}	/* not a run, do next literal, assume setup for literals */
      } else if (nc <= 0) break;
      nc--;
      /* increment the literal count */
      /* printf("literal, lrun = %d, nc = %d, ic,i = %d %d\n", lrun, nc, ic,i);*/
      if (++lrun > 127)	{		/* need a new literal run count */
	x[ic] = 127;
	/* printf("ic = %d, i,r1 = %d %d\n", ic,i,r1); */
	/* bump to next uint8_t boundary */
	i = (r1+7)/8;	ic = i++;     r1 = 8*i;		lrun = 1;
	/* printf("next ic = %d\n", ic); */
      }
      /* first the fixed slice portion */
      /* printf("y.i = %d\n", y.i);*/
      r3=(y.i>>slice);
      i=r1>>3;	/* uint8_t number */
      j=r1 & 7;		/* bit number */
      if ( i > limit ) return -1;			/* bad news, went too far */
      /* now load nb bytes into x */
      /*low order uint8_t of y.i is first in stream */
#if WORDS_BIGENDIAN
      if (j == 0) {y.i=(y.i & mask); x[i]=y.b[3];}
      else { y.i=(y.i & mask)<<j; x[i]=x[i] | y.b[3];}
      if (nb>1) { x[i+1]=y.b[2]; if (nb>2) x[i+2]=y.b[1]; }
#else
      if (j == 0) {y.i=(y.i & mask); x[i]=y.b[0];}
      else { y.i=(y.i & mask)<<j; x[i]=x[i] | y.b[0];}
      if (nb>1) { x[i+1]=y.b[1]; if (nb>2) x[i+2]=y.b[2]; }
#endif 
      r1=r1+slice;       			/* bump r1 pass the fixed part */
      i=r1>>3;                j=r1 & 7;
      /* note that r3 is the # of bits required minus 1 */
      /* printf("r3 = %d\n", r3);*/
      if (r3==0) { if (j ==0 ) {x[i]=bits[j];} else {x[i]=x[i]|bits[j];}
      r1+=1;} else    {
	r3=2*r3;        if (r3<0) r3 = -r3-1;
	if (r3<31)  {
	  r0=j+r3;        /* this is the bit that needs setting offset from x[i] */
	  if (r0 < 8) { if (j == 0) x[i]=bits[r0]; else x[i]=x[i]|bits[r0];}
	  else {if (j == 0) x[i]=0;  j=r0%8; if (r0 < 16) x[i+1]=bits[j];
	  else { i2=i+r0/8; for (k=i+1;k<i2;k++) x[k]=0;  x[i2]=bits[j]; }
	  }
	  r1+=1+r3;
	} else {        /* big one exception, should be rare */
	  /* does not need to be efficient, used rarely */
	  /* printf("big one \n");*/
	  if (j == 0) x[i]=0;     /* gotta zero the virgins */
	  r0=j+31;        j=r0%8;         i2=i+r0/8;
	  for (k=i+1;k<i2;k++) x[k]=0;    x[i2]=bits[j];
	  /* recompute the difference and load 17 bits (always 3 bytes) */
	  r1=r1+32;
	  i=r1/8;
	  j=r1%8;
	  if (j == 0) x[i]=0;     /* gotta zero the virgins */
	  y.i=((*(d-1))& 0x1ffff) << j;
#if WORDS_BIGENDIAN
	  x[i]=x[i] | y.b[3]; x[i+1]=y.b[2];      x[i+2]=y.b[1];
#else
	  x[i]=x[i] | y.b[0]; x[i+1]=y.b[1];      x[i+2]=y.b[2];
#endif
	  r1=r1+17;       } /* end of big one exception */
      }       /* end of (r3==0) conditional */
      /* printf("end of literal, i,r1 = %d %d\n", i,r1);*/
      /* printf(" x[r1/8] = %d\n",  x[r1/8]);*/
    }       /* end of ix loop */
    /* some checks here */
    /* bump to next uint8_t boundary */
    /* a final literal ? */
    /* printf("final lrun = %d, ic = %d\n", lrun, ic);*/
    if (lrun != 0) { x[ic] = lrun;	lrun = 0; }
    i=(r1+7)/8;
  ended_on_run:
    r1=8*i;
  }       /* end of iy loop */
  ch->tsize = i = i + 14;
  /* we have to put these in a form readable by the Vax (these may be used
     by fcwrite) */
#if WORDS_BIGENDIAN
  swapl(&(ch->tsize),1); swapl(&(ch->bsize),1); swapl(&(ch->nblocks),1); 
#endif
  free(dif);
  return  i;      /*return # of bytes used */
}       /* end of routine */
 /*--------------------------------------------------------------------------*/
int32_t anacrunchrun8(uint8_t *x, uint8_t array[], int32_t slice, int32_t nx, int32_t ny, int32_t limit)
 /* compress 8 bit array into x (a uint8_t array) using ny blocks each of size
 nx, bit slice size slice, returns # of bytes in x */
{
  struct compresshead {
    int32_t     tsize,nblocks,bsize;
    uint8_t    slice_size,type; } *ch;
  uint8_t	*p;
  unsigned nb;
  unsigned register i,j,r1;
  int32_t	r0,r3,mask, nrun, lrun, ic;
  int32_t	*dif, *d, nc, zq, yq, *dd;
  int32_t	i2,k,iy;
  union { int32_t i; short w; unsigned char b[4]; } y;
  /* begin execution */
  if (limit < 25)
    return luxerror("limit (%d) too small in crunchrun8", 0, limit);
  limit = limit - 24;	/* some margin since we don't check all times */
  mask=1; for (i=0;i<slice;i++) mask=2*mask;
  mask=mask-1; /* no inline expon. in C */
  /* determine the # of bytes to transfer to 32 bit int32_t for fixed portion */
  if (slice == 0) nb=0; else { if (slice < 2 ) nb=1;
  else { if (slice < 10) nb=2; else nb=3;    }};
  y.i=0;
  /* do the compression header */
  ch = (struct compresshead *) x;
  /* important note - can't use the sizeof(struct compresshead) because it
     is 14 on some machines and rounded up to 16 on others */
  x = x + 14;
  ch->bsize = nx;  ch->nblocks = ny;  ch->slice_size = slice;  ch->type = 3;
  i=0;    r1=0;
  dif = (int32_t *) malloc(nx*4);			/* line buffer */
  for (iy=0;iy<ny;iy++) {                 	/* start of iy (outer) loop */
    /* load the first value */
    x[i++] = array[iy*nx];
 
    /* compute and store the first differences for this line */
    p = (array+nx*iy);	nc=nx-1;
    d=dif; yq=(int32_t) *p++;	zq=(int32_t) *p++;
    while (nc--) { *d++ = zq - yq; yq = zq; zq = (int32_t) *p++; }
    r1=r1+8;
    p = (array+nx*iy);	nc=nx-1;
    d=dif;
    ic = i++;			/* count position */
    r1=r1+8;		/* to cover first count */
    lrun = 0;		/* literal count) */
    while (1) {
      /* look for a run */
      /* printf("*d, *(d+1) = %d %d, nc = %d\n",*d, *(d+1), nc );*/
      y.i = *d++;
      if (nc > 1) {
	while ( y.i == *d ) {	/* at least a run of 2 */
	  dd = d+1;	nrun = 2;
	  while ( nc-- > 2 && y.i == *dd) {
	    /* printf("run!, y.i, *dd = %d %d, nc = %d\n", y.i, *dd, nc ); */
	    nrun++;  dd++; }
	  /* short runs are not worth it, make the legal limit 4 */
	  /* printf("nrun = %d, nc = %d\n", nrun,nc);*/
	  if ( nrun >= 4 ) {	/* code the run */
	    /* a previous literal ? */
	    if (lrun != 0) {
	      /* printf("previous was literal, ic, i = %d %d\n", ic,i);*/
	      x[ic] = lrun;	i = (r1+7)/8;	lrun = 0;
	      /* printf("now, i = %d\n",i );*/
	    } else i=ic;
	    while (nrun  > 128 )	{ /* a big one, multiple runs */
	      /* printf("big run, nrun = %d\n", nrun); */
	      /* need only 2 bytes to represent run, runs can't be 17 bits */
	      if (nrun == 129)	/* beware the dreaded 129 */
		{ x[i++] = 0x82; nrun -= 127;} else { x[i++] = 0x81;	nrun -= 128; }
#if WORDS_BIGENDIAN
	      x[i++]=y.b[3];	x[i++]=y.b[2];
#else
	      x[i++]=y.b[0];	x[i++]=y.b[1];
#endif
	    }
	    /* printf("encoding run, nrun = %d, i=%d, iy = %d\n",nrun,i,iy); */
#if WORDS_BIGENDIAN
	    x[i++] = -(nrun-1);	x[i++]=y.b[3];	x[i++]=y.b[2];
#else
	    x[i++] = -(nrun-1);	x[i++]=y.b[0];	x[i++]=y.b[1];
#endif
	    /* prepare for a literal and for next run check */
	    nc--;
	    if (nc <= 0) goto ended_on_run;
	    lrun = 0;	ic = i++;	r1 = 8*i;	d = dd;
	    y.i = *d++;
	    /* printf("after a run, new y.i = %d, new *d = %d\n", y.i, *d);*/
	  } else { nc = nc + nrun -1;	break; }
	}	/* not a run, do next literal, assume setup for literals */
      } else if (nc <= 0) break;
      nc--;
      /* increment the literal count */
      /* printf("literal, lrun = %d, nc = %d, ic,i = %d %d\n", lrun, nc, ic,i);*/
      if (++lrun > 127)	{		/* need a new literal run count */
	x[ic] = 127;
	/* printf("ic = %d, i,r1 = %d %d\n", ic,i,r1); */
	/* bump to next uint8_t boundary */
	i = (r1+7)/8;	ic = i++;     r1 = 8*i;		lrun = 1;
	/* printf("next ic = %d\n", ic); */
      }
      /* first the fixed slice portion */
      /* printf("y.i = %d\n", y.i);*/
      r3=(y.i>>slice);
      i=r1>>3;	/* uint8_t number */
      j=r1 & 7;		/* bit number */
      if ( i > limit ) return -1;			/* bad news, went too far */
      /* now load nb bytes into x */
      /*low order uint8_t of y.i is first in stream */
#if WORDS_BIGENDIAN
      if (j == 0) {y.i=(y.i & mask); x[i]=y.b[3];}
      else { y.i=(y.i & mask)<<j; x[i]=x[i] | y.b[3];}
      if (nb>1) { x[i+1]=y.b[2]; if (nb>2) x[i+2]=y.b[1]; }
#else
      if (j == 0) {y.i=(y.i & mask); x[i]=y.b[0];}
      else { y.i=(y.i & mask)<<j; x[i]=x[i] | y.b[0];}
      if (nb>1) { x[i+1]=y.b[1]; if (nb>2) x[i+2]=y.b[2]; }
#endif
 
      r1=r1+slice;       			/* bump r1 pass the fixed part */
      i=r1>>3;                j=r1 & 7;
      /* note that r3 is the # of bits required minus 1 */
      if (r3==0) { if (j ==0 ) {x[i]=bits[j];} else {x[i]=x[i]|bits[j];}
      r1+=1;} else    {
	r3=2*r3;        if (r3<0) r3 = -r3-1;
	if (r3<31)  {
	  r0=j+r3;        /* this is the bit that needs setting offset from x[i] */
	  if (r0 < 8) { if (j == 0) x[i]=bits[r0]; else x[i]=x[i]|bits[r0];}
	  else {if (j == 0) x[i]=0;  j=r0%8; if (r0 < 16) x[i+1]=bits[j];
	  else { i2=i+r0/8; for (k=i+1;k<i2;k++) x[k]=0;  x[i2]=bits[j]; }
	  }
	  r1+=1+r3;
	} else {        /* big one exception, should be rare */
	  /* does not need to be efficient, used rarely */
	  /* printf("big one \n");*/
	  if (j == 0) x[i]=0;     /* gotta zero the virgins */
	  r0=j+31;        j=r0%8;         i2=i+r0/8;
	  for (k=i+1;k<i2;k++) x[k]=0;    x[i2]=bits[j];
	  /* recompute the difference and load 9 bits (always 2 bytes) */
	  r1=r1+32;
	  i=r1/8;
	  j=r1%8;
	  if (j == 0) x[i]=0;     /* gotta zero the virgins */
	  y.i=((*(d-1))& 0x1ff) << j;
#if WORDS_BIGENDIAN
	  x[i]=x[i] | y.b[3]; x[i+1]=y.b[2];
#else
	  x[i]=x[i] | y.b[0]; x[i+1]=y.b[1];
#endif
	  r1=r1+9;       } /* end of big one exception */
      }       /* end of (r3==0) conditional */
      /* printf("end of literal, i,r1 = %d %d\n", i,r1);*/
      /* printf(" x[r1/8] = %d\n",  x[r1/8]);*/
    }       /* end of ix loop */
    /* some checks here */
    /* bump to next uint8_t boundary */
    /* a final literal ? */
    /* printf("final lrun = %d, ic = %d\n", lrun, ic);*/
    if (lrun != 0) { x[ic] = lrun;	lrun = 0; }
    i=(r1+7)/8;
  ended_on_run:
    r1=8*i;
  }       /* end of iy loop */
  ch->tsize = i = i + 14;
  /* we have to put these in a form readable by the Vax (these may be used
     by fcwrite) */
#if WORDS_BIGENDIAN
  swapl(&(ch->tsize),1); swapl(&(ch->bsize),1); swapl(&(ch->nblocks),1); 
#endif
  free(dif);
  return  i;      /*return # of bytes used */
}
 /*--------------------------------------------------------------------------*/
int32_t anadecrunchrun(uint8_t *x, short array[], int32_t r9, int32_t nx, int32_t ny)
 /* decompress a bit stream in x; result is n I*2 elements, put in array;
	 bit slice size r9 */
 /* this version handles the run length encoding used in anacrunchrun */
{
  short iq;
  int32_t r0,r1,r2,r4,nb,mask,nrun,n,nc;
  int32_t j,in,i,k,iy;
  unsigned char xq;
  union { int32_t i; short w; unsigned char b[4]; } y;
  /* begin execution */
  mask=1; for (i=0;i<r9;i++) mask=2*mask; mask=mask-1;
  /*printf("slice width = %d\n",r9);*/
  /*printf ("mask = %x, %d\n",mask,mask);*/
  /* determine the # of bytes to transfer to 32 bit int32_t for fixed portion */
  if (r9 == 0) nb=0; else { if (r9 < 2 ) nb=1;
  else { if (r9 < 10) nb=2; else nb=3;    }};
  y.i=0;
  i=0;    r1=0;   in=0;
  for (iy=0;iy<ny;iy++) {                 /* start of iy (outer) loop */
    /* get the first value, assume bytes reversed */
#if WORDS_BIGENDIAN
    y.b[1]=x[i++];	y.b[0]=x[i++];    iq=y.w; array[in++]=iq;
#else
    y.b[0]=x[i++];	y.b[1]=x[i++];    iq=y.w; array[in++]=iq;
#endif
    /* printf("first value = %d 0x%x\n",iq, iq); */
    r1=r1+16;
    r2=0;
    nc=nx-1;
    while (nc>0) {
      /* look at the next run length code */
      /* printf("i = %d\n", i); */
      nrun = (int32_t) x[i++];
      /* printf("nrun = %d\n", nrun); */
      if (nrun > 127) {	/* a run of a constant difference */
	n = 255 - nrun + 2;
	nc = nc - n;
#if WORDS_BIGENDIAN
	y.b[1]=x[i++];	y.b[0]=x[i++];
#else
	y.b[0]=x[i++];	y.b[1]=x[i++];
#endif
	/* printf("increment (run) = %d\n", y.w); */
	while (n--) {
	  array[in] = array[in-1] + y.w;	in++;
	}
	iq = array[in-1];	r1=8*i;
      } else {	/* a literal */
	r1 = 8 * i;
	nc = nc - nrun;
	while(nrun--) {
	  /* first the fixed slice portion */
	  i=r1/8;         j=r1%8;
	  /* printf("start literal, i,r1 = %d %d\n", i,r1); */
#if WORDS_BIGENDIAN
	  y.b[3]=x[i];
	  /* test effect on timing */
	  if (nb>1) { y.b[2]=x[i+1]; if (nb>2) y.b[1]=x[i+2]; }
#else
	  y.b[0]=x[i];
	  /* test effect on timing */
	  if (nb>1) { y.b[1]=x[i+1]; if (nb>2) y.b[2]=x[i+2]; }
#endif
	  /* shift and mask out the bit slice */
	  r2=(y.i>>j) & mask;
	  /* printf("r2 = %x, %d\n",r2,r2);*/
	  /* the variable bit portion, find the first set bit */
	  r1=r1+r9;       /* bump r1 pass the fixed part */
	  i=r1/8;         j=r1%8;
	  if ((xq=x[i]>>j) != 0) {
	    /* caught it on first uint8_t, find the bit */
	    if ((xq&1) != 0) r0=1; else {
	      if ((xq&2) != 0) r0=2; else {
		if ((xq&4) != 0) r0=3; else {
		  if ((xq&8) != 0) r0=4; else {
		    if ((xq&16) != 0) r0=5; else {
		      if ((xq&32) != 0) r0=6; else {
			if ((xq&64) != 0) r0=7; else {
			  if ((xq&128) != 0) r0=8; }}}}}}}}       else {
			    /* not in first uint8_t (or part of one) checked, carry on, first count bits in
			       that first uint8_t */
			    r0=8-j;
			    /* check up to 4 more bytes, if not found than an error */
			    for (k=i+1;k<i+5;k++) { if ( (xq=x[k]) != 0 ) {
			      /* caught it here, find the bit and then jump from loop */
			      if ((xq&1) != 0) r0+=1; else {
				if ((xq&2) != 0) r0+=2; else {
				  if ((xq&4) != 0) r0+=3; else {
				    if ((xq&8) != 0) r0+=4; else {
				      if ((xq&16) != 0) r0+=5; else {
					if ((xq&32) != 0) r0+=6; else {
					  if ((xq&64) != 0) r0+=7; else {
					    if ((xq&128) != 0) r0+=8; }}}}}}} break; } else { r0=r0+8; 
					    /* add 8 bits for each all zero uint8_t */
					    if (r0 > 32) { printf("DECRUNCH -- bad bit sequence, cannot continue\n");
					    printf("i = %d, r1 = %d, iy = %d\n",i,r1,iy);
					    return -1; }       }       }       }
	  r1=r1+r0;       /* update pointer */
	  /* r0 even or odd determines sign of difference */
	  if ((r0&1) != 0) { 
	    /* positive case */
	    r0=(r0/2)<<r9;  iq=iq+r2;       iq=iq+r0;       array[in]=iq;
	    /* printf("r0,r2,iq = %d %d %d\n", r0,r2,iq);*/
	  } else
	    { if (r0 == 32) { 
	      /* a long one, yank out the next 17 bits and use as difference */
	      i=r1/8;         j=r1%8;
#if WORDS_BIGENDIAN
	      y.b[3]=x[i];
	      y.b[2]=x[i+1]; y.b[1]=x[i+2];
#else
	      y.b[0]=x[i];
	      y.b[1]=x[i+1]; y.b[2]=x[i+2];
#endif
	      /* shift and mask out the 17 bit slice */
	      r2=(y.i>>j) & 0x1ffff;
	      r1=r1+17;
	      /* if the top bit was set, do a sign extend, note that 32 bit arithmetic used*/
	      if ( (r2& 0x10000) != 0 ) r2=r2 | 0xffff0000;
	      /* printf("big one, r2 = %d, array[in-1] = %d\n", r2, array[in-1]);*/
	      r4=array[in-1]; r4=r4+r2; array[in]=r4; iq=r4;
	    } else {
	      /* minus case (normal) */
	      r0=(-r0/2)<<r9; iq=iq+r2;       iq=iq+r0;       array[in]=iq;
	      /* printf("r0,r2,iq = %d %d %d\n", r0,r2,iq); */
	    }}
	  /* printf("end literal, i,r1 = %d %d\n", i,r1); */
	  in=in+1;
	}
	/* printf("r1, i after nrun exhausted = %d %d\n",r1,i); */
	i=(r1+7)/8;     r1=8*i;
	/* printf("new i = %d\n",i); */
      }
    }   	    /* end of ix loop */
    if (nc < 0) {
      printf("bad loop in decrunchrun, nc=%d, iy=%d, in= %d\n",nc,iy,in);  return -1; }
 
    i=(r1+7)/8;     r1=8*i;                 }   	    /* end of iy loop */
  return 1;
}  						     /* end of routine */
 /*--------------------------------------------------------------------------*/
int32_t anadecrunchrun8(uint8_t x[], uint8_t array[], int32_t r9, int32_t nx, int32_t ny)
 /* decompress a bit stream in x; result is n I*2 elements, put in array;
	 bit slice size r9 */
 /* this version handles the run length encoding used in anacrunchrun */
{
  uint8_t iq;
  int32_t r0,r1,r2,r4,nb,mask,nrun,n,nc;
  int32_t j,in,i,k,iy;
  unsigned char xq;
  union { int32_t i; short w; unsigned char b[4]; } y;
  /* begin execution */
  mask=1; for (i=0;i<r9;i++) mask=2*mask; mask=mask-1;
  /* determine the # of bytes to transfer to 32 bit int32_t for fixed portion */
  if (r9 == 0) nb=0; else { if (r9 < 2 ) nb=1;
  else { if (r9 < 10) nb=2; else nb=3;    }};
  y.i=0;
  i=0;    r1=0;   in=0;
  for (iy=0;iy<ny;iy++) {                 /* start of iy (outer) loop */
    /* get the first value */
    iq=x[i++];	array[in++]=iq;
    /* printf("first value = %d 0x%x\n",iq, iq); */
    r1=r1+16;
    r2=0;
    nc=nx-1;
    /* printf("nc = %d\n", nc); */
    while (nc>0) {
      /* look at the next run length code */
      /* printf("i = %d\n", i); */
      nrun = (int32_t) x[i++];
      /* printf("nrun = %d\n", nrun); */
      if (nrun > 127) {	/* a run of a constant difference */
	n = 255 - nrun + 2;
	nc = nc - n;
#if WORDS_BIGENDIAN
	y.b[1]=x[i++];	y.b[0]=x[i++];
#else
	y.b[0]=x[i++];	y.b[1]=x[i++];
#endif
	/* printf("increment (run) = %d of length %d\n", y.w,n); */
	while (n--) {
	  array[in] = array[in-1] + y.w;	in++;
	}
	iq = array[in-1];	r1=8*i;
      } else {	/* a literal */
	r1 = 8 * i;
	nc = nc - nrun;
	while(nrun--) {
	  /* first the fixed slice portion */
	  i=r1/8;         j=r1%8;
	  /* printf("start literal, i,r1 = %d %d\n", i,r1); */
#if WORDS_BIGENDIAN
	  y.b[3]=x[i];
	  /* test effect on timing */
	  if (nb>1) { y.b[2]=x[i+1]; if (nb>2) y.b[1]=x[i+2]; }
#else
	  y.b[0]=x[i];
	  /* test effect on timing */
	  if (nb>1) { y.b[1]=x[i+1]; if (nb>2) y.b[2]=x[i+2]; }
#endif
	  /* shift and mask out the bit slice */
	  r2=(y.i>>j) & mask;
	  /* the variable bit portion, find the first set bit */
	  r1=r1+r9;       /* bump r1 pass the fixed part */
	  i=r1/8;         j=r1%8;
	  if ((xq=x[i]>>j) != 0) {
	    /* caught it on first uint8_t, find the bit */
	    if ((xq&1) != 0) r0=1; else {
	      if ((xq&2) != 0) r0=2; else {
		if ((xq&4) != 0) r0=3; else {
		  if ((xq&8) != 0) r0=4; else {
		    if ((xq&16) != 0) r0=5; else {
		      if ((xq&32) != 0) r0=6; else {
			if ((xq&64) != 0) r0=7; else {
			  if ((xq&128) != 0) r0=8;
			}
		      }
		    }
		  }
		}
	      }
	    }
	  } else {
	    /* not in first uint8_t (or part of one) checked, carry on, first count bits in
	       that first uint8_t */
	    r0=8-j;
	    /* check up to 4 more bytes, if not found than an error */
	    for (k=i+1;k<i+5;k++) { if ( (xq=x[k]) != 0 ) {
	      /* caught it here, find the bit and then jump from loop */
	      if ((xq&1) != 0) r0+=1; else {
		if ((xq&2) != 0) r0+=2; else {
		  if ((xq&4) != 0) r0+=3; else {
		    if ((xq&8) != 0) r0+=4; else {
		      if ((xq&16) != 0) r0+=5; else {
			if ((xq&32) != 0) r0+=6; else {
			  if ((xq&64) != 0) r0+=7; else {
			    if ((xq&128) != 0) r0+=8;
			  }
			}
		      }
		    }
		  }
		}
	      }
	      break;
	    } else {
	      r0=r0+8; 
	      /* add 8 bits for each all zero uint8_t */
	      if (r0 > 32) { printf("DECRUNCH -- bad bit sequence, cannot continue\n");
	      printf("i = %d, r1 = %d, iy = %d\n",i,r1,iy);
	      return -1; }       }       }       }
	  r1=r1+r0;       /* update pointer */
	  /* r0 even or odd determines sign of difference */
	  if ((r0&1) != 0) { 
	    /* positive case */
	    r0=(r0/2)<<r9;  iq=iq+r2;       iq=iq+r0;       array[in]=iq;
	    /* printf("r0,r2,iq = %d %d %d\n", r0,r2,iq);*/
	  } else
	    { if (r0 == 32) { 
	      /* a long one, yank out the next 9 bits and use as difference */
	      i=r1/8;         j=r1%8;
#if WORDS_BIGENDIAN
	      y.b[3]=x[i];
	      y.b[2]=x[i+1];
#else
	      y.b[0]=x[i];
	      y.b[1]=x[i+1];
#endif
	      /* shift and mask out the 9 bit slice */
	      r2=(y.i>>j) & 0x1ff;
	      r1=r1+9;
	      /* if the top bit was set, do a sign extend, note that 32 bit arithmetic used*/
	      if ( (r2& 0x100) != 0 ) r2=r2 | 0xffffff00;
	      /* printf("long one decoded, r2 = %d, array[in-1]=%d\n", r2, array[in-1]); */
	      r4=array[in-1]; r4=r4+r2; array[in]=r4; iq=r4;
	    } else {
	      /* minus case (normal) */
	      r0=(-r0/2)<<r9; iq=iq+r2;       iq=iq+r0;       array[in]=iq;
	      /* printf("r0,r2,iq = %d %d %d\n", r0,r2,iq); */
	    }}
	  /* printf("end literal, i,r1 = %d %d\n", i,r1); */
	  in=in+1;
	}
	/* printf("r1, i after literal nrun exhausted = %d %d\n",r1,i); */
	i=(r1+7)/8;     r1=8*i;
	/* printf("new i = %d\n",i); */
      }
    }   	    /* end of ix loop */
    if (nc < 0) {
      printf("bad loop in decrunchrun8, nc=%d, iy=%d, in= %d\n",nc,iy,in);
      return -1; }
 
    i=(r1+7)/8;     r1=8*i;                 }   	    /* end of iy loop */
  return 1;
}  						     /* end of routine */
 /*--------------------------------------------------------------------------*/
