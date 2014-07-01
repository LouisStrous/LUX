/* This is file sort.c.

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
/* File sort.c */
/* LUX sorting routines. */
/* file sorts, various sort routines for different types of data */
 /* taken from Press etal */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "luxdefs.h"
/*------------------------------------------------------------------------- */
#define ALN2I 1.442695022
#define TINY 1.0e-5
/*------------------------------------------------------------------------- */
void shell_s(Int n, char *arr[])
{
  Int	nn,m,j,i,lognb2;
  char	*t;

  lognb2 = (log((double) n)*ALN2I + TINY);
  m = n;
  for (nn = 1; nn <= lognb2; nn++) {
    m >>= 1;
    for (j = m; j < n; j++) {
      i = j - m;
      t = arr[j];
      while (i >= 0 && strcmp(arr[i], t) > 0) {
	arr[i + m] = arr[i];
	i -= m;
      }
      arr[i + m] = t;
    }
  }
}
/*------------------------------------------------------------------------- */
void shell_f(Int n, float arr[])
{
  Int	nn,m,j,i,lognb2;
  float	t;

  lognb2 = (log((double) n)*ALN2I + TINY);
  m = n;
  for (nn = 1; nn <= lognb2; nn++) {
    m >>= 1;
    for (j = m; j < n; j++) {
      i = j - m;
      t = arr[j];
      while (i >= 0 && arr[i] > t) {
	arr[i + m] = arr[i];
	i -= m;
      }
      arr[i + m] = t;
    }
  }
}
/*------------------------------------------------------------------------- */
void shell_b(Int n, uint8_t arr[])
{
  Int	nn,m,j,i,lognb2;
  uint8_t	t;

  lognb2 = (log((double) n)*ALN2I + TINY);
  m = n;
  for (nn = 1; nn <= lognb2; nn++) {
    m >>= 1;
    for (j = m; j < n; j++) {
      i = j - m;
      t = arr[j];
      while (i >= 0 && arr[i] > t) {
	arr[i + m] = arr[i];
	i -= m;
      }
      arr[i + m] = t;
    }
  }
}
/*------------------------------------------------------------------------- */
void shell_l(Int n, Int arr[])
{
 Int	nn,m,j,i,lognb2;
 Int	t;

 lognb2 = (log((double) n)*ALN2I + TINY);
 m = n;
 for (nn = 1; nn <= lognb2; nn++) {
   m >>= 1;
   for (j = m; j < n; j++) {
     i = j - m;
     t = arr[j];
     while (i >= 0 && arr[i] > t) {
       arr[i + m] = arr[i];
       i -= m;
     }
     arr[i + m]=t;
   }
 }
}
/*------------------------------------------------------------------------- */
void shell_w(Int n, int16_t arr[])
{
  Int	nn,m,j,i,lognb2;
  int16_t	t;

  lognb2 = (log((double) n)*ALN2I + TINY);
  m = n;
  for (nn = 1; nn <= lognb2; nn++) {
    m >>= 1;
    for (j = m; j < n; j++) {
      i = j - m;
      t = arr[j];
      while (i >= 0 && arr[i] > t) {
	arr[i + m] = arr[i];
	i -= m;
      }
      arr[i + m] = t;
    }
  }
}
/*------------------------------------------------------------------------- */
void shell_d(Int n, double arr[])
{
  Int	nn,m,j,i,lognb2;
  double	t;

  lognb2 = (log((double) n)*ALN2I + TINY);
  m = n;
  for (nn = 1; nn <= lognb2; nn++) {
    m >>= 1;
    for (j = m; j < n; j++) {
      i = j - m;
      t = arr[j];
      while (i >= 0 && arr[i] > t) {
	arr[i + m] = arr[i];
	i -= m;
      }
      arr[i + m] = t;
    }
  }
}
/*------------------------------------------------------------------------- */
void sort_s(Int n, char *ra[])
{
  Int	l,j,ir,i;
  char	*rra;

  l = (n/2);
  ir = n-1;
  for (;;) {
    if (l > 0)
      rra = ra[--l];
    else {
      rra = ra[ir];
      ra[ir] = ra[0];
      if (--ir == 0) {
	ra[0] = rra;
	return;
      }
    }
    i = l;
    j = l + l + 1;
    while (j <= ir) {
      if (j < ir && strcmp(ra[j], ra[j+1]) < 0)
	j++;
      if (strcmp(rra, ra[j]) < 0) {
	ra[i] = ra[j];
	j += (i = j) + 1;
      } else
	j = ir + 1;
    }
    ra[i] = rra;
  }
}
/*------------------------------------------------------------------------- */
void sort_f(Int n, float ra[])
{
  Int	l,j,ir,i;
  float	rra;

  l = (n/2);
  ir = n-1;
  for (;;) {
    if (l > 0)
      rra = ra[--l];
    else {
      rra = ra[ir];
      ra[ir] = ra[0];
      if (--ir == 0) {
	ra[0] = rra;
	return;
      }
    }
    i = l;
    j = l + l + 1;
    while (j <= ir) {
      if (j < ir && ra[j] < ra[j+1])
	j++;
      if (rra < ra[j]) {
	ra[i] = ra[j];
	j += (i=j) + 1;
      } else
	j = ir + 1;
    }
    ra[i] = rra;
  }
}
/*------------------------------------------------------------------------- */
void sort_b(Int n, uint8_t ra[])
{
  Int	l,j,ir,i;
  uint8_t	rra;

  l = (n/2);
  ir = n-1;
  for (;;) {
    if (l > 0)
      rra = ra[--l];
    else {
      rra = ra[ir];
      ra[ir] = ra[0];
      if (--ir == 0) {
	ra[0] = rra;
	return;
      }
    }
    i = l;
    j = l + l + 1;
    while (j <= ir) {
      if (j < ir && ra[j] < ra[j+1])
	j++;
      if (rra < ra[j]) {
	ra[i] = ra[j];
	j += (i=j) + 1;
      } else
	j = ir + 1;
    }
    ra[i] = rra;
  }
}
/*------------------------------------------------------------------------- */
void sort_w(Int n, int16_t ra[])
{
  Int	l,j,ir,i;
  int16_t	rra;

  l = (n/2);
  ir = n-1;
  for (;;) {
    if (l > 0)
      rra = ra[--l];
    else {
      rra = ra[ir];
      ra[ir] = ra[0];
      if (--ir == 0) {
	ra[0] = rra;
	return;
      }
    }
    i = l;
    j = l + l + 1;
    while (j <= ir) {
      if (j < ir && ra[j] < ra[j+1])
	j++;
      if (rra < ra[j]) {
	ra[i] = ra[j];
	j += (i=j) + 1;
      } else
	j = ir + 1;
    }
    ra[i] = rra;
  }
}
/*------------------------------------------------------------------------- */
void sort_l(Int n, Int ra[])
{
  Int	l,j,ir,i;
  long	rra;
  
  l = (n/2);
  ir = n-1;
  for (;;) {
    if (l > 0)
      rra = ra[--l];
    else {
      rra = ra[ir];
      ra[ir] = ra[0];
      if (--ir == 0) {
	ra[0] = rra;
	return;
      }
    }
    i = l;
    j = l + l + 1;
    while (j <= ir) {
      if (j < ir && ra[j] < ra[j+1])
	j++;
      if (rra < ra[j]) {
	ra[i] = ra[j];
	j += (i=j) + 1;
      } else
	j = ir + 1;
    }
    ra[i] = rra;
  }
}
/*------------------------------------------------------------------------ */
void sort_d(Int n, double ra[])
{
  Int	l,j,ir,i;
  double rra;

  l = (n/2);
  ir = n-1;
  for (;;) {
    if (l > 0)
      rra = ra[--l];
    else {
      rra = ra[ir];
      ra[ir] = ra[0];
      if (--ir == 0) {
	ra[0] = rra;
	return;
      }
    }
    i = l;
    j = l + l + 1;
    while (j <= ir) {
      if (j < ir && ra[j] < ra[j+1])
	j++;
      if (rra < ra[j]) {
	ra[i] = ra[j];
	j += (i=j) + 1;
   } else
     j = ir + 1;
    }
    ra[i] = rra;
  }
}
/*------------------------------------------------------------------------- */
void indexx_s(Int n, char *ra[], Int indx[])
{
  Int	l,j,ir,i,indxt;
  char	*q;
 
  for (i = 0; i < n; i++)
    indx[i] = i;
  l = (n/2);
  ir = n-1;
  for (;;) {
    if (l > 0)
      q=ra[(indxt = indx[--l])];
    else {
      q = ra[(indxt = indx[ir])];
      indx[ir] = indx[0];
      if (--ir == 0) {
	indx[0] = indxt;
	return;
      }
    }
    i = l;
    j = l + l + 1;
    while (j <= ir) {
      if (j < ir && strcmp(ra[indx[j]], ra[indx[j+1]]) < 0)
	j++;
      if (strcmp(q, ra[indx[j]]) < 0) {
	indx[i] = indx[j];
	j += (i = j) + 1;
      } else
	j = ir + 1;
    }
    indx[i] = indxt;
  }
}
/*------------------------------------------------------------------------- */
void indexx_d(Int n, double ra[], Int indx[])
{
  Int	l,j,ir,i,indxt;
  double	q;
 
  for (i = 0; i < n; i++)
    indx[i] = i;
  l = (n/2);
  ir = n-1;
  for (;;) {
    if (l > 0)
      q=ra[(indxt = indx[--l])];
    else {
      q = ra[(indxt = indx[ir])];
      indx[ir] = indx[0];
      if (--ir == 0) {
	indx[0] = indxt;
	return;
      }
    }
    i = l;
    j = l + l + 1;
    while (j <= ir) {
      if (j < ir && ra[indx[j]] < ra[indx[j+1]])
	j++;
      if (q < ra[indx[j]]) {
	indx[i] = indx[j];
	j += (i = j) + 1;
      } else
	j = ir + 1;
    }
    indx[i] = indxt;
  }
}
/*------------------------------------------------------------------------- */
void indexx_b(Int n, uint8_t ra[], Int indx[])
{
  Int	l,j,ir,i,indxt;
  uint8_t	q;
 
  for (i = 0; i < n; i++)
    indx[i] = i;
  l = (n/2);
  ir = n - 1;
  for (;;) {
    if (l > 0)
      q = ra[(indxt = indx[--l])];
    else {
      q = ra[(indxt = indx[ir])];
      indx[ir] = indx[0];
      if (--ir == 0) {
	indx[0] = indxt;
	return;
      }
    }
    i = l;
    j = l + l + 1;
    while (j <= ir) {
      if (j < ir && ra[indx[j]] < ra[indx[j+1]])
	j++;
      if (q < ra[indx[j]]) {
	indx[i] = indx[j];
	j += (i = j) + 1;
      } else
	j = ir + 1;
    }
    indx[i] = indxt;
  }
}
/*------------------------------------------------------------------------- */
void indexx_w(Int n, int16_t ra[], Int indx[])
{
  Int	l,j,ir,i,indxt;
  int16_t	q;
 
  for (i = 0; i < n; i++)
    indx[i] = i;
  l = (n/2);
  ir = n - 1;
  for (;;) {
    if (l > 0)
      q=ra[(indxt = indx[--l])];
    else {
      q=ra[(indxt = indx[ir])];
      indx[ir] = indx[0];
      if (--ir == 0) {
	indx[0] = indxt;
	return;
      }
    }
    i = l;
    j = l + l + 1;
    while (j <= ir) {
      if (j < ir && ra[indx[j]] < ra[indx[j+1]])
	j++;
      if (q < ra[indx[j]]) {
	indx[i] = indx[j];
	j += (i = j) + 1;
      } else
	j = ir + 1;
    }
    indx[i] = indxt;
  }
}
/*------------------------------------------------------------------------- */
void indexx_l(Int n, Int ra[], Int indx[])
{
  Int	l,j,ir,i,indxt;
  Int	q;
 
  for (i = 0; i < n; i++)
    indx[i] = i;
  l = (n/2);
  ir = n - 1;
  for (;;) {
    if (l > 0)
      q=ra[(indxt = indx[--l])];
    else {
      q=ra[(indxt = indx[ir])];
      indx[ir] = indx[0];
      if (--ir == 0) {
	indx[0] = indxt;
	return;
      }
    }
    i = l;
    j = l + l + 1;
    while (j <= ir) {
      if (j < ir && ra[indx[j]] < ra[indx[j+1]])
	j++;
      if (q < ra[indx[j]]) {
	indx[i] = indx[j];
	j += (i = j) + 1;
      } else
	j = ir + 1;
    }
    indx[i] = indxt;
  }
}
/*------------------------------------------------------------------------- */
void indexx_f(Int n, float ra[], Int indx[])
{
  Int	l,j,ir,i,indxt;
  float	q;
  
  for (i = 0; i < n; i++)
    indx[i] = i;
  l = (n/2);
  ir = n-1;
  for (;;) {
    if (l > 0)
      q=ra[(indxt = indx[--l])];
    else {
      q = ra[(indxt = indx[ir])];
      indx[ir] = indx[0];
      if (--ir == 0) {
	indx[0] = indxt;
	return;
      }
    }
    i = l;
    j = l + l + 1;
    while (j <= ir) {
      if (j < ir && ra[indx[j]] < ra[indx[j+1]])
	j++;
      if (q < ra[indx[j]]) {
	indx[i] = indx[j];
	j += (i = j) + 1;
      } else
	j = ir + 1;
    }
    indx[i] = indxt;
  }
}
/*------------------------------------------------------------------------- */
void indexxr_f(Int n, float ra[], Int indx[])
{
  Int	l,j,ir,i,indxt;
  float	q;
  
  for (i = 0; i < n; i++)
    indx[i] = i;
  l = (n/2);
  ir = n-1;
  for (;;) {
    if (l > 0)
      q=ra[(indxt = indx[--l])];
    else {
      q = ra[(indxt = indx[ir])];
      indx[ir] = indx[0];
      if (--ir == 0) {
	indx[0] = indxt;
	return;
      }
    }
    i = l;
    j = l + l + 1;
    while (j <= ir) {
      if (j < ir && ra[indx[j]] > ra[indx[j+1]])
	j++;
      if (q > ra[indx[j]]) {
	indx[i] = indx[j];
	j += (i = j) + 1;
      } else
	j = ir + 1;
    }
    indx[i] = indxt;
  }
}
/*------------------------------------------------------------------------- */
void indexxr_d(Int n, double ra[], Int indx[])
{
  Int	l,j,ir,i,indxt;
  double q;
  
  for (i = 0; i < n; i++)
    indx[i] = i;
  l = (n/2);
  ir = n-1;
  for (;;) {
    if (l > 0)
      q=ra[(indxt = indx[--l])];
    else {
      q = ra[(indxt = indx[ir])];
      indx[ir] = indx[0];
      if (--ir == 0) {
	indx[0] = indxt;
	return;
      }
    }
    i = l;
    j = l + l + 1;
    while (j <= ir) {
      if (j < ir && ra[indx[j]] > ra[indx[j+1]])
	j++;
      if (q > ra[indx[j]]) {
	indx[i] = indx[j];
	j += (i = j) + 1;
      } else
	j = ir + 1;
    }
    indx[i] = indxt;
  }
}
/*------------------------------------------------------------------------- */
#undef ALN2I
#undef TINY

void invertPermutation(Int *data, Int n)
/* assumes data[] contains a permutation of the numbers between 0 and */
/* <n - 1> (inclusive), and rearranges them into the inverse permutation. */
/* I.e., if beforehand data[i] = j, then afterwards data[j] = i. */
/* LS 25aug2000 */
{
  Int	nloop, i, j, k;

  i = 0;
  nloop = 0;
  do {
    while (data[i] < 0)
      i++;
    j = data[i];
    while (1) {
      k = data[j];
      if (k < 0)
	break;
      data[j] = -i - 1;
      nloop++;
      i = j;
      j = k;
    };
  } while (nloop < n);
  i = n;
  while (i--) {
    *data = -*data - 1;
    data++;
  }
}
/*------------------------------------------------------------------------- */
