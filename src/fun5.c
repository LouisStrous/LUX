/* internal ana subroutines and functions */
/* mostly fft related stuff and solar mappings */
 /*this is file fun5.c */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <math.h>
#include <string.h>
#include "install.h"
#include "action.h"
static char rcsid[] __attribute__ ((unused)) =
"$Id: fun5.c,v 4.0 2001/02/07 20:37:01 strous Exp $";

static Double	subdx, subdy;
static Double	xoff, yoff;
static Double	a1, a2, a3,a4,a5, a6, a7, a8, a9 ,a10, a11, a12, a13, a14, a15;
extern Int	badmatch;
Double	meritc;
/*------------------------------------------------------------------------- */
Int ana_subshift(Int narg, Int ps[]) /* LCT for a cell  */
     /* wants 2 arrays, already F*8 and extracted, both the same size */
     /* returns the shift */
{
  Int		iq, jq, *d;
  Double	*x1, *x2;
  void	subshift(Double *, Double *, Int, Int);

  iq = ps[0];
  if (symbol_class(iq) != ANA_ARRAY
      || array_num_dims(iq) != 2)
    return cerror(NEED_2D_ARR, iq);
  iq = ana_double(1, &iq);
  x1 = (Double *) array_data(iq);
  d = array_dims(iq);
  jq = ps[1];
  if (symbol_class(jq) != ANA_ARRAY
      || array_num_dims(jq) != 2)
    return cerror(NEED_2D_ARR, jq);
  if (d[0] != array_dims(jq)[0]
      || d[1] != array_dims(jq)[1])
    return cerror(INCMP_DIMS, jq);
  jq = ana_double(1, &jq);
  x2 = (Double *) array_data(jq);
  
  subshift(x1, x2, d[0], d[1]);

  /* expect result in xoff and yoff */
  if (redef_scalar(ps[2], ANA_DOUBLE, &xoff) != 1)
    return -1;
  if (redef_scalar(ps[3], ANA_DOUBLE, &yoff) != 1)
    return -1;
  return 1;
}
/*------------------------------------------------------------------------- */
Int ana_subshiftc(Int narg, Int ps[]) /* LCT for a cell, sym version */
 /* subshiftc, s1b, s2b, xoff, yoff, mask  */
 /* 3/4/97 added apodizer array, optional */
 /* wants 2 or 3 arrays, already F*8 and extracted, both the same size */
 /* returns the shift */
{
  Int	iq, jq, kq;
  Double	*x1, *x2, *msk;
  Int	nx, ny;
  Double	subshiftc(Double *, Double *, Int, Int),
    subshiftc_apod(Double *, Double *, Double *, Int, Int);

  iq = ps[0];
  jq = ps[1];
  if (!symbolIsNumericalArray(iq) || array_num_dims(iq) != 2)
    return cerror(NEED_2D_ARR, iq);
  if (!symbolIsNumericalArray(jq) || array_num_dims(jq) != 2)
    return cerror(NEED_NUM_ARR, jq);
  nx = array_dims(iq)[0];	ny = array_dims(iq)[1];
  if (array_dims(jq)[0] != nx || array_dims(jq)[1] != ny)
    return cerror(INCMP_ARG, jq);
 /* if a mask, also some rules */
  if (narg > 4) {
    kq = ps[4];
    if (!symbolIsNumericalArray(kq) || array_num_dims(kq) != 2)
      return cerror(NEED_2D_ARR, kq);
    if (array_dims(kq)[0] != nx - 1 || array_dims(kq)[1] != ny - 1)
      return cerror(INCMP_ARG, kq);
    kq = ana_double(1, &kq);
    msk = array_data(kq);
  }
  iq = ana_double(1, &iq);
  jq = ana_double(1, &jq);
  x1 = array_data(iq);
  x2 = array_data(jq);

  switch (narg) {
    case 4:
      meritc = subshiftc(x1, x2, nx, ny);
      break;
    case 5:
      meritc = subshiftc_apod(x1, x2, msk, nx, ny);
      break;
  }
  /* expect result in xoff and yoff */
  if (redef_scalar( ps[2], 4, &xoff) != ANA_OK)
    return ANA_ERROR;
  if (redef_scalar( ps[3], 4, &yoff) != ANA_OK)
    return ANA_ERROR;
  return ANA_OK;
}
/*------------------------------------------------------------------------- */
Double mert(Double sx, Double sy)
 {
 Double	w0, w1,w2,w3,mq;
 w0 = (1-sx)*(1-sy);
 w1 = sx*(1-sy);
 w2 = sy*(1-sx);
 w3 = sx*sy;
 mq = w0*w0*a1 + w1*w1*a2 + w2*w2*a3 + w3*w3*a4 + w0*w1*a5 + w0*w2*a6
 	+ w0*w3*a7 + w1*w2*a8 + w1*w3*a9 + w2*w3*a10 + w0*a11 + w1*a12
	+ w2*a13 + w3*a14 + a15;
 return mq;
 }
 /*------------------------------------------------------------------------- */
Double mertc(sx,sy)
 Double sx,sy;
 {
 Double	xq;
 xq = a1 + sx*sx*a2 + sy*sy*a3 + sx*sx*sy*sy*a4 + sx*a5 + sy*a6 + sx*sy*a7 + sx*sx*sy*a9 + sx*sy*sy*a10;
 return xq;
 }
 /*------------------------------------------------------------------------- */
Double sxvalue(Double sy)
 {
 Double	syc, c1, c2, xq;
 syc = 1 - sy;
 c1 = syc*syc*(a5-2.0*a1) +syc*sy*(a7-2.0*a6+a8) +sy*sy*(a10-2.0*a3) +syc*(a12-a11) +sy*(a14-a13);
 c2 = syc*syc*(a1+a2-a5) +sy*sy*(a3+a4-a10) +syc*sy*(a6-a7-a8+a9);
 if (c2 == 0.0) {
 /* no longer print this, it happens for 0 counts, instead bump up badmatch */
 /* printf("sxvalue singular, c1, c2 = %g %g\n", c1,c2);*/
 badmatch++;
 return -1.0;}
 if ( isnan(c1) == 1 || isnan(c2) ==1 ) {
 printf("sxvalue problem:\n");
 printf("sy, syc = %14.6e %14.6e\n", sy,syc);
 printf("a1,a2,a3,a4 = %14.6e %14.6e %14.6e %14.6e\n",a1,a2,a3,a4);
 printf("a5,a6,a7 = %14.6e %14.6e %14.6e \n",a5,a6,a7);
 printf("a8,a9,a10,a11 = %14.6e %14.6e %14.6e %14.6e\n",a8,a9,a10,a11);
 printf("a12,a13,a14,a15 = %14.6e %14.6e %14.6e %14.6e\n",a12,a13,a14,a15);
 return -1.0;
 }
 xq = -.5*c1/c2;
 return xq;
 }
 /*------------------------------------------------------------------------- */
