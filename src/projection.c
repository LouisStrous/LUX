/* This is file projection.c.

Copyright 2013 Louis Strous

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
/* File projection.c */
/* LUX routines dealing with 2D projections of 3D objects. */
/* This is file projection.c which contains code for 3D projections and */
/* plots.  All projective stuff is done in homogeneous coordinates. */
/* See "Fundamentals of Interactive Computer Graphics" by J.D. Foley */
/* and A. van Dam, Addison Wesley Publishing Co. */

#include <math.h>
#include <float.h>
#include <string.h>
#include <stdlib.h>
#include "install.h"
#include "action.h"

Int	tkplot(Float, Float, Int, Int),
  createFullProjection(Float *matrix, Float *perspective, Float *oblique),
  lux_erase(Int, Int []);

/* A general projection matrix P has 4 by 4 elements, and includes scaling, */
/* rotation, translation, and a perspective transformation. */
/* A point X = (x,y,z) has homogeneous coordinates k*(x,y,z,1) with */
/* arbitrary non-zero k.  The projection of X according to matrix P is */
/* PX in homogeneous coordinates. */

Float	defaultProjection[16] = 
{ 1.0, 0.0, 0.0, 0.0,
  0.0, 1.0, 0.0, 0.0,
  0.0, 0.0, 1.0, 0.0,
  0.0, 0.0, 0.0, 1.0 };
Float	perspective[3] = { 0.5, 0.5, 0.0 }, oblique[2], projection[16],
  projectMatrix[16] = 
{ 1.0, 0.0, 0.0, 0.0,
  0.0, 1.0, 0.0, 0.0,
  0.0, 0.0, 1.0, 0.0,
  0.0, 0.0, 0.0, 1.0 };
/* Ultrix RISC cc has trouble with external declarations of arrays */
/* (they tend to refer to address 0x0), but no trouble with external */
/* declarations of pointers to arrays, so we define pointers to the */
/* above arrays and refer to those in external declarations.  LS 13jul94 */
Float	*currentOblique = oblique, *currentProjection = projection,
	*currentPerspective = perspective;
extern Int	projectSym, iorder, iyhigh, ipltyp;
extern Float	xmin, xmax, ymin, ymax, xfac, yfac, wxb, wxt, wyb, wyt,
  *p3d;
extern char	useProjection;
Float	zmin, zmax, irzf = 1, dvz, wzb, wzt = 1, ticzr = 0.5, ticz = 0.01;
Int	ifzz = 1, ndlabz = 2, ndz = 8, ndzs, ilabz = 1, fstepz = 0;
#define CURRENT		0	/* current coordinate system */
#define ORIGINAL	1	/* original coordinate system */
Int	projCoords = CURRENT;
/*-----------------------------------------------------------------------*/
Int setProjection(Float *projection, Float *matrix, Int size)
     /* copy user-supplied matrix to current projection matrix. */
     /* The user-supplied matrix may have 12 or 16 elements. */
{
  switch (size) {
    case 16:
      memcpy(projection, matrix, 16*sizeof(Float));
      break;
    case 12:
      memcpy(projection, matrix, 12*sizeof(Float));
      break;
    default:
      return -1;
  }
  return 1;
}
/*-----------------------------------------------------------------------*/
Int translateProjection(Float *projection, Float tx, Float ty, Float tz)
/* enter a translation over (tx,ty,tz) into projection matrix <projection> */
{
  Int	i;
  Float	t;

  if (projCoords == CURRENT)
    for (i = 0; i < 3; i++) {
      t = *projection++ * tx;
      t += *projection++ * ty;
      t += *projection++ * tz;
      *projection++ += t;
    }
  else {			/* original coordinate system */
    projection[3] += tx;
    projection[7] += ty;
    projection[11] += tz;
  }
  return 1;
}
/*-----------------------------------------------------------------------*/
Int rotateProjection(Float *projection, Float ax, Float ay, Float az)
/* enter rotations over <ax> degrees along the X axis, <ay> degrees */
/* along the Y axis, and <az> degrees along the Z axis, into projection */
/* matrix <projection> */
{
  Float	sx, sy, sz, rot[9], temp[12], *p, *q, *r, cx, cy, cz;
  Int	i, j;

  /* first calculate sines and cosines */
  ax *= DEG;
  cx = cos(ax);
  sx = sin(ax);
  ay *= DEG;
  cy = cos(ay);
  sy = sin(ay);
  az *= DEG;
  cz = cos(az);
  sz = sin(az);
  /* next, calculate the complete rotation matrix */
  rot[0] = cy*cz;
  rot[1] = sx*sy;
  rot[2] = cx*sy;
  rot[3] = cy*sz;
  rot[4] = cx*cz + rot[1]*sz;
  rot[1] = rot[1]*cz - cx*sz;
  rot[5] = rot[2]*sz - sx*cz;
  rot[2] = rot[2]*cz + sx*sz;
  rot[6] = -sy;
  rot[7] = sx*cy;
  rot[8] = cx*cy;
  /* now calculate the new projection matrix and store in temp */
  p = projection;
  q = temp;
  r = rot;
  if (projCoords == CURRENT) {	/* rotate around current axes */
    for (j = 0; j < 3; j++) {
      for (i = 0; i < 4; i++) {
	*q = *p * *r;
	*q += *++r * *(p += 4);
	*q++ += *++r * *(p += 4);
	p -= 7;
	r -= 2;
      } 
      p = projection;
      r += 3;
    }
    q = temp;
  } else {			/* original coordinate system */
    for (j = 0; j < 3; j++) {
      for (i = 0; i < 3; i++) {
	*q = *p * *r;		/* project around "original" axes */
	*q += *++p * *(r += 3);
	*q++ += *++p * *(r += 3);
	p -= 2;
	r -= 5;
      }
      p += 4;
      q++;
      r = rot;
    }
    p = projection;
    q = temp;
    q[3] = p[3];
    q[7] = p[7];
    q[11] = p[11];
  }
  /* now copy temp to projection matrix */
  for (i = 0; i < 12; i++)
    *p++ = *q++;
  return 1;
}
/*-----------------------------------------------------------------------*/
Int scaleProjection(Float *projection, Float sx, Float sy, Float sz)
/* enter scalings with factor <sx> along the X axis, <sy> along the Y axis, */
/* and <sz> along the Z axis into projection matrix <projection> */
{
  Int	i;

  if (projCoords == CURRENT) {
    for (i = 0; i < 4; i++)
      *projection++ *= sx;
    for (i = 0; i < 4; i++)
      *projection++ *= sy;
    for (i = 0; i < 4; i++)
      *projection++ *= sz;
  } else				/* original coordinate system */
    for (i = 0; i < 3; i++) {
      *projection++ *= sx;
      *projection++ *= sy;
      *projection++ *= sz;
      projection++;
    }
  return 1;
}
/*-----------------------------------------------------------------------*/
Int multiplyProjection(Float *projection, Float *matrix)
/* multiply projection <matrix> into projection matrix <projection> */
{
  Float	result[16], *p = projection, *q = matrix, *r = result;
  Int	i, j, k;

  for (k = 0; k < 4; k++) {
    for (j = 0; j < 4; j++) {
      *r = 0.0;
      for (i = 0; i < 4; i++) {
	*r += *p++ * *q;
	q += 4;
      }
      p -= 4;
      q -= 15;
      r++;
    }
    q -= 19;
  }
  memcpy(projection, result, 16*sizeof(Float));
  return 1;
}
/*-----------------------------------------------------------------------*/
Int lux_projection(Int narg, Int ps[])
/* change the projection parameters.  The three transformations */
/* translation, rotation, and scaling must be added into the projection */
/* matrix in the opposite order from the one in which they are to be */
/* applied to the data points. */
/* syntax:  PROJECT[,matrix,translate,rotate,scale,perspective,oblique] */
/* mode keywords:  /RESET (1) -> reset to default */
/*  /ORIGINAL (2) -> use original coordinate system instead of current */
/* the general matrix has the form
      r11 r21 r31 t1
      r12 r22 r32 t2
      r13 r23 r33 t3
        0   0   0  1
   where the r are due to rotations, and the t due to translations. */
