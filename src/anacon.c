/* File anacon.c */
/* ANA contouring engine. */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "action.h"
static char rcsid[] __attribute__ ((unused)) =
 "$Id: anacon.c,v 4.0 2001/02/07 20:36:54 strous Exp $";
/* anacon.f -- translated by f2c (version of 28 March 1990  0:01:01).
   Modified by Louis Strous 27jul94
*/

Int anacon(Float *xarr, Int nx, Int ny, Float *xlev, Int numlev,
	    Int itvord, Float xl, Float yl, Float xsc, Float ysc,
	    Float cut)
{
  /* System generated locals */
  Int i_1, i_2, i_3;
  Int	tkdash(Float *, Float *, Int *, Int *);

  /* Local variables */
  Float cell[5];
  Int minc, ninc, icol, incr, ndsh, imax, ntim;
  Float xmin, xmax;
  Int irow, j, k, l, ibase;
  Float xmean;
  Int n2, n3, n4;
  Float aa[4], bb[4];
  Int ii, in;
  Float xa[4], ya[4];
  Int ns;
  Int ibindx;
  Int inccon(Int, Int, Int, Int *, Int *, Int *, Int *, Int *, Int *, Int *);
  Float cellavg(Float *, Int);
  extern /* Subroutine */ Int tkdash();
  Float xz, yz;
  Float tmp;

/*   xarr  is data array to contour (Float*4) */
/*   nx,ny  is size of xarr */
/*   xlev   is a table of contour levels Float*4 */
/*   numlev are the number of levels to contour */
/*  itvord  is the desired flip of the array - see inccon below for 
details*/
/*   xsc,ysc are the coords. of the upper rhs */
/*   xl,yl are the coordinates of the lower left corner for the contour */


  /* Parameter adjustments */
  --xlev;
  --xarr;

  /* Function Body */
  xsc -= xl;
  ysc -= yl;
  if (itvord % 2 == 0) {
/* match tektronix x,y with data x,y */
    ysc /= ny;
    xsc /= nx;
  } else {
    ysc /= nx;
    xsc /= ny;
  }
/* must reset ix,iy to center the contour on the raster */
  xz = xl + xsc / (Float)2.;
  yz = yl + ysc / (Float)2.;

/* set up increments and limits for array processing according to itvord 
*/

  inccon(itvord, nx, ny, &ibase, &ibindx, &incr, &ninc, &n2, &n3, &n4);
  minc = ny - 1;
  if (ninc == minc) {
    minc = nx - 1;
  }
  i_1 = minc;
  for (irow = 1; irow <= i_1; ++irow) {
/* row counter for position on grinnell */
    i_2 = ninc;
    for (icol = 1; icol <= i_2; ++icol) {
/* col counter for position of grinnell */
      cell[0] = xarr[ibase];
      cell[1] = xarr[ibase + n2];
      cell[2] = xarr[ibase + n3];
      cell[3] = xarr[ibase + n4];
      cell[4] = cell[0];
      /* set ibase for next cell */
      ibase += ibindx;
      xmax = cell[0];
      xmin = cell[0];
      imax = 1;
      for (j = 2; j <= 4; ++j) {
	/* find max,min of cell */
        if (cell[j - 1] > xmax)
        { xmax = cell[j - 1];  imax = j; }
        else if (cell[j - 1] < xmin)
        { xmin = cell[j - 1]; }
      }

/*  now check each cell for contours */

      i_3 = numlev;
      for (k = 1; k <= i_3; ++k) {
	if (xlev[k] <= xmax && xlev[k] >= xmin) {
/*  level goes through cell */
	  ndsh = 1;
	  if (xlev[k] < cut) {
	    ndsh = 2;
	  }
	  ns = 0;
/* number of sides */
	  for (l = 1; l <= 4; ++l) {
	    if ((xlev[k] >= cell[l - 1] && xlev[k] < cell[l])
		|| (xlev[k] < cell[l - 1] && xlev[k] >= cell[l])) {


/*  calculate endpoints of line segment on the 
side of the cell */

	      ++ns;
	      if (l == 1) {
/* top */
		xa[ns - 1] = (Float) (icol - 1) +
		  (xlev[k] - cell[l - 1]) / (cell[l] - cell[l - 1]);
		ya[ns - 1] = (Float) (irow - 1);
	      } else if (l == 2) {
/* right side */
		xa[ns - 1] = (Float) icol;
		ya[ns - 1] = (Float) (irow - 1) +
		  (xlev[k] - cell[l - 1]) / (cell[l] - cell[l - 1]);
	      } else if (l == 3) {
/* bottom side */
		xa[ns - 1] = (Float) (icol - 1) +
		  (xlev[k] - cell[l]) / (cell[l - 1] - cell[l]);
		ya[ns - 1] = (Float) irow;
	      } else {
/* left side */
		xa[ns - 1] = (Float) (icol - 1);
		ya[ns - 1] = (Float) (irow - 1) +
		  (xlev[k] - cell[l]) / (cell[l - 1] - cell[l]);
	      }
	    }
	  }
	  
	  if (ns > 2) {
	    ntim = 2;
	    xmean = cellavg(cell, 4);
/*  if the present level is greater than the mean, 
then the contour */
/*  is drawn between the max point and the center. (
imax gives max */
/*  point) */
/*  first calculate all screen coordinates */
	    in = imax;
	    for (ii = 1; ii <= 4; ++ii) {
	      aa[ii - 1] = xz + xa[in - 1] * xsc;
	      bb[ii - 1] = yz + ya[in - 1] * ysc;
	      --in;
	      if (in < 1) {
		in = 4;
	      }
	    }
	    if (xlev[k] <= xmean) {
/* switch 2 and 4 so that point 1 connects to 
point 2 and point 3 to 4. */
	      tmp = aa[3];
	      aa[3] = aa[1];
	      aa[1] = tmp;
	      tmp = bb[3];
	      bb[3] = bb[1];
	      bb[1] = tmp;
	    }
	  } else if (ns == 2) {
	    ntim = 1;
	    for (ii = 1; ii <= 2; ++ii) {
	      aa[ii - 1] = xz + xa[ii - 1] * xsc;
	      bb[ii - 1] = yz + ya[ii - 1] * ysc;
	    }
	  }
/* now draw the contours */
/* ns.gt.2 */
	  if (ns >= 2) {
	    tkdash(aa, bb, &ndsh, &ntim);
	  }
	}
/* xlev(k).gt.... */
      }
/* k=1,numlev */
    }
/* icol=1,nx-1 */
    ibase += incr;
/* reset for next row or col */
  }
/* irow=1,ny-1 */
/* L999: */
  return 0;
} /* anacon_ */