Double syvalue(Double sx)
 {
 Double	sxc, c1, c2, xq;
 sxc = 1 - sx;
 c1 = sxc*sxc*(a6-2.0*a1) +sx*sx*(a9-2.0*a2) +sxc*sx*(a7-2.0*a5+a8) +sxc*(a13-a11) +sx*(a14-a12);
 c2 =  sxc*sxc*(a1+a3-a6) +sx*sx*(a2+a4-a9) +sxc*sx*(a5-a7-a8+a10);
 if (c2 == 0.0) {
 /* no longer print this, it happens for 0 counts, instead bump up badmatch */
 /* printf("syvalue singular, c1, c2 = %g %g\n",c1,c2); */
 badmatch++;
 return -1.0;}
 if ( isnan(c1) == 1 || isnan(c2) ==1 ) {
 printf("syvalue problem:\n");
 printf("sx, sxc = %14.6e %14.6e\n", sx,sxc);
 printf("a1,a2,a3,a4 = %14.6e %14.6e %14.6e %14.6e\n",a1,a2,a3,a4);
 printf("a5,a6,a7 = %14.6e %14.6e %14.6e \n",a5,a6,a7);
 printf("a8,a9,a10,a11 = %14.6e %14.6e %14.6e %14.6e\n",a8,a9,a10,a11);
 printf("a12,a13,a14,a15 = %14.6e %14.6e %14.6e %14.6e\n",a12,a13,a14,a15);
 return -1.0;
 }
 xq = -.5*c1/c2;
 return xq;
 }
 /*------------------------------------------------------------------------- */
void getsxsy(void)
 {
 /* iterate to the min (if any), loading results in globals subdx and subdy */
 /* seed with syz = 0.5 */
 Int	n;
 Double	sxz, syz;
 syz=sxz=.5;
 n = 11;
 while (n--) {
 /* get new sxz */
 sxz = sxvalue(syz);
 if ( sxz < -0.5 || sxz > 1.5) { subdx = subdy = 0.0;  return; }
 /* and get a new syz */
 syz = syvalue(sxz);
 if ( syz < -0.5 || syz > 1.5) { subdx = subdy = 0.0;  return; }
 /* printf("      sxz, syz = %g %g\n", sxz,syz); */
 
 }
 /* for small shifts, let values very close to the line but on the
 other side remain as contenders */
 /* 4/30/96, try another scheme, let them have a value along the line
 and then we will compare on merit, this will allow 0's to be a result
 when the merit is really at a min along the line */
 if (sxz >= 0.0 && sxz <=1.0 && syz >= 0.0 && syz <=1.0 ) {
 /* these OK */
 subdx = sxz;	subdy = syz; } else {
 /* maybe OK to use a zero or even a 1 if rather cloe to the line */
 if (sxz >= -0.1 && sxz <=1.1 && syz >= -0.1 && syz <=1.1 ) {
 /* just range limit them, this should sort it out */
 subdx = MAX(subdx, 0.0);	subdy = MAX(subdy, 0.0);
 subdx = MIN(subdx, 1.0);	subdy = MIN(subdy, 1.0);
 } else { subdx =0;	subdy =0; } }
 /*printf("final sxz, syz = %g %g\n", sxz,syz);*/
 return;
 }
 /*------------------------------------------------------------------------- */
