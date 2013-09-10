/* File tense.c */
/* ANA routines for interpolation along curves. */
/* tense.f -- translated by f2c (version of 28 March 1990  0:01:01). */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <math.h>
#include "anadefs.h"
#include "output.h"
static char rcsid[] __attribute__ ((unused)) =
 "$Id: tense.c,v 4.0 2001/02/07 20:37:08 strous Exp $";

Int curv1_(Int *n, Double *x, Double *y, Double *slp1, Double *slpn,
	   Double *yp, Double *temp, Double *sigma, Double *xf, Double *yf,
	   Int *nf)
{
    /* System generated locals */
    Int ret_val, i_1, i_2;

    /* Builtin functions */
    Int s_wsle(), do_lio(), e_wsle();

    /* Local variables */
    static Int ibak;
    static Double deln, dels, exps, diag1, diag2, delx1, delx2, slpp1, 
	    exps1;
    static Int i, j;
    static Double t, delx12, delnn, sinhs, c1, c2, c3;
    static Int i1;
    static Double slppn, delnm1, sinhd1, sinhd2, diagin, spdiag, sigmap, 
	    sinhin;
    static Int nm1;
    static Double dx1, dx2;
    static Int np1;
    static Double del1, del2;


/* --	generates spline under tension for monotonic x */
/* --	NOTE THAT THIS IS A ANA_FUNCTION IN ORDER TO RETURN SUCCESS OR 
FAILURE */
    /* Parameter adjustments */
    --yf;
    --xf;
    --temp;
    --yp;
    --y;
    --x;

    /* Function Body */
    if (*sigma == 0.) {
        printf("tension cannot be 0\n");
	ret_val = 0;
	return ret_val;
    }
    nm1 = *n - 1;
    np1 = *n + 1;
    delx1 = x[2] - x[1];
    dx1 = (y[2] - y[1]) / delx1;
    if (*sigma < 0.) {
	goto L50;
    }
    slpp1 = *slp1;
    slppn = *slpn;
L10:
    sigmap = ABS(*sigma) * (Float) (*n - 1) / (x[*n] - x[1]);
/* 	FOR INTERACTIVE USE, WE SLOW THIS ROUTINE DOWN BY DOING SOME */
/* 	EXTRA CHECKING FOR THE USER, THIS COULD PREVENT MUCH TIME LOST */
/* 	FROM A CRASH */
    dx2 = (Float)0.;
    i_1 = *n;
    for (i = 2; i <= i_1; ++i) {
	delx2 = x[i] - x[i - 1];
	if (delx2 <= 0.) {
	    goto L990;
	}
/* ANA_ERROR, EXIT */
	dels = sigmap * delx2;
	if (dels > dx2) {
	    dx2 = dels;
	}
/* FIND MAX */
/* L12: */
    }
    if (dx2 > 50.) {
/* -- IF IT IS TOO BIG, WE GOT TROUBLE; WE WILL AUTO REDUCE TO FIT */
        printf("warning, tension reduced to avoid overflows!\n");
	sigmap = sigmap * (Float)50. / dx2;
    }
    dels = sigmap * delx1;
    exps = exp(dels);
    sinhs = (exps - (Float)1. / exps) * (Float).5;
    sinhin = (Float)1. / (delx1 * sinhs);
    diag1 = sinhin * (dels * (Float).5 * (exps + (Float)1. / exps) - sinhs);
    diagin = (Float)1. / diag1;
    yp[1] = diagin * (dx1 - slpp1);
    spdiag = sinhin * (sinhs - dels);
    temp[1] = diagin * spdiag;
    if (*n == 2) {
	goto L30;
    }
    i_1 = nm1;
    for (i = 2; i <= i_1; ++i) {
	delx2 = x[i + 1] - x[i];
	dx2 = (y[i + 1] - y[i]) / delx2;
	dels = sigmap * delx2;
	exps = exp(dels);
	sinhs = (exps - (Float)1. / exps) * (Float).5;
	sinhin = (Float)1. / (delx2 * sinhs);
	diag2 = sinhin * (dels * ((exps + (Float)1. / exps) * (Float).5) - 
		sinhs);
	diagin = (Float)1. / (diag1 + diag2 - spdiag * temp[i - 1]);
	yp[i] = diagin * (dx2 - dx1 - spdiag * yp[i - 1]);
	spdiag = sinhin * (sinhs - dels);
	temp[i] = diagin * spdiag;
	dx1 = dx2;
	diag1 = diag2;
/* L20: */
    }
L30:
    diagin = (Float)1. / (diag1 - spdiag * temp[nm1]);
    yp[*n] = diagin * (slppn - dx2 - spdiag * yp[nm1]);
    i_1 = *n;
    for (i = 2; i <= i_1; ++i) {
	ibak = np1 - i;
	yp[ibak] -= temp[ibak] * yp[ibak + 1];
/* L40: */
    }
    goto L200;
L50:
    if (*n == 2) {
	goto L60;
    }
    delx2 = x[3] - x[2];
    delx12 = x[3] - x[1];
    c1 = -(delx12 + delx1) / delx12 / delx1;
    c2 = delx12 / delx1 / delx2;
    c3 = -delx1 / delx12 / delx2;
    slpp1 = c1 * y[1] + c2 * y[2] + c3 * y[3];
    deln = x[*n] - x[nm1];
    delnm1 = x[nm1] - x[*n - 2];
    delnn = x[*n] - x[*n - 2];
    c1 = (delnn + deln) / delnn / deln;
    c2 = -delnn / deln / delnm1;
    c3 = deln / delnn / delnm1;
    slppn = c3 * y[*n - 2] + c2 * y[nm1] + c1 * y[*n];
    goto L10;
L60:
    yp[1] = (Float)0.;
    yp[2] = (Float)0.;
L200:
    i1 = 2;
    i_1 = *nf;
    for (j = 1; j <= i_1; ++j) {
	t = xf[j];
L210:
	i_2 = *n;
	for (i = i1; i <= i_2; ++i) {
	    if (x[i] - t <= 0.) {
		goto L220;
	    } else {
		goto L230;
	    }
L220:
	    ;
	}
	i = *n;
L230:
	if (x[i - 1] <= t || t <= x[1]) {
	    goto L240;
	}
	i1 = 2;
	goto L210;
L240:
	del1 = t - x[i - 1];
	del2 = x[i] - t;
	dels = x[i] - x[i - 1];
	exps1 = exp(sigmap * del1);
	sinhd1 = (exps1 - (Float)1. / exps1) * (Float).5;
	exps = exp(sigmap * del2);
	sinhd2 = (exps - (Float)1. / exps) * (Float).5;
	exps = exps1 * exps;
	sinhs = (exps - (Float)1. / exps) * (Float).5;
	yf[j] = (yp[i] * sinhd1 + yp[i - 1] * sinhd2) / sinhs + ((y[i] - yp[i]
		) * del1 + (y[i - 1] - yp[i - 1]) * del2) / dels;
	i1 = i;
/* L300: */
    }
    ret_val = 1;
    return ret_val;
L990:
/* ANA_ERROR */
    printf("independent vector not monotonically increasing\n");
    ret_val = 0;
    return ret_val;
} /* curv1_ */