Float cellavg(Float *cell, Int nav)
{
  Float ret_val;
  Int ii;
  
  /* Parameter adjustments */
  --cell;

  /* Function Body */
  ret_val = (Float) 0.;
  for (ii = 1; ii <= nav; ++ii) {
    ret_val += cell[ii];
  }
  ret_val /= (Float) nav;
  return ret_val;
}


Int inccon(Int itv, Int nx, Int ny, Int *ibase, Int *ibindx, Int *incr,
	    Int *ninc, Int *n2, Int *n3, Int *n4)
{
/* get limits and increments for processing the raw array */
/* inputs: */
/* itv is the order to process : */
/* 	    0  left to right, bottom to top */
/* 	    1  bottom to top, left to right */
/* 	    2  left to right, top to bottom */
/* 	    3  top to bottom, left to right */
/* 	    4  right to left, top to bottom */
/* 	    5  top to bottom, right to left */
/* 	    6  right to left, bottom to top */
/* 	    7  bottom to top, right to left */
/* nx,ny  size of raw array (columns,rows) */

/* outputs: */
/* ibase base index to begin processing */
/* ibindx index used to increment base address during processing */
/* incr used at end of a row or column to reset base address */
/* n2,n3,n4 offsets in base address for 4 point cell */

/* note that even though itv may indicate array is to be processed */
/* vertically in a certain direction (either top to bottom or bottom to */

/* top), the outputs are such that the array is processed in the */
/* opposite direction.  this is because the contours are actually drawn */

/* from the bottom of the screen to the top. */

/* --	7/7/86        orders 1 and 5 seem to be reversed, patch to 
fix */
/* 	the comments and logic in parts of this seem wrong, should be cleaned 
*/
/* 	up someday, beware ! */
  if (itv % 2 == 0) {
/* process horizontal before vertical */
    *ibindx = 1;
    if (itv > 3) {
      *ibindx = -1;
    }
/* right to left */
    *incr = 1;
    if (itv == 2 || itv == 6) {
      *incr = (nx << 1) - 1;
    }
    if (itv == 2 || itv == 4) {
      *incr = -(*incr);
    }
    *ninc = nx - 1;
/* process to end of row before reset */
  } else {
/* process vertical before horizontal */
    *ibindx = nx;
/* processing top to bottom */
    if (itv == 5 || itv == 7) {
      *ibindx = -(nx);
    }
/* nope, bottom to top */
    *incr = (ny - 1) * nx + 1;
/* reset a col to begin processing next col */
    if (itv == 1 || itv == 5) {
      *incr += -2;
    }
    if (itv == 3 || itv == 1) {
      *incr = -(*incr);
    }
    *ninc = ny - 1;
/* process to end of column before reset */
  }
/* now get initial base address */
  if (itv == 2 || itv == 7) {
    *ibase = (ny - 1) * nx + 1;
/* start at lower left */
  } else if (itv == 4 || itv == 5) {
    *ibase = nx * ny;
/* lower right */
  } else if (itv == 0 || itv == 1) {
    *ibase = 1;
/* upper left */
  } else {
    *ibase = nx;
/* upper right */
  }
/* now need to fill the cell properly - figure offsets from base address 
*/
  if (itv == 0) {
    *n2 = 1;
    *n3 = nx + 1;
    *n4 = nx;
  } else if (itv == 3) {
    *n2 = nx;
    *n3 = nx - 1;
    *n4 = -1;
  } else if (itv == 2) {
    *n2 = 1;
    *n3 = -(nx) + 1;
    *n4 = -(nx);
  } else if (itv == 5) {
    *n2 = -(nx);
    *n3 = -(nx) - 1;
    *n4 = -1;
  } else if (itv == 4) {
    *n2 = -1;
    *n3 = -(nx) - 1;
    *n4 = -(nx);
  } else if (itv == 7) {
    *n2 = -(nx);
    *n3 = -(nx) + 1;
    *n4 = 1;
  } else if (itv == 6) {
    *n2 = -1;
    *n3 = nx - 1;
    *n4 = nx;
  } else if (itv == 1) {
    *n2 = nx;
    *n3 = nx + 1;
    *n4 = 1;
  }
    return 0;
}
