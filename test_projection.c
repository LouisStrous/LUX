/* HEADERS */
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
/* END HEADERS */

#define DEG (M_PI/180)

#define CURRENT		0	/* current coordinate system */
#define ORIGINAL	1	/* original coordinate system */
int	projCoords = CURRENT;

int rotateProjection(float *projection, float ax, float ay, float az)
/* enter rotations over <ax> degrees along the X axis, <ay> degrees */
/* along the Y axis, and <az> degrees along the Z axis, into projection */
/* matrix <projection> */
{
  float	sx, sy, sz, rot[9], temp[12], *p, *q, *r, cx, cy, cz;
  int	i, j;

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

#define EPS (2)
int approxeq(const double v1, const double v2)
{
  const double d = fabs(v1 - v2);
  return d <= EPS*DBL_EPSILON*fabs(v1) || d <= EPS*DBL_EPSILON*fabs(v2);
}
int approxeqf(const float v1, const float v2)
{
  const float d = fabs(v1 - v2);
  return d <= EPS*FLT_EPSILON*fabs(v1) || d <= EPS*FLT_EPSILON*fabs(v2)
    || (fabs(v1) < EPS*FLT_EPSILON && fabs(v2) < EPS*FLT_EPSILON
	&& d < EPS*FLT_EPSILON);
}

int main(void) {
  float p[12];
  struct {
    float p[12];
    float ax;
    float ay;
    float az;
    float q[12];
  } tests[] = {
    { { 1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0 },
      90, 0, 0,
      { 1, 0, 0, 0,
	0, 0, -1, 0,
	0, 1, 0, 0 } },
    { { 1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0 },
      0, 90, 0,
      { 0, 0, 1, 0,
	0, 1, 0, 0,
	-1, 0, 0, 0 } },
    { { 1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0 },
      0, 0, 90,
      { 0, -1, 0, 0,
	1, 0, 0, 0,
	0, 0, 1, 0 } },
  };
  int i;

  printf("%d tests\n", sizeof(tests)/sizeof(tests[0]));
  printf("FLT_EPSILON = %g\n", FLT_EPSILON);
  for (i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
    int j;
    memcpy(p, tests[i].p, sizeof(tests[i].p));
    rotateProjection(p, tests[i].ax, tests[i].ay, tests[i].az);
    for (j = 0; j < 12; j++)
      if (!approxeqf(p[j], tests[i].q[j]))
	printf("%d %d: expected %g found %g\n",
	       i, j, tests[i].q[j], p[j]);
  }
  return 0;
}