void  subshift(x, r, nx, ny)
 /* q is the reference cell (old m1), r is the one we interpolate (old m2) */
 /* we assume that the arrays have already been converted to F*8 */
 Double	*r, *x;
 Int     nx, ny;
 {
 Int     nxs;
 Double  sum, cs0, cs1, cs2, cs3, t2, t1, t0, t3;
 Double  parts[5][5], xx[3][3], xdx[3][2], xdy[2][3], xmmpp[2][2], xppmm[2][2]; 
 Double  partsdx[5][3], partsdy[3][5], partsppmm[3][3], partsmmpp[3][3];
 Double  cmm,c0m,cpm,cm0,c00,cp0,cmp,c0p,cpp,sumxx;
 Double	 qbest, qcur, outside, qd;
 Int     i, j, nxm2, nym2, ii, jj, mflag;
 Double	*rp, *rp2, *row, *rowq, *qp;

 nxs = nx;
 nxm2 = nx - 2;
 nym2 = ny - 2;
  
 /* the sums of squares */
 /* first rearrange as 25 pieces in an array of 5x5, mostly to have
 it semi organized, may be faster to do it differently */
 rp = r;	t0 = *rp++;
 parts[0][0] = t0*t0;
 t0 = *rp++;
 parts[0][1] = t0*t0;
 i = nx-4;	sum = 0.0;
 while (i--) {   t0 = *rp++;  sum += t0 * t0; }
 parts[0][2] = sum;
 t0 = *rp++;
 parts[0][3] = t0*t0;
 t0 = *rp++;
 parts[0][4] = t0*t0;
 rp = r+nxs;	t0 = *rp++;
 parts[1][0] = t0*t0;
 t0 = *rp++;
 parts[1][1] = t0*t0;
 i = nx-4;	sum = 0.0;
 while (i--) {   t0 = *rp++;  sum += t0 * t0; }
 parts[1][2] = sum;
 t0 = *rp++;
 parts[1][3] = t0*t0;
 t0 = *rp++;
 parts[1][4] = t0*t0;
 /* third row has column sums and center sum */
 row = r + nxs+ nxs;
 j = ny-4;	cs0 = cs1 = cs2 = cs3 = sum = 0.0;
 while (j--) {
 rp = row;	t0 = *rp++;
 row = row + nxs;
 cs0 += t0 * t0;
 t0 = *rp++;
 cs1 += t0 * t0;
 i = nx-4;
 while (i--) {   t0 = *rp++;  sum += t0 * t0; }
 t0 = *rp++;
 cs2 += t0 * t0;
 t0 = *rp++;
 cs3 += t0 * t0;
 }
 parts[2][0] = cs0;
 parts[2][1] = cs1;
 parts[2][2] = sum;
 parts[2][3] = cs2;
 parts[2][4] = cs3;
 rp = row;	t0 = *rp++;
 parts[3][0] = t0*t0;
 t0 = *rp++;
 parts[3][1] = t0*t0;
 i = nx-4;	sum = 0.0;
 while (i--) {   t0 = *rp++;  sum += t0 * t0; }
 parts[3][2] = sum;
 t0 = *rp++;
 parts[3][3] = t0*t0;
 t0 = *rp++;
 parts[3][4] = t0*t0;
 rp = row+nxs;	t0 = *rp++;
 parts[4][0] = t0*t0;
 t0 = *rp++;
 parts[4][1] = t0*t0;
 i = nx-4;	sum = 0.0;
 while (i--) {   t0 = *rp++;  sum += t0 * t0; }
 parts[4][2] = sum;
 t0 = *rp++;
 parts[4][3] = t0*t0;
 t0 = *rp++;
 parts[4][4] = t0*t0;

 /* now each sum of squares is a sum of 9 parts */
 for (j=0;j<3;j++) { for (i=0;i<3;i++) {
	xx[j][i] = 0.0;
 	for (jj=0;jj<3;jj++) { for (ii=0;ii<3;ii++) {
	xx[j][i] += parts[j+jj][i+ii]; }}}}

 /* the cross term cases with delta 1 in the x direction */
 /* we build 15 sums which are combined into 6 results */
 rp = r;	t0 = *rp++;	t1 = *rp++;
 partsdx[0][0] = t0 * t1;
 i = nx-3;	sum = 0.0;
 while (i--) {  t0 = t1; t1 = *rp++;  sum += t0 * t1; }
 partsdx[0][1] = sum;
 /* one more for the top row */
 t0 = t1; t1 = *rp++;
 partsdx[0][2] = t0 * t1;
 /* second row much the same */
 rp = r+nxs;	t0 = *rp++;	t1 = *rp++;
 partsdx[1][0] = t0 * t1;
 i = nx-3;	sum = 0.0;
 while (i--) {  t0 = t1; t1 = *rp++;  sum += t0 * t1; }
 partsdx[1][1] = sum;
 t0 = t1; t1 = *rp++;
 partsdx[1][2] = t0 * t1;
 /* third row has larger areas, 2 columns and the center */
 row = r + nxs + nxs;
 j = ny-4;	cs0 = cs1 = sum = 0.0;
 while (j--) {
 rp = row;	t0 = *rp++;	t1 = *rp++;
 row = row + nxs;
 cs0 += t0 * t1;	/* left column sum */
 i = nx-3;
 while (i--) {  t0 = t1; t1 = *rp++;  sum += t0 * t1; }
 t0 = t1; t1 = *rp++;
 cs1 += t0 * t1;
 }
 partsdx[2][0] = cs0;
 partsdx[2][1] = sum;
 partsdx[2][2] = cs1;
 /* the last 2 rows are just one high again */
 rp = row;	t0 = *rp++;	t1 = *rp++;
 partsdx[3][0] = t0 * t1;
 i = nx-3;	sum = 0.0;
 while (i--) {  t0 = t1; t1 = *rp++;  sum += t0 * t1; }
 partsdx[3][1] = sum;
 /* one more for the top row */
 t0 = t1; t1 = *rp++;
 partsdx[3][2] = t0 * t1;
 /* second row much the same */
 rp = row+nxs;	t0 = *rp++;	t1 = *rp++;
 partsdx[4][0] = t0 * t1;
 i = nx-3;	sum = 0.0;
 while (i--) {  t0 = t1; t1 = *rp++;  sum += t0 * t1; }
 partsdx[4][1] = sum;
 t0 = t1; t1 = *rp++;
 partsdx[4][2] = t0 * t1;
 /* now each sum of cross terms is a sum of 9 parts */
 for (j=0;j<3;j++) { for (i=0;i<2;i++) {
	xdx[j][i] = 0.0;
 	for (jj=0;jj<3;jj++) { for (ii=0;ii<2;ii++) {
	xdx[j][i] += partsdx[j+jj][i+ii]; }}}}

 /* the terms with a simple shift in y are similar in concept */
 /* we build 15 sums which are combined into 6 results */
 rp = r;	rp2 = rp+nxs;
 partsdy[0][0] = *rp++ * *rp2++;
 partsdy[0][1] = *rp++ * *rp2++;

 i = nx-4;	sum = 0.0;
 while (i--) {  sum += *rp++ * *rp2++; }
 partsdy[0][2] = sum;
 partsdy[0][3] = *rp++ * *rp2++;
 partsdy[0][4] = *rp++ * *rp2++;
 /* second row has column sums and center sum */
 row = r + nxs;
 j = ny-3;	cs0 = cs1 = cs2 = cs3 = sum = 0.0;
 while (j--) {
 rp = row;	rp2 = rp+nxs;
 row = row + nxs;
 cs0 += *rp++ * *rp2++;	/* left column sum */
 cs1 += *rp++ * *rp2++;	/* second column sum */
 i = nx-4;
 while (i--) {  sum += *rp++ * *rp2++; } /* center */
 cs2 += *rp++ * *rp2++;	/* second last column sum */
 cs3 += *rp++ * *rp2++;	/* last column sum */
 }
 partsdy[1][0] = cs0;
 partsdy[1][1] = cs1;
 partsdy[1][2] = sum;
 partsdy[1][3] = cs2;
 partsdy[1][4] = cs3;
 /* the last row */
 rp = row;	rp2 = rp+nxs;
 partsdy[2][0] = *rp++ * *rp2++;
 partsdy[2][1] = *rp++ * *rp2++;
 i = nx-4;	sum = 0.0;
 while (i--) {  sum += *rp++ * *rp2++; }
 partsdy[2][2] = sum;
 partsdy[2][3] = *rp++ * *rp2++;
 partsdy[2][4] = *rp++ * *rp2++;
 /* now each sum of cross terms is a sum of 9 parts */
 for (j=0;j<2;j++) { for (i=0;i<3;i++) {
	xdy[j][i] = 0.0;
 	for (jj=0;jj<2;jj++) { for (ii=0;ii<3;ii++) {
	xdy[j][i] += partsdy[j+jj][i+ii]; }}}}
 /* diagonal terms, 2 types, first the larger gap, mmpp type */
 /* we build 9 sums which are combined into 4 results */
 rp = r;	rp2 = rp+nxs+1;
 partsmmpp[0][0] = *rp++ * *rp2++;
 i = nx-3;	sum = 0.0;
 while (i--) {  sum += *rp++ * *rp2++; }
 partsmmpp[0][1] = sum;
 partsmmpp[0][2] = *rp++ * *rp2++;
 row = r + nxs;
 j=ny-3;	cs0 = cs1 = sum = 0.0;
 while (j--) {
 rp = row;	rp2 = rp+nxs+1;
 row = row + nxs;
 cs0 += *rp++ * *rp2++;	/* left column sum */
 i = nx-3;
 while (i--) {  sum += *rp++ * *rp2++; }
 cs1 += *rp++ * *rp2++;	/* right column sum */
 }
 partsmmpp[1][0] = cs0;
 partsmmpp[1][1] = sum;
 partsmmpp[1][2] = cs1;
 rp = row;	rp2 = rp+nxs+1;
 partsmmpp[2][0] = *rp++ * *rp2++;
 i = nx-3;	sum = 0.0;
 while (i--) {  sum += *rp++ * *rp2++; }
 partsmmpp[2][1] = sum;
 partsmmpp[2][2] = *rp++ * *rp2++;
 /* now each sum of cross terms is a sum of 4 parts */
 for (j=0;j<2;j++) { for (i=0;i<2;i++) {
	xmmpp[j][i] = 0.0;
 	for (jj=0;jj<2;jj++) { for (ii=0;ii<2;ii++) {
	xmmpp[j][i] += partsmmpp[j+jj][i+ii]; }}}}

 /* diagonal terms, 2 types, first the larger gap, mmpp type */
 /* we build 9 sums which are combined into 4 results */
 rp = r+1;	rp2 = rp+nxs-1;
 partsppmm[0][0] = *rp++ * *rp2++;
 i = nx-3;	sum = 0.0;
 while (i--) {  sum += *rp++ * *rp2++; }
 partsppmm[0][1] = sum;
 partsppmm[0][2] = *rp++ * *rp2++;
 row = r + nxs+1;
 j=ny-3;	cs0 = cs1 = sum = 0.0;
 while (j--) {
 rp = row;	rp2 = rp+nxs-1;
 row = row + nxs;
 cs0 += *rp++ * *rp2++;	/* left column sum */
 i = nx-3;
 while (i--) {  sum += *rp++ * *rp2++; }
 cs1 += *rp++ * *rp2++;	/* right column sum */
 }
 partsppmm[1][0] = cs0;
 partsppmm[1][1] = sum;
 partsppmm[1][2] = cs1;
 rp = row;	rp2 = rp+nxs-1;
 partsppmm[2][0] = *rp++ * *rp2++;
 i = nx-3;	sum = 0.0;
 while (i--) {  sum += *rp++ * *rp2++; }
 partsppmm[2][1] = sum;
 partsppmm[2][2] = *rp++ * *rp2++;
 /* now each sum of cross terms is a sum of 4 parts */
 for (j=0;j<2;j++) { for (i=0;i<2;i++) {
	xppmm[j][i] = 0.0;
 	for (jj=0;jj<2;jj++) { for (ii=0;ii<2;ii++) {
	xppmm[j][i] += partsppmm[j+jj][i+ii]; }}}}

 /* next the cases with values from both images */
 cmm=c0m=cpm=cm0=c00=cp0=cmp=c0p=cpp=sumxx = 0.0;
 nym2 = ny-2;	j = nym2;	nxm2 = nx-2;
 row = r;
 rowq = x+1+nxs;
 while (j--) {
 rp = row;
 rp2 = rp + 1;
 qp = rowq;
 row += nxs;	rowq += nxs;
 i = nxm2;
 t2 = *rp++;
 t3 = *rp++;
 while (i--) {
 t0 = *qp++;
 t1 = t2;	t2 = t3;
 t3 = *rp++;
 sumxx += t0 * t0;
 cmm += t0 * t1;
 c0m += t0 * t2;
 cpm += t0 * t3;
 }
 }

 row = r+nxs;
 rowq = x+1+nxs;
 j = nym2;
 while (j--) {
 rp = row;
 rp2 = rp + 1;
 qp = rowq;
 row += nxs;	rowq += nxs;
 i = nxm2;
 t2 = *rp++;
 t3 = *rp++;
 while (i--) {
 t0 = *qp++;
 t1 = t2;	t2 = t3;
 t3 = *rp++;
 cm0 += t0 * t1;
 c00 += t0 * t2;
 cp0 += t0 * t3;
 }
 }

 row = r+nxs+nxs;
 rowq = x+1+nxs;
 j = nym2;
 while (j--) {
 rp = row;
 rp2 = rp + 1;
 qp = rowq;
 row += nxs;	rowq += nxs;
 i = nxm2;
 t2 = *rp++;
 t3 = *rp++;
 while (i--) {
 t0 = *qp++;
 t1 = t2;	t2 = t3;
 t3 = *rp++;
 cmp += t0 * t1;
 c0p += t0 * t2;
 cpp += t0 * t3;
 }
 }
 /* have all the terms, now collect ones needed for quad 1 */
 a1 = xx[1][1];  a4 = xx[0][0];  a2 = xx[1][0];  a3 = xx[0][1];
 a5 = 2.0*xdx[1][0];	 a10 = 2.0*xdx[0][0];
 a9 = 2.0*xdy[0][0];	 a6 = 2.0*xdy[0][1];
 a7 = 2.0*xmmpp[0][0];	 a8 = 2.0*xppmm[0][0];
 a15 = sumxx;	a11 = -2.*c00;  a12 = -2.*cm0;  a13 = -2.*c0m;  a14 = -2.*cmm;
 /*
 printf("QUAD 1\n");
 printf("a1,a2,a3,a4 = %14.6e %14.6e %14.6e %14.6e\n",a1,a2,a3,a4);
 printf("a5,a6,a7 = %14.6e %14.6e %14.6e \n",a5,a6,a7);
 printf("a8,a9,a10,a11 = %14.6e %14.6e %14.6e %14.6e\n",a8,a9,a10,a11);
 printf("a12,a13,a14,a15 = %14.6e %14.6e %14.6e %14.6e\n",a12,a13,a14,a15);
 */

 mflag = 0;	/* counts the number of MINs */
 qbest = 0.0;	/* the best */
 outside = 0.0;
 xoff = yoff = 0.0;
 getsxsy();
 /* check if we got a MIN, only if somebody not zero */
 if (subdx != 0 || subdy != 0) { xoff = - subdx;  yoff = -subdy;
 	qbest = mert(subdx, subdy); mflag += 1;
	if (subdx<0) outside = -subdx;
		else if (subdx>1.0) outside = subdx-1.0;
	if (subdy<0) outside = MAX(-subdy, outside);
		else if (subdy>1.0) outside = MAX(outside, subdy-1.0);
	}

 a4 = xx[0][2];  a2 = xx[1][2];
 a5 = 2.0*xdx[1][1];	 a10 = 2.0*xdx[0][1];
 a9 = 2.0*xdy[0][2];
 a7 = 2.0*xppmm[0][1];	 a8 = 2.0*xmmpp[0][1];
 a12 = -2.*cp0;  a14 = -2.*cpm;
 /*
 printf("QUAD 2\n");
 printf("a1,a2,a3,a4 = %14.6e %14.6e %14.6e %14.6e\n",a1,a2,a3,a4);
 printf("a5,a6,a7 = %14.6e %14.6e %14.6e \n",a5,a6,a7);
 printf("a8,a9,a10,a11 = %14.6e %14.6e %14.6e %14.6e\n",a8,a9,a10,a11);
 printf("a12,a13,a14,a15 = %14.6e %14.6e %14.6e %14.6e\n",a12,a13,a14,a15);
 */

 getsxsy();
 /* check if we got a MIN, only if somebody not zero */
 if (subdx != 0 || subdy != 0) {
 	qcur = mert(subdx, subdy);
	/* find our outside status */
	qd = 0.0;
	if (subdx<0) qd = -subdx;
		else if (subdx>1.0) qd = subdx-1.0;
	if (subdy<0) qd = MAX(-subdy, qd);
		else if (subdy>1.0) qd = MAX(qd, subdy-1.0);
	if ( mflag == 0 )
	 { xoff =  subdx;  yoff = -subdy; qbest = qcur; outside = qd;
	 } else {
	 /* we have competition */
	 if ( outside != 0.0) {
	 /* we are a contender, we decide on the outside issue if the previous
	 was outside without reference to merit */
	  if (qd < outside) { outside = qd;
	  	xoff =  subdx;  yoff = -subdy; qbest = qcur;}
	  } else {
	 /* the competition has an outside of zip, if we also have zip
	 then we compare on merit, if we have an outside problem here
	 then the previous guy gets the honors */
	 if (qd == 0.0 && (qbest > qcur))
	  	{xoff =  subdx;  yoff = -subdy; qbest = qcur;}}
	 }
	 mflag += 1;
	 }

 a4 = xx[2][2];  a3 = xx[2][1];
 a10 = 2.0*xdx[2][1];
 a9 = 2.0*xdy[1][2];	 a6 = 2.0*xdy[1][1];
 a7 = 2.0*xmmpp[1][1];	 a8 = 2.0*xppmm[1][1];
 a13 = -2.*c0p;  a14 = -2.*cpp;
 /*
 printf("QUAD 4\n");
 printf("a1,a2,a3,a4 = %14.6e %14.6e %14.6e %14.6e\n",a1,a2,a3,a4);
 printf("a5,a6,a7 = %14.6e %14.6e %14.6e \n",a5,a6,a7);
 printf("a8,a9,a10,a11 = %14.6e %14.6e %14.6e %14.6e\n",a8,a9,a10,a11);
 printf("a12,a13,a14,a15 = %14.6e %14.6e %14.6e %14.6e\n",a12,a13,a14,a15);
 */

 getsxsy();
 /* check if we got a MIN, only if somebody not zero */
 if (subdx != 0 || subdy != 0) {
 	qcur = mert(subdx, subdy);
	/* find our outside status */
	qd = 0.0;
	if (subdx<0) qd = -subdx;
		else if (subdx>1.0) qd = subdx-1.0;
	if (subdy<0) qd = MAX(-subdy, qd);
		else if (subdy>1.0) qd = MAX(subdy-1.0, qd);
	if ( mflag == 0 )
	 { xoff =  subdx;  yoff = subdy; qbest = qcur; outside = qd;
	 } else {
	 /* we have competition */
	 if ( outside != 0.0) {
	 /* we are a contender, we decide on the outside issue if the previous
	 was outside without reference to merit */
	  if (qd < outside) { outside = qd;
	  	xoff =  subdx;  yoff = subdy; qbest = qcur;}
	  } else {
	 /* the competition has an outside of zip, if we also have zip
	 then we compare on merit, if we have an outside problem here
	 then the previous guy gets the honors */
	 if (qd == 0.0 && (qbest > qcur))
	    {xoff =  subdx;  yoff = subdy; qbest = qcur;}}
	 }
	 mflag += 1;
	}


 a4 = xx[2][0];  a2 = xx[1][0];
 a5 = 2.0*xdx[1][0];	 a10 = 2.0*xdx[2][0];
 a9 = 2.0*xdy[1][0];
 a7 = 2.0*xppmm[1][0];	 a8 = 2.0*xmmpp[1][0];
 a12 = -2.*cm0;  a14 = -2.*cmp;
 /*
 printf("QUAD 3\n");
 printf("a1,a2,a3,a4 = %14.6e %14.6e %14.6e %14.6e\n",a1,a2,a3,a4);
 printf("a5,a6,a7 = %14.6e %14.6e %14.6e \n",a5,a6,a7);
 printf("a8,a9,a10,a11 = %14.6e %14.6e %14.6e %14.6e\n",a8,a9,a10,a11);
 printf("a12,a13,a14,a15 = %14.6e %14.6e %14.6e %14.6e\n",a12,a13,a14,a15);
 */

 getsxsy();
 /* check if we got a MIN, only if somebody not zero */
 if (subdx != 0 || subdy != 0) {
    qcur = mert(subdx, subdy);
    /* find our outside status */
    qd = 0.0;
    if (subdx<0) qd = -subdx;
    	else if (subdx>1.0) qd = subdx-1.0;
    if (subdy<0) qd = MAX(-subdy, qd);
    	else if (subdy>1.0) qd = MAX(qd, subdy-1.0);
    if ( mflag == 0 )
     { xoff =  -subdx;  yoff = subdy; qbest = qcur; outside = qd;
     } else {
     /* we have competition */
     if ( outside != 0.0) {
     /* we are a contender, we decide on the outside issue if the previous
     was outside without reference to merit */
      if (qd < outside) { outside = qd;
	    xoff =  -subdx;  yoff = subdy; qbest = qcur;}
      } else {
     /* the competition has an outside of zip, if we also have zip
     then we compare on merit, if we have an outside problem here
     then the previous guy gets the honors */
     if (qd == 0.0 && (qbest > qcur))
	    {xoff =  -subdx;  yoff = subdy; qbest = qcur;}}
     }
     mflag += 1;
    }
 /* mflag contains the number of MINs in case we want to document that or
 use the no MIN condition to search further */
 if (mflag == 0) { xoff = yoff = 0.0; }  /* zero these for no MIN case */

 return;
 }
 /*------------------------------------------------------------------------- */