/* projection of vector (x,y,z) is done by calculating M.column(x,y,z,1) */
/* to (x',y',z',a') and then (x'/a',y'/a',z'/a') = (x'',y'',z''). */
/* Then, (x'',y'') are the screen coordinates, and z'' is the Z-value. */
/* The x, y, z form a right-handed coordinate system. */
/* To add a transformation T to M in the current (screen) coordinates, */
/* calculate T.M.  To add in the original (data cube) coordinates, */
/* calculate M.T. */
{
  Int	iq, n;
  Float	*arg, *q;

  if (internalMode & 1)	{	/* /RESET: reset to default */
    memcpy(projectMatrix, defaultProjection, sizeof(defaultProjection));
    currentOblique[0] = currentOblique[1] = 0.0;
    currentPerspective[0] = currentPerspective[1] = 0.5;
    currentPerspective[2] = 0.0;
  }
  projCoords = (internalMode & 2)? ORIGINAL: CURRENT;
  if (narg-- && (iq = *ps++)) {		/* matrix */
    if (symbol_class(iq) != LUX_ARRAY)
      return cerror(NEED_ARR, iq);
    iq = lux_float(1, &iq);	/* ensure FLOAT */
    n = array_size(iq);
    q = (Float *) array_data(iq);
    if (setProjection(projectMatrix, q, n) < 0)
      return cerror(NEED_4x4_ARR, iq);
  }
  /* translation over [t1, t2, t3]: */
  /*      1 0 0 t1
     T =  0 1 0 t2 ;  new = old . T
          0 0 1 t3
          0 0 0  1                   */
  if (narg && (iq = *ps++)) {		/* translation */
    narg--;
    if (symbol_class(iq) != LUX_ARRAY)
      return cerror(NEED_ARR, iq);
    iq = lux_float(1, &iq);
    n = array_size(iq);
    if (n != 3)
      return cerror(NEED_3_ARR, iq);
    arg = (Float *) array_data(iq);
    translateProjection(projectMatrix, arg[0], arg[1], arg[2]);
  }    
  /* rotation over angle a1 around the x axis:
         1       0        0 0
    R =  0 cos(a1) -sin(a1) 0
         0 sin(a1)  cos(a1) 0
         0       0        0 1
     rotation over angle a2 around the y axis:
       cos(a2) 0 sin(a2) 0
             0 1       0 0
      -sin(a2) 0 cos(a2) 0
             0 0       0 1
     rotation over angle a3 around the z axis:
       cos(a3) -sin(a3) 0 0
       sin(a3)  cos(a3) 0 0
             0        0 1 0
             0        0 0 1
     in all cases,  new = R . old */
  if (narg && (iq = *ps++)) {	/* rotation */
    narg--; 
    if (symbol_class(iq) != LUX_ARRAY)
      return cerror(NEED_ARR, iq);
    iq = lux_float(1, &iq);
    n = array_size(iq);
    if (n != 3)
      return cerror(NEED_3_ARR, iq);
    q = arg = (Float *) array_data(iq);
    rotateProjection(projectMatrix, q[0], q[1], q[2]);
  }
  /* scaling with factors [s1, s2, s3]:
       s1  0  0 0
        0 s2  0 0
        0  0 s3 0
        0  0  0 1 */
  if (narg && (iq = *ps++)) {	/* scaling */
    narg--;
    if (symbol_class(iq) != LUX_ARRAY)
      return cerror(NEED_ARR, iq);
    iq = lux_float(1, &iq);
    n = array_size(iq);
    if (n != 3)
      return cerror(NEED_3_ARR, iq);
    arg = (Float *) array_data(iq);
    scaleProjection(projectMatrix, arg[0], arg[1], arg[2]);
  }
  /* selection of projection method.  Choice:  perspective or oblique */
  /* perspective requires 3 parameters:  the distance of the eye behind */
  /* the projection plane, and the two relative screen coordinates of */
  /* the center of the projection.  oblique requires only 2 parameters, */
  /* a length and an angle. */
  if (narg >= 2 && *ps && ps[1]) {
    puts("WARNING - Both perspective and oblique projections specified!");
    puts("Using perspective projection.");
    narg--;
    ps[1] = 0;
  }
  if (narg && (iq = *ps++)) {	/* perspective projection */
    narg--;
    if (symbol_class(iq) == LUX_ARRAY) {
      /* must have three elements:  (x, y, z) */
      iq = lux_float(1, &iq);
      n = array_size(iq);
      if (n != 3)
	return cerror(NEED_3_ARR, ps[-1]);
      arg = (Float *) array_data(iq);
      memcpy(currentPerspective, arg, 3*sizeof(Float));
    } else
      /* only distance of eye to projection plane */
      currentPerspective[2] = float_arg(iq);
    if (currentPerspective[2]) {
      currentPerspective[2] = -1.0/currentPerspective[2];
      /* reset oblique to off */
      currentOblique[0] = currentOblique[1] = 0.0;
    } else {			/* turn off perspective */
      currentOblique[0] = 1.0;
      currentOblique[1] = 0.0;
    }
  }
  if (narg && (iq = *ps++)) {	/* oblique projection */
    narg--;
    if (symbol_class(iq) != LUX_ARRAY)
      return cerror(NEED_ARR, iq);
    iq = lux_float(1, &iq);
    n = array_size(iq);
    if (n != 2)
      return cerror(NEED_2_ARR, ps[-1]);
    arg = (Float *) array_data(iq);
    memcpy(currentOblique, arg, 2*sizeof(Float));
    /* reset perspective */
    currentPerspective[0] = currentPerspective[1] = 0.5;
    currentPerspective[2] = 0.0; /* i.e. off */
  }
  createFullProjection(projectMatrix, currentPerspective, currentOblique);
  memcpy(p3d, currentProjection, 16*sizeof(Float));
  return 1;
}
/*---------------------------------------------------------------------*/
Int createFullProjection(Float *matrix, Float *perspective, Float *oblique)
/* combine rotation/scale/translation matrix with perspective, obliquity, */
/* and put in currentProjection.  perspective and obliquity determination */
/* may be done only once for each projection, but rotations, scalings, */
/* and translations may be compounded as often as desired. */
  /* we leave the z coordinate intact;  the projected point has z = 0, */
  /* and the original z coordinate is needed in z-buffer algorithms */
{
  Float	*p1, *p2, c;
  Int	i;

  memcpy(currentProjection, matrix, 16*sizeof(Float));
  /* perspective transformation */
  /*   1 0 -xc/p 0
       0 1 -yc/p 0
       0 0     q 0
       0 0  -1/p 1
     q = 0 -> projection unto plane z = 0.
     actually, we wish to keep z for possible use in z-buffer
     algorithms, so set q = -1. */
  if (perspective[2]) {
    c = perspective[0]*perspective[2];
    p1 = currentProjection;
    p2 = currentProjection + 8;
    for (i = 0; i < 4; i++)
      *p1++ += *p2++ * c;
    p2 -= 4;
    c = perspective[1]*perspective[2];
    for (i = 0; i < 4; i++)
      *p1++ += *p2++ * c;
    c = perspective[2];
    for (i = 0; i < 4; i++)
      *p2++ += *p1++ * c;
  }
  /* oblique transformation */
  /*  1 0 -l*cos(a) 0
      0 1 -l*sin(a) 0
      0 0         q 0
      0 0         0 1
    q = 0 -> projection unto plane z = 0.
    for z-buffer, keep z -> q = -1. */
  if (oblique[0] != 0.0) {
    c = -oblique[0]*cos(oblique[1]*DEG);
    p1 = currentProjection;
    p2 = currentProjection + 8;
    for (i = 0; i < 4; i++)
      *p1++ += *p2++ * c;
    p2 -= 4;
    c = -oblique[0]*sin(oblique[1]*DEG);
    for (i = 0; i < 4; i++)
      *p1++ += *p2++ * c;
  }
  return 1;
}
/*---------------------------------------------------------------------*/
Float	projected[4];
Int transform(Float x, Float y, Float z)
/* transforms point (x,y,z,1) according to the current projection matrix */
{
  Float	*pm, *p;
  Int	i;

  p = projected;
  pm = currentProjection;
  for (i = 0; i < 4; i++) {
    *p = *pm++ * x;
    *p += *pm++ * y;
    *p += *pm++ * z;
    *p++ += *pm++;
  }
  return 1;
}
/*---------------------------------------------------------------------*/
Int transform4(Float x, Float y, Float z, Float t)
/* transforms point (x,y,z,t) according to the current projection matrix */
{
  Float	*pm, *p;
  Int	i;

  p = projected;
  pm = currentProjection;
  for (i = 0; i < 4; i++) {
    *p = *pm++ * x; 
    *p += *pm++ * y;
    *p += *pm++ * z;
    *p++ += *pm++ * t;
  }
  return 1;
}
/*---------------------------------------------------------------------*/
Int project(Float x, Float y, Float z)
/* projects point (x,y,z) according to the current projection matrix */
/* and transforms from homogeneous to ordinary coordinates */
{
  Float	*pm, *p;
  Int	i;

  p = projected;
  pm = currentProjection;
  for (i = 0; i < 4; i++) {
    *p = *pm++ * x;
    *p += *pm++ * y;
    *p -= *pm++ * z;		/* else the x y z axes are not right-handed */
    *p++ += *pm++;
  }
  /* scale first three homogeneous coordinates with the fourth one */
  p = projected;
  for (i = 0; i < 3; i++)
    *p++ /= projected[3];
  return 1;
}
/*---------------------------------------------------------------------*/
Int lux_project(Int narg, Int ps[])
/* development routine.  uses project to project a 3-element vector */
/* using the current projection matrix */
{
  Int	iq, n, result;
  Float	*p, *p2;

  iq = *ps;
  if (symbol_class(iq) != LUX_ARRAY)
    return cerror(NEED_ARR, iq);
  iq = lux_float(1, &iq);
  n = array_size(iq);
  if (n % 3)
    return luxerror("Need multiple of three elements", *ps);
  p = (Float *) array_data(iq);
  result = array_clone(iq, LUX_FLOAT);
  p2 = (Float *) array_data(result);
  createFullProjection(projectMatrix, currentPerspective, currentOblique);
  n /= 3;
  memcpy(p3d, currentProjection, 16*sizeof(Int));
  while (n--) {
    project(*p, p[1], p[2]);
    memcpy(p2, projected, 3*sizeof(Float));
    p += 3;
    p2 += 3;
  };
  return result;
}
/*---------------------------------------------------------------------*/
Int tkproj(Float x, Float y, Float z, Int mode)
/* projects the point and draws a line to the projection
   mode: 0 -> move but don't draw; 1 -> draw */
{
  project(x, y, z);
  tkplot(projected[0], projected[1], mode, 0);
  return 1;
}
/*---------------------------------------------------------------------*/
Int lux_fitUnitCube(Int narg, Int ps[])
/* scales and translates the projection so that the projected unit cube */
/* fits nicely on the screen */
{
  Float	minx, maxx, miny, maxy, s, sy;

  createFullProjection(projectMatrix, currentPerspective, currentOblique);
  project(0, 0, 0);
  minx = maxx = *projected;
  miny = maxy = projected[1];
  project(1, 0, 0);
  if (*projected < minx)
    minx = *projected;
  else if (*projected > maxx)
    maxx = *projected;
  if (projected[1] < miny)
    miny = projected[1];
  else if (projected[1] > maxy)
    maxy = projected[1];
  project(0, 1, 0);
  if (*projected < minx)
    minx = *projected;
  else if (*projected > maxx)
    maxx = *projected;
  if (projected[1] < miny)
    miny = projected[1];
  else if (projected[1] > maxy)
    maxy = projected[1];
  project(0, 0, 1);
  if (*projected < minx)
    minx = *projected;
  else if (*projected > maxx)
    maxx = *projected;
  if (projected[1] < miny)
    miny = projected[1];
  else if (projected[1] > maxy)
    maxy = projected[1];
  project(1, 1, 0);
  if (*projected < minx)
    minx = *projected;
  else if (*projected > maxx)
    maxx = *projected;
  if (projected[1] < miny)
    miny = projected[1];
  else if (projected[1] > maxy)
    maxy = projected[1];
  project(1, 0, 1);
  if (*projected < minx)
    minx = *projected;
  else if (*projected > maxx)
    maxx = *projected;
  if (projected[1] < miny)
    miny = projected[1];
  else if (projected[1] > maxy)
    maxy = projected[1];
  project(0, 1, 1);
  if (*projected < minx)
    minx = *projected;
  else if (*projected > maxx)
    maxx = *projected;
  if (projected[1] < miny)
    miny = projected[1];
  else if (projected[1] > maxy)
    maxy = projected[1];
  project(1, 1, 1);
  if (*projected < minx)
    minx = *projected;
  else if (*projected > maxx)
    maxx = *projected;
  if (projected[1] < miny)
    miny = projected[1];
  else if (projected[1] > maxy)
    maxy = projected[1];
  s = 0.9/(maxx - minx);
  sy = 0.9/(maxy - miny);
  if (sy < s)
    s = sy;
  /* scale so the cube fits on screen */
  projectMatrix[0] *= s;
  projectMatrix[5] *= s;
  projectMatrix[10] *= s;
  /* translate so the cube is on screen */
  projectMatrix[3] -= (maxx + minx - 1)/2;
  projectMatrix[7] -= (maxy + miny - 1)/2;
  return 1;
}
/*---------------------------------------------------------------------*/
Int axis(Float ds, Float t, Float tr, Float v, Float dv, Int nt,
	 Int nd, Int flip)
