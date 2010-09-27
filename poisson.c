/* File poisson.c
   ANA routines to calculate Laplacian and anti-Laplacian */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "action.h"
#include <string.h>		/* for memcpy */

typedef struct {
  int num_levels;
  int *nx;
  int *ny;
  int type;
  pointer data;
  pointer *levels;
} *Pyramid;

int ana_laplace2d(int narg, int ps[])
/* LAPLACE(img) calculates the Laplacian of 2D <img> */
{
  int img, nx, ny, result, i, j;
  pointer src, tgt;

  img = ps[0];
  if (!symbolIsRealArray(img))
    return cerror(NEED_REAL_ARR, img);
  if (array_num_dims(img) != 2)
    return cerror(NEED_2D_ARR, img);
  if (array_type(img) < ANA_FLOAT)
    img = ana_float(1, &img);	/* get temp FLOAT version */
  nx = array_dims(img)[0];
  ny = array_dims(img)[1];
  src.f = array_data(img);

  result = array_clone(img, array_type(img)); /* create output variable */
  tgt.f = array_data(result);

  switch (array_type(img)) {
  case ANA_FLOAT:
    /* top edge */
    /* top left corner */
    *tgt.f++ = src.f[1] + src.f[nx] - 4*src.f[0];
    src.f++;
    /* top middle part */
    for (i = 1; i < nx - 1; i++) {
      *tgt.f++ = src.f[-1] + src.f[1] + src.f[nx] - 4*src.f[0];
      src.f++;
    }
    /* top right corner */
    *tgt.f++ = src.f[-1] + src.f[nx] - 4*src.f[0];
    src.f++;

    /* middle part */
    for (j = 1; j < ny - 1; j++) {
      /* left edge */
      *tgt.f++ = src.f[1] + src.f[-nx] + src.f[nx] - 4*src.f[0];
      src.f++;
      /* middle part */
      for (i = 1; i < nx - 1; i++) {
	*tgt.f++ = src.f[-1] + src.f[1] + src.f[-nx] + src.f[nx] - 4*src.f[0];
	src.f++;
      }
      /* right edge */
      *tgt.f++ = src.f[-1] + src.f[-nx] + src.f[nx] - 4*src.f[0];
      src.f++;
    }

    /* bottom edge */
    /* bottom left corner */
    *tgt.f++ = src.f[1] + src.f[-nx] - 4*src.f[0];
    src.f++;
    /* bottom middle part */
    for (i = 1; i < nx - 1; i++) {
      *tgt.f++ = src.f[-1] + src.f[1] + src.f[-nx] - 4*src.f[0];
      src.f++;
    }
    /* bottom right corner */
    *tgt.f++ = src.f[-1] + src.f[-nx] - 4*src.f[0];
    src.f++;
    break;
  case ANA_DOUBLE:
    /* top edge */
    /* top left corner */
    *tgt.d++ = src.d[1] + src.d[nx] - 4*src.d[0];
    src.d++;
    /* top middle part */
    for (i = 1; i < nx - 1; i++) {
      *tgt.d++ = src.d[-1] + src.d[1] + src.d[nx] - 4*src.d[0];
      src.d++;
    }
    /* top right corner */
    *tgt.d++ = src.d[-1] + src.d[nx] - 4*src.d[0];
    src.d++;

    /* middle part */
    for (j = 1; j < ny - 1; j++) {
      /* left edge */
      *tgt.d++ = src.d[1] + src.d[-nx] + src.d[nx] - 4*src.d[0];
      src.d++;
      /* middle part */
      for (i = 1; i < nx - 1; i++) {
	*tgt.d++ = src.d[-1] + src.d[1] + src.d[-nx] + src.d[nx] - 4*src.d[0];
	src.d++;
      }
      /* right edge */
      *tgt.d++ = src.d[-1] + src.d[-nx] + src.d[nx] - 4*src.d[0];
      src.d++;
    }

    /* bottom edge */
    /* bottom left corner */
    *tgt.d++ = src.d[1] + src.d[-nx] - 4*src.d[0];
    src.d++;
    /* bottom middle part */
    for (i = 1; i < nx - 1; i++) {
      *tgt.d++ = src.d[-1] + src.d[1] + src.d[-nx] - 4*src.d[0];
      src.d++;
    }
    /* bottom right corner */
    *tgt.d++ = src.d[-1] + src.d[-nx] - 4*src.d[0];
    src.d++;
    break;
  }
  
  return result;
}
  