Double sxvaluec(Double sy)
 {
 Double c1, c2, xq;
 /* 3/4/97 changed a7+a8 to just a7 */
 c1 = a5 + sy*a7 + sy*sy*a10;
 c2 =  a2 + sy*sy*a4 +sy*a9;
 if (c2 == 0.0) {
 /* bump badmatch instead of printing */
 /* printf("sxvaluec singular, c1, c2 = %g %g\n", c1,c2); */
 badmatch++;
 return 0.0;}
 if ( isnan(c1) == 1 || isnan(c2) ==1 ) {
 printf("sxvaluec problem:\n");
 printf("sy = %14.6e %14.6e\n", sy);
 printf("a1,a2,a3,a4 = %14.6e %14.6e %14.6e %14.6e\n",a1,a2,a3,a4);
 printf("a5,a6,a7 = %14.6e %14.6e %14.6e \n",a5,a6,a7);
 printf("a8,a9,a10 = %14.6e %14.6e %14.6e\n",a8,a9,a10);
 return 0.0;
 }
 xq = -.5*c1/c2;
 return xq;
 }
 /*------------------------------------------------------------------------- */
Double syvaluec(Double sx)
 {
 Double	c1, c2, xq;
 /* 3/4/97 changed a7+a8 to just a7 */
 c1 = a6 + sx*a7 + sx*sx*a9;
 c2 =  a3 + sx*sx*a4 +sx*a10;
 if (c2 == 0.0) {
 /* bump badmatch instead of printing */
 /* printf("syvalue singular, c1, c2 = %g %g\n",c1,c2); */
 badmatch++;
 return 0.0;}
 if ( isnan(c1) == 1 || isnan(c2) ==1 ) {
 printf("syvaluec problem:\n");
 printf("sx = %14.6e %14.6e\n", sx);
 printf("a1,a2,a3,a4 = %14.6e %14.6e %14.6e %14.6e\n",a1,a2,a3,a4);
 printf("a5,a6,a7 = %14.6e %14.6e %14.6e \n",a5,a6,a7);
 printf("a8,a9,a10 = %14.6e %14.6e %14.6e\n",a8,a9,a10);
 return 0.0;
 }
 xq = -.5*c1/c2;
 return xq;
 }
 /*------------------------------------------------------------------------- */