/* draws an x-axis in the current 3D projection, starting at the origin,
   consisting of <nd> divisions of length <ds> each, with major ticks
   of size <t> in the y-direction every <nt> divisions, with minor ticks
   of size <t>*<tr>, with <v> the value of the first label, <dv> the change
   value per division, and <flip> non-zero if the label has to be read
   in the y-direction rather than in the x-direction. */
/* at present, every point in every character in every label is projected.
   This really isn't necessary, unless one end of the axis is much closer to
   the eye than the other end.  This will be changed at some point. */
{
  Float	s = 0, xp, yp;
  Int	i;

  tkproj(0, 0, 0, 0);
  xp = projected[0];
  yp = projected[1];
  tkproj(0, t, 0, 1);		/* major tick mark */
  if (dv != 0.0) {
    useProjection = 1;
    /*    if (nlabel(v, 0, 0, flip? 4: 2, 0) < 0)
	  return LUX_ERROR; */
    useProjection = 0;
  }
  tkplot(xp, yp, 0, 0);		/* back to axis */
  for (i = 1; i <= nd; i++) {
    s += ds;
    tkproj(s, 0, 0, 1);		/* next part of axis proper */
    xp = projected[0];
    yp = projected[1];
    if (i % nt)			/* minor tick mark */
      tkproj(s, t*tr, 0, 1);
    else {			/* major tick mark */
      tkproj(s, t, 0, 1);
      if (dv != 0.0) {
	v += dv*nt;
	useProjection = 1;
	/* if (nlabel(v, s, 0, flip? 4: 2, 0) < 0)
	   return LUX_ERROR; */
	useProjection = 0;
      }
    }
    tkplot(xp, yp, 0, 0);
  }
  return 1;
}
/*---------------------------------------------------------------------*/
Int lux_plot3d(Int narg, Int ps[])
/* development routine. */
{
  Int	iq, nx, ny, i, ixlog, iylog, izlog;
  Float	*src, x, y, *p1, *p2;
  extern Int	ier, ipltyp, ifz, ifzx, ndlabx, ndlaby, irxf, iryf, ndxs,
    ndys, ndx, nd, ilabx, ilaby, tkCoordSys;
  extern Float	plims[], xmin, ymin, xmax, ymax, dv, dvx, wxb, wxt, wyb, wyt,
    ticx, ticy, ticxr, ticyr;
  Int	setl(Float *, Float *, Int *, Float *, Int, Float, Float);
  Int	sform(Float, Float);
  Int	mm(Float *, Int, Float *, Float *);
  Float	thisProjection[16], bbb[4], btb[4], btt[4], dx, dy, dz;
  char	hide;

  hide = (internalMode & 1);	/* hidden-line removal? */
  iq = *ps;			/* data */
  if (symbol_class(iq) != LUX_ARRAY)
    return cerror(NEED_ARR, iq);
  iq = lux_float(1, &iq);
  if (array_num_dims(iq) != 2)
    return cerror(NEED_2D_ARR, *ps);
  nx = array_dims(iq)[0];
  ny = array_dims(iq)[1];
  src = (Float *) array_data(iq);
  /* determine data cube boundaries */
  ixlog = ipltyp % 2;
  iylog = (ipltyp/2) % 2;
  izlog = (ipltyp/4) % 2;
  if (!ifz && !iylog) 
    ymin = MIN(ymin, 0.0);
  if (!ifzx && !ixlog)
    xmin = MIN(xmin, 0.0);
  if (!ifzz && !izlog)
    zmin = MIN(zmin, 0.0);
  if (!plims[0]) 
    xmin = 0;
  if (!plims[1])
    xmax = nx - 1;
  if (!plims[2])
    ymin = 0;
  if (!plims[3])
    ymax = ny - 1;
  if (!plims[4] || !plims[5])
    mm(src, nx*ny, &zmax, &zmin);
  i = 0;
  if (zmin < -FLT_MAX) {
    zmin = -PLOT_INFTY; 
    i = 1;
  }
  if (zmax > FLT_MAX) {
    zmax = PLOT_INFTY; 
    i = 1;
  }
  if (i) 
    puts("WARNING - Infinity in plot data");
  if (zmax == zmin)
    zmax = zmin + 1.0;
  ndlabx = MAX(ndlabx, 1);
  ndlaby = MAX(ndlaby, 1);
  ndlabz = MAX(ndlabz, 1);
  /* check for rounding and get limits and tick spacings */
  if (!iryf && !iylog) {
    nd = 8;
    if (ndys > 0)
      nd = ndys;		/* # divisions on axis */
    dv = (ymax - ymin)/(Float) nd; /* data per division */
  } else
    setl(&ymax, &ymin, &nd, &dv, iylog, wyb, wyt);
  if (!irxf && !ixlog) {
    ndx = 8;
    if (ndxs > 0)
      ndx = ndxs;		/* # divisions on axis */
    dvx = (xmax - xmin)/(Float) ndx; /* data per division */
  } else
    setl(&xmax, &xmin, &ndx, &dvx, ixlog, wxb, wxt);
  if (!irzf && !izlog) {
    ndz = 8;
    if (ndzs > 0)
      ndz = ndzs;		/* # divisions on axis */
    dvz = (zmax - zmin)/(Float) ndz; /* data per division */
  }
  else
    setl(&zmax, &zmin, &ndz, &dvz, izlog, wzb, wzt);
  if (ier)
    lux_erase(0, 0);		/* erase screen */
  /* we draw all three axes by using routine axis, which draws an x axis. */
  /* by manipulating the projection matrix, we get the y and z axes through */
  /* the same routine.*/
  tkCoordSys = LUX_DVI;
  createFullProjection(projectMatrix, currentPerspective, currentOblique);
  memcpy(p3d, currentProjection, 16*sizeof(Float));

  if (internalMode & 2) {	/* /CUBE */
    /* for debugging purposes: draw unit cube */
    tkproj(0,0,0,0); 
    tkproj(1,0,0,1);
    tkproj(1,1,0,1);
    tkproj(0,1,0,1); 
    tkproj(0,0,0,1);
    tkproj(0,0,1,1);
    tkproj(0,1,1,1);
    tkproj(1,1,1,1); 
    tkproj(1,0,1,1);
    tkproj(0,0,1,1);
    tkproj(0,1,0,0);
    tkproj(0,1,1,1);
    tkproj(1,1,0,0);
    tkproj(1,1,1,1);
    tkproj(1,0,0,0);
    tkproj(1,0,1,1);
  }

  /* recapitulate:  projectMatrix holds all but perspective/oblique. */
  /* projection is a matrix which gets set by createFullProjection and is */
  /* used by project(), and currentProjection is a pointer to it. */
  /* thisProjection is a local matrix that we use to build the required */
  /* projection for each axis.  We need a local one because we use */
  /* project() in building the matrix, so currentProjection is out. */

  i = 0;
  if (ilabx && ticx) {
    transform(wxb, wyb, wzb);
    i = 1;
    memcpy(bbb, projected, 4*sizeof(Float));
  }
  if (ilaby && ticy) {
    transform(wxb, wyt, wzb);
    i = 1;
    memcpy(btb, projected, 4*sizeof(Float));
  }
  if (ilabz && ticz) {
    transform(wxb, wyt, wzt);
    i = 1;
    memcpy(btt, projected, 4*sizeof(Float));
  }
  memcpy(thisProjection, currentProjection, 16*sizeof(Float));
  /* draw x axis */
  if (!ilabx || !ticx) {	/* just a line */
    tkproj(wxb, wyb, wzb, 0);
    tkproj(wxt, wyb, wzb, 1);
  } else {
    if (ilabx)
      sform(wxb, wxt);
    p1 = currentProjection;
    for (i = 0; i < 3; i++) {
      *p1++ *= wxt - wxb;
      p1 += 2;
      *p1++ = bbb[i];
    }
    currentProjection[15] = bbb[3];
    axis(dvx/(xmax - xmin), ticx, ticxr, xmin, dvx,
	 ndlabx, ndx, 0);
  }
  /* draw y axis */
  if (!ilaby || !ticy) {	/* just a line */
    tkproj(wxb, wyb, wzb, 0);
    tkproj(wxb, wyt, wzb, 1);
  } else {
    if (ilaby)
      sform(wyb, wyt);
    currentProjection[0] = thisProjection[1]*(wyb - wyt);
    currentProjection[1] = thisProjection[0];
    currentProjection[3] = btb[0];
    currentProjection[4] = thisProjection[5]*(wyb - wyt);
    currentProjection[5] = thisProjection[4];
    currentProjection[7] = btb[1];
    currentProjection[8] = thisProjection[9]*(wyb - wyt);
    currentProjection[9] = thisProjection[8];
    currentProjection[11] = btb[2];
    currentProjection[12] = thisProjection[13]*(wyb - wyt);
    currentProjection[13] = thisProjection[12];
    currentProjection[15] = btb[3];
    axis(dv/(ymax - ymin), ticy, ticyr, ymax, -dv,
	 ndlaby, nd, 1);
  }
  /* draw z axis */
  if (!ilabz || !ticz) {	/* just a line */
    tkproj(wxb, wyt, wzb, 0);
    tkproj(wxb, wyt, wzt, 1);
  } else {
    if (ilabz)
      sform(wzb, wzt);
    currentProjection[0] = thisProjection[2]*(wzb - wzt);
    currentProjection[1] = thisProjection[0];
    currentProjection[3] = btt[0];
    currentProjection[4] = thisProjection[6]*(wzb - wzt);
    currentProjection[5] = thisProjection[4];
    currentProjection[7] = btt[1];
    currentProjection[8] = thisProjection[10]*(wzb - wzt);
    currentProjection[9] = thisProjection[8];
    currentProjection[11] = btt[2];
    currentProjection[12] = thisProjection[14]*(wzb - wzt);
    currentProjection[13] = thisProjection[12];
    currentProjection[15] = btt[3];
    axis(dvz/(zmax - zmin), ticz, ticzr, zmax, -dvz,
	 ndlabz, ndz, 1);
  }

  /* for the 3D plot we want a transformation that goes immediately from */
  /* (x,y,z) - x and y running from 0 through nx-1 or ny-1, and z ordinary */
  /* data values - to screen coordinates. */
  dx = (wxt - wxb)/(nx - 1);
  dy = (wyt - wyb)/(ny - 1);
  dz = (wzt - wzb)/(zmax - zmin);
  p1 = currentProjection;
  p2 = thisProjection;
  for (i = 0; i < 4; i++) {
    *p1++ = *p2++ * dx;
    *p1++ = *p2++ * dy;
    *p1++ = *p2++ * dz;
    *p1 = bbb[i] - p1[-1]*zmin;
    p1++;
    p2++;
  }

  if (!hide) {
    for (y = 0; y < ny; y++) {	/* first, draw in x direction */
      tkproj(0, y, *src++, 0);
      for (x = 1; x < nx; x++)
	tkproj(x, y, *src++, 1);
    }
    src -= nx*ny;
				/* then, draw in y direction */
    for (x = 0; x < nx; x++) {
      tkproj(x, 0, *src, 0);
      src += nx;
      for (y = 1; y < ny; y++) {
	tkproj(x, y, *src, 1);
	src += nx;
      }
      src -= nx*ny - 1;
    }
  }
	
  return 1;
}
/*---------------------------------------------------------------------*/
/* The following are a bunch of routines to handle polygons. */
/* scanPolygon() returns the boundaries of horizontal slices of a polygon */

