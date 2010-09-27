/* File sort.c */
/* ANA sorting routines. */
/* file sorts, various sort routines for different types of data */
 /* taken from Press etal */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "anaparser.h"
static char rcsid[] __attribute__ ((unused)) =
 "$Id: sort.c,v 4.0 2001/02/07 20:37:04 strous Exp $";
/*------------------------------------------------------------------------- */
#define ALN2I 1.442695022
#define TINY 1.0e-5
/*------------------------------------------------------------------------- */
void shell_s(int n, char *arr[])
{
  int	nn,m,j,i,lognb2;
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
void shell_f(int n, float arr[])
{
  int	nn,m,j,i,lognb2;
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
void shell_b(int n, byte arr[])
{
  int	nn,m,j,i,lognb2;
  byte	t;

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
void shell_l(int n, int arr[])
{
 int	nn,m,j,i,lognb2;
 int	t;

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
void shell_w(int n, word arr[])
{
  int	nn,m,j,i,lognb2;
  word	t;

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
void shell_d(int n, double arr[])
{
  int	nn,m,j,i,lognb2;
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
void sort_s(int n, char *ra[])
{
  int	l,j,ir,i;
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
void sort_f(int n, float ra[])
{
  int	l,j,ir,i;
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
void sort_b(int n, byte ra[])
{
  int	l,j,ir,i;
  byte	rra;

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
void sort_w(int n, word ra[])
{
  int	l,j,ir,i;
  word	rra;

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
void sort_l(int n, int ra[])
{
  int	l,j,ir,i;
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
void sort_d(int n, double ra[])
{
  int	l,j,ir,i;
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
void indexx_s(int n, char *ra[], int indx[])
{
  int	l,j,ir,i,indxt;
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
void indexx_d(int n, double ra[], int indx[])
{
  int	l,j,ir,i,indxt;
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
void indexx_b(int n, byte ra[], int indx[])
{
  int	l,j,ir,i,indxt;
  byte	q;
 
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
void indexx_w(int n, word ra[], int indx[])
{
  int	l,j,ir,i,indxt;
  word	q;
 
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
void indexx_l(int n, int ra[], int indx[])
{
  int	l,j,ir,i,indxt;
  int	q;
 
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
void indexx_f(int n, float ra[], int indx[])
{
  int	l,j,ir,i,indxt;
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
void indexxr_f(int n, float ra[], int indx[])
{
  int	l,j,ir,i,indxt;
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
#undef ALN2I
#undef TINY

void invertPermutation(int *data, int n)
/* assumes data[] contains a permutation of the numbers between 0 and */
/* <n - 1> (inclusive), and rearranges them into the inverse permutation. */
/* I.e., if beforehand data[i] = j, then afterwards data[j] = i. */
/* LS 25aug2000 */
{
  int	nloop, i, j, k;

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
