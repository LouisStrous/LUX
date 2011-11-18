#include "anaparser.h"
#include "error.h"
#include <math.h>
#include <obstack.h>
#define obstack_chunk_alloc malloc
#define obstack_chunk_free free

struct obstack *registered_functions = NULL, *registered_subroutines = NULL;

void register_ana_f(int (*f)(int, int []), char *name, int min_arg,
                    int max_arg, char *spec)
{
  internalRoutine ir;

  if (!registered_functions) {
    registered_functions = malloc(sizeof(*registered_functions));
    obstack_init(registered_functions);
  }
  ir.name = name;
  ir.minArg = min_arg;
  ir.maxArg = max_arg;
  ir.ptr = f;
  ir.keys = spec;
  obstack_grow(registered_functions, &ir, sizeof(ir));
}
/*-----------------------------------------------------------------------*/
void register_ana_s(int (*f)(int, int []), char *name, int min_arg,
                    int max_arg, char * spec)
{
  internalRoutine ir;

  if (!registered_subroutines) {
    registered_subroutines = malloc(sizeof(*registered_subroutines));
    obstack_init(registered_subroutines);
  }
  ir.name = name;
  ir.minArg = min_arg;
  ir.maxArg = max_arg;
  ir.ptr = f;
  ir.keys = spec;
  obstack_grow(registered_subroutines, &ir, sizeof(ir));
}