/* each vertex is the endpoint of its associated edge */
struct vertexStruct
{ Int	x;			/* vertex x coordinate */
  Int	y;			/* vertex y coordinate */
  char	type;			/* vertex type in polygon */
  char  dir;			/* edge direction */
  Float	slope; };		/* edge slope */
typedef struct vertexStruct	Vertex;

enum { VERTEX_TOP, VERTEX_FLAT_TOP, VERTEX_SIDE, VERTEX_FLAT_BOTTOM,
	 VERTEX_BOTTOM, VERTEX_HORIZONTAL };
enum { POLY_INIT, POLY_NEXT, POLY_END };
#define X_UP	2
#define Y_UP	1

Int comparePtrs(const void *ptr1, const void *ptr2)
     /* auxilliary function for qsort in scanPolygon */
{
  if ((*(Vertex **) ptr1)->y < (*(Vertex **) ptr2)->y)
    return -1;
  if ((*(Vertex **) ptr1)->y > (*(Vertex **) ptr2)->y)
    return 1;
  return 0;
}
/*---------------------------------------------------------------------*/
#if 0
Int scanPolygon(Int *x1, Int *x2, Int *y, char code)
/* scans through an integer-coordinate polygon, depending on <code>. */
/* the polygon is assumed non-self-intersecting! */
/* <code>: POLY_INIT -> initialize with a new polygon.  the polygon */
/*                      vertices must be in *x2, *y, and the number */
/*                      of vertices in *x1. */
/*         POLY_NEXT -> goes through the polygon, bottom to top and left */
/*                      to right.  returns the next polygon segment, */
/*                      bounded by x coordinates *x1 and *y1 and at y */
/*                      coordinate *y. */
/*         POLY_END ->  nice clean-up. */
/* IN DEVELOPMENT */
{
  static Vertex	*vertex, **yIndex, **activeVertex;
  static Int	nVertex, currentY, nActivePoints, currentVertex;
  static char	newY;
  static Float	*activeX;
  Int	i, j;
  Vertex	*p;

  switch (code) {
    case POLY_INIT:		/* initialize new polygon */
      nVertex = *x1;		/* number of vertices */
      if (nVertex < 1)
	return luxerror("Less than one vertex in polygon!", 0);
      allocate(vertex, nVertex + 2, Vertex); /* room for coordinates */
      for (i = 0; i < nVertex; i++) {
	vertex[i + 1].x = x2[i];
	vertex[i + 1].y = y[i];
      }
      vertex[0].x = vertex[nVertex].x; /* copy last in front of first */
      vertex[0].y = vertex[nVertex].y;
      vertex[nVertex + 1].x = vertex[1].x; /* copy first before last */
      vertex[nVertex + 1].y = vertex[1].y; /* copy first before last */
      for (i = 1; i <= nVertex; i++) { /* determine vertex type */
	vertex[i].dir = 0;	/* 0 means the edge points down left */
	if (vertex[i].y > vertex[i - 1].y) {
	  vertex[i].dir = Y_UP;	/* edge points up */
	  if (vertex[i].y > vertex[i + 1].y)
	    vertex[i].type = VERTEX_TOP;
	  else if (vertex[i].y < vertex[i + 1].y)
	    vertex[i].type = VERTEX_SIDE;
	  else vertex[i].type = VERTEX_FLAT_TOP; }
	else if (vertex[i].y < vertex[i - 1].y) {
	  if (vertex[i].y > vertex[i + 1].y)
	    vertex[i].type = VERTEX_SIDE;
	  else if (vertex[i].y < vertex[i + 1].y)
	    vertex[i].type = VERTEX_BOTTOM;
	  else vertex[i].type = VERTEX_FLAT_BOTTOM;
	} else {
	  vertex[i].dir = Y_UP;	/* edge is horizontal */
	  if (vertex[i].y > vertex[i + 1].y)
	    vertex[i].type = VERTEX_FLAT_TOP;
	  else if (vertex[i].y < vertex[i + 1].y)
	    vertex[i].type = VERTEX_FLAT_BOTTOM;
	  else vertex[i].type = VERTEX_HORIZONTAL;
	}
	if (vertex[i].x >= vertex[i - 1].x)
	  vertex[i].dir |= X_UP; /* edge points to the right */
	vertex[i].slope = (vertex[i].x - vertex[i - 1].x)/
	  (vertex[i].y - vertex[i - 1].y);
      }
      vertex[0].dir = vertex[nVertex].dir; /* copy last before first */
      vertex[0].type = vertex[nVertex].type;
      vertex[0].slope = vertex[nVertex].slope;
      vertex[nVertex + 1].dir = vertex[1].dir; /* copy first before last */
      vertex[nVertex + 1].type = vertex[1].type;
      vertex[nVertex + 1].slope = vertex[1].slope;
      /* get indices to sorted y coordinates */
      allocate(yIndex, nVertex, Vertex *);
      for (i = 0; i < nVertex; i++)
	yIndex[i] = vertex + i + 1;
      qsort(yIndex, nVertex, sizeof(Vertex *), comparePtrs);
      /* now *yIndex[0] has the lowest y value */
      allocate(activeX, nVertex - 1, Float);
      allocate(activeVertex, nVertex - 1, Vertex *);
      currentVertex = 0;
      currentY = yIndex[currentVertex]->y;
      nActivePoints = 0;
      newY = 1;
      break;
    case POLY_NEXT:
      if (newY) {			/* starting new scan line */
	if (currentY == yIndex[currentVertex]->y) { /* reached next vertex */
	  switch (yIndex[currentVertex]->type) {
	    case VERTEX_BOTTOM:	/* add two new active points */
	      /* first, find the position to insert them at */
	      /* (I use a linear search: bad!  ought to be replaced */
	      /* with a binary one at some point) */
	      i = 0;
	      while (i < nActivePoints
		     && yIndex[currentVertex]->x > activeX[i]) i++;
	      if (i != nActivePoints)
		memmove(activeX + 2, activeX, nActivePoints - i);
	      activeX[i] = activeX[i + 1] = yIndex[currentVertex]->x;
	      j = (yIndex[currentVertex]->slope < 0)? 0: 1;
	      activeVertex[i] = yIndex[currentVertex] + j;
	      activeVertex[i + 1] = yIndex[currentVertex] + 1 - j;
	      nActivePoints += 2;
	      break;
	    case VERTEX_SIDE:	/* replace slope */
	      if (yIndex[currentVertex]->dir & Y_UP)
		p = yIndex[currentVertex];
	      else p = yIndex[currentVertex] + 1;
	      break;
	    }
	}
      }
      break;
    case POLY_END:
      free(yIndex);
      free(vertex);
      free(activeX);
      free(activeVertex);
      break;
    }
  return 1;
}
/*---------------------------------------------------------------------*/
Int lux_inPolygon(Int narg, Int ps[])
/* returns indices to all (integer-coordinate) points inside an */
/* integer-coordinate polygon.  Syntax: I = INPOLYGON(XVERTEX,YVERTEX) */
/* IN DEVELOPMENT */
{
  Int	iq, nx, ny, *x, *y;

  iq = ps[0];			/* x coordinate of the vertices */
  if (symbol_class(iq) != LUX_ARRAY)
    return cerror(NEED_ARR, iq);
  iq = lux_long(1, &iq);	/* make integer */
  nx = array_size(iq);
  x = (Int *) array_data(iq);
  iq = ps[1];			/* y coordinate of the vertices */
  if (symbol_class(iq) != LUX_ARRAY)
    return cerror(NEED_ARR, iq);
  iq = lux_long(1, &iq);
  ny = array_size(iq);
  y = (Int *) array_data(iq);
  if (nx != ny)
    return cerror(INCMP_ARR, ps[1]);
  scanPolygon(&nx, x, y, POLY_INIT);
  return 1;
}
#endif
/*---------------------------------------------------------------------*/
Int tkhide(Float x, Float y, Float z, Int mode)
{
  Int	hiddenLine(Float, Float, Int);

  project(x, y, z);
  return hiddenLine(projected[0], projected[1], mode);
}
/*---------------------------------------------------------------------*/
/* Our hidden line algorithm uses the painter's algorithm: we start
   drawing the near side of the plot and work our way back, making sure
   not to draw over something we've already drawn.  We want to be able
   to draw only parts of the complete grid (e.g., those parts that
   satisfy some user-defined criterion).  We keep track of the outer
   edges of the areas that have already been drawn, and update those
   edges each time we draw outside them.  */