Double  subshiftc(xa, xb, nx, ny)
 /* we assume that the arrays have already been converted to F*8 */
 Double	*xa, *xb;
 Int     nx, ny;
 {
 Int     nxs;
 Double  t2, t1, t4, t3, d1, d2, d3, d4, sxz, syz;
 Double	 x0, x1, x2, x3, y0, y1, y2, y3;
 Int     i, j, n, stride;
 Double	*xpa1, *xpa2, *xpb1, *xpb2;

 nxs = nx;
 stride = nxs - nx;
 a1=a2=a3=a4=a5=a6=a7=a8=a9=a10=0.0; 
 xpa1 = xa;
 xpa2 = xa + nxs;
 xpb1 = xb;
 xpb2 = xb + nxs;
 
 j = ny - 1;
 while (j--) {
 x0 = *xpa1++;
 y0 = *xpb1++;
 x2 = *xpa2++;
 y2 = *xpb2++;
 
 i = nx -1;
 while (i--) {
 y3 = *xpb2++;
 x1 = *xpa1++;
 y1 = *xpb1++;
 x3 = *xpa2++;
 d1 = x0 - y3;
 d2 = x2 - y1;
 d3 = x1 - y2;
 d4 = x3 - y0;
 t1 = d1 + d2 + d3 + d4;
 t2 = d1 + d2 - d3 - d4;
 t3 = d1 - d2 + d3 - d4;
 t4 = d1 - d2 - d3 + d4;
 x0 = x1;
 x2 = x3;
 y0 = y1;
 y2 = y3;
 a1 += t1*t1;
 a2 += t2*t2;
 a3 += t3*t3;
 a4 += t4*t4;
 a5 += t1*t2;
 a6 += t1*t3;
 a7 += t1*t4;
 a8 += t2*t3;
 a9 += t2*t4;
 a10 += t3*t4;
 }
 xpa1 += stride;
 xpa2 += stride;
 xpb1 += stride;
 xpb2 += stride;
 }
 a1 = .0625*a1;
 a2 = .25 * a2;
 a3 = .25 * a3;
 a5 = .25 * a5;
 a6 = .25 * a6;
 a7 = .5 * a7;
 a8 = .5 * a8;
 /* 3/4/97 changed a7 to be a7+a8 */
 a7 = a7 + a8;
 
 /* only 1 quad here so do the MAX find in line */
 xoff = yoff = 0.0;
 n = 11;
 syz = 0.0;	/* zero seed */
 while (n--) {
 /* get new sxz */
 sxz = sxvaluec(syz);
 if ( sxz < -0.5 || sxz > 0.5) { badmatch++; xoff = yoff = 0.0;  return 0.0; }
 /* and get a new syz */
 syz = syvaluec(sxz);
 if ( syz < -0.5 || syz > 0.5) { badmatch++; xoff = yoff = 0.0;  return 0.0; }
 /* printf("      sxz, syz = %g %g\n", sxz,syz); */
 }
 xoff = 2.*sxz;	yoff = 2.*syz;
 
 return mertc(sxz, syz);
 }
