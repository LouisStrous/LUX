#include "anadefs.h"
#include "error.h"
#include <math.h>
#include <obstack.h>
#include "bindings.h"
#define obstack_chunk_alloc malloc
#define obstack_chunk_free free

extern Int internalMode;

Int standard_args(Int, Int [], char const *, pointer **, loopInfo **);
Int setAxes(loopInfo *, Int, Int *, Int);
Int advanceLoop(loopInfo *, pointer *);

struct obstack *registered_functions = NULL, *registered_subroutines = NULL;

void register_ana_f(Int (*f)(Int, Int []), char *name, Int min_arg,
                    Int max_arg, char *spec)
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
void register_ana_s(Int (*f)(Int, Int []), char *name, Int min_arg,
                    Int max_arg, char * spec)
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
/*
  The name of the binding functions is built up as follows:

  ana_<boundfuncspec>_<stdargspec>_<ptrspec>_<sf>_

  <boundfuncspec> = an encoding of the declaration of the bound C
  function, consisting of <returntype>_<paramtypes>.

  <returntype> = an encoding of the return type of the bound C
  function.

  <paramtypes> = concatenated encoding of the parameter types of the
  bound C function.

  For the encodings in <returntype> and <paramtypes>, 'void' is
  encoded as 'v', 'Double' as 'd', 'Int' as 'i', 'Double *' as 'dp',
  'Double (*)[3][4]' as 'dp34'.  The encoding 'sd' refers to three
  adjacent parameters (Double *data, size_t count, size_t stride) that
  describe a 'Double' vector slice.  A repeat of a particular
  parameter type (for a repeat count of at least 3) is indicated by
  appending 'T' and the repeat count.  For example, (Double, Double,
  Double) is encoded as dT3.

  <stdargspec> = an encoding of the arguments type specification in
  the most elaborate call to standard_args for this binding.  The
  encoding is obtained from the arguments type specification by:

  - removing '>', ';', and all but the first 'i', all but the
    first 'o', and all but the first 'r'
  - changing '&' to 'q', '+' to 'p', '-' to 'm', '*' to 'a'
  - changing '[...]' to 'c', '{...}' to 'x'

  A repeat of a particular unit (from a variable type specification
  like 'D' to up to but not including the next one; for a repeat count
  of at least 3, or if the abbreviated version is shorter than the
  full version) is indicated by appending 'T' and the repeat count.
  For example, 'DqDqDqDq' gets abbreviated to 'DqT4'.

  <ptrspec> = the concatenation of the pointer index that is used for
  each of the bound function's parameters.  If the bound function's
  return value is stored using a pointer/info index, then that pointer
  index is concatenated at the end, after '_'.  If a particular bound
  function's parameter does not depend on a pointer/info index, then
  'z' is specified for it.  3 or more adjacent indices are replaced by
  <first-index>T<last-index>.

  <sf> = 's' if the binding is to an ANA function, 's' if to an ANA
  subroutine.

 */