typedef struct {
  Float	x;			/* x coordinate of first end of segment */
  Float y;			/* y coordinate of first end of segment */
  Float dx;			/* difference in x to second end */
  Float dy;			/* difference in y to second end */
  Byte status;
} lineSegment;

typedef struct {
  Float xmin;			/* least x coordinate */
  Float xmax;			/* greatest x coordinate */
  Float ymin;			/* least y coordinate */
  Float ymax;			/* greatest y coordinate */
  Int	nTopSegment;		/* number of segments in top boundary */
  Int	nBottomSegment;		/* number of segments in bottom boundary */
  Int	nTopFree;		/* allocated number of top segments */
  Int	nBottomFree;		/* allocated number of bottom segments */
  lineSegment	*top;		/* top segments */
  lineSegment	*bottom;	/* bottom segments */
} drawnArea;

#define HL_MOVE		1	/* move to new point */
#define HL_DRAW		2	/* draw to new point */
#define HL_NEW_CANVAS	4	/* start a new drawing (i.e., empty canvas) */
#define HL_NEW_AREA	8	/* start a new drawing area on existing
				   canvas */

#define	HL_DRAWN_BLOCK		10
#define HL_SEGMENT_BLOCK	100

Int hiddenLine(Float x, Float y, Int mode)
{
  static drawnArea	*area = NULL; /* pointer to drawn areas */
  static Int	nDrawnFree,	/* number of areas for which memory is
				   currently allocated and available */
    nDrawn;			/* number of areas currently being drawn */
  Int	i;

  if (mode & (HL_NEW_CANVAS | HL_NEW_AREA)) {
    if (mode & HL_NEW_CANVAS) {
      if (area) {			/* have some memory already */
	/* get rid of the old stuff */
	for (i = 0; i < nDrawn; i++) {
	  free(area[i].top);
	  free(area[i].bottom);
	  area[i].nTopFree = area[i].nBottomFree = area[i].nTopSegment =
	    area[i].nBottomSegment = 0;
	  area[i].top = area[i].bottom = NULL;
	}	/* end of for (i) */
      } /* end of if (area) */
      else {			/* no memory allocated yet */
	area = (drawnArea *) malloc(HL_DRAWN_BLOCK*sizeof(drawnArea));
	if (!area)
	  return cerror(ALLOC_ERR, 0);
	nDrawnFree = HL_DRAWN_BLOCK;
      } /* end of if (area) else */
      nDrawn = 0;
    } /* end of if (mode & HL_NEW) */
    if (mode & HL_NEW_AREA) {
      if (!nDrawnFree) {
	nDrawnFree = HL_DRAWN_BLOCK;
	area = (drawnArea *) realloc(area,
				     (nDrawn + nDrawnFree)*sizeof(drawnArea));
	if (!area)
	  return cerror(ALLOC_ERR, 0);
      }	/* end of if (!nDrawnFree) */
    } /* end of if (mode & HL_NEW_AREA) */
    area[nDrawn].xmin = area[nDrawn].ymin = -FLT_MAX;
    area[nDrawn].xmax = area[nDrawn].ymax = FLT_MAX;
    area[nDrawn].nTopSegment = area[nDrawn].nBottomSegment = nDrawn;
    area[nDrawn].top = malloc(HL_SEGMENT_BLOCK*sizeof(lineSegment));
    area[nDrawn].bottom = malloc(HL_SEGMENT_BLOCK*sizeof(lineSegment));
    if (!area[nDrawn].top || !area[nDrawn].bottom)
      return cerror(ALLOC_ERR, 0);
    area[nDrawn].nTopFree = area[nDrawn].nBottomFree = HL_SEGMENT_BLOCK;
    nDrawn++;
    nDrawnFree--;
  } /* end of if (mode & (HL_NEW | HL_NEW_AREA)) */

  return LUX_OK;
}
/*---------------------------------------------------------------------*/
Int lux_projectmap(Int narg, Int ps[])
/* PROJECTIMAGE(map, h [, HDIST=delta, ANGLE=angle, MAG=mag, XMAP=xmap, */
/*              YMAP=ymap, SIZE=n]) */
/* returns an array of size <n(0)> by <n(1)> elements (or <n> by <n> if */
/* <n> is a scalar) containing the projection of <map> as seen from a */
/* point at height <h> above, and at distance <delta> at angle <angle> */
/* (in degrees) from the center of the image.  <h> and <delta> are */
/* measured in units of one image element.  By default, the projection */
/* measures 90 degrees in both directions.  Optionally, the projection */
/* can be magnified by a factor <mag>, and the center of the projection */
/* can be shifted over <xmap>,<ymap> elements, measured from the lower */
/* left of the projection.  LS 9feb2001 */
{
  pointer	src, trgt;
  Int	imx, imy, xmap, ymap, result, dims[2], i, j, index, stride, sx, sy,
    u2, v2, nx, ny;
  Double h, delta, angle, mag, c, s, bigdelta, c1, c2, c3, c4, c5, c6, c7,
    x, y, d, u, v, dd, du, dv;

  if (!symbolIsRealArray(ps[0]) || array_num_dims(ps[0]) != 2)
    return cerror(NEED_2D_ARR, ps[0]);
  src.v = array_data(ps[0]);
  imx = array_dims(ps[0])[0];
  imy = array_dims(ps[0])[1];

  h = double_arg(ps[1]);		/* <h> */
  if (h <= 0)
    h = 1;
  delta = (narg > 2 && ps[2])? double_arg(ps[2]): 0.0;	/* <delta> */
  angle = (narg > 3 && ps[3])? double_arg(ps[3])*DEG: 0.0;	/* <angle> */
  c = -sin(angle);
  s = cos(angle);

  mag = (narg > 4 && ps[4])? double_arg(ps[4]): 1.0; /* <mag> */
  if (mag <= 0)
    mag = 1.0;
  xmap = (narg > 5 && ps[5])? int_arg(ps[5]): imx/2; /* <xmap> */
  ymap = (narg > 6 && ps[6])? int_arg(ps[6]): imy/2; /* <ymap> */
  if (narg > 7 && ps[7]) {
    if (symbolIsScalar(ps[7]))
      nx = ny = int_arg(ps[7]);
    else if (symbolIsRealArray(ps[7])) {
      if (array_size(ps[7]) >= 2) {
	result = lux_long(1, &ps[7]);
	nx = ((Int *) array_data(result))[0];
	ny = ((Int *) array_data(result))[1];
	if (result != ps[7])
	  zap(result);
      } else if (array_size(ps[7]) == 1) {
	result = lux_long(1, &ps[7]);
	nx = ny = ((Int *) array_data(result))[0];
	if (result != ps[7])
	  zap(result);
      } else return cerror(ILL_CLASS, ps[7]);
    } else return cerror(ILL_CLASS, ps[7]);
  } else
    nx = ny = 512;
  dims[0] = nx;
  dims[1] = ny;

  stride = lux_type_size[array_type(ps[0])];
  result = array_scratch(array_type(ps[0]), 2, dims);
  trgt.v = array_data(result);
  zerobytes(trgt.v, array_size(result)*stride);

  bigdelta = sqrt(delta*delta + h*h);

  c1 = c*delta;
  c2 = s*bigdelta;
  c3 = c*h;
  c4 = s*delta;
  c5 = -c*bigdelta;
  c6 = s*h;
  c7 = -delta/h;

  if (ny > nx)
    d = 1.0/ny;
  else
    d = 1.0/nx;
  y = -(ny - 1)*0.5*d;
  x = -(nx - 1)*0.5*d;

  sx = xmap - delta*c;
  sy = ymap - delta*s;

  du = c2*d;
  dv = c5*d;

  for (j = 0; j < ny; j++) {
    u = c1 + x*c2 + y*c3;
    v = c4 + x*c5 + y*c6;
    dd = mag*(1 + y*c7);
    dd = dd > 0? 1.0/dd: 0.0;
    for (i = 0; i < nx; i++) {
      u += du;
      v += dv;
      u2 = (Int) (u*dd + sx);
      v2 = (Int) (v*dd + sy);
      if (u2 >= 0 && u2 < imx && v2 >= 0 && v2 < imy) {
	index = u2 + v2*imx;
	memcpy(trgt.b, src.b + index*stride, stride);
      };
      trgt.b += stride;
    }
    y += d;
  }
  return result;
}
