/* This is file fun6.c.

Copyright 2013 Louis Strous, Richard Shine

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
/* fun6.c */
#include <math.h>
#include <strings.h>		/* for bzero */
#include "action.h"

 /*------------------------------------------------------------------------- */
static void rdct_spike(Word *start, Int ystride, Float *ws)
 /* does reverse dct for one cell, specialize for a spike smooth */
{
  Float	tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
  Float	tmp10, tmp11, tmp12, tmp13;
  Float	z5, z11, z13, z10, z12;

  /* no de-zag and de-quantize here */
  /* Pass 1: process columns. */
  /* we don't check for columns of zeroes since this usually uses full
     precision */
{
  register Float *wsptr = ws;
  Int	nq = 8;

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
  }
}

  /* Pass 2: process rows. */
{
  register Float *wsptr;
  register short *elemptr;
  Int	nq = 8;
  
  wsptr = ws;
  elemptr = start;
  while (nq--) {
      /* Even part */

    /* tmp10 = wsptr[0] + wsptr[4] + bias; */
    /* tmp11 = wsptr[0] - wsptr[4] + bias; */
    tmp10 = wsptr[0] + wsptr[4];
    tmp11 = wsptr[0] - wsptr[4];

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
  }
}
}
/*---------------------------------------------------------------------------*/
#define ALN2I 1.442695022
#define TINY 1.0e-5
Int	despike_count = 0, cell_smooth_type = 1;
Int ana_despike(Int narg, Int ps[])
/* despike function RAS */
/* the call is x = despike(array, [frac, level, niter, cell_flag, rms]) */
{
  Int	iq, result_sym, type, nx, ny, n, m, level=7, sum, nc, cell_flag=0;
  Int	lognb2, cell_malloc = 0, cell_count, jj, jc;
  Int	nxc, nyc, cell_flag_sign, niter=1, rms_flag_sign, rms_flag;
  Int	sign_flag, bad_flag, bb_flag, save_niter, ntotal, dim[2];
  Byte	*cell_status;
  Float	frac = 0.25, fsum, cfrac, tq, rms=0.0, fdif;
  Word	*p, *q, *ptr, *p1, *p2, *p3, *out, *out2;
  Word	arr[16], *pps, *ss;

  lognb2 = (log((Double) 16)*ALN2I+TINY);

  if (narg > 1 && ps[1] && float_arg_stat(ps[1], &frac) != 1)
    return ANA_ERROR;
  if (narg > 2 && ps[2] && int_arg_stat(ps[2], &level) != 1)
    return ANA_ERROR;
  if (level > 15)
    level = 15;
  if (narg > 3 && ps[3] && int_arg_stat(ps[3], &niter) != 1)
    return ANA_ERROR;
  if (narg > 4 && ps[4] && int_arg_stat(ps[4], &cell_flag) != 1)
    return ANA_ERROR;
  if (narg > 5 && ps[5] && float_arg_stat(ps[5], &rms) != 1)
    return ANA_ERROR;

 if (cell_flag > 0)
   cell_flag_sign = 1;
 else
   cell_flag_sign = 0;
 cell_flag = ABS(cell_flag);
 if (rms != 0.0) {
   rms_flag = 1;
   if (rms > 0)
     rms_flag_sign = 1;
   else {
     rms_flag_sign = 0;
     rms = -rms;
   }
 } else
   rms_flag = 0;
 /* get pointer to array, must be 2-D here */
 iq = ps[0];
 if (symbol_class(iq) != ANA_ARRAY)
   return cerror(NEED_ARR, iq);
 type = array_type(iq);
 if (type != ANA_WORD)
   return anaerror("Need WORD argument", iq); /* must be I*2 */
 if (array_num_dims(iq) != 2)
   return cerror(NEED_2D_ARR, iq);
 nx = array_dims(iq)[0];
 ny = array_dims(iq)[1];
 if (nx < 5 || ny < 5)
   return anaerror("dimensions must be 5 or greater, nx, ny = %d %d\n", nx,ny);

 /* the cell_flag is used to indicate that we destroy entire cells which
 are corrupted by cell_flag or more spikes, but this is intended only
 for TRACE style JPEG data so we insist that the dimensions are multiples
 of 8. Extend to include two checks indicated by cell_flag and rms_flag,
 both need bad cells marked */
 bb_flag = cell_flag || rms_flag;	/* use bb_flag */
 if (bb_flag && (nx%8 || ny%8))
   bb_flag = 0;
 /* if we survived that, get the cell count, we use scrat to store cell
 results */
 if (bb_flag) {
  cell_count = nx*ny/64;
  if (cell_count <= (NSCRAT*sizeof(Int) + ((char *) scrat - curScrat)))
    cell_status = (Byte *) scrat;
  else {
    cell_status = (Byte *) malloc(cell_count);
    if (!cell_status)
      return cerror(ALLOC_ERR, 0);
    cell_malloc = 1;
  }
  bzero(cell_status, cell_count);
 }
 ptr = array_data(iq);
 dim[0] = nx;
 dim[1] = ny;
 result_sym = array_scratch(type, 2, dim); /* for the result */
 out = array_data(result_sym);	/* output */
 p = (short *) ptr;		/* input  */
 cfrac = 1.0 - frac;
 nc = ntotal = 0;
 nxc = nx/8;
 nyc = ny/8;
 niter = ABS(niter);
 if (niter > 20) 
   return anaerror("DESPIKE - error, excessive # of iterations = %d\n", niter);

 /* add internal iteration 10/8/98 */
 /* if there are 2 or more iterations, we need an extra array, we can't do the
    despike step in place because it "erodes" from the low y direction. */
 if (niter > 1) {
   out2 = malloc(nx*ny*sizeof(Word));
   if (!out2)
     return cerror(ALLOC_ERR, 0);
 }
 save_niter = niter;

 while (niter--) {
   /* depending on niter%2, we choose where q points, together with the
 ptr substitution at the end of the loop, we ping pong between the two
 areas, always ending on the out array */
 if (niter%2) { q = out2; } else { q = out; }
 /* load top 2 rows */
 m = 2;
 while (m--) {
 p = ptr;  n = nx;  while(n--) *q++ = *p++;
 ptr = ptr + nx;
 }
 /* skip the outer edges while testing algorithms */
 m = ny-4; jj = 2;  /* jj is used for cell addressing */
 while (m--) {
 /* skip the outer edges while testing algorithms */
 p = ptr;
 n = 2;
 while (n--) *q++ = *p++;
 n = nx-4;
 p2 = p - 1;
 p1 = p2 - nx;
 p3 = p2 + nx;
 while (n--) {
  /* add the 8 */
  tq = (cfrac * (Float) *p);
  sum = *p1 + *(p1+1) + *(p1+2) + *p2 + *(p2+2) + *p3 + *(p3+1) + *(p3+2);
  p1++;  p2++;  p3++;
  fsum = (Float) sum * 0.125;
  /* now the test */
  if ( fsum <= tq) {  /* we have a bady, zap it */
    nc++;
  /* check if we are black balling cells */
  if (bb_flag) {
  /* get the cell index */
  jc = jj/8;
  iq = (p-ptr)/8 + jc*(nxc);
  cell_status[iq] += 1;
  }
    if (level < 0) { *q++ = 0; p++; } else {
     /* load up sortie and find the desired one */
     ss = arr;	pps = p - 2*nx -2;
     *ss++ = *pps++; *ss++ = *pps++; *ss++ = *pps++; *ss++ = *pps++; *ss++ = *pps++;
     *ss++ = *(p - nx -2);  *ss++ = *(p - nx +2);
     *ss++ = *(p -2);  *ss++ = *(p +2);
     *ss++ = *(p + nx -2);  *ss++ = *(p + nx +2);
     pps = p +2 *nx - 2;
     *ss++ = *pps++; *ss++ = *pps++; *ss++ = *pps++; *ss++ = *pps++; *ss++ = *pps++;
      /* a built in sorter here */
      { Int nn,m,j,i,n=16;
        short t;
        m=n;
        for (nn=1;nn<=lognb2;nn++) {
         m >>= 1;
        for (j=m;j<n;j++) {
         i=j-m;
         t=arr[j];
         while (i >= 0 && arr[i] > t) {
         arr[i+m]=arr[i];
         i -= m;
        }
        arr[i+m]=t;
       }
     }
     }
      /* now get the indicated one using level */
      *q++ = arr[level];  p++;
     }
    
    } else {
    *q++ = *p++;    /* looks OK (this time) */
    }
 }
 n = 2;
 while (n--) *q++ = *p++;
 ptr = ptr + nx; jj++;
 }
 /* load last 2 rows */
 m = 2;
 while (m--) {
 p = ptr;  n = nx;  while(n--) *q++ = *p++;
 ptr = ptr + nx;
 } 
 /* end of this iteration, reconfigure for next one, note that for
 a single iteration we don't need out2 */
 ntotal += nc;
 if (niter%2) { ptr = out2; } else { ptr = out; }
 }
 if (save_niter > 1) free(out2);
 printf("despike got %d spikes in %d iterations\n", ntotal, save_niter);


 if (bb_flag) {
  Int badcells = 0;
  Float	dc[9], ws[64], *pf;
  Int 	i, ix, jx, ic, stride = nx - 8, ii, jj, istart,k;
  Int	ioff[3], joff[3];
  for (i=0;i<cell_count;i++) {
   /* we can have a bad cell by the number of spikes (the strike out
   option) and/or we can check the rms, if doing both we check the
   strikes first of course */
    if (!cell_status[i]) continue;  /* if no spikes at all, skip */
    bad_flag = 0;
    if (cell_flag && (cell_status[i] >= cell_flag)) { bad_flag = 1;
    	sign_flag = cell_flag_sign; }
    if (rms_flag && !bad_flag) {
     short	*ps1, *ps2, *tmp;
     sign_flag = rms_flag_sign;
     /* check the rms for any cell with a spike */
     jc = i/nxc;		ic = i%nxc;
      istart = ic*8 + jc*8*nx;
      q = out + istart;
      ps1 = q;	ps2 = q+1;
      fsum = 0.0;
      fdif = 0.0;
      nc = 8;
      /* get the checkerboard metric */
      while (nc--) { m = 4; while(m--) {
      	fsum += (Float) *q++; fsum += (Float) *q++;
	fdif = fdif + *ps1 - *ps2;
	ps1 += 2;	ps2 += 2;
      	}
      tmp = ps1; ps1 = ps2 + stride;  ps2 = tmp + stride;
      q += stride; }
      /* printf("rms/fsum = %12.2f,  i = %d\n", ABS(fdif)/fsum, i); */
      if ( (ABS(fdif)/fsum) > rms) { bad_flag = 1;
      	/* printf("hit at i = %d\n", i); */ }
    }
    if (bad_flag) {
    badcells++;
    jc = i/nxc;		ic = i%nxc;
    /* testing various cell smooth options */
    if (sign_flag == 0) cell_smooth_type = 0; else cell_smooth_type = 1;
    switch (cell_smooth_type) {
    case 0:
    dc[4] = 0.0;
    /* we need the means of the neighboring 8 cells */
    if (ic>0) ioff[0] = ic - 1; else ioff[0] = ic+1;
    ioff[1] = ic;
    if (ic < (nxc-1)) ioff[2] = ic+1; else ioff[2] = ic-1;
    if (jc>0) joff[0] = jc - 1; else joff[0] = jc+1;
    joff[1] = jc;
    if (jc < (nyc-1)) joff[2] = jc+1; else joff[2] = jc-1;
    
    for (ii=0;ii<3;ii++) {
     ix = ioff[ii]*8;
     for (jj=0;jj<3;jj++) {
      if (ii != 1 || jj != 1) {
      jx = joff[jj]*8;
      istart = ix + jx*nx;
      q = out + istart;
      k = ii + jj*3;
      fsum = 0.0;
      nc = 8;
      while (nc--) { m = 8; while(m--) fsum += (Float) *q++; q += stride; }
      dc[k] = fsum*0.015625; /* that's 1/64 */
      dc[4] += dc[k];
      }
   } }
    dc[4] = dc[4]*0.125;
    /* set target cell just to mean for next test */
    q = out + ic*8 + jc*8*nx;
    nc = 8;
    if (sign_flag)
      { /* try the AC predict trick for a smooth gradient over the cell */
      nc = 64;  pf = ws; while (nc--) *pf++ = 0.0;
      ws[0] = dc[4];
      ws[1] = 1.13885*(dc[3] - dc[5]);
      ws[8] = 1.13885*(dc[1] - dc[7]);
      ws[16] =  0.27881*(dc[1]+dc[7] - 2.*dc[4]);
      ws[9] = 0.16213*((dc[0]-dc[2])-(dc[6]-dc[8]));
      ws[2] = 0.27881*(dc[3]+dc[5] - 2.*dc[4]);
      rdct_spike(q, nx, ws);
      }
    else
      while (nc--) { m = 8; while(m--) *q++ = 0; q += stride; }
    
    break;
    case 1:
    {
    Int	a1, a2, acc, t1, t2, nxmo=nx-8 , nymo=ny-8;
    /* get array index for start of cell */
    ix = ic*8;  jx = jc*8;
    istart = ix + jx*nx;
    p = out + istart;  /* first point in cell */
    q = p;
    /* do 2 passes, first in x then y, allows in place smnoothing but
    loses a bit in the accumulation */
    nc = 8;	/* the 8 rows */
    while (nc--) {
    /* get a point to left of cell if available */
    if (ix) { p--;	
    	a2 = (Int)*p++;
    	a1 = (Int)*p++;
    	a2 = a2 + a1;
     } else {
    	a1 = (Int)*p++;
    	a2 = a1 + a1;
     }
     /* proceed over 7 points */
     m = 7;
     while (m--) {
     acc = *p++;  t1 = acc;  acc += a1;  a1 = t1;
     t2 = acc;  acc += a2;  a2 = t2;
     *q++ = (short) (acc>>2); }
     /* for the last value, use a point to right of cell if available */
     if (ix < nxmo ) {
     *q++ = (short) ((*p + a1 + a2)>>2);
     } else {
     *q++ = (short) ((a1 + a1 + a2)>>2);
     }
     p += stride;	q = p;
    }
    /* now in the other direction */
    p = out + istart;  /* first point in cell */
    p2 = p;
    q = p;
    nc = 8;	/* the 8 columns */
    while (nc--) {
    /* get a point to bottom of cell if available */
    if (jx) { p = p - nx;
   	a2 = (Int)*p;	p += nx;
    	a1 = (Int)*p;	p += nx;
    	a2 = a2 + a1;
     } else {
    	a1 = (Int)*p;	p += nx;
    	a2 = a1 + a1;
     }
     /* proceed over 7 points */
     m = 7;
     while (m--) {
     acc = *p;	p += nx;  t1 = acc;  acc += a1;  a1 = t1;
     t2 = acc;  acc += a2;  a2 = t2;
     *q = (short) (acc>>2); q += nx; }
     /* for the last value, use a point to right of cell if available */
     if (jx < nymo ) {
     *q = (short) ((*p + a1 + a2)>>2);
     } else {
     *q = (short) ((a1 + a1 + a2)>>2);
     }
     p = p2 + 1;	q = p2 = p;
     }


    }
    break;
    }
 } }
 if (cell_malloc) free(cell_status);
 printf("bad cell count = %d\n", badcells);
 }
 return result_sym;
}
/*------------------------------------------------------------------------- */
Int ana_reorder(Int narg, Int ps[])/* reorder function */
/* the call is x = reorder(array, order)
   where array must be a 2-D array, returns the re-ordered result */