/*------------------------------------------------------------------------- */
Double  subshiftc_apod(xa, xb, gg, nx, ny)
 /* this version includes an apodizing function already prepared in gg
 which must be dimensioned (nx-1) by (ny-1) */
 /* we assume that the arrays have already been converted to F*8 */
 Double	*xa, *xb, *gg;
 Int     nx, ny;
 {
 Int     nxs;
 Double  t2, t1, t4, t3, d1, d2, d3, d4, sxz, syz;
 Double	 x0, x1, x2, x3, y0, y1, y2, y3, gapod;
 Int     i, j, n, stride;
 Double	*xpa1, *xpa2, *xpb1, *xpb2, xq;

 nxs = nx;
 stride = nxs - nx;
 a1=a2=a3=a4=a5=a6=a7=a8=a9=a10=0.0; 
 xpa1 = xa;
 xpa2 = xa + nxs;
 xpb1 = xb;
 xpb2 = xb + nxs;
 
 j = ny - 1;
 while (j--) {
 x0 = *xpa1++;
 y0 = *xpb1++;
 x2 = *xpa2++;
 y2 = *xpb2++;
 
 i = nx -1;
 while (i--) {
 y3 = *xpb2++;
 x1 = *xpa1++;
 y1 = *xpb1++;
 x3 = *xpa2++;
 gapod = *gg++;
 d1 = x0 - y3;
 d2 = x2 - y1;
 d3 = x1 - y2;
 d4 = x3 - y0;
 t1 = d1 + d2 + d3 + d4;
 t2 = d1 + d2 - d3 - d4;
 t3 = d1 - d2 + d3 - d4;
 t4 = d1 - d2 - d3 + d4;
 x0 = x1;
 x2 = x3;
 y0 = y1;
 y2 = y3;
 xq = t1*gapod;
 a1 += xq*t1;
 a5 += xq*t2;
 a6 += xq*t3;
 a7 += xq*t4;
 xq = t2*gapod;
 a2 += xq*t2;
 a8 += xq*t3;
 a9 += xq*t4;
 xq = t3*gapod;
 a3 += xq*t3;
 a10 += xq*t4;
 a4 += gapod*t4*t4;
 }
 xpa1 += stride;
 xpa2 += stride;
 xpb1 += stride;
 xpb2 += stride;
 }
 a1 = .0625*a1;
 a2 = .25 * a2;
 a3 = .25 * a3;
 a5 = .25 * a5;
 a6 = .25 * a6;
 a7 = .5 * a7;
 a8 = .5 * a8;
 /* 3/4/97 changed a7 to be a7+a8 */
 a7 = a7 + a8;
 
 /* only 1 quad here so do the max find in line */
 xoff = yoff = 0.0;
 n = 11;
 syz = 0.0;	/* zero seed */
 while (n--) {
 /* get new sxz */
 sxz = sxvaluec(syz);
 if ( sxz < -0.5 || sxz > 0.5) { xoff = yoff = 0.0;  return 0.0; }
 /* and get a new syz */
 syz = syvaluec(sxz);
 if ( syz < -0.5 || syz > 0.5) { xoff = yoff = 0.0;  return 0.0; }
 /* printf("      sxz, syz = %g %g\n", sxz,syz); */
 }
 xoff = 2.*sxz;	yoff = 2.*syz;
 
 return mertc(sxz, syz);
 }
 /*------------------------------------------------------------------------- */