/*===========================================================================
*/
Int kurv1_(Int *n, Double *x, Double *y, Double *slp1, Double *slpn,
	   Double *xp, Double *yp, Double *temp, Double *sigma, Double *t,
	   Double *xs, Double *ys, Int *nf)
{
    /* Initialized data */

    static Double degrad = .017453292;

    /* System generated locals */
    Int ret_val, i_1, i_2;
    Double d_1, d_2;

    /* Builtin functions */
    Int s_wsle(), do_lio(), e_wsle();

    /* Local variables */
    static Double deln, dels, delx, dely, exps, diag1, diag2, dels1, 
	    dels2, delx1, dely1, delx2, dely2, slpp1, exps1;
    static Int i, k;
    static Double s, dels12, delnn, c1, c2, c3, sinhs;
    static Int i1;
    static Double slppn, delnm1, sinhd1, sinhd2, diagin, tn, spdiag, 
	    sx, sy, sigmap, sinhin;
    static Int nm1;
    static Double dx1, dy1, dx2, dy2, sum, del1, del2;


/* --	generates spline under tension for any open (x,y) curve */
/* --	NOTE THAT THIS IS A ANA_FUNCTION IN ORDER TO RETURN SUCCESS OR 
FAILURE */
    /* Parameter adjustments */
    --ys;
    --xs;
    --t;
    --temp;
    --yp;
    --xp;
    --y;
    --x;

    /* Function Body */
    ret_val = 1;
    if (*sigma == 0.) {
        printf("tension cannot be 0\n");
	ret_val = 0;
	return ret_val;
    }
    nm1 = *n - 1;
    delx1 = x[2] - x[1];
    dely1 = y[2] - y[1];
    dels1 = sqrt(delx1 * delx1 + dely1 * dely1);
    dx1 = delx1 / dels1;
    dy1 = dely1 / dels1;
    s = dels1;
/* --	check if slopes were input (yes if sigma>0) */
    if (*sigma > 0.) {
/* --	convert input slopes from degress to radians */
	slpp1 = *slp1 * degrad;
	slppn = *slpn * degrad;
    } else {
/* determine slopes */
	if (*n == 2) {
/* --	if only 2 points and no slopes, we have to assume a 
straight line */
	    xp[1] = (Float)0.;
	    xp[2] = (Float)0.;
	    yp[1] = (Float)0.;
	    yp[2] = (Float)0.;
	    sigmap = ABS(*sigma) * (Float) (*n - 1) / s;
	} else {
/* Computing 2nd power */
	    d_1 = x[3] - x[2];
/* Computing 2nd power */
	    d_2 = y[3] - y[2];
	    dels2 = sqrt(d_1 * d_1 + d_2 * d_2);
	    dels12 = dels1 + dels2;
	    c1 = -(dels12 + dels1) / dels12 / dels1;
	    c2 = dels12 / dels1 / dels2;
	    c3 = -dels1 / dels12 / dels2;
	    sx = c1 * x[1] + c2 * x[2] + c3 * x[3];
	    sy = c1 * y[1] + c2 * y[2] + c3 * y[3];
	    slpp1 = atan2(sy, sx);
/* Computing 2nd power */
	    d_1 = x[*n - 2] - x[nm1];
/* Computing 2nd power */
	    d_2 = y[*n - 2] - y[nm1];
	    delnm1 = sqrt(d_1 * d_1 + d_2 * d_2);
/* Computing 2nd power */
	    d_1 = x[nm1] - x[*n];
/* Computing 2nd power */
	    d_2 = y[nm1] - y[*n];
	    deln = sqrt(d_1 * d_1 + d_2 * d_2);
	    delnn = delnm1 + deln;
	    c1 = (delnn + deln) / delnn / deln;
	    c2 = -delnn / deln / delnm1;
	    c3 = deln / delnn / delnm1;
	    sx = c3 * x[*n - 2] + c2 * x[nm1] + c1 * x[*n];
	    sy = c3 * y[*n - 2] + c2 * y[nm1] + c1 * y[*n];
	    slppn = atan2(sy, sx);
	}
    }
/* --	slopes are now done */
    printf("slopes are %g %g\n", slpp1, slppn);
    xp[1] = dx1 - cos(slpp1);
    yp[1] = dy1 - sin(slpp1);
    temp[1] = dels1;
    if (*n > 2) {
	i_1 = nm1;
	for (i = 2; i <= i_1; ++i) {
	    delx2 = x[i + 1] - x[i];
	    dely2 = y[i + 1] - y[i];
	    dels2 = sqrt(delx2 * delx2 + dely2 * dely2);
	    dx2 = delx2 / dels2;
	    dy2 = dely2 / dels2;
	    xp[i] = dx2 - dx1;
	    yp[i] = dy2 - dy1;
	    temp[i] = dels2;
	    delx1 = delx2;
	    dely1 = dely2;
	    dels1 = dels2;
	    dx1 = dx2;
	    dy1 = dy2;
	    s += dels1;
	}
    }
    xp[*n] = cos(slppn) - dx1;
    yp[*n] = sin(slppn) - dy1;
    sigmap = ABS(*sigma) * (Float) (*n - 1) / s;
    dels = sigmap * temp[1];
    exps = exp(dels);
/* 	TYPE *,'DELS,EXPS =',DELS,EXPS */
    sinhs = (exps - (Float)1. / exps) * (Float).5;
    sinhin = (Float)1. / (temp[1] * sinhs);
    diag1 = sinhin * (dels * (Float).5 * (exps + (Float)1. / exps) - sinhs);
    diagin = (Float)1. / diag1;
    xp[1] = diagin * xp[1];
    yp[1] = diagin * yp[1];
    spdiag = sinhin * (sinhs - dels);
    temp[1] = diagin * spdiag;
    if (*n > 2) {
	i_1 = nm1;
	for (i = 2; i <= i_1; ++i) {
	    dels = sigmap * temp[i];
	    exps = exp(dels);
/* 	TYPE *,'DELS,EXPS =',DELS,EXPS */
	    sinhs = (exps - (Float)1. / exps) * (Float).5;
	    sinhin = (Float)1. / (temp[i] * sinhs);
	    diag2 = sinhin * (dels * ((exps + (Float)1. / exps) * (Float).5) 
		    - sinhs);
	    diagin = (Float)1. / (diag1 + diag2 - spdiag * temp[i - 1]);
	    xp[i] = diagin * (xp[i] - spdiag * xp[i - 1]);
	    yp[i] = diagin * (yp[i] - spdiag * yp[i - 1]);
	    spdiag = sinhin * (sinhs - dels);
	    temp[i] = diagin * spdiag;
	    diag1 = diag2;
	}
    }
    diagin = (Float)1. / (diag1 - spdiag * temp[nm1]);
    xp[*n] = diagin * (xp[*n] - spdiag * xp[nm1]);
    yp[*n] = diagin * (yp[*n] - spdiag * yp[nm1]);
    for (i = *n - 1; i >= 1; --i) {
	xp[i] -= temp[i] * xp[i + 1];
	yp[i] -= temp[i] * yp[i + 1];
    }
/* --	done with the setup, now do the interpolations */

    i = 2;
    sum = (Float)0.;
    delx = x[2] - x[1];
    dely = y[2] - y[1];
    dels = sqrt(delx * delx + dely * dely);
    i_1 = *nf;
    for (k = 1; k <= i_1; ++k) {
/* --	loop over the nf input values of T */
	tn = (d_1 = t[k] * s, ABS(d_1));
L10:
	if (tn < sum) {
/* --	drop back */
	    --i;
	    if (i >= 2) {
		delx = x[i] - x[i - 1];
		dely = y[i] - y[i - 1];
		dels = sqrt(delx * delx + dely * dely);
		sum -= dels;
		goto L10;
	    } else {
		xs[k] = x[1];
		ys[k] = y[1];
		goto L50;
	    }
	}
	if (tn > sum + dels) {
/* --	go forward */
	    i1 = i;
	    i_2 = *n;
	    for (i = i1; i <= i_2; ++i) {
		delx = x[i] - x[i - 1];
		dely = y[i] - y[i - 1];
		dels = sqrt(delx * delx + dely * dely);
		if (sum + dels > tn) {
		    goto L40;
		}
		sum += dels;
	    }
	    xs[k] = x[*n];
	    ys[k] = y[*n];
	    goto L50;
L40:
	    ;
	}
	del1 = tn - sum;
	del2 = dels - del1;
	exps1 = exp(sigmap * del1);
	sinhd1 = (exps1 - (Float)1. / exps1) * (Float).5;
	exps = exp(sigmap * del2);
	sinhd2 = (exps - (Float)1. / exps) * (Float).5;
	exps = exps1 * exps;
	sinhs = (exps - (Float)1. / exps) * (Float).5;
	xs[k] = (xp[i] * sinhd1 + xp[i - 1] * sinhd2) / sinhs + ((x[i] - xp[i]
		) * del1 + (x[i - 1] - xp[i - 1]) * del2) / dels;
	ys[k] = (yp[i] * sinhd1 + yp[i - 1] * sinhd2) / sinhs + ((y[i] - yp[i]
		) * del1 + (y[i - 1] - yp[i - 1]) * del2) / dels;
L50:
	;
    }
    return ret_val;
} /* kurv1_ */