/* reordering is reversals and transposes of a 2-D array, there are
   8 ways to "flip" an array, the "order" ranges from 0-7 accordingly */
{
  Int  iorder, iq, result_sym, type, nx, ny, m, inc, dim[2];
  Byte   *p, *q, *ptr;

  if (int_arg_stat(ps[1], &iorder) != 1)
    return ANA_ERROR;
  /* get pointer to array, must be 2-D here */
  iq = ps[0];
  if (!symbolIsArray(iq))
    return cerror(NEED_ARR, iq);
  type = array_type(iq);

  if (array_num_dims(iq) != 2)
    return cerror(NEED_2D_ARR, iq);
  nx = array_dims(iq)[0];
  ny = array_dims(iq)[1];
  ptr = array_data(iq);
  if (iorder >= 4) {
    m = nx;
    nx = ny;
    ny = m;
  }
  dim[0] = nx;
  dim[1] = ny;
  result_sym = array_scratch(type, 2, dim); /* for the result */
  q = array_data(result_sym);
  p = (Byte *) ptr;
  if (iorder == 0)        /* no change, make a copy */
    bcopy(p, q, nx*ny*ana_type_size[type]);
  else {
    /* outer switch for type, inners for orientation */
    switch (type) {
      case ANA_BYTE:
	if (iorder < 4) {
	  register  Byte *pp, *qq;
	  register  Int  nn, mm, nxx, inc;

	  nxx = nx;
	  mm = ny;
	  qq = q;

	  switch (iorder) {
	    case 1:		/* reverse in x */
	      pp = p + nx;
	      inc = 2*nx;
	      while (mm--) {
		nn = nxx;
		while (nn) { 
		  *qq++ = *--pp;
		  nn--;
		}
		pp += inc;
	      }
	      break;
	    case 2:		/* just reverse in y */
	      pp = p + nx * ny - nx;
	      inc = -2*nx;
	      while (mm--) {
		nn = nxx;
		while (nn) {
		  *qq++ = *pp++;
		  nn--;
		}
		pp += inc;
	      }
	      break;
	    case 3:		/* reverse in x and y */
	      pp = p + nx*ny;
	      while (mm--) {
		nn = nxx;
		while (nn) {
		  *qq++ = *--pp;
		  nn--;
		}
	      }
	      break;
	  }
	} else {
	  register  Byte *pp, *qq;
	  register  Int  nn, mm, nyy, inc;

	  mm = ny;
	  qq = q;
	  switch (iorder) {
	    case 4:		/* transpose in x and y */
	      pp = p;
	      inc = -nx*ny + 1;
	      nyy = ny;
	      break;
	    case 5:		/* transpose plus reverse in y */
	      pp = p +nx*ny - ny;
	      inc = nx*ny + 1;
	      nyy = -ny;
	      break;
	    case 6:		/* transpose plus reverse in x */
	      pp = p + ny - 1;
	      inc = -nx*ny - 1;
	      nyy = ny;
	      break;
	    case 7:		/* transpose plus reverse in x,y */
	      pp = p + ny*nx - 1;
	      inc = nx*ny - 1;
	      nyy = -ny;
	      break;
	  } 
	  while (mm--) {
	    nn= nx;
	    while (nn) {
	      *qq++ = *pp;
	      pp += nyy;
	      nn--;
	    }
	    pp += inc;
	  }
	}
	break;
      case ANA_WORD:
	if (iorder < 4) {
	  register  short *pp, *qq;
	  register  Int  nn, mm, nxx, inc;

	  nxx = nx;
	  mm = ny;
	  qq = (short *) q;

	  switch (iorder) {
	    case 1:		/* reverse in x */
	      pp = (short *) p + nx;
	      inc = 2*nx;
	      while (mm--) {
		nn = nxx;
		while (nn) {
		  *qq++ = *--pp;
		  nn--;
		}
		pp += inc;
	      }
	      break;
	    case 2:		/* just reverse in y */
	      pp = (short *) p + nx * ny - nx;
	      inc = -2*nx;
	      while (mm--) {
		nn = nxx;
		while (nn) {
		  *qq++ = *pp++;
		  nn--;
		}
		pp += inc;
	      }
	      break;
	    case 3:		/* reverse in x and y */
	      pp = (short *) p + nx*ny;
	      while (mm--) {
		nn = nxx;
		while (nn) {
		  *qq++ = *--pp;
		  nn--;
		}
	      }
	      break;
	  } 
	} else {
	  register  short *pp, *qq;
	  register  Int  nyy, nn, mm;

	  switch (iorder) {
	    case 4:		/* transpose in x and y */
	      pp = (short *) p;
	      inc = -nx*ny + 1;
	      nyy = ny;
	      break;
	    case 5:		/* transpose plus reverse in y */
	      pp = (short *) p +nx*ny - ny;
	      inc = nx*ny + 1;
	      nyy = -ny;
	      break;
	    case 6:		/* transpose plus reverse in x */
	      pp = (short *) p + ny - 1;
	      inc = -nx*ny - 1;
	      nyy = ny;
	      break;
	    case 7:        /* transpose plus reverse in x,y */
	      pp = (short *) p + ny*nx - 1;
	      inc = nx*ny - 1;
	      nyy = -ny;
	      break;
	  }
	  qq = (short *) q;
	  mm = ny;
	  while (mm--) {
	    nn= nx;
	    while (nn) {
	      *qq++ = *pp;
	      pp += nyy;
	      nn--;
	    }	    
	    pp += inc;
	  }
	}
	break;
      case ANA_LONG:
	if (iorder < 4) {
	  register  Int *pp, *qq;
	  register  Int  nn, mm, nxx;

	  nxx = nx;
	  mm = ny;
	  qq = (Int *) q;

	  switch (iorder) {
	    case 1:		/* reverse in x */
	    {
	      register  Int *pp = (Int *) p + nx;
	      register  Int inc = 2*nx;

	      while (mm--) {
		nn = nxx;
		while (nn) {
		  *qq++ = *--pp;
		  nn--;
		}
		pp += inc;
	      }
	    }
	    break;
	    case 2:		/* just reverse in y */
	      pp = (Int *) p + nx * ny - nx;
	      inc = -2*nx;
	      while (mm--) {
		nn = nxx;
		while (nn) {
		  *qq++ = *pp++;
		  nn--;
		}
		pp += inc;
	      }
	      break;
	    case 3:		/* reverse in x and y */
	      pp = (Int *) p + nx*ny;
	      while (mm--) {
		nn = nxx;
		while (nn) {
		  *qq++ = *--pp;
		  nn--;
		}
	      }
	      break;
	  } 
	} else {
	  register  Int *pp, *qq;
	  register  Int  nn, mm, nyy;
	  
	  mm = ny;
	  qq = (Int *) q;
	  switch (iorder) {
	    case 4:		/* transpose in x and y */
	      pp = (Int *) p;
	      inc = -nx*ny + 1;
	      nyy = ny;
	      break;
	    case 5:		/* transpose plus reverse in y */
	      pp = (Int *) p +nx*ny - ny;
	      inc = nx*ny + 1;
	      nyy = -ny;
	      break;
	    case 6:		/* transpose plus reverse in x */
	      pp = (Int *) p + ny - 1;
	      inc = -nx*ny - 1;
	      nyy = ny;
	      break;
	    case 7:		/* transpose plus reverse in x,y */
	      pp = (Int *) p + ny*nx - 1;
	      inc = nx*ny - 1;
	      nyy = -ny;
	      break;
	  } 
	  while (mm--) {
	    nn= nx;
	    while (nn) {
	      *qq++ = *pp;
	      pp += nyy;
	      nn--;
	    }
	    pp += inc;
	  }
	}
	break;
      case ANA_FLOAT:
	if (iorder < 4) {
	  register  Float *pp, *qq;
	  register  Int  nn, mm, nxx;

	  nxx = nx;
	  mm = ny;
	  qq = (Float *) q;

	  switch (iorder) {
	    case 1:		/* reverse in x */
	      pp = (Float *) p + nx;
	      inc = 2*nx;
	      while (mm--) {
		nn = nxx;
		while (nn) {
		  *qq++ = *--pp;
		  nn--;
		}
		pp += inc;
	      }
	      break;
	    case 2:		/* just reverse in y */
	      pp = (Float *) p + nx * ny - nx;
	      inc = -2*nx;
	      while (mm--) {
		nn = nxx;
		while (nn) {
		  *qq++ = *pp++;
		  nn--;
		}
		pp += inc;
	      }
	      break;
	    case 3:		/* reverse in x and y */
	      pp = (Float *) p + nx*ny;
	      while (mm--) {
		nn = nxx;
		while (nn) {
		  *qq++ = *--pp;
		  nn--;
		}
	      }
	      break;
	  } 
	} else {
	  register  Float *pp, *qq;
	  register  Int  nn, mm, nyy;
	  
	  mm = ny;
	  qq = (Float *) q;
	  switch (iorder) {
	    case 4:		/* transpose in x and y */
	      pp = (Float *) p;
	      inc = -nx*ny + 1;
	      nyy = ny;
	      break;
	    case 5:		/* transpose plus reverse in y */
	      pp = (Float *) p +nx*ny - ny;
	      inc = nx*ny + 1;
	      nyy = -ny;
	      break;
	    case 6:		/* transpose plus reverse in x */
	      pp = (Float *) p + ny - 1;
	      inc = -nx*ny - 1;
	      nyy = ny;
	      break;
	    case 7:		/* transpose plus reverse in x,y */
	      pp = (Float *) p + ny*nx - 1;
	      inc = nx*ny - 1;
	      nyy = -ny;
	      break;
	  } 
	  while (mm--) {
	    nn= nx;
	    while (nn) {
	      *qq++ = *pp;
	      pp += nyy;
	      nn--;
	    }
	    pp += inc;
	  }
	}
	break;
      case ANA_DOUBLE:
	if (iorder < 4) {
	  register  Double *pp, *qq;
	  register  Int  nn, mm, nxx;

	  nxx = nx;
	  mm = ny;
	  qq = (Double *) q;

	  switch (iorder) {
	    case 1:		/* reverse in x */
	      pp = (Double *) p + nx;
	      inc = 2*nx;
	      while (mm--) {
		nn = nxx;
		while (nn) {
		  *qq++ = *--pp;
		  nn--;
		}
		pp += inc;
	      }
	      break;
	    case 2:		/* just reverse in y */
	      pp = (Double *) p + nx * ny - nx;
	      inc = -2*nx;
	      while (mm--) {
		nn = nxx;
		while (nn) {
		  *qq++ = *pp++;
		  nn--;
		}
		pp += inc;
	      }
	      break;
	    case 3:		/* reverse in x and y */
	      pp = (Double *) p + nx*ny;
	      while (mm--) {
		nn = nxx;
		while (nn) {
		  *qq++ = *--pp;
		  nn--;
		}
	      }
	      break;
	  } 
	} else {
	  register  Double *pp, *qq;
	  register  Int  nn, mm, nyy;
	  
	  mm = ny;
	  qq = (Double *) q;
	  switch (iorder) {
	    case 4:		/* transpose in x and y */
	      pp = (Double *) p;
	      inc = -nx*ny + 1;
	      nyy = ny;
	      break;
	    case 5:		/* transpose plus reverse in y */
	      pp = (Double *) p +nx*ny - ny;
	      inc = nx*ny + 1;
	      nyy = -ny;
	      break;
	    case 6:		/* transpose plus reverse in x */
	      pp = (Double *) p + ny - 1;
	      inc = -nx*ny - 1;
	      nyy = ny;
	      break;
	    case 7:		/* transpose plus reverse in x,y */
	      pp = (Double *) p + ny*nx - 1;
	      inc = nx*ny - 1;
	      nyy = -ny;
	      break;
	  } 
	  while (mm--) {
	    nn= nx;
	    while (nn) {
	      *qq++ = *pp;
	      pp += nyy;
	      nn--;
	    }
	    pp += inc;
	  }
	}
	break;
    } 
  }
  return result_sym;
}
/*------------------------------------------------------------------------- */