int gauss_seidel_2d2o(pointer b, pointer x, scalar sx, scalar sy, int type,
		      int nx, int ny)
{
  int i, j;
  scalar s;
  
  switch (type) {
  case ANA_FLOAT:
    /* initialization */
    s.f = 1/(2*(sx.f + sy.f));
    /* top left corner */
    *x.f = (sx.f*x.f[1] + sy.f*x.f[nx] - *b.f)*s.f;
    x.f++;
    b.f++;
    /* top middle */
    for (i = 1; i < nx - 1; i++) {
      *x.f = (sx.f*(x.f[-1] + x.f[1]) + sy.f*x.f[nx] - *b.f)*s.f;
      x.f++;
      b.f++;
    }
    /* top right corner */
    *x.f = (sx.f*x.f[-1] + sy.f*x.f[nx] - *b.f)*s.f;
    x.f++;
    b.f++;
    for (j = 1; j < ny - 1; j++) {
      /* left edge */
      *x.f = (sx.f*x.f[1] + sy.f*(x.f[-nx] + x.f[nx]) - *b.f)*s.f;
      x.f++;
      b.f++;
      /* middle */
      for (i = 1; i < nx - 1; i++) {
	*x.f = (sx.f*(x.f[-1] + x.f[1])
		+ sy.f*(x.f[-nx] + x.f[nx])
		- *b.f)*s.f;
	x.f++;
	b.f++;
      }
      /* right edge */
      *x.f = (sx.f*x.f[-1] + sy.f*(x.f[-nx] + x.f[nx]) - *b.f)*s.f;
      x.f++;
      b.f++;
    }
    /* bottom left corner */
    *x.f = (sx.f*x.f[1] + sy.f*x.f[-nx] - *b.f)*s.f;
    x.f++;
    b.f++;
    /* bottom middle */
    for (i = 1; i < nx - 1; i++) {
      *x.f = (sx.f*(x.f[-1] + x.f[1]) + sy.f*x.f[-nx] - *b.f)*s.f;
      x.f++;
      b.f++;
    }
    /* bottom right corner */
    *x.f = (sx.f*x.f[-1] + sy.f*x.f[-nx] - *b.f)*s.f;
    x.f++;
    b.f++;
    break;
  case ANA_DOUBLE:
    /* initialization */
    s.d = 1/(2*(sx.d + sy.d));
    /* top left corner */
    *x.d = (sx.d*x.d[1] + sy.d*x.d[nx] - *b.d)*s.d;
    x.d++;
    b.d++;
    /* top middle */
    for (i = 1; i < nx - 1; i++) {
      *x.d = (sx.d*(x.d[-1] + x.d[1]) + sy.d*x.d[nx] - *b.d)*s.d;
      x.d++;
      b.d++;
    }
    /* top right corner */
    *x.d = (sx.d*x.d[-1] + sy.d*x.d[nx] - *b.d)*s.d;
    x.d++;
    b.d++;
    for (j = 1; j < ny - 1; j++) {
      /* left edge */
      *x.d = (sx.d*x.d[1] + sy.d*(x.d[-nx] + x.d[nx]) - *b.d)*s.d;
      x.d++;
      b.d++;
      /* middle */
      for (i = 1; i < nx - 1; i++) {
	*x.d = (sx.d*(x.d[-1] + x.d[1])
		+ sy.d*(x.d[-nx] + x.d[nx])
		- *b.d)*s.d;
	x.d++;
	b.d++;
      }
      /* right edge */
      *x.d = (sx.d*x.d[-1] + sy.d*(x.d[-nx] + x.d[nx]) - *b.d)*s.d;
      x.d++;
      b.d++;
    }
    /* bottom left corner */
    *x.d = (sx.d*x.d[1] + sy.d*x.d[-nx] - *b.d)*s.d;
    x.d++;
    b.d++;
    /* bottom middle */
    for (i = 1; i < nx - 1; i++) {
      *x.d = (sx.d*(x.d[-1] + x.d[1]) + sy.d*x.d[-nx] - *b.d)*s.d;
      x.d++;
      b.d++;
    }
    /* bottom right corner */
    *x.d = (sx.d*x.d[-1] + sy.d*x.d[-nx] - *b.d)*s.d;
    x.d++;
    b.d++;
    break;
  }
  return 0;
}