/*-----------------------------------------------------------------------*/
Int ana_v_dT3dp3_iDT3rDp3p3_0T3_f_(Int narg, Int ps[], void (*f)(Double, Double, Double, Double (*)[3]))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D;i>D;i>D;rD+3,+3", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  f(*ptrs[0].d, *ptrs[1].d, *ptrs[2].d, (Double (*)[3]) ptrs[3].d);
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_i_idpT4_iL1D3arDcq_0T222_f_(Int narg, Int ps[], Int (*f)(Int, Double *, Double *, Double *, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "iL1;i>D3*;rD[-]&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  size_t nelem = infos[1].nelem/3;
  while (nelem--) {
    f(*ptrs[0].l, ptrs[1].d, &ptrs[2].d[0], &ptrs[2].d[1], &ptrs[2].d[2]);
    ptrs[1].d += 3;
    ptrs[2].d += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_i_idT3dp_iL1D3arDcq_10002_f_(Int narg, Int ps[], Int (*f)(Int, Double, Double, Double, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "iL1;i>D3*;rD[-]&", &ptrs, &infos)) < 0)
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
Int ana_v_dT3d3_iD3aD1D1rD3q_120333_f_(Int narg, Int ps[], Int (*f)(Double, Double, Double *, Double *, Double *, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D3*;i>D1;i>D1;rD3&", &ptrs, &infos)) < 0)
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
Int ana_v_dT3d3_iD3DcqrDcq_0T2_f_(Int narg, Int ps[], void (*f)(Double *, Double *, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D3*;i>D[-]&;rD[-]&", &ptrs, &infos)) < 0)
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
Int ana_d_dpdp_iD3aD3qrDm3q_01_2_f_(Int narg, Int ps[], Double (*f)(Double *, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D3*;i>D3&;rD-3&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[2].nelem--) {
    *ptrs[2].d++ = f(ptrs[0].d, ptrs[1].d);
    ptrs[0].d += 3;
    ptrs[1].d += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dpT3_iD3arDm3p2q_011_f_(Int narg, Int ps[], void (*f)(Double *, Double *, Double *))
{
  Int iq;
  pointer *ptrs;
  loopInfo *infos;

  if ((iq = standard_args(narg, ps, "i>D3*;r>D-3+2&", &ptrs, &infos)) < 0)
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
Int ana_v_dpdp3_iD3arDp3q_01_f_(Int narg, Int ps[], void (*f)(Double [3], Double [3][3]))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D3*;rD+3&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--) {
    f(ptrs[0].d, (Double (*)[3]) ptrs[1].d);
    ptrs[0].d += 3;
    ptrs[1].d += 9;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dpT4_iD3arD3q_0111_f_(Int narg, Int ps[], void (*f)(Double *, Double *, Double *, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D3*;rD3&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  Int nelem = infos[0].nelem/3;
  while (nelem--) {
    f(ptrs[0].d, &ptrs[1].d[0], &ptrs[1].d[1], &ptrs[1].d[2]);
    ptrs[0].d += 3;
    ptrs[1].d += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dpT3_iD3aoDm3qoDq_0T2_s_(Int narg, Int ps[], void (*f)(Double *, Double *, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D3*;oD-3&;oD&", &ptrs, &infos)) < 0)
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
Int ana_i_dpiidp_iDaLrDq_00T2_f_(Int narg, Int ps[], Int (*f)(Double *, Int, Int, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;iL;rD&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  size_t nrepeat = infos[0].nelem/infos[0].rdims[0];
  while (nrepeat--) {
    f(ptrs[0].d, infos[0].rdims[0], *ptrs[1].l, ptrs[2].d);
    ptrs[0].d += infos[0].rdims[0];
    ptrs[2].d += infos[0].rdims[0];
  }
  return iq;  
}
/*-----------------------------------------------------------------------*/
Int ana_i_dpiT3dp_iDaiLiLrDq_00T3_f_(Int narg, Int ps[], Int (*f)(Double *, Int, Int, Int, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;iL;iL;rD&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  size_t nrepeat = infos[0].nelem/infos[0].rdims[0];
  while (nrepeat--) {
    f(ptrs[0].d, infos[0].rdims[0], *ptrs[1].l, *ptrs[2].l, ptrs[3].d);
    ptrs[0].d += infos[0].rdims[0];
    ptrs[3].d += infos[0].rdims[0];
  }
  return iq;  
}
/*-----------------------------------------------------------------------*/
Int ana_v_dp3dp3dp_iD23aD23qoDcm3q_0T2_f_(Int narg, Int ps[], void (*f)(Double [2][3], Double [2][3], Double [2]))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D2,3*;i>D2,3&;oD[-],-3&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  size_t nelem = infos[0].nelem/6;
  while (nelem--) {
    f((Double (*)[3]) ptrs[0].d, (Double (*)[3]) ptrs[1].d, 
      (Double *) ptrs[2].d);
    ptrs[0].d += 6;
    ptrs[1].d += 6;
    ptrs[2].d += 2;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dp3T3_iD23aD23qoDcq_0T2_f_(Int narg, Int ps[], void (*f)(Double [2][3], Double [2][3], Double [2][3]))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D2,3*;i>D2,3&;oD[-]&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  size_t nelem = infos[0].nelem/6;
  while (nelem--) {
    f((Double (*)[3]) ptrs[0].d, (Double (*)[3]) ptrs[1].d, 
      (Double (*)[3]) ptrs[2].d);
    ptrs[0].d += 6;
    ptrs[1].d += 6;
    ptrs[2].d += 6;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dp3dpdp_iD23aoDm2m3qDcq_0T2_s_(Int narg, Int ps[], void (*f)(Double [2][3], Double *, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D2,3*;oD-2-3&;oD[-]&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  size_t nelem = infos[0].nelem/6;
  while (nelem--) {
    f((Double (*)[3]) ptrs[0].d, ptrs[1].d++, ptrs[2].d++);
    ptrs[0].d += 6;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dp3dpT6_iD23aoDm2m3aDcT5_0T6_s_(Int narg, Int ps[], void (*f)(Double [2][3], Double *, Double *, Double *, Double *, Double *, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D2,3*;oD-2-3*;oD[-];oD[-];oD[-];oD[-];oD[-]", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  size_t nelem = infos[0].nelem/6;
  while (nelem--) {
    f((Double (*)[3]) ptrs[0].d, ptrs[1].d++, ptrs[2].d++, ptrs[3].d++,
      ptrs[4].d++, ptrs[5].d++, ptrs[6].d++);
    ptrs[0].d += 6;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_i_dp3dpT6_iD23aoDm2m3aDcT5_0T6_s_(Int narg, Int ps[], Int (*f)(Double [2][3], Double *, Double *, Double *, Double *, Double *, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D2,3*;oD-2-3*;oD[-];oD[-];oD[-];oD[-];oD[-]", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  size_t nelem = infos[0].nelem/6;
  while (nelem--) {
    f((Double (*)[3]) ptrs[0].d, ptrs[1].d++, ptrs[2].d++, ptrs[3].d++,
      ptrs[4].d++, ptrs[5].d++, ptrs[6].d++);
    ptrs[0].d += 6;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dp3dpdp_iD33aDm3arDcq_0T2_f_(Int narg, Int ps[], void (*f)(Double [3][3], Double [3], Double [3]))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq, step;

  iq = ANA_ERROR;
  if ((iq = standard_args(narg, ps, "i>D3,3*;i>D-3*;rD[-]&", &ptrs, &infos))
      > 0)
    step = 9;
  else if ((iq = standard_args(narg, ps, "i>D3,3;i>D-3*;rD[-]&", &ptrs, &infos))
           > 0)
    step = 0;
  if (iq > 0) {
    size_t nelem = infos[1].nelem/3;
    while (nelem--) {
      f((Double (*)[3]) ptrs[0].d, ptrs[1].d, ptrs[2].d);
      ptrs[0].d += step;
      ptrs[1].d += 3;
      ptrs[2].d += 3;
    }
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dp3T3_iD33aDaoDc_0T2_f_(Int narg, Int ps[], void (*f)(Double [3][3], Double [2][3], Double [2][3]))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D3,3*;iD*;oD[-]", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  size_t nelem = infos[0].nelem/9;
  while (nelem--) {
    f((Double (*)[3]) ptrs[0].d, (Double (*)[3]) ptrs[1].d,
      (Double (*)[3]) ptrs[2].d);
    ptrs[0].d += 9;
    ptrs[1].d += 6;
    ptrs[2].d += 6;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dp3T3_iD33aDarDq_0T2_f_(Int narg, Int ps[], void (*f)(Double (*)[3], Double (*)[3], Double (*)[3]))
{
  Int iq;
  pointer *ptrs;
  loopInfo *infos;
  size_t nelem;

  if ((iq = standard_args(narg, ps, "i>D3,3*;i>D*;rD&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  Double (*r1)[3] = (Double (*)[3]) ptrs[0].d;
  Double (*r2)[3] = (Double (*)[3]) ptrs[1].d;
  Double (*r3)[3] = (Double (*)[3]) ptrs[2].d;
  nelem = infos[0].nelem/9;
  while (nelem--) {
    f(r1, r2, r3);
    r1 += 3;
    r2 += 3;
    r3 += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dp3ddp3dp3_iD33aDmmaDarDq_0T3_f_(Int narg, Int ps[], void (*f)(Double (*)[3], Double, Double (*)[3], Double (*)[3]))
{
  Int iq;
  pointer *ptrs, era;
  loopInfo *infos;
  size_t nelem;

  if ((iq = standard_args(narg, ps, "i>D3,3*;i>D-,-*;i>D*;rD&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  era = ptrs[1];
  Double (*rc2i)[3] = (Double (*)[3]) ptrs[0].d;
  Double (*rpom)[3] = (Double (*)[3]) ptrs[2].d;
  Double (*rc2t)[3] = (Double (*)[3]) ptrs[3].d;
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
Int ana_d_dp3d_iD33aDmmarDmmq_01_2_f_(Int narg, Int ps[], Double (*f)(Double (*)[3], Double))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D3,3*;i>D-,-*;rD-,-&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  Double (*rnpb)[3] = (Double (*)[3]) ptrs[0].d;
  while (infos[1].nelem--) {
    *ptrs[2].d++ = f(rnpb, *ptrs[1].d++);
    rnpb += 3;
  }
  return iq;  
}
/*-----------------------------------------------------------------------*/
Int ana_v_dp3dp_iD33arDm3q_01_f_(Int narg, Int ps[], void (*f)(Double [3][3], Double [3]))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D3,3*;rD-3&", &ptrs, &infos)) < 0)
    return ANA_ERROR;

  size_t nelem = infos[0].nelem/9;
  while (nelem--) {
    f((Double (*)[3]) ptrs[0].d, ptrs[1].d);
    ptrs[0].d += 9;
    ptrs[1].d += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dp3dp3_iD33arDq_01_f_(Int narg, Int ps[], void (*f)(Double [3][3], Double [3][3]))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D3,3*;rD&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  
  size_t nelem = infos[0].nelem/9;
  while (nelem--) {
    f((Double (*)[3]) ptrs[0].d, (Double (*)[3]) ptrs[1].d);
    ptrs[0].d += 9;
    ptrs[1].d += 9;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dp3dpdp_iD33aoDm3m3qT2_0T2_s_(Int narg, Int ps[], void (*f)(Double (*)[3], Double *, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D3,3*;oD-3,-3&;oD-3,-3&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  
  size_t nelem = infos[0].nelem/9;
  while (nelem--) {
    f((Double (*)[3]) ptrs[0].d, ptrs[1].d++, ptrs[2].d++);
    ptrs[0].d += 9;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dT8dp3_iDaT5rDp3p3q_0zzz1T5_f_(Int narg, Int ps[], void (*f)(Double, Double, Double, Double, Double, Double, Double, Double, Double (*)[3]))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;i>D*;i>D*;rD+3,+3&",
                          &ptrs, &infos)) < 0)
    return ANA_ERROR;
  Double *jd = ptrs[0].d;
  Double *dpsi = ptrs[1].d;
  Double *deps = ptrs[2].d;
  Double *xp = ptrs[3].d;
  Double *yp = ptrs[4].d;
  Double (*rc2t)[3] = (Double (*)[3]) ptrs[5].d;
  size_t nelem = infos[0].nelem;
  while (nelem--) {
    f(*jd++, 0, 0, 0, *dpsi++, *deps++, *xp++, *yp++, rc2t);
    rc2t += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dT6dp3_iDaT4rDp3p3q_0z11T4_f_(Int narg, Int ps[], void (*f)(Double, Double, Double, Double, Double, Double, Double (*)[3]))
{
  Int iq;
  pointer *ptrs;
  loopInfo *infos;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;i>D*;rD+3,+3&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--) {
    Double d, t;
    d = floor(*ptrs[1].d - 0.5) + 0.5;
    t = *ptrs[1].d - d;
    f(*ptrs[0].d++, 0.0, d, t, *ptrs[2].d++, *ptrs[3].d++, (Double (*)[3]) ptrs[4].d);
    ptrs[4].d += 9;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_d_dT6_iLaDaD1T3rDq_0z1T4_5_f_(Int narg, Int ps[], Double (*f)(Double, Double, Double, Double, Double, Double))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>L*;i>D*;i>D1;i>D1;i>D1;r>D&", &ptrs, &infos)) < 0)
    return ANA_ERROR;

  switch (infos[0].type) {
  case ANA_LONG:
    while (infos[0].nelem--)
      *ptrs[5].d++ = f((Double) *ptrs[0].l++, 0.0, *ptrs[1].d, *ptrs[2].d,
                       *ptrs[3].d, *ptrs[4].d);
    break;
  case ANA_FLOAT:
    while (infos[0].nelem--)
      *ptrs[5].d++ = f((Double) *ptrs[0].f++, 0.0, *ptrs[1].d, *ptrs[2].d,
                       *ptrs[3].d, *ptrs[4].d);
    break;
  case ANA_DOUBLE:
    while (infos[0].nelem--)
      *ptrs[5].d++ = f(*ptrs[0].d++, 0.0, *ptrs[1].d, *ptrs[2].d, 
                       *ptrs[3].d, *ptrs[4].d);
    break;
  default:
    break;
  }
  return iq;  
}
/*-----------------------------------------------------------------------*/
Int ana_i_ddidp3_iDaL1rDp3p2q_0z12_f_(Int narg, Int ps[], Int (*f)(Double, Double, Int, Double (*)[3]))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;iL1;rD+3,+2&", &ptrs, &infos)) < 0)
    return ANA_ERROR;

  while (infos[0].nelem--) {
    f(*ptrs[0].d++, 0.0, *ptrs[1].l, (Double (*)[3]) ptrs[2].d);
    ptrs[2].d += 6;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_i_dddp3dp3_iLaoDp2p3qT2_0z12_s_(Int narg, Int ps[], Int (*f)(Double, Double, Double (*)[3], Double (*)[3]))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>L*;oD+2,+3&;oD+2,+3&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  Double (*pvh)[3] = (Double (*)[3]) ptrs[1].d;
  Double (*pvb)[3] = (Double (*)[3]) ptrs[2].d;

  switch (infos[0].type) {
  case ANA_LONG:
    while (infos[0].nelem--) {
      f((Double) *ptrs[0].l++, 0.0, pvh, pvb);
      pvh += 2;
      pvb += 2;
    }
    break;
  case ANA_FLOAT:
    while (infos[0].nelem--) {
      f((Double) *ptrs[0].f++, 0.0, pvh, pvb);
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
  default:
    break;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dddp3dp3_iDaiDp3p3arDcq_0z12_f_(Int narg, Int ps[], void (*f)(Double, Double, Double (*)[3], Double (*)[3]))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D+3,+3*;rD[-]&", &ptrs, &infos)) < 0)
    return ANA_ERROR;

  Double (*r1)[3] = (Double (*)[3]) ptrs[1].d;
  Double (*r2)[3] = (Double (*)[3]) ptrs[2].d;

  while (infos[0].nelem--) {
    f(*ptrs[0].d++, 0.0, r1, r2);
    r1 += 3;
    r2 += 3;
  }
  return iq;  
}
/*-----------------------------------------------------------------------*/
Int ana_v_dT4dpT3_iDaDaoDqT3_0z1z2T4_s_(Int narg, Int ps[], void (*f)(Double, Double, Double, Double, Double *, Double *, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;oD&;oD&;oD&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, 0.0, *ptrs[1].d++, 0.0, ptrs[2].d++, ptrs[3].d++, ptrs[4].d++);
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dT4dp3_iDaT3rDp3p3q_0z1T3_f_(Int narg, Int ps[], void (*f)(Double, Double, Double, Double, Double (*)[3]))
{
  Int iq;
  Double (*tgt)[3];
  pointer *ptrs;
  loopInfo *infos;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;rD+3,+3&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  tgt = (Double (*)[3]) ptrs[3].d;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, 0.0, *ptrs[1].d++, *ptrs[2].d++, tgt);
    tgt += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dddpT3dp3T5_iDaoDqT3Dp3p3qT5_0z1T8_s_(Int narg, Int ps[], void (*f)(Double, Double, Double *, Double *, Double *, Double (*)[3], Double (*)[3], Double (*)[3], Double (*)[3], Double (*)[3]))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;oD&;oD&;oD&;oD+3,+3&;oD+3,+3&;oD+3,+3&;oD+3,+3&;oD+3,+3&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, 0.0, ptrs[1].d++, ptrs[2].d++, ptrs[3].d++,
      (Double (*)[3]) ptrs[4].d, (Double (*)[3]) ptrs[5].d,
      (Double (*)[3]) ptrs[6].d, (Double (*)[3]) ptrs[7].d,
      (Double (*)[3]) ptrs[8].d);
    ptrs[4].d += 9;
    ptrs[5].d += 9;
    ptrs[6].d += 9;
    ptrs[7].d += 9;
    ptrs[8].d += 9;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dT4dpdp3T5_iDaT3oDqDp3p3qT5_0z1T8_s_(Int narg, Int ps[], void (*f)(Double, Double, Double, Double, Double *, Double (*)[3], Double (*)[3], Double (*)[3], Double (*)[3], Double (*)[3]))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;oD&;oD+3,+3&;oD+3,+3&;oD+3,+3&;oD+3,+3&;oD+3,+3&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, 0.0, *ptrs[1].d++, *ptrs[2].d++, ptrs[3].d++,
      (Double (*)[3]) ptrs[4].d, (Double (*)[3]) ptrs[5].d,
      (Double (*)[3]) ptrs[6].d, (Double (*)[3]) ptrs[7].d,
      (Double (*)[3]) ptrs[8].d);
    ptrs[4].d += 9;
    ptrs[5].d += 9;
    ptrs[6].d += 9;
    ptrs[7].d += 9;
    ptrs[8].d += 9;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_d_dT4_iLaDaDarDq_0z12_f_(Int narg, Int ps[], Double (*f)(Double, Double, Double, Double))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>L*;i>D*;i>D*;r>D&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  switch (infos[0].type) {
  case ANA_LONG:
    while (infos[1].nelem--)
      *ptrs[3].d++ = f((Double) *ptrs[0].l++, 0.0, *ptrs[1].d++, *ptrs[2].d++);
    break;
  case ANA_FLOAT:
    while (infos[1].nelem--)
      *ptrs[3].d++ = f((Double) *ptrs[0].f++, 0.0, *ptrs[1].d++, *ptrs[2].d++);
    break;
  case ANA_DOUBLE:
    while (infos[1].nelem--)
      *ptrs[3].d++ = f(*ptrs[0].d++, 0.0, *ptrs[1].d++, *ptrs[2].d++);
    break;
  default:
    break;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_i_dT3dpdp_iDaDarDq_0z122_f_(Int narg, Int ps[], Int (*f)(Double, Double, Double, Double *, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;rD&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--) {
    Double ut1, ut2;
    f(*ptrs[0].d++, 0.0, *ptrs[1].d++, &ut1, &ut2);
    *ptrs[2].d++ = ut1 + ut2;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dddp3_iDarDp3p3q_0z1_f_(Int narg, Int ps[], void (*f)(Double, Double, Double (*)[3]))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;rD+3,+3&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  Double (*r)[3] = (Double (*)[3]) ptrs[1].d;

  while (infos[0].nelem--) {
    f(*ptrs[0].d++, 0.0, r);
    r += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dddp3T3_iLaoD33oDp3p3qDcq_0z1T3_s_(Int narg, Int ps[], void (*f)(Double, Double, Double (*)[3], Double (*)[3], Double (*)[3]))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>L*;oD3,3;oD+3,+3&;oD[-]&", &ptrs, &infos)) < 0)
    return ANA_ERROR;

  switch (infos[0].type) {
  case ANA_LONG:
    while (infos[0].nelem--) {
      f((Double) *ptrs[0].l++, 0.0, (Double (*)[3]) ptrs[1].d,
        (Double (*)[3]) ptrs[2].d, (Double (*)[3]) ptrs[3].d);
      ptrs[2].d += 9;
      ptrs[3].d += 9;
    }
    break;
  case ANA_FLOAT: 
    while (infos[0].nelem--) {
      f((Double) *ptrs[0].f++, 0.0, (Double (*)[3]) ptrs[1].d,
        (Double (*)[3]) ptrs[2].d, (Double (*)[3]) ptrs[3].d);
      ptrs[2].d += 9;
      ptrs[3].d += 9;
    }
    break;
  case ANA_DOUBLE:
    while (infos[0].nelem--) {
      f(*ptrs[0].d++, 0.0, (Double (*)[3]) ptrs[1].d,
        (Double (*)[3]) ptrs[2].d, (Double (*)[3]) ptrs[3].d);
      ptrs[2].d += 9;
      ptrs[3].d += 9;
    }
    break;
  default:
    break;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_i_dddpdp_iDarDq_0z11_f_(Int narg, Int ps[], Int (*f)(Double, Double, Double *, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;rD&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--) {
    Double tt1, tt2;
    f(*ptrs[0].d++, 0.0, &tt1, &tt2);
    *ptrs[1].d++ = tt1 + tt2;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dddpdp_iDaoDqDq_0z12_s_(Int narg, Int ps[], void (*f)(Double, Double, Double *, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;oD&;oD&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, 0.0, ptrs[1].d++, ptrs[2].d++);
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dddpT3_iDaoDqT3_0z1T3_s_(Int narg, Int ps[], void (*f)(Double, Double, Double *, Double *, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;oD&;oD&;oD&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, 0.0, ptrs[1].d++, ptrs[2].d++, ptrs[3].d++);
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dddpT4_iDaoDqT4_0z1T4_s_(Int narg, Int ps[], void (*f)(Double, Double, Double *, Double *, Double *, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;oD&;oD&;oD&;oD&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, 0.0, ptrs[1].d++, ptrs[2].d++, ptrs[3].d++, ptrs[4].d++);
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dddpT16_iDaoDqT16_0z1T16_s_(Int narg, Int ps[], void (*f)(Double, Double, Double *, Double *, Double *, Double *, Double *, Double *, Double *, Double *, Double *, Double *, Double *, Double *, Double *, Double *, Double *, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, 0.0, ptrs[1].d++, ptrs[2].d++, ptrs[3].d++, ptrs[4].d++,
      ptrs[5].d++, ptrs[6].d++, ptrs[7].d++, ptrs[8].d++, ptrs[9].d++,
      ptrs[10].d++, ptrs[11].d++, ptrs[12].d++, ptrs[13].d++, ptrs[14].d++,
      ptrs[15].d++, ptrs[16].d++);
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_iddipT3dp_iLarDp3q_0z1111_f_(Int narg, Int ps[], Int (*f)(Double, Double, Int *, Int *, Int *, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq, y, m, d;
  Double fd;

  if ((iq = standard_args(narg, ps, "i>L*;r>D+3&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  
  switch (infos[0].type) {
  case ANA_LONG:
    while (infos[0].nelem--) {
      if (f((Double) *ptrs[0].l++, 0.0, &y, &m, &d, &fd)) {
        ptrs[1].d[0] = 0.0;
        ptrs[1].d[1] = 0.0;
        ptrs[1].d[2] = 0.0;
      } else {
        ptrs[1].d[0] = (Double) y;
        ptrs[1].d[1] = (Double) m;
        ptrs[1].d[2] = (Double) d + fd;
      }
      ptrs[1].d += 3;
    }
    break;
  case ANA_FLOAT:
    while (infos[0].nelem--) {
      if (f((Double) *ptrs[0].f++, 0.0, &y, &m, &d, &fd)) {
        ptrs[1].d[0] = 0.0;
        ptrs[1].d[1] = 0.0;
        ptrs[1].d[2] = 0.0;
      } else {
        ptrs[1].d[0] = (Double) y;
        ptrs[1].d[1] = (Double) m;
        ptrs[1].d[2] = (Double) d + fd;
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
        ptrs[1].d[0] = (Double) y;
        ptrs[1].d[1] = (Double) m;
        ptrs[1].d[2] = (Double) d + fd;
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
Int ana_d_dd_iDarDq_0z_1_f_(Int narg, Int ps[], Double (*f)(Double, Double))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;rD&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--)
    *ptrs[1].d++ = f(*ptrs[0].d++, 0.0);
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_i_dT5dp_iD3aD1D1rD3q_120003_f_(Int narg, Int ps[], Int (*f)(Double, Double, Double, Double, Double, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D3*;iD1;iD1;rD3&", &ptrs, &infos)) < 0)
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
Int ana_d_dp_iD3arDm3q_0_1_f_(Int narg, Int ps[], Double (*f)(Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D3*;rD-3&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  Int nelem = infos[0].nelem/3;
  while (nelem--) {
    *ptrs[1].d++ = f(ptrs[0].d);
    ptrs[0].d += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_ddp3_iD1D33_01_s_(Int narg, Int ps[], void (*f)(Double, Double [3][3]))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D1;i>D3,3", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, (Double (*)[3]) ptrs[1].d);
    ptrs[1].d += 9;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dT4dpdp_iDaT3oDqDq_0T2z34_s_(Int narg, Int ps[], void (*f)(Double, Double, Double, Double, Double *, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;oD&;oD&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, 0.0, ptrs[3].d++, ptrs[4].d++);
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dT4dpT4_iDaDaDoDqT4_0T2z3T6_s_(Int narg, Int ps[], void (*f)(Double, Double, Double, Double, Double *, Double *, Double *, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D;oD&;oD&;oD&;oD&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, 0.0, ptrs[3].d++,
      ptrs[4].d++, ptrs[5].d++, ptrs[6].d++);
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_d_dT4dp3_iDaDaDp3p3arDq_00112_3_f_(Int narg, Int ps[], Double (*f)(Double, Double, Double, Double, Double (*)[3]))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D+3,+3*;rD&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  Double (*rnpb)[3] = (Double (*)[3]) ptrs[2].d;
  while (infos[0].nelem--) {
    Double uta, utb, tta, ttb;
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
Int ana_i_dT10dpT6_iDaT8oDqT6_0T6z7z8T13_s_(Int narg, Int ps[], Int (*f)(Double, Double, Double, Double, Double, Double, Double, Double, Double, Double, Double *, Double *, Double *, Double *, Double *, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;i>D*;i>D*;i>D*;i>D*;i>D*;oD&;oD&;oD&;oD&;oD&;oD&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++, *ptrs[4].d++,
      *ptrs[5].d++, *ptrs[6].d++, 0.0, *ptrs[7].d++, 0.0, ptrs[8].d++,
      ptrs[9].d++, ptrs[10].d++, ptrs[11].d++, ptrs[12].d++, ptrs[13].d++);
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_i_dT6dp3_iDaT6oDp2p3q_0T6_f_(Int narg, Int ps[], Int (*f)(Double, Double, Double, Double, Double, Double, Double [2][3]))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;i>D*;i>D*;i>D*;oD+2,+3&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++, *ptrs[4].d++,
      *ptrs[5].d++, (Double (*)[3]) ptrs[6].d);
    ptrs[6].d += 6;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dT6dp3_iDaT6oDp2p3q_0T6_f_(Int narg, Int ps[], void (*f)(Double, Double, Double, Double, Double, Double, Double [2][3]))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;i>D*;i>D*;i>D*;oD+2,+3&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++, *ptrs[4].d++,
      *ptrs[5].d++, (Double (*)[3]) ptrs[6].d);
    ptrs[6].d += 6;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dT6dpT6_iDaT6oDqT6_0T11_s_(Int narg, Int ps[], void (*f)(Double, Double, Double, Double, Double, Double, Double *, Double *, Double *, Double *, Double *, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;i>D*;i>D*;i>D*;oD&;oD&;oD&;oD&;oD&;oD&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++,
      *ptrs[4].d++, *ptrs[5].d++, ptrs[6].d++, ptrs[7].d++,
      ptrs[8].d++, ptrs[9].d++, ptrs[10].d++, ptrs[11].d++);
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dT4dp3_iDaT4rDp3p3q_0T4_f_(Int narg, Int ps[], void (*f)(Double, Double, Double, Double, Double (*)[3]))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;i>D*;rD+3,+3&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  Double (*r)[3] = (Double (*)[3]) ptrs[4].d;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++, r);
    r += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dT4dpdp_iDaT4oDqDq_0T5_s_(Int narg, Int ps[], void (*f)(Double, Double, Double, Double, Double *, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;i>D*;oD&;oD&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++,
      ptrs[4].d++, ptrs[5].d++);
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_d_dT4_iDaT4rDq_0T3_4_f_(Int narg, Int ps[], Double (*f)(Double, Double, Double, Double))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;i>D*;rD&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--)
    *ptrs[4].d++ = f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++);
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_d_dT4_iDaDarDq_0011_2_f_(Int narg, Int ps[], Double (*f)(Double, Double, Double, Double))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;rD&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--) {
    Double uta, utb, tta, ttb;
    uta = floor(*ptrs[0].d);
    utb = *ptrs[0].d++ - uta;
    tta = floor(*ptrs[1].d);
    ttb = *ptrs[1].d++ - tta;
    *ptrs[2].d++ = f(uta, utb, tta, ttb);
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dT3dp_iDaT3rDp3q_0T3_f_(Int narg, Int ps[], void (*f)(Double, Double, Double, Double [3]))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;rD+3&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, ptrs[3].d);
    ptrs[3].d += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dT3dp3_iDaT3rDp3p3q_0T3_f_(Int narg, Int ps[], void (*f)(Double, Double, Double, Double (*)[3]))
{
  Int iq;
  Double (*r)[3];
  pointer *ptrs;
  loopInfo *infos;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;rD+3,+3&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  r = (Double (*)[3]) ptrs[3].d;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, r);
    r += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dddp_iDaDarDp3q_0T2_f_(Int narg, Int ps[], void (*f)(Double, Double, Double [3]))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;rD+3&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, *ptrs[1].d++, ptrs[2].d);
    ptrs[2].d += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_d_dd_iLarDq_0z_1_f_(Int narg, Int ps[], Double (*f)(Double, Double))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>L*;rD&", &ptrs, &infos)) < 0)
    return ANA_ERROR;

  switch (infos[0].type) {
  case ANA_LONG:
    while (infos[1].nelem--)
      *ptrs[1].d++ = f((Double) *ptrs[0].l++, 0.0);
    break;
  case ANA_FLOAT:
    while (infos[1].nelem--)
      *ptrs[1].d++ = f((Double) *ptrs[0].f++, 0.0);
    break;
  case ANA_DOUBLE:
    while (infos[1].nelem--)
      *ptrs[1].d++ = f(*ptrs[0].d++, 0.0);
    break;
  default:
    break;
  }
  return iq;  
}
/*-----------------------------------------------------------------------*/
Int ana_d_d_iDarDq_0_1_f_(Int narg, Int ps[], Double (*f)(Double))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;rD&", &ptrs, &infos)) < 0)
    return ANA_ERROR;

  while (infos[0].nelem--)
    *ptrs[1].d++ = f(*ptrs[0].d++);
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_i_idpdp_iLarDp2q_011_f_(Int narg, Int ps[], Int (*f)(Int, Double *, Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "iL*;rD+2&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--) {
    f(*ptrs[0].l++, &ptrs[1].d[0], &ptrs[1].d[1]);
    ptrs[1].d += 2;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_d_sd_iDaLarDxq_000_2_f_(Int narg, Int ps[], Double (*f)(Double *, size_t count, size_t stride))
{
  pointer *ptrs, ptrs0, ptrsr;
  loopInfo *infos;
  Int iq, iret;
  Int *axes, naxes, oneaxis[1] = { 0 }, allaxes;

  switch (narg) {
  case 1:                       /* source */
    if ((iq = standard_args(narg, ps, "i>D*;rD-&", &ptrs, &infos)) < 0)
      return ANA_ERROR;
    axes = oneaxis;
    naxes = 1;
    setAxes(&infos[0], 0, NULL, SL_EACHROW);
    iret = 1;
    break;    
  case 2:                       /* source, axes */
    if ((iq = standard_args(narg, ps, "i>D*;iL*;rD{-}&", &ptrs, &infos)) < 0)
      return ANA_ERROR;
    axes = ptrs[1].l;
    naxes = infos[1].nelem;
    if (setAxes(&infos[0], infos[1].nelem, ptrs[1].l,
		SL_EACHROW | SL_UNIQUEAXES) < 0)
      return ANA_ERROR;
    iret = 2;
    break;
  }

  if (internalMode & 1) {	/* /ALLAXES */
    naxes = infos[0].ndim;
    axes = malloc(naxes*sizeof(Int));
    allaxes = 1;
    Int i;
    for (i = 0; i < naxes; i++)
      axes[i] = i;
  } else
    allaxes = 0;  ptrs0 = ptrs[0];
  ptrsr = ptrs[iret];
  Int iaxis;
  for (iaxis = 0; iaxis < naxes; iaxis++) {
    setAxes(&infos[0], 1, &axes[iaxis], SL_EACHROW);
    ptrs[0] = ptrs0;
    ptrs[iret] = ptrsr;
    do {
      *ptrs[iret].d = f(ptrs[0].d, infos[0].rdims[0], infos[0].rsinglestep[0]);
      ptrs[0].d += infos[0].rsinglestep[1];
    } while (advanceLoop(&infos[iret], &ptrs[iret]),
	     advanceLoop(&infos[0], &ptrs[0]) < infos[0].rndim);
  }
  if (allaxes)
    free(axes);
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_ivarl_copy_eachaxis_(Int narg, Int ps[], Int (*f)(Double *, size_t count, size_t stride), Int isfunction)
/* copy input to output, apply function to output, go through each axis separately */
{
  pointer *ptrs, ptrs0, ptrsr;
  loopInfo *infos;
  Int iq, iret;
  Int *axes, naxes, oneaxis[1] = { 0 }, allaxes;

  switch (narg) {
  case 1:                       /* source */
    if ((iq = standard_args(narg, ps, isfunction? "i>D*;rD&": "i>D*", &ptrs, &infos)) < 0)
      return ANA_ERROR;
    axes = oneaxis;
    naxes = 1;
    setAxes(&infos[0], 0, NULL, SL_EACHROW);
    if (isfunction) {
      iret = 1;
      setAxes(&infos[iret], 0, NULL, SL_EACHROW);
    } else
      iret = 0;
    break;    
  case 2:                       /* source, axes */
    if ((iq = standard_args(narg, ps, isfunction? "i>D*;iL*;rD&": "i>D*;iL*", &ptrs, &infos)) < 0)
      return ANA_ERROR;
    axes = ptrs[1].l;
    naxes = infos[1].nelem;
    if (setAxes(&infos[0], infos[1].nelem, ptrs[1].l,
		SL_EACHROW | SL_UNIQUEAXES) < 0)
      return ANA_ERROR;
    if (isfunction) {
      iret = 2;
      if (setAxes(&infos[iret], infos[1].nelem, ptrs[1].l,
		  SL_EACHROW | SL_UNIQUEAXES) < 0)
	return ANA_ERROR;
    } else
      iret = 0;
    break;
  }

  if (internalMode & 1) {	/* /ALLAXES */
    naxes = infos[0].ndim;
    axes = malloc(naxes*sizeof(Int));
    allaxes = 1;
    Int i;
    for (i = 0; i < naxes; i++)
      axes[i] = i;
  } else
    allaxes = 0;

  ptrs0 = ptrs[0];
  if (isfunction) {
    memcpy(ptrs[iret].d, ptrs[0].d, infos[0].nelem*sizeof(Double));
    ptrsr = ptrs[iret];
  }
  Int iaxis;
  for (iaxis = 0; iaxis < naxes; iaxis++) {
    setAxes(&infos[0], 1, &axes[iaxis], SL_EACHROW);
    ptrs[0] = ptrs0;
    if (isfunction) {
      setAxes(&infos[iret], 1, &axes[iaxis], SL_EACHROW);
      ptrs[iret] = ptrsr;
    }
    do {
      f(ptrs[iret].d, infos[iret].rdims[0], infos[iret].rsinglestep[0]);
      ptrs[iret].d += infos[iret].rsinglestep[0]*infos[iret].rdims[0];
    } while (advanceLoop(&infos[iret], &ptrs[iret]) < infos[iret].rndim);
  }
  if (allaxes)
    free(axes);
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_i_sd_iDaLarDq_000_f_(Int narg, Int ps[], Int (*f)(Double *, size_t count, size_t stride))
{
  return ana_ivarl_copy_eachaxis_(narg, ps, f, 1);
}
/*-----------------------------------------------------------------------*/
Int ana_i_sd_iDaLa_000_s_(Int narg, Int ps[], Int (*f)(Double *, size_t count, size_t stride))
{
  return ana_ivarl_copy_eachaxis_(narg, ps, f, 0);
}
/*-----------------------------------------------------------------------*/
Int ana_i_sdddsd_iDaLDDrDq_000T333_f_(Int narg, Int ps[], Int (*f)
		    (Double *, size_t srcount, size_t srcstride,
		     Double par1, Double par2,
		     Double *, size_t tgtcount, size_t tgtstride))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq, ipar1, ipar2, iret;

  switch (narg) {
  case 2:                       /* source, param1; (param2 = 0) */
    if ((iq = standard_args(narg, ps, "i>D*;i>D;rD&", &ptrs, &infos)) < 0)
      return ANA_ERROR;
    ipar1 = 1;
    ipar2 = -1;
    iret = 2;
    setAxes(&infos[0], 0, NULL, SL_EACHROW);
    setAxes(&infos[iret], 0, NULL, SL_EACHROW);
    break;    
  case 3:                       /* source, param1, param2 */
    if ((iq = standard_args(narg, ps, "i>D*;i>D;i>D;rD&", &ptrs, &infos)) < 0)
      return ANA_ERROR;
    ipar1 = 1;
    ipar2 = 2;
    iret = 3;
    setAxes(&infos[0], 0, NULL, SL_EACHROW);
    setAxes(&infos[iret], 0, NULL, SL_EACHROW);
    break;    
  case 4:                       /* source, axis, param1, param2 */
    if ((iq = standard_args(narg, ps, "i>D*;iL;i>D;i>D;rD&", &ptrs, &infos)) < 0)
      return ANA_ERROR;
    ipar1 = 2;
    ipar2 = 3;
    iret = 4;
    setAxes(&infos[0], infos[1].nelem, ptrs[1].l, SL_EACHROW);
    setAxes(&infos[iret], infos[1].nelem, ptrs[1].l, SL_EACHROW);
    break;
  }
  do {
    f(ptrs[0].d, infos[0].rdims[0], infos[0].rsinglestep[0], 
      *ptrs[ipar1].d, (ipar2 >= 0? *ptrs[ipar2].d: 0.0),
      ptrs[iret].d, infos[0].rdims[0], infos[iret].rsinglestep[0]);
    ptrs[0].d += infos[0].rsinglestep[1];
    ptrs[iret].d += infos[iret].rsinglestep[1];
  } while (advanceLoop(&infos[0], &ptrs[0]),
	   advanceLoop(&infos[iret], &ptrs[iret])
           < infos[iret].rndim);
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dp3_rD33_0_f_(Int narg, Int ps[], void (*f)(Double (*)[3]))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "rD3,3", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  f((Double (*)[3]) ptrs[0].d);
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dp3dp_oD33D3_01_s_(Int narg, Int ps[], void (*f)(Double (*)[3], Double *))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "oD3,3;oD3", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  f((Double (*)[3]) ptrs[0].d, (Double *) ptrs[1].d);
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_dpT3_rD3_000_f_(Int narg, Int ps[], void (*f)(Double *, Double *, Double *))
{
  pointer *tgts;
  Int iq;

  if ((iq = standard_args(narg, ps, "rD3", &tgts, NULL)) < 0)
    return ANA_ERROR;
  f(&tgts[0].d[0], &tgts[0].d[1], &tgts[0].d[2]);
  return iq;
}
/*-----------------------------------------------------------------------*/
Int ana_v_sddsd_iDaD1rDq_012_f_(Int narg, Int ps[], void (*f)(Double *, size_t, size_t, Double, Double *, size_t, size_t))
{
  pointer *ptrs;
  loopInfo *infos;
  Int iq;

  if ((iq = standard_args(narg, ps, "i>D*;iD1?;rD&", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  Double width = ptrs[1].d? ptrs[1].d[0]: 3;
  f(&ptrs[0].d[0], infos[0].dims[0], 1, width, &ptrs[2].d[0], infos[2].dims[0], 1);
  return iq;
}