Int ana_dilate(Int narg, Int ps[])
/* dilates a 2D image.  LS 9nov98 */
{
  Int	nx, ny, result, type, n;
  pointer	data, out;

  if (!symbolIsNumericalArray(ps[0]) /* not a numerical array */
      || array_num_dims(ps[0]) != 2) /* or isn't two-dimensionsal */
    return cerror(NEED_2D_ARR, ps[0]);

  nx = array_dims(ps[0])[0];
  ny = array_dims(ps[0])[1];

  data.b = array_data(ps[0]);

  type = array_type(ps[0]);
  if (type >= ANA_FLOAT)
    type = ANA_LONG;
  result = array_clone(ps[0], type);
  out.b = array_data(result);

  switch (type) {
    case ANA_BYTE:
      memcpy(out.b, data.b, nx*ana_type_size[type]); /* top row: just copy */
      ny -= 2;
      out.b += nx;
      data.b += nx;
      while (ny--) {
	*out.b++ = *data.b++;	/* left edge: just copy */
	n = nx - 2;
	while (n--) {
	  *out.b++ = (*data.b || data.b[1] || data.b[1 + nx] || data.b[nx]
		      || data.b[nx - 1] || data.b[-1] || data.b[-1 - nx]
		      || data.b[-nx] || data.b[-nx + 1]);
	  data.b++;
	}
	*out.b++ = *data.b++;	/* right edge: just copy */
      }
      memcpy(out.b, data.b, nx*ana_type_size[type]); /* top row: just copy */
      break;
    case ANA_WORD:
      memcpy(out.w, data.w, nx*ana_type_size[type]); /* top row: just copy */
      ny -= 2;
      out.w += nx;
      data.w += nx;
      while (ny--) {
	*out.w++ = *data.w++;	/* left edge: just copy */
	n = nx - 2;
	while (n--) {
	  *out.w++ = (*data.w || data.w[1] || data.w[1 + nx] || data.w[nx]
		      || data.w[nx - 1] || data.w[-1] || data.w[-1 - nx]
		      || data.w[-nx] || data.w[-nx + 1]);
	  data.w++;
	}
	*out.w++ = *data.w++;	/* right edge: just copy */
      }
      memcpy(out.w, data.w, nx*ana_type_size[type]); /* top row: just copy */
      break;
    case ANA_LONG:
      memcpy(out.l, data.l, nx*ana_type_size[type]); /* top row: just copy */
      ny -= 2;
      out.l += nx;
      data.l += nx;
      while (ny--) {
	*out.l++ = *data.l++;	/* left edge: just copy */
	n = nx - 2;
	while (n--) {
	  *out.l++ = (*data.l || data.l[1] || data.l[1 + nx] || data.l[nx]
		      || data.l[nx - 1] || data.l[-1] || data.l[-1 - nx]
		      || data.l[-nx] || data.l[-nx + 1]);
	  data.l++;
	}
	*out.l++ = *data.l++;	/* right edge: just copy */
      }
      memcpy(out.l, data.l, nx*ana_type_size[type]); /* top row: just copy */
      break;
    case ANA_FLOAT:
      n = nx;
      while (n--)
	*out.l++ = (*data.f++ != 0);
      ny -= 2;
      while (ny--) {
	*out.l++ = (*data.f++ != 0); /* left edge: just copy */
	n = nx - 2;
	while (n--) {
	  *out.l++ = (*data.f || data.f[1] || data.f[1 + nx] || data.f[nx]
		      || data.f[nx - 1] || data.f[-1] || data.f[-1 - nx]
		      || data.f[-nx] || data.f[-nx + 1]);
	  data.f++;
	}
	*out.l++ = (*data.f++ != 0); /* right edge: just copy */
      }
      n = nx;
      while (n--)
	*out.l++ = (*data.f++ != 0);
      break;
    case ANA_DOUBLE:
      n = nx;
      while (n--)
	*out.l++ = (*data.d++ != 0);
      ny -= 2;
      while (ny--) {
	*out.d++ = (*data.d++ != 0); /* left edge: just copy */
	n = nx - 2;
	while (n--) {
	  *out.l++ = (*data.d || data.d[1] || data.d[1 + nx] || data.d[nx]
		      || data.d[nx - 1] || data.d[-1] || data.d[-1 - nx]
		      || data.d[-nx] || data.d[-nx + 1]);
	  data.d++;
	}
	*out.l++ = (*data.d++ != 0); /* right edge: just copy */
      }
      n = nx;
      while (n--)
	*out.l++ = (*data.d++ != 0);
      break;
  }
  return result;
}
/*------------------------------------------------------------------------- */
Int ana_erode(Int narg, Int ps[])
/* erodes a 2D image.  LS 9nov98, 9may2000 */
{
  Int	nx, ny, result, type, n;
  pointer	data, out;
  char	zeroedge;

  if (!symbolIsNumericalArray(ps[0]) /* not a numerical array */
      || array_num_dims(ps[0]) != 2) /* or isn't two-dimensionsal */
    return cerror(NEED_2D_ARR, ps[0]);

  nx = array_dims(ps[0])[0];
  ny = array_dims(ps[0])[1];

  data.b = array_data(ps[0]);

  type = array_type(ps[0]);
  if (type >= ANA_FLOAT)
    type = ANA_LONG;
  result = array_clone(ps[0], type);
  out.b = array_data(result);

  zeroedge = (internalMode & 1); /* /ZEROEDGE */

  switch (type) {
    case ANA_BYTE:
      n = nx;
      if (zeroedge) {
	while (n--)
	  *out.b++ = 0;
	data.b += nx;
      } else
	while (n--)
	  *out.b++ = (*data.b++ != 0);
      ny -= 2;
      while (ny--) {
	*out.b++ = zeroedge? 0: (*data.b != 0); /* left edge */
	data.b++;
	n = nx - 2;
	while (n--) {
	  *out.b++ = (*data.b && data.b[1] && data.b[1 + nx] && data.b[nx]
		      && data.b[nx - 1] && data.b[-1] && data.b[-1 - nx]
		      && data.b[-nx] && data.b[-nx + 1]);
	  data.b++;
	}
	*out.b++ = zeroedge? 0: (*data.b != 0); /* right edge */
	data.b++;
      }
      n = nx;
      if (zeroedge)
	while (n--)
	  *out.b++ = 0;
      else
	while (n--)
	  *out.b++ = (*data.b++ != 0);
      break;
    case ANA_WORD:
      n = nx;
      if (zeroedge) {
	while (n--)
	  *out.w++ = 0;
	data.w += nx;
      } else
	while (n--)
	  *out.w++ = (*data.w++ != 0);
      ny -= 2;
      while (ny--) {
	*out.w++ = zeroedge? 0: (*data.w != 0); /* left edge */
	data.w++;
	n = nx - 2;
	while (n--) {
	  *out.w++ = (*data.w && data.w[1] && data.w[1 + nx] && data.w[nx]
		      && data.w[nx - 1] && data.w[-1] && data.w[-1 - nx]
		      && data.w[-nx] && data.w[-nx + 1]);
	  data.w++;
	}
	*out.w++ = zeroedge? 0: (*data.w != 0); /* right edge */
	data.w++;
      }
      n = nx;
      if (zeroedge)
	while (n--)
	  *out.w++ = 0;
      else
	while (n--)
	  *out.w++ = (*data.w++ != 0);
      break;
    case ANA_LONG:
      n = nx;
      if (zeroedge) {
	while (n--)
	  *out.l++ = 0;
	data.l += nx;
      } else
	while (n--)
	  *out.l++ = (*data.l++ != 0);
      ny -= 2;
      while (ny--) {
	*out.l++ = zeroedge? 0: (*data.l != 0); /* left edge */
	data.l++;
	n = nx - 2;
	while (n--) {
	  *out.l++ = (*data.l && data.l[1] && data.l[1 + nx] && data.l[nx]
		      && data.l[nx - 1] && data.l[-1] && data.l[-1 - nx]
		      && data.l[-nx] && data.l[-nx + 1]);
	  data.l++;
	}
	*out.l++ = zeroedge? 0: (*data.l != 0); /* right edge */
	data.l++;
      }
      n = nx;
      if (zeroedge)
	while (n--)
	  *out.l++ = 0;
      else
	while (n--)
	  *out.l++ = (*data.l++ != 0);
      break;
    case ANA_FLOAT:
      n = nx;
      if (zeroedge) {
	while (n--)
	  *out.l++ = 0;
	data.f += nx;
      } else
	while (n--)
	  *out.l++ = (*data.f++ != 0);
      ny -= 2;
      while (ny--) {
	*out.l++ = zeroedge? 0: (*data.f != 0); /* left edge */
	data.f++;
	n = nx - 2;
	while (n--) {
	  *out.l++ = (*data.f && data.f[1] && data.f[1 + nx] && data.f[nx]
		      && data.f[nx - 1] && data.f[-1] && data.f[-1 - nx]
		      && data.f[-nx] && data.f[-nx + 1]);
	  data.f++;
	}
	*out.l++ = zeroedge? 0: (*data.f != 0); /* right edge */
	data.f++;
      }
      n = nx;
      if (zeroedge)
	while (n--)
	  *out.l++ = 0;
      else
	while (n--)
	  *out.l++ = (*data.f++ != 0);
      break;
    case ANA_DOUBLE:
      n = nx;
      if (zeroedge) {
	while (n--)
	  *out.l++ = 0;
	data.d += nx;
      } else
	while (n--)
	  *out.l++ = (*data.d++ != 0);
      ny -= 2;
      while (ny--) {
	*out.l++ = zeroedge? 0: (*data.d != 0); /* left edge */
	data.d++;
	n = nx - 2;
	while (n--) {
	  *out.l++ = (*data.d && data.d[1] && data.d[1 + nx] && data.d[nx]
		      && data.d[nx - 1] && data.d[-1] && data.d[-1 - nx]
		      && data.d[-nx] && data.d[-nx + 1]);
	  data.d++;
	}
	*out.l++ = zeroedge? 0: (*data.d != 0); /* right edge */
	data.d++;
      }
      n = nx;
      if (zeroedge)
	while (n--)
	  *out.l++ = 0;
      else
	while (n--)
	  *out.l++ = (*data.d++ != 0);
      break;
  }
  return result;
}
/*------------------------------------------------------------------------- */