int restrict2(pointer b, pointer x, int type, int nx, int ny, scalar sx, 
	      scalar sy, int do_residual, pointer tgt)
{
  int i, j, nx2, ny2;
  pointer r0, r;		/* nx+2 by 3 elements */

  nx2 = nx/2;
  ny2 = ny/2;
  switch (type) {
  case ANA_FLOAT:
    r0.f = malloc(nx*3*sizeof(*r0.f));

    r.f = r0.f + 2*nx;		/* bottom row of r.f */

    /* fill in row */
    if (do_residual) {
      /* top left */
      *r.f++ = sx.f*(x.f[1] - 2*x.f[0]) + sy.f*(x.f[nx] - 2*x.f[0]) - *b.f++;
      x.f++;
      /* top middle */
      for (i = 1; i < nx - 1; i++) {
	*r.f++ = sx.f*(x.f[1] + x.f[-1] - 2*x.f[0]) + sy.f*(x.f[nx] - 2*x.f[0])
	  - *b.f++;
	x.f++;
      }
      /* top right */
      *r.f++ = sx.f*(x.f[-1] - 2*x.f[0]) + sy.f*(x.f[nx] - 2*x.f[0]) - *b.f++;
      x.f++;
    } else {
      memcpy(r.f, x.f, nx*sizeof(*r.f));
      r.f += nx;
      x.f += nx;
    }

    /* do vertically middle part */
    for (j = 0; j < ny2 - 1; j++) {
      r.f = r0.f + nx;		/* to beginning of middle row */
      memcpy(r.f - nx, r.f + nx, nx*sizeof(*r.f));
      /* left column */
      if (do_residual) {
	r.f[0] = sx.f*(x.f[1] - 2*x.f[0])
	  + sy.f*(x.f[nx] + x.f[-nx] - 2*x.f[0]) - b.f[0];
	r.f[nx] = sx.f*(x.f[nx + 1] - 2*x.f[nx])
	  + sy.f*(x.f[2*nx] + x.f[0] - 2*x.f[nx]) - b.f[nx];
	b.f++;
      } else {
	r.f[0] = x.f[0];
	r.f[nx] = x.f[nx];
      }
      r.f++;
      x.f++;
      /* middle columns */
      for (i = 0; i < nx2 - 1; i++) {
	if (do_residual) {
	  r.f[nx] = sx.f*(x.f[nx + 1] + x.f[nx - 1] - 2*x.f[nx])
	    + sy.f*(x.f[2*nx] + x.f[0] - 2*x.f[nx]) - b.f[nx];
	  *r.f++ = sx.f*(x.f[1] + x.f[-1] - 2*x.f[0])
	    + sy.f*(x.f[nx] + x.f[-nx] - 2*x.f[0]) - *b.f++;
	  x.f++;
	  r.f[nx] = sx.f*(x.f[nx + 1] + x.f[nx - 1] - 2*x.f[nx])
	    + sy.f*(x.f[2*nx] + x.f[0] - 2*x.f[nx]) - b.f[nx];
	  *r.f++ = sx.f*(x.f[1] + x.f[-1] - 2*x.f[0])
	    + sy.f*(x.f[nx] + x.f[-nx] - 2*x.f[0]) - *b.f++;
	  x.f++;
	} else {
	  r.f[nx] = x.f[nx];
	  *r.f++ = *x.f++;
	  r.f[nx] = x.f[nx];
	  *r.f++ = *x.f++;
	}
	/*  calculate restricted version */
	*tgt.f++ = r.f[-nx - 3]/16 + r.f[-nx - 2]/8 + r.f[-nx - 1]/16
	  + r.f[-3]/8 + r.f[-2]/4 + r.f[-1]/8
	  + r.f[nx - 3]/16 + r.f[nx - 2]/8 + r.f[nx - 1]/16;
      }
      /* right column */
      if (do_residual) {
	r.f[nx] = sx.f*(x.f[nx + 1] + x.f[nx - 1] - 2*x.f[nx])
	  + sy.f*(x.f[2*nx] + x.f[0] - 2*x.f[nx]) - b.f[nx];
	*r.f++ = sx.f*(x.f[1] + x.f[-1] - 2*x.f[0])
	  + sy.f*(x.f[nx] + x.f[-nx] - 2*x.f[0]) - *b.f++;
	x.f++;
	r.f[nx] = sx.f*(x.f[nx - 1] - 2*x.f[nx])
	  + sy.f*(x.f[2*nx] + x.f[0] - 2*x.f[nx]) - b.f[nx];
	*r.f++ = sx.f*(x.f[-1] - 2*x.f[0])
	  + sy.f*(x.f[nx] + x.f[-nx] - 2*x.f[0]) - *b.f++;
	x.f++;
      } else {
	r.f[nx] = x.f[nx];
	*r.f++ = *x.f++;
	r.f[nx] = x.f[nx];
	*r.f++ = *x.f++;
      }
      /*  calculate restricted version */
      *tgt.f++ = r.f[-nx - 3]/16 + r.f[-nx - 2]/8 + r.f[-nx - 1]/16
	+ r.f[-3]/8 + r.f[-2]/4 + r.f[-1]/8
	+ r.f[nx - 3]/16 + r.f[nx - 2]/8 + r.f[nx - 1]/16;
      x.f += nx;
    }

    /* bottom row */
    r.f = r0.f + nx;		/* to beginning of middle row */
    memcpy(r.f - nx, r.f + nx, nx*sizeof(*r.f));
    /* left column */
    if (do_residual) {
      r.f[0] = sx.f*(x.f[1] - 2*x.f[0])
	+ sy.f*(x.f[nx] + x.f[-nx] - 2*x.f[0]) - b.f[0];
      r.f[nx] = sx.f*(x.f[nx + 1] - 2*x.f[nx])
	+ sy.f*(x.f[0] - 2*x.f[nx]) - b.f[nx];
      b.f++;
    } else {
      r.f[0] = x.f[0];
      r.f[nx] = x.f[nx];
    }
    r.f++;
    x.f++;
    /* middle columns */
    for (i = 0; i < nx2 - 1; i++) {
      if (do_residual) {
	r.f[nx] = sx.f*(x.f[nx + 1] + x.f[nx - 1] - 2*x.f[nx])
	  + sy.f*(x.f[0] - 2*x.f[nx]) - b.f[nx];
	*r.f++ = sx.f*(x.f[1] + x.f[-1] - 2*x.f[0])
	  + sy.f*(x.f[nx] + x.f[-nx] - 2*x.f[0]) - *b.f++;
	x.f++;
	r.f[nx] = sx.f*(x.f[nx + 1] + x.f[nx - 1] - 2*x.f[nx])
	  + sy.f*(x.f[0] - 2*x.f[nx]) - b.f[nx];
	*r.f++ = sx.f*(x.f[1] + x.f[-1] - 2*x.f[0])
	  + sy.f*(x.f[nx] + x.f[-nx] - 2*x.f[0]) - *b.f++;
	x.f++;
      } else {
	r.f[nx] = x.f[nx];
	*r.f++ = *x.f++;
	r.f[nx] = x.f[nx];
	*r.f++ = *x.f++;
      }
      /*  calculate restricted version */
      *tgt.f++ = r.f[-nx - 3]/16 + r.f[-nx - 2]/8 + r.f[-nx - 1]/16
	+ r.f[-3]/8 + r.f[-2]/4 + r.f[-1]/8
	+ r.f[nx - 3]/16 + r.f[nx - 2]/8 + r.f[nx - 1]/16;
    }
    /* right column */
    if (do_residual) {
      r.f[nx] = sx.f*(x.f[nx + 1] + x.f[nx - 1] - 2*x.f[nx])
	+ sy.f*(x.f[0] - 2*x.f[nx]) - b.f[nx];
      *r.f++ = sx.f*(x.f[1] + x.f[-1] - 2*x.f[0])
	+ sy.f*(x.f[nx] + x.f[-nx] - 2*x.f[0]) - *b.f++;
      x.f++;
      r.f[nx] = sx.f*(x.f[nx - 1] - 2*x.f[nx])
	+ sy.f*(x.f[0] - 2*x.f[nx]) - b.f[nx];
      *r.f++ = sx.f*(x.f[-1] - 2*x.f[0])
	+ sy.f*(x.f[nx] + x.f[-nx] - 2*x.f[0]) - *b.f++;
      x.f++;
    } else {
      r.f[nx] = x.f[nx];
      *r.f++ = *x.f++;
      r.f[nx] = x.f[nx];
      *r.f++ = *x.f++;
    }
    /*  calculate restricted version */
    *tgt.f++ = r.f[-nx - 3]/16 + r.f[-nx - 2]/8 + r.f[-nx - 1]/16
      + r.f[-3]/8 + r.f[-2]/4 + r.f[-1]/8
      + r.f[nx - 3]/16 + r.f[nx - 2]/8 + r.f[nx - 1]/16;

    free(r0.f);
    break;
  }
}