/*===========================================================================
==*/
Int kurvp1_(Int *n, Double *x, Double *y, Double *xp, Double *yp,
	    Double *temp, Double *sigma, Double *t, Double *xs, Double *ys,
	    Int *nf)
{
    /* System generated locals */
    Int ret_val, i_1, i_2;
    Double d_1;

    /* Builtin functions */
    Int s_wsle(), do_lio(), e_wsle();

    /* Local variables */
    static Int ibak;
    static Double dels, delx, dely, exps, diag1, diag2, dels1, dels2, 
	    delx1, dely1, delx2, dely2, exps1;
    static Int i, k;
    static Double s, sinhs;
    static Int i1;
    static Double sinhd1, spdig1, sinhd2, diagin, tn, spdiag, sigmap, 
	    sinhin;
    static Int im1, ip1, nm1;
    static Double dx1, dy1;
    static Int np1;
    static Double dx2, dy2, sum, del1, del2;


/* --	generates spline under tension for any closed loop (x,y) 
curve */
/* --	NOTE THAT THIS IS A ANA_FUNCTION IN ORDER TO RETURN SUCCESS OR 
FAILURE */
    /* Parameter adjustments */
    --ys;
    --xs;
    --t;
    --temp;
    --yp;
    --xp;
    --y;
    --x;

    /* Function Body */
    ret_val = 1;
    if (*sigma == 0.) {
        printf("tension cannot be 0\n");
	ret_val = 0;
	return ret_val;
    }
    nm1 = *n - 1;
    np1 = *n + 1;
    delx1 = x[2] - x[1];
    dely1 = y[2] - y[1];
    dels1 = sqrt(delx1 * delx1 + dely1 * dely1);
    dx1 = delx1 / dels1;
    dy1 = dely1 / dels1;
    xp[1] = dx1;
    yp[1] = dy1;
    temp[1] = dels1;
    s = dels1;
    i_1 = *n;
    for (i = 2; i <= i_1; ++i) {
	ip1 = i + 1;
	if (i == *n) {
	    ip1 = 1;
	}
	delx2 = x[ip1] - x[i];
	dely2 = y[ip1] - y[i];
	dels2 = sqrt(delx2 * delx2 + dely2 * dely2);
	dx2 = delx2 / dels2;
	dy2 = dely2 / dels2;
	xp[i] = dx2 - dx1;
	yp[i] = dy2 - dy1;
	temp[i] = dels2;
	delx1 = delx2;
	dely1 = dely2;
	dels1 = dels2;
	dx1 = dx2;
	dy1 = dy2;
	s += dels1;
    }
    xp[1] -= dx1;
    yp[1] -= dy1;
    sigmap = ABS(*sigma) * (Float) (*n) / s;
    dels = sigmap * temp[*n];
    exps = exp(dels);
    sinhs = (exps - (Float)1. / exps) * (Float).5;
    sinhin = (Float)1. / (temp[*n] * sinhs);
    diag1 = sinhin * (dels * (Float).5 * (exps + (Float)1. / exps) - sinhs);
    diagin = (Float)1. / diag1;
    spdig1 = sinhin * (sinhs - dels);
    spdiag = (Float)0.;
    i_1 = *n;
    for (i = 1; i <= i_1; ++i) {
	dels = sigmap * temp[i];
	exps = exp(dels);
	sinhs = (exps - (Float)1. / exps) * (Float).5;
	sinhin = (Float)1. / (temp[i] * sinhs);
	diag2 = sinhin * (dels * ((exps + (Float)1. / exps) * (Float).5) - 
		sinhs);
	if (i == *n) {
	    goto L30;
	}
	diagin = (Float)1. / (diag1 + diag2 - spdiag * temp[i - 1]);
	xp[i] = diagin * (xp[i] - spdiag * xp[i - 1]);
	yp[i] = diagin * (yp[i] - spdiag * yp[i - 1]);
/* 	REMOVED TYPO IN NEXT LINE	SHINE 2/2/93 */
	temp[*n + i] = -diagin * temp[nm1 + i] * spdiag;
	if (i == 1) {
	    temp[np1] = -diagin * spdig1;
	}
	spdiag = sinhin * (sinhs - dels);
	temp[i] = diagin * spdiag;
	diag1 = diag2;
    }
L30:
    temp[nm1] = temp[*n + nm1] - temp[nm1];
    if (*n == 2) {
	goto L50;
    }
    i_1 = *n;
    for (i = 3; i <= i_1; ++i) {
	ibak = np1 - i;
	xp[ibak] -= temp[ibak] * xp[ibak + 1];
	yp[ibak] -= temp[ibak] * yp[ibak + 1];
	temp[ibak] = temp[*n + ibak] - temp[ibak] * temp[ibak + 1];
    }
L50:
    xp[*n] = (xp[*n] - spdig1 * xp[1] - spdiag * xp[nm1]) / (diag1 + diag2 + 
	    spdig1 * temp[1] + spdiag * temp[nm1]);
    yp[*n] = (yp[*n] - spdig1 * yp[1] - spdiag * yp[nm1]) / (diag1 + diag2 + 
	    spdig1 * temp[1] + spdiag * temp[nm1]);
    i_1 = nm1;
    for (i = 1; i <= i_1; ++i) {
	xp[i] += temp[i] * xp[*n];
	yp[i] += temp[i] * yp[*n];
    }
/* --	done with setup, now do the interpolations */

    i = 2;
    sum = (Float)0.;
    delx = x[2] - x[1];
    dely = y[2] - y[1];
    dels = sqrt(delx * delx + dely * dely);
    i_1 = *nf;
    for (k = 1; k <= i_1; ++k) {
/* --	loop over the nf input values of T */
	tn = (d_1 = t[k] * s, ABS(d_1));
	if (tn > s) {
	    tn = s;
	}
/* 	type *,'start, k,tn,sum,dels',k,tn,sum,dels */
L10:
	if (tn < sum) {
/* --	drop back */
/* 	type *,'drop back, I=',I */
	    --i;
	    if (i >= 2) {
		delx = x[i] - x[i - 1];
		dely = y[i] - y[i - 1];
		dels = sqrt(delx * delx + dely * dely);
		sum -= dels;
		goto L10;
	    } else {
/* --	I was 1, we must have dels since we never start with 
1 */
		i = *n;
		sum -= dels;
/* 	type *,'drop back, I was 1' */
		goto L10;
	    }
	}
	if (tn > sum + dels) {
/* --	go forward */
/* 	type *,'go forward, I=',I */
	    i1 = i;
	    i_2 = *n;
	    for (i = i1; i <= i_2; ++i) {
		delx = x[i] - x[i - 1];
		dely = y[i] - y[i - 1];
		dels = sqrt(delx * delx + dely * dely);
		if (sum + dels > tn) {
		    goto L40;
		}
		sum += dels;
	    }
/* --	must be between N and 1 */
	    i = 1;
	    dels = s - sum;
	}
L40:
	im1 = i - 1;
	if (im1 == 0) {
	    im1 = *n;
	}
/* 	type *,'resultants, i,im1,tn,sum,dels',i,im1,tn,sum,dels */
/* 	type *,'xp,yp',XP(I),YP(I) */
	del1 = tn - sum;
	del2 = dels - del1;
	exps1 = exp(sigmap * del1);
	sinhd1 = (exps1 - (Float)1. / exps1) * (Float).5;
	exps = exp(sigmap * del2);
	sinhd2 = (exps - (Float)1. / exps) * (Float).5;
	exps = exps1 * exps;
	sinhs = (exps - (Float)1. / exps) * (Float).5;
	xs[k] = (xp[i] * sinhd1 + xp[im1] * sinhd2) / sinhs + ((x[i] - xp[i]) 
		* del1 + (x[im1] - xp[im1]) * del2) / dels;
	ys[k] = (yp[i] * sinhd1 + yp[im1] * sinhd2) / sinhs + ((y[i] - yp[i]) 
		* del1 + (y[im1] - yp[im1]) * del2) / dels;
    }
    return ret_val;
} /* kurvp1_ */