/*-----------------------------------------------------------------------*/
int ana_iDDoC33_f_(int narg, int ps[], void (*f)(double, double, double, double (*)[3]))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D;i>D;i>D;rD+3,+3", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  f(*ptrs[0].d, *ptrs[1].d, *ptrs[2].d, (double (*)[3]) ptrs[3].d);
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_iLb3od3rl_f_(int narg, int ps[], int (*f)(int, double *, double *, double *, double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D3*;iL1;rD3*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  size_t nelem = infos[0].nelem/3;
  while (nelem--) {
    f(*ptrs[1].l, ptrs[0].d, &ptrs[2].d[0], &ptrs[2].d[1], &ptrs[2].d[2]);
    ptrs[0].d += 3;
    ptrs[2].d += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_iLd3ob3rl_f_(int narg, int ps[], int (*f)(int, double, double, double, double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D3*;iL1;rD3*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  size_t nelem = infos[0].nelem/3;
  while (nelem--) {
    f(*ptrs[1].l, ptrs[0].d[0], ptrs[0].d[1], ptrs[0].d[2], ptrs[2].d);
    ptrs[0].d += 3;
    ptrs[2].d += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_ib3DDod3rl_f_(int narg, int ps[], int (*f)(double, double, double *, double *, double *, double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D3*;i>D1;i>D1;rD3*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  size_t nelem = infos[0].nelem/3;
  while (nelem--) {
    f(*ptrs[1].d, *ptrs[2].d, ptrs[0].d, &ptrs[3].d[0], &ptrs[3].d[1],
      &ptrs[3].d[2]);
    ptrs[0].d += 3;
    ptrs[3].d += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_ib3b3ob3_f_(int narg, int ps[], void (*f)(double *, double *, double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D3*;i>D[-];rD[-]", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  size_t nelem = infos[0].nelem/3;
  while (nelem--) {
    f(ptrs[0].d, ptrs[1].d, ptrs[2].d);
    ptrs[0].d += 3;
    ptrs[1].d += 3;
    ptrs[2].d += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_ib3b3rd_f_(int narg, int ps[], double (*f)(double *, double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D3*;i>D3*;rD-3*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[2].nelem--) {
    *ptrs[2].d++ = f(ptrs[0].d, ptrs[1].d);
    ptrs[0].d += 3;
    ptrs[1].d += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_ib3rb2_f_(int narg, int ps[], void (*f)(double *, double *, double *))
{
  int iq;
  pointer *ptrs;
  loopInfo *infos;

  if ((iq = standard_args(narg, ps, "i>D3*;r>D-3+2*", &ptrs, &infos)) < 0)
    return ANA_ERROR;

  size_t nelem = infos[0].nelem/3;
  while (nelem--) {
    f(ptrs[0].d, &ptrs[1].d[0], &ptrs[1].d[1]);
    ptrs[0].d += 3;
    ptrs[1].d += 2;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_ib3oc33_f_(int narg, int ps[], void (*f)(double [3], double [3][3]))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D3*;rD+3*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--) {
    f(ptrs[0].d, (double (*)[3]) ptrs[1].d);
    ptrs[0].d += 3;
    ptrs[1].d += 9;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_ib3od3_f_(int narg, int ps[], void (*f)(double *, double *, double *, double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D3*;rD3*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--) {
    f(ptrs[0].d++, &ptrs[1].d[0], &ptrs[1].d[1], &ptrs[1].d[2]);
    ptrs[1].d += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_ib3odb3_s_(int narg, int ps[], void (*f)(double *, double *, double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D3*;oD-3*;oD*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  size_t nelem = infos[0].nelem/3;
  while (nelem--) {
    f(ptrs[0].d, ptrs[1].d++, ptrs[2].d);
    ptrs[0].d += 3;
    ptrs[2].d += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_ic23c23ob2_f_(int narg, int ps[], void (*f)(double [2][3], double [2][3], double [2]))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D2,3*;i>D2,3*;oD[-],-3*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  size_t nelem = infos[0].nelem/6;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, (double (*)[3]) ptrs[1].d, 
      (double *) ptrs[2].d);
    ptrs[0].d += 6;
    ptrs[1].d += 6;
    ptrs[2].d += 2;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_ic23c23oc23_f_(int narg, int ps[], void (*f)(double [2][3], double [2][3], double [2][3]))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D2,3*;i>D2,3*;oD[-]", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  size_t nelem = infos[0].nelem/6;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, (double (*)[3]) ptrs[1].d, 
      (double (*)[3]) ptrs[2].d);
    ptrs[0].d += 6;
    ptrs[1].d += 6;
    ptrs[2].d += 6;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_ic23odd_s_(int narg, int ps[], void (*f)(double [2][3], double *, double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D2,3*;oD-2-3*;oD[-]", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  size_t nelem = infos[0].nelem/6;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, ptrs[1].d++, ptrs[2].d++);
    ptrs[0].d += 6;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_ic23odddddd_s_(int narg, int ps[], void (*f)(double [2][3], double *, double *, double *, double *, double *, double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D2,3*;oD-2-3*;oD[-];oD[-];oD[-];oD[-];oD[-]", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  size_t nelem = infos[0].nelem/6;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, ptrs[1].d++, ptrs[2].d++, ptrs[3].d++,
      ptrs[4].d++, ptrs[5].d++, ptrs[6].d++);
    ptrs[0].d += 6;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_ic23oddddddrl_s_(int narg, int ps[], int (*f)(double [2][3], double *, double *, double *, double *, double *, double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D2,3*;oD-2-3*;oD[-];oD[-];oD[-];oD[-];oD[-]", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  size_t nelem = infos[0].nelem/6;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, ptrs[1].d++, ptrs[2].d++, ptrs[3].d++,
      ptrs[4].d++, ptrs[5].d++, ptrs[6].d++);
    ptrs[0].d += 6;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_ic33b3ob3_f_(int narg, int ps[], void (*f)(double [3][3], double [3], double [3]))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D3,3*;i>D-3;oD[-]", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  size_t nelem = infos[0].nelem/9;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, ptrs[1].d, ptrs[2].d);
    ptrs[0].d += 9;
    ptrs[1].d += 3;
    ptrs[2].d += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_ic33c23oc23_f_(int narg, int ps[], void (*f)(double [3][3], double [2][3], double [2][3]))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D3,3*;iD*;oD[-]", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  size_t nelem = infos[0].nelem/9;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, (double (*)[3]) ptrs[1].d,
      (double (*)[3]) ptrs[2].d);
    ptrs[0].d += 9;
    ptrs[1].d += 6;
    ptrs[2].d += 6;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_ic33dc33oc33_f_(int narg, int ps[], void (*f)(double (*)[3], double, double (*)[3], double (*)[3]))
{
  int iq;
  pointer *ptrs, era;
  loopInfo *infos;
  size_t nelem;

  if ((iq = standard_args(narg, ps, "i>D3,3*;i>D-,-*;i>D*;rD*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  era = ptrs[1];
  double (*rc2i)[3] = (double (*)[3]) ptrs[0].d;
  double (*rpom)[3] = (double (*)[3]) ptrs[2].d;
  double (*rc2t)[3] = (double (*)[3]) ptrs[3].d;
  nelem = infos[1].nelem;
  while (nelem--) {
    f(rc2i, *era.d++, rpom, rc2t);
    rc2i += 3;
    rpom += 3;
    rc2t += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_ic33drd_f_(int narg, int ps[], double (*f)(double (*)[3], double))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D3,3*;i>D-,-*;rD-,-*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  double (*rnpb)[3] = (double (*)[3]) ptrs[1].d;
  while (infos[1].nelem--) {
    *ptrs[2].d++ = iauEors(rnpb, *ptrs[1].d++);
    rnpb += 3;
  }
  return iq;  
}
/*-----------------------------------------------------------------------*/
int ana_ic33ob3_f_(int narg, int ps[], void (*f)(double [3][3], double [3]))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D3,3*;oD-3*", &ptrs, &infos)) < 0)
    return ANA_ERROR;

  size_t nelem = infos[0].nelem/9;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, ptrs[1].d);
    ptrs[0].d += 9;
    ptrs[1].d += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_ic33oc33_f_(int narg, int ps[], void (*f)(double [3][3], double [3][3]))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D3,3*;rD*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  
  size_t nelem = infos[0].nelem/9;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, (double (*)[3]) ptrs[1].d);
    ptrs[0].d += 9;
    ptrs[1].d += 9;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_ic33odd_s_(int narg, int ps[], void (*f)(double (*)[3], double *, double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D3,3*;oD-3,-3*;oD-3,-3*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  
  size_t nelem = infos[0].nelem/9;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, ptrs[1].d++, ptrs[2].d++);
    ptrs[0].d += 9;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_id000ddddoc33_f_(int narg, int ps[], void (*f)(double, double, double, double, double, double, double, double, double (*)[3]))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;i>D*;i>D*;rD+3,+3*",
                          &ptrs, &infos)) < 0)
    return ANA_ERROR;
  double *jd = ptrs[0].d;
  double *dpsi = ptrs[1].d;
  double *deps = ptrs[2].d;
  double *xp = ptrs[3].d;
  double *yp = ptrs[4].d;
  double (*rc2t)[3] = (double (*)[3]) ptrs[5].d;
  size_t nelem = infos[0].nelem;
  while (nelem--) {
    f(*jd++, 0, 0, 0, *dpsi++, *deps++, *xp++, *yp++, rc2t);
    rc2t += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_id0d0ddoc33_mod_f_(int narg, int ps[], void (*f)(double, double, double, double, double, double, double (*)[3]))
{
  int iq;
  pointer *ptrs;
  loopInfo *infos;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;i>D*;rD+3,+3*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--) {
    double d, t;
    d = floor(*ptrs[1].d - 0.5) + 0.5;
    t = *ptrs[1].d - d;
    f(*ptrs[0].d++, 0.0, d, t, *ptrs[2].d++, *ptrs[3].d++, (double (*)[3]) ptrs[4].d);
    ptrs[4].d += 9;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_idd0DDDrd_f_(int narg, int ps[], double (*f)(double, double, double, double, double, double))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>L*;i>D*;i>D1;i>D1;i>D1;r>D*", &ptrs, &infos)) < 0)
    return ANA_ERROR;

  switch (infos[0].type) {
  case ANA_LONG:
    while (infos[0].nelem--)
      *ptrs[5].d++ = f((double) *ptrs[0].l++, 0.0, *ptrs[1].d, *ptrs[2].d,
                       *ptrs[3].d, *ptrs[4].d);
    break;
  case ANA_FLOAT:
    while (infos[0].nelem--)
      *ptrs[5].d++ = f((double) *ptrs[0].f++, 0.0, *ptrs[1].d, *ptrs[2].d,
                       *ptrs[3].d, *ptrs[4].d);
    break;
  case ANA_DOUBLE:
    while (infos[0].nelem--)
      *ptrs[5].d++ = f(*ptrs[0].d++, 0.0, *ptrs[1].d, *ptrs[2].d, 
                       *ptrs[3].d, *ptrs[4].d);
    break;
  }
  return iq;  
}
/*-----------------------------------------------------------------------*/
int ana_id0Loc23rl_f_(int narg, int ps[], int (*f)(double, double, int, double (*)[3]))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;iL1;rD+2,+3*", &ptrs, &infos)) < 0)
    return ANA_ERROR;

  while (infos[0].nelem--) {
    f(*ptrs[0].d++, 0.0, *ptrs[1].l, (double (*)[3]) ptrs[2].d);
    ptrs[2].d += 6;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_id0c23c23rl_s_(int narg, int ps[], int (*f)(double, double, double (*)[3], double (*)[3]))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>L*;oD+2,+3*;oD+2,+3*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  double (*pvh)[3] = (double (*)[3]) ptrs[1].d;
  double (*pvb)[3] = (double (*)[3]) ptrs[2].d;

  switch (infos[0].type) {
  case ANA_LONG:
    while (infos[0].nelem--) {
      f((double) *ptrs[0].l++, 0.0, pvh, pvb);
      pvh += 2;
      pvb += 2;
    }
    break;
  case ANA_FLOAT:
    while (infos[0].nelem--) {
      f((double) *ptrs[0].f++, 0.0, pvh, pvb);
      pvh += 2;
      pvb += 2;
    }
    break;
  case ANA_DOUBLE:
    while (infos[0].nelem--) {
      f(*ptrs[0].d++, 0.0, pvh, pvb);
      pvh += 2;
      pvb += 2;
    }
    break;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_id0c33oc33_f_(int narg, int ps[], void (*f)(double, double, double (*)[3], double (*)[3]))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D+3,+3*;rD[-]*", &ptrs, &infos)) < 0)
    return ANA_ERROR;

  double (*r1)[3] = (double (*)[3]) ptrs[1].d;
  double (*r2)[3] = (double (*)[3]) ptrs[2].d;

  while (infos[0].nelem--) {
    f(*ptrs[0].d++, 0.0, r1, r2);
    r1 += 3;
    r2 += 3;
  }
  return iq;  
}
/*-----------------------------------------------------------------------*/
int ana_id0d0oddd_s_(int narg, int ps[], void (*f)(double, double, double, double, double *, double *, double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;oD*;oD*;oD*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, 0.0, *ptrs[1].d++, 0.0, ptrs[2].d++, ptrs[3].d++, ptrs[4].d++);
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_id0ddoc33_f_(int narg, int ps[], void (*f)(double, double, double, double, double (*)[3]))
{
  int iq;
  double (*tgt)[3];
  pointer *ptrs;
  loopInfo *infos;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;rD+3,+3*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  tgt = (double (*)[3]) ptrs[3].d;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, 0.0, *ptrs[1].d++, *ptrs[2].d++, tgt);
    tgt += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_id0ddodc33c33c33c33c33_s_(int narg, int ps[], void (*f)(double, double, double, double, double *, double (*)[3], double (*)[3], double (*)[3], double (*)[3], double (*)[3]))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;oD*;oD+3,+3*;oD+3,+3*;oD+3,+3;oD+3,+3*;oD+3,+3*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, 0.0, *ptrs[1].d++, *ptrs[2].d++, ptrs[3].d++,
      (double (*)[3]) ptrs[4].d, (double (*)[3]) ptrs[5].d,
      (double (*)[3]) ptrs[6].d, (double (*)[3]) ptrs[7].d,
      (double (*)[3]) ptrs[8].d);
    ptrs[4].d += 9;
    ptrs[5].d += 9;
    ptrs[6].d += 9;
    ptrs[7].d += 9;
    ptrs[8].d += 9;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_id0ddrd_f_(int narg, int ps[], double (*f)(double, double, double, double))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>L*;i>D*;i>D*;r>D*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  switch (infos[0].type) {
  case ANA_LONG:
    while (infos[1].nelem--)
      *ptrs[3].d++ = f((double) *ptrs[0].l++, 0.0, *ptrs[1].d++, *ptrs[2].d++);
    break;
  case ANA_FLOAT:
    while (infos[1].nelem--)
      *ptrs[3].d++ = f((double) *ptrs[0].f++, 0.0, *ptrs[1].d++, *ptrs[2].d++);
    break;
  case ANA_DOUBLE:
    while (infos[1].nelem--)
      *ptrs[3].d++ = f(*ptrs[0].d++, 0.0, *ptrs[1].d++, *ptrs[2].d++);
    break;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_id0doddrl_jd_f_(int narg, int ps[], int (*f)(double, double, double, double *, double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;rD*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--) {
    double ut1, ut2;
    f(*ptrs[0].d++, 0.0, *ptrs[1].d++, &ut1, &ut2);
    *ptrs[2].d++ = ut1 + ut2;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_id0oc33_f_(int narg, int ps[], void (*f)(double, double, double (*)[3]))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;rD+3,+3*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  double (*r)[3] = (double (*)[3]) ptrs[1].d;

  while (infos[0].nelem--) {
    f(*ptrs[0].d++, 0.0, r);
    r += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_id0oC33c33c33_s_(int narg, int ps[], void (*f)(double, double, double (*)[3], double (*)[3], double (*)[3]))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>L*;oD3,3;oD+3,+3*;oD[-]*", &ptrs, &infos)) < 0)
    return ANA_ERROR;

  switch (infos[0].type) {
  case ANA_LONG:
    while (infos[0].nelem--) {
      f((double) *ptrs[0].l++, 0.0, (double (*)[3]) ptrs[1].d,
        (double (*)[3]) ptrs[2].d, (double (*)[3]) ptrs[3].d);
      ptrs[2].d += 9;
      ptrs[3].d += 9;
    }
    break;
  case ANA_FLOAT: 
    while (infos[0].nelem--) {
      f((double) *ptrs[0].f++, 0.0, (double (*)[3]) ptrs[1].d,
        (double (*)[3]) ptrs[2].d, (double (*)[3]) ptrs[3].d);
      ptrs[2].d += 9;
      ptrs[3].d += 9;
    }
    break;
  case ANA_DOUBLE:
    while (infos[0].nelem--) {
      f(*ptrs[0].d++, 0.0, (double (*)[3]) ptrs[1].d,
        (double (*)[3]) ptrs[2].d, (double (*)[3]) ptrs[3].d);
      ptrs[2].d += 9;
      ptrs[3].d += 9;
    }
    break;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_id0oddrl_jd_f_(int narg, int ps[], int (*f)(double, double, double *, double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;rD*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--) {
    double tt1, tt2;
    f(*ptrs[0].d++, 0.0, &tt1, &tt2);
    *ptrs[1].d++ = tt1 + tt2;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_id0odd_s_(int narg, int ps[], void (*f)(double, double, double *, double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;oD*;oD*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, 0.0, ptrs[1].d++, ptrs[2].d++);
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_id0oddd_s_(int narg, int ps[], void (*f)(double, double, double *, double *, double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;oD*;oD*;oD*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, 0.0, ptrs[1].d++, ptrs[2].d++, ptrs[3].d++);
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_id0odddd_s_(int narg, int ps[], void (*f)(double, double, double *, double *, double *, double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;oD*;oD*;oD*;oD*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, 0.0, ptrs[1].d++, ptrs[2].d++, ptrs[3].d++, ptrs[4].d++);
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_id0odddddddddddddddd_s_(int narg, int ps[], void (*f)(double, double, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;oD*;oD*;oD*;oD*;oD*;oD*;oD*;oD*;oD*;oD*;oD*;oD*;oD*;oD*;oD*;oD*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, 0.0, ptrs[1].d++, ptrs[2].d++, ptrs[3].d++, ptrs[4].d++,
      ptrs[5].d++, ptrs[6].d++, ptrs[7].d++, ptrs[8].d++, ptrs[9].d++,
      ptrs[10].d++, ptrs[11].d++, ptrs[12].d++, ptrs[13].d++, ptrs[14].d++,
      ptrs[15].d++, ptrs[16].d++);
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_id0ollldrl_cal_f_(int narg, int ps[], int (*f)(double, double, int *, int *, int *, double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq, y, m, d;
  double fd;

  if ((iq = standard_args(narg, ps, "i>L*;r>D+3*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  
  switch (infos[0].type) {
  case ANA_LONG:
    while (infos[0].nelem--) {
      if (f((double) *ptrs[0].l++, 0.0, &y, &m, &d, &fd)) {
        ptrs[1].d[0] = 0.0;
        ptrs[1].d[1] = 0.0;
        ptrs[1].d[2] = 0.0;
      } else {
        ptrs[1].d[0] = (double) y;
        ptrs[1].d[1] = (double) m;
        ptrs[1].d[2] = (double) d + fd;
      }
      ptrs[1].d += 3;
    }
    break;
  case ANA_FLOAT:
    while (infos[0].nelem--) {
      if (f((double) *ptrs[0].f++, 0.0, &y, &m, &d, &fd)) {
        ptrs[1].d[0] = 0.0;
        ptrs[1].d[1] = 0.0;
        ptrs[1].d[2] = 0.0;
      } else {
        ptrs[1].d[0] = (double) y;
        ptrs[1].d[1] = (double) m;
        ptrs[1].d[2] = (double) d + fd;
      }
      ptrs[1].d += 3;
    }
    break;
  case ANA_DOUBLE:
    while (infos[0].nelem--) {
      if (f(*ptrs[0].d++, 0.0, &y, &m, &d, &fd)) {
        ptrs[1].d[0] = 0.0;
        ptrs[1].d[1] = 0.0;
        ptrs[1].d[2] = 0.0;
      } else {
        ptrs[1].d[0] = (double) y;
        ptrs[1].d[1] = (double) m;
        ptrs[1].d[2] = (double) d + fd;
      }
      ptrs[1].d += 3;
    }
    break;
  default:
    return cerror(ILL_ARG, ps[0]);
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_id0rd_f_(int narg, int ps[], double (*f)(double, double))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;rD*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--)
    *ptrs[1].d++ = f(*ptrs[0].d++, 0.0);
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_id3DDob3rl_f_(int narg, int ps[], int (*f)(double, double, double, double, double, double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D3*;iD1;iD1;rD3*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  size_t nelem = infos[0].nelem/3;
  while (nelem--) {
    f(*ptrs[1].d, *ptrs[2].d, ptrs[0].d[0], ptrs[0].d[1], ptrs[0].d[2], ptrs[3].d);
    ptrs[0].d += 3;
    ptrs[3].d += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_id3rd_f_(int narg, int ps[], double (*f)(double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D3*;rD-3*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--) {
    *ptrs[1].d++ = f(ptrs[0].d);
    ptrs[0].d += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_idc33_s_(int narg, int ps[], void (*f)(double, double [3][3]))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D+3,+3*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, (double (*)[3]) ptrs[1].d);
    ptrs[1].d += 9;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_iddd0odd_s_(int narg, int ps[], void (*f)(double, double, double, double, double *, double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;oD*;oD*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, 0.0, ptrs[3].d++, ptrs[4].d++);
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_iddd0odddd_s_(int narg, int ps[], void (*f)(double, double, double, double, double *, double *, double *, double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D;oD*;oD*;oD*;oD*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, 0.0, ptrs[3].d++,
      ptrs[4].d++, ptrs[5].d++, ptrs[6].d++);
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_iddddc33rd_mod_f_(int narg, int ps[], double (*f)(double, double, double, double, double (*)[3]))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D+3,+3*;rD*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  double (*rnpb)[3] = (double (*)[3]) ptrs[2].d;
  while (infos[0].nelem--) {
    double uta, utb, tta, ttb;
    uta = floor(*ptrs[0].d);
    utb = *ptrs[0].d++ - uta;
    tta = floor(*ptrs[1].d);
    ttb = *ptrs[1].d++ - tta;
    *ptrs[3].d++ = f(uta, utb, tta, ttb, rnpb);
    rnpb += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_iddddddd0d0oddddddrl_s_(int narg, int ps[], int (*f)(double, double, double, double, double, double, double, double, double, double, double *, double *, double *, double *, double *, double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;i>D*;i>D*;i>D*;i>D*;i>D*;oD*;oD*;oD*;oD*;oD*;oD*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++, *ptrs[4].d++,
      *ptrs[5].d++, *ptrs[6].d++, 0.0, *ptrs[7].d++, 0.0, ptrs[8].d++,
      ptrs[9].d++, ptrs[10].d++, ptrs[11].d++, ptrs[12].d++, ptrs[13].d++);
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_iddddddoc23rl_f_(int narg, int ps[], int (*f)(double, double, double, double, double, double, double [2][3]))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;i>D*;i>D*;i>D*;oD+2,+3*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++, *ptrs[4].d++,
      *ptrs[5].d++, (double (*)[3]) ptrs[6].d);
    ptrs[6].d += 6;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_iddddddoc23_f_(int narg, int ps[], void (*f)(double, double, double, double, double, double, double [2][3]))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;i>D*;i>D*;i>D*;oD+2,+3*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++, *ptrs[4].d++,
      *ptrs[5].d++, (double (*)[3]) ptrs[6].d);
    ptrs[6].d += 6;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_iddddddodddddd_s_(int narg, int ps[], void (*f)(double, double, double, double, double, double, double *, double *, double *, double *, double *, double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;i>D*;i>D*;i>D*;oD*;oD*;oD*;oD*;oD*;oD*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++,
      *ptrs[4].d++, *ptrs[5].d++, ptrs[6].d++, ptrs[7].d++,
      ptrs[8].d++, ptrs[9].d++, ptrs[10].d++, ptrs[11].d++);
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_iddddoc33_f_(int narg, int ps[], void (*f)(double, double, double, double, double (*)[3]))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;i>D*;rD+3,+3*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  double (*r)[3] = (double (*)[3]) ptrs[4].d;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++, r);
    r += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_iddddodd_s_(int narg, int ps[], void (*f)(double, double, double, double, double *, double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;i>D*;oD*;oD*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++,
      ptrs[4].d++, ptrs[5].d++);
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_iddddrd_f_(int narg, int ps[], double (*f)(double, double, double, double))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;i>D*;rD*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--)
    *ptrs[4].d++ = f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++);
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_iddddrd_mod_f_(int narg, int ps[], double (*f)(double, double, double, double))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;rD*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--) {
    double uta, utb, tta, ttb;
    uta = floor(*ptrs[0].d);
    utb = *ptrs[0].d++ - uta;
    tta = floor(*ptrs[1].d);
    ttb = *ptrs[1].d++ - tta;
    *ptrs[2].d++ = f(uta, utb, tta, ttb);
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_idddob3_f_(int narg, int ps[], void (*f)(double, double, double, double [3]))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;rD+3*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, ptrs[3].d);
    ptrs[3].d += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_idddoc33_f_(int narg, int ps[], void (*f)(double, double, double, double (*)[3]))
{
  int iq;
  double (*r)[3];
  pointer *ptrs;
  loopInfo *infos;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;rD+3,+3*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  r = (double (*)[3]) ptrs[3].d;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, r);
    r += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_iddob3_f_(int narg, int ps[], void (*f)(double, double, double [3]))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;rD+3*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, *ptrs[1].d++, ptrs[2].d);
    ptrs[2].d += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_iddrd_f_(int narg, int ps[], double (*f)(double, double))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>L*;rD*", &ptrs, &infos)) < 0)
    return ANA_ERROR;

  switch (infos[0].type) {
  case ANA_LONG:
    while (infos[1].nelem--)
      *ptrs[1].d++ = f((double) *ptrs[0].l++, 0.0);
    break;
  case ANA_FLOAT:
    while (infos[1].nelem--)
      *ptrs[1].d++ = f((double) *ptrs[0].f++, 0.0);
    break;
  case ANA_DOUBLE:
    while (infos[1].nelem--)
      *ptrs[1].d++ = f(*ptrs[0].d++, 0.0);
    break;
  }
  return iq;  
}
/*-----------------------------------------------------------------------*/
int ana_idrd_f_(int narg, int ps[], double (*f)(double))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;rD*", &ptrs, &infos)) < 0)
    return ANA_ERROR;

  while (infos[0].nelem--)
    *ptrs[1].d++ = f(*ptrs[0].d++);
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_ilob2rl_f_(int narg, int ps[], int (*f)(int, double *, double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "iL*;rD+2*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--) {
    f(*ptrs[0].l++, &ptrs[1].d[0], &ptrs[1].d[1]);
    ptrs[1].d += 2;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_oC33_f_(int narg, int ps[], void (*f)(double (*)[3]))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "rD3,3", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  f((double (*)[3]) ptrs[0].d);
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_oC33B3_s_(int narg, int ps[], void (*f)(double (*)[3], double *))
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "oD3,3;oD3", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  f((double (*)[3]) ptrs[0].d, (double *) ptrs[1].d);
  return iq;
}
/*-----------------------------------------------------------------------*/
int ana_oddd_combine_f_(int narg, int ps[], void (*f)(double *, double *, double *))
{
  pointer *tgts;
  int iq;

  if ((iq = standard_args(narg, ps, "rD3", &tgts, NULL)) < 0)
    return ANA_ERROR;
  f(&tgts[0].d[0], &tgts[0].d[1], &tgts[0].d[2]);
  return iq;
}
/*-----------------------------------------------------------------------*/