int restrict_residual(pointer b, pointer x, int type, int nx, int ny,
		      scalar sx, scalar sy, pointer tgt)
{
  return restrict2(b, x, type, nx, ny, sx, sy, 1, tgt);
}

int restrict(pointer x, int type, int nx, int ny, pointer tgt)
{
  scalar dummy;

  dummy.d = 0;
  return restrict2(x, x, type, nx, ny, dummy, dummy, 0, tgt);
}

int ana_antilaplace2d(int narg, int ps[])
{
  int img, result = ANA_ERROR, nx, ny, type, nx2, ny2, nlevel, i, nelem;
  pointer src, tgt;
  scalar sx, sy;
  Pyramid pyramid;

  img = ps[0];
  if (!symbolIsRealArray(img))
    return cerror(NEED_REAL_ARR, img);
  if (array_num_dims(img) != 2)
    return cerror(NEED_2D_ARR, img);
  if (array_type(img) < ANA_FLOAT)
    img = ana_float(1, &img);	/* get temp FLOAT version */
  type = array_type(img);
  nx = array_dims(img)[0];
  ny = array_dims(img)[1];
  src.f = array_data(img);

  nx2 = nx;
  ny2 = ny;
  nlevel = nelem = 0;
  while (nx2 > 0 || ny2 > 0) {
    nelem += nx2*ny2;
    nx2 /= 2;
    ny2 /= 2;
    nlevel++;
  }
  ALLOCATE(pyramid, 1);
  if (!pyramid)
    return cerror(ALLOC_ERR, 0);
  ALLOCATE(pyramid->nx, nlevel);
  ALLOCATE(pyramid->ny, nlevel);
  switch (type) {
  case ANA_FLOAT:
    ALLOCATE(pyramid->data.f, nelem);
    break;
  case ANA_DOUBLE:
    ALLOCATE(pyramid->data.d, nelem);
    break;
  }
  ALLOCATE(pyramid->levels, nlevel);
  if (!pyramid->nx || !pyramid->ny || !pyramid->data.f || !pyramid->levels) {
    result = cerror(ALLOC_ERR, 0);
    goto free_pyramid;
  }
  pyramid->num_levels = nlevel;
  nx2 = nx;
  ny2 = ny;
  pyramid->type = type;
  switch (type) {
  case ANA_FLOAT:
    tgt.f = pyramid->data.f;
    memcpy(tgt.f, src.f, nx*ny*sizeof(*src.f));
    for (i = 0; i < nlevel; i++) { 
      pyramid->levels[i].f = tgt.f;
      pyramid->nx[i] = nx2;
      pyramid->ny[i] = ny2;
      tgt.f += nx2*ny2;
      if (nx2 > 1)
	nx2 /= 2;
      if (ny2 > 1)
	ny2 /= 2;
    }
    sx.f = sy.f = 1;
    break;
  case ANA_DOUBLE:
    tgt.d = pyramid->data.d;
    memcpy(tgt.d, src.d, nx*ny*sizeof(*src.d));
    for (i = 0; i < nlevel; i++) {
      pyramid->levels[i].d = tgt.d;
      pyramid->nx[i] = nx2;
      pyramid->ny[i] = ny2;
      tgt.d += nx2*ny2;
      if (nx2 > 1)
	nx2 /= 2;
      if (ny2 > 1)
	ny2 /= 2;
    }
    sx.d = sy.d = 1;
    break;
  }

  nx2 = nx;
  ny2 = ny;
  for (i = 0; i < nlevel - 1; i++) {
    restrict(pyramid->levels[i], type, nx2, ny2, pyramid->levels[i + 1]);
    if (nx2 > 1)
      nx2 /= 2;
    if (ny2 > 1)
      ny2 /= 2;
  }

  //  gauss_seidel_2d2o(src, tgt, sx, sy, type, nx, ny);

  {
    int ndim, *dims;

    ndim = array_num_dims(img);
    ALLOCATE(dims, ndim + 1);
    memcpy(dims, array_dims(img), ndim*sizeof(*dims));
    dims[ndim] = 2;
    result = array_scratch(type, ndim + 1, dims);
    free(dims);
  }
  memcpy(array_data(result), pyramid->levels[0].f, 
	 nx*ny*2*ana_type_size[type]);
  
 free_pyramid:
  free(pyramid->data.f);
  free(pyramid->levels);
  free(pyramid->ny);
  free(pyramid->nx);
  free(pyramid);
  return result;
}

/*

P[k] = problem of solving a discrete Poisson equation T x = b for x on
a grid of size ~ 2^k

(T x)[i,j] = (x[i-1,j] + x[i+1,j] + x[i,j-1] + x[i,j+1] - 4*x[i,j])/h[k]

Multigrid v-cycle:

function MGV(i, b[i], x[i])
if i == 0 then			// 1 unknown
 solve P[0] exactly for x[0]
 return x[0]
else
 dampen high-frequency errors in x[i]
 calculate errors r[i] = T x[i] - b[i]
 restrict r to coarser grid: r[i] => r[i-1]
 calculate d[i-1] = MGV(i-1, r[i-1], 0)
 interpolate d to finer grid: d[i-1] => d[i]
 improve x: x[i] = x[i] - d[i]
 dampen high-frequency errors in x[i]
 return x[i]
endif

*/

