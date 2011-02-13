/* File eval.c */
/* ANA expression evaluator and auxilliary routines. */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <math.h>
#include <string.h>
#include <ctype.h>
#include "action.h"
#include "install.h"
static char rcsid[] __attribute__ ((unused)) =
 "$Id: eval.c,v 4.0 2001/02/07 20:36:59 strous Exp $";

char *binOpName[] = {		/* name of binary operators, for messages */
  "addition", "subtraction", "multiplication", "division",
  "integer-division", "modulus",
 "symmetric-modulus", "highest-of-two", "lowest-of-two", "equal-to",
 "greater-than", "greater-than-or-equal-to", "less-than",
 "less-than-or-equal-to", "not-equal-to", "logical-or", "logical-and",
 "logical-exclusive-or", "power-taking", "and-if", "or-if",
};

char *binOpSign[] = {		/* token of binary operators, for messages */
  "+", "-", "*", "/", "\\", "%", "SMOD", ">", "<", "EQ", "GT", "GE", "LT", "LE",
 "NE", "OR", "AND", "XOR", "^", "ANDIF", "ORIF"
};

static int	lhs,		/* the current left-hand side operand */
		rhs,		/* the current right-hand side operand */
		binOp,		/* the current binary operator */
		topType,	/* the data type of the result */
		nRepeat,	/* the number of operations */
		lhsType,	/* the data type of the LHS */
		rhsType;	/* the data type of the RHS */
static pointer	lp,		/* pointer to the LHS values */
		rp,		/* pointer to the RHS values */
		tp;		/* pointer to the result values */

int	internal_routine(int, internalRoutine *), /* interal routine call */
	usr_routine(int);	/* user routine call */
char	evalScalPtr = 1;	/* we need to evaluate scalar pointers, too */

int	newSymbol(int kind, ...), nextCompileLevel(FILE *fp, char *filename),
  ana_neg_func(int, int []);
void	embed(int target, int context), zap(int symbol);
/*----------------------------------------------------------*/
void ana_bin_pow(void)
     /* power-taking with two array operands */
{
  scalar	mod1, arg1, mod2, arg2;

  switch (lhsType) {
    case ANA_BYTE: 
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.b++, (double) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.b++, (double) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.b++, (double) *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.b++, (double) *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = pow((double) *lp.b++, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    if (*lp.b) {
	      mod1.f = log(*lp.b);
	      mod2.f = exp(rp.cf->real*mod1.f);
	      arg2.f = mod1.f*rp.cf->imaginary;
	      tp.cf->real = mod2.f*cos(arg2.f);
	      tp.cf->imaginary = mod2.f*sin(arg2.f);
	    } else
	      tp.cf->real = tp.cd->imaginary = 0.0;
	    lp.b++;
	    rp.cf++;
	    tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    if (*lp.b) {
	      mod1.d = log(*lp.b);
	      mod2.d = exp(rp.cd->real*mod1.d);
	      arg2.d = mod1.d*rp.cd->imaginary;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    lp.b++;
	    rp.cd++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.w++, (double) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.w++, (double) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.w++, (double) *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.w++, (double) *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = pow((double) *lp.w++, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    if (*lp.w) {
	      mod1.f = log(fabs((double) *lp.w));
	      arg1.f = (*lp.w >= 0.0)? 0.0: M_PI;
	      mod2.f = exp(rp.cf->real*mod1.f - rp.cf->imaginary*arg1.f);
	      arg2.f = mod1.f*rp.cf->imaginary + rp.cf->real*arg1.f;
	      tp.cf->real = mod2.f*cos(arg2.f);
	      tp.cf->imaginary = mod2.f*sin(arg2.f);
	    } else
	      tp.cf->real = tp.cf->imaginary = 0.0;
	    lp.w++;
	    rp.cf++;
	    tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    if (*lp.w) {
	      mod1.d = log(fabs((double) *lp.w));
	      arg1.d = (*lp.w >= 0.0)? 0.0: M_PI;
	      mod2.d = exp(rp.cd->real*mod1.d - rp.cd->imaginary*arg1.d);
	      arg2.d = mod1.d*rp.cd->imaginary + rp.cd->real*arg1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    lp.w++;
	    rp.cd++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.l++, (double) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.l++, (double) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.l++, (double) *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.l++, (double) *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = pow((double) *lp.l++, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    if (*lp.l) {
	      mod1.f = log(fabs((double) *lp.l));
	      arg1.f = (*lp.l >= 0.0)? 0.0: M_PI;
	      mod2.f = exp(rp.cf->real*mod1.f - rp.cf->imaginary*arg1.f);
	      arg2.f = mod1.f*rp.cf->imaginary + rp.cf->real*arg1.f;
	      tp.cf->real = mod2.f*cos(arg2.f);
	      tp.cf->imaginary = mod2.f*sin(arg2.f);
	    } else
	      tp.cf->real = tp.cf->imaginary = 0.0;
	    lp.l++;
	    rp.cf++;
	    tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    if (*lp.l) {
	      mod1.d = log(fabs((double) *lp.l));
	      arg1.d = (*lp.l >= 0.0)? 0.0: M_PI;
	      mod2.d = exp(rp.cd->real*mod1.d - rp.cd->imaginary*arg1.d);
	      arg2.d = mod1.d*rp.cd->imaginary + rp.cd->real*arg1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    lp.l++;
	    rp.cd++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.f++, (double) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.f++, (double) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.f++, (double) *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.f++, (double) *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = pow((double) *lp.f++, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    if (*lp.f) {
	      mod1.f = log(fabs((double) *lp.f));
	      arg1.f = (*lp.f >= 0.0)? 0.0: M_PI;
	      mod2.f = exp(rp.cf->real*mod1.f - rp.cf->imaginary*arg1.f);
	      arg2.f = mod1.f*rp.cf->imaginary + rp.cf->real*arg1.f;
	      tp.cf->real = mod2.f*cos(arg2.f);
	      tp.cf->imaginary = mod2.f*sin(arg2.f);
	    } else
	      tp.cf->real = tp.cf->imaginary = 0.0;
	    lp.f++;
	    rp.cf++;
	    tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    if (*lp.f) {
	      mod1.d = log(fabs((double) *lp.f));
	      arg1.d = (*lp.f >= 0.0)? 0.0: M_PI;
	      mod2.d = exp(rp.cd->real*mod1.d - rp.cd->imaginary*arg1.d);
	      arg2.d = mod1.d*rp.cd->imaginary + rp.cd->real*arg1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    lp.f++;
	    rp.cd++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.d++ = pow(*lp.d++, (double) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.d++ = pow(*lp.d++, (double) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.d++ = pow(*lp.d++, (double) *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.d++ = pow(*lp.d++, (double) *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = pow(*lp.d++, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    if (*lp.d) {
	      mod1.d = log(fabs((double) *lp.d));
	      arg1.d = (*lp.d > 0.0)? 0.0: M_PI;
	      mod2.d = exp(rp.cf->real*mod1.d - rp.cf->imaginary*arg1.d);
	      arg2.d = mod1.d*rp.cf->imaginary + rp.cf->real*arg1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    lp.d++;
	    rp.cf++;
	    tp.cd++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    if (*lp.d) {
	      mod1.d = log(fabs((double) *lp.d));
	      arg1.d = (*lp.d >= 0.0)? 0.0: M_PI;
	      mod2.d = exp(rp.cd->real*mod1.d - rp.cd->imaginary*arg1.d);
	      arg2.d = mod1.d*rp.cd->imaginary + rp.cd->real*arg1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    lp.d++;
	    rp.cd++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    mod1.f = lp.cf->real*lp.cf->real
	      + lp.cf->imaginary*lp.cf->imaginary;
	    if (mod1.f) {
	      mod1.f = 0.5*log(mod1.f);
	      arg1.f = atan2(lp.cf->imaginary, lp.cf->real);
	      mod2.f = exp(*rp.b * mod1.f);
	      arg2.f = *rp.b * arg1.f;
	      tp.cf->real = mod2.f*cos(arg2.f);
	      tp.cf->imaginary = mod2.f*sin(arg2.f);
	    } else
	      tp.cf->real = tp.cf->imaginary = 0.0;
	    lp.cf++;
	    rp.b++;
	    tp.cf++;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    mod1.f = lp.cf->real*lp.cf->real
	      + lp.cf->imaginary*lp.cf->imaginary;
	    if (mod1.f) {
	      mod1.f = 0.5*log(mod1.f);
	      arg1.f = atan2(lp.cf->imaginary, lp.cf->real);
	      mod2.f = exp(*rp.w * mod1.f);
	      arg2.f = *rp.w * arg1.f;
	      tp.cf->real = mod2.f*cos(arg2.f);
	      tp.cf->imaginary = mod2.f*sin(arg2.f);
	    } else
	      tp.cf->real = tp.cf->imaginary = 0.0;
	    lp.cf++;
	    rp.w++;
	    tp.cf++;
	    }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    mod1.f = lp.cf->real*lp.cf->real
	      + lp.cf->imaginary*lp.cf->imaginary;
	    if (mod1.f) {
	      mod1.f = 0.5*log(mod1.f);
	      arg1.f = atan2(lp.cf->imaginary, lp.cf->real);
	      mod2.f = exp(*rp.l * mod1.f);
	      arg2.f = *rp.l * arg1.f;
	      tp.cf->real = mod2.f*cos(arg2.f);
	      tp.cf->imaginary = mod2.f*sin(arg2.f);
	    } else
	      tp.cf->real = tp.cf->imaginary = 0.0;
	    lp.cf++;
	    rp.l++;
	    tp.cf++;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    mod1.f = lp.cf->real*lp.cf->real
	      + lp.cf->imaginary*lp.cf->imaginary;
	    if (mod1.f) {
	      mod1.f = 0.5*log(mod1.f);
	      arg1.f = atan2(lp.cf->imaginary, lp.cf->real);
	      mod2.f = exp(*rp.f * mod1.f);
	      arg2.f = *rp.f * arg1.f;
	      tp.cf->real = mod2.f*cos(arg2.f);
	      tp.cf->imaginary = mod2.f*sin(arg2.f);
	    } else
	      tp.cf->real = tp.cf->imaginary = 0.0;
	    lp.cf++;
	    rp.f++;
	    tp.cf++;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    mod1.d = lp.cf->real*lp.cf->real
	      + lp.cf->imaginary*lp.cf->imaginary;
	    if (mod1.d) {
	      mod1.d = 0.5*log(mod1.d);
	      arg1.d = atan2(lp.cf->imaginary, lp.cf->real);
	      mod2.d = exp(*rp.d * mod1.d);
	      arg2.d = *rp.d * arg1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    rp.d++;
	    lp.cf++;
	    tp.cd++;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    mod1.f = lp.cf->real*lp.cf->real
	      + lp.cf->imaginary*lp.cf->imaginary;
	    if (mod1.f) {
	      mod1.f = 0.5*log(mod1.f);
	      arg1.f = atan2(lp.cf->imaginary, lp.cf->real);
	      mod2.f = exp(rp.cf->real*mod1.f - rp.cf->imaginary*arg1.f);
	      arg2.f = rp.cf->real*arg1.f + rp.cf->imaginary*mod1.f;
	      tp.cf->real = mod2.f*cos(arg2.f);
	      tp.cf->imaginary = mod2.f*sin(arg2.f);
	    } else
	      tp.cf->real = tp.cf->imaginary = 0.0;
	    lp.cf++;
	    rp.cf++;
	    tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    mod1.d = lp.cf->real*lp.cf->real
	      + lp.cf->imaginary*lp.cf->imaginary;
	    if (mod1.d) {
	      mod1.d = 0.5*log(mod1.d);
	      arg1.d = atan2(lp.cf->imaginary, lp.cf->real);
	      mod2.d = exp(rp.cd->real*mod1.d - rp.cd->imaginary*arg1.d);
	      arg2.d = rp.cd->real*arg1.d + rp.cd->imaginary*mod1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    lp.cf++;
	    rp.cd++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    mod1.d = lp.cd->real*lp.cd->real
	      + lp.cd->imaginary*lp.cd->imaginary;
	    if (mod1.d) {
	      mod1.d = 0.5*log(mod1.d);
	      arg1.d = atan2(lp.cd->imaginary, lp.cd->real);
	      mod2.d = exp(*rp.b * mod1.d);
	      arg2.d = *rp.b * arg1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    rp.b++;
	    lp.cd++;
	    tp.cd++;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    mod1.d = lp.cd->real*lp.cd->real
	      + lp.cd->imaginary*lp.cd->imaginary;
	    if (mod1.d) {
	      mod1.d = 0.5*log(mod1.d);
	      arg1.d = atan2(lp.cd->imaginary, lp.cd->real);
	      mod2.d = exp(*rp.w * mod1.d);
	      arg2.d = *rp.w * arg1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    lp.cd++;
	    rp.w++;
	    tp.cd++;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    mod1.d = lp.cd->real*lp.cd->real
	      + lp.cd->imaginary*lp.cd->imaginary;
	    if (mod1.d) {
	      mod1.d = 0.5*log(mod1.d);
	      arg1.d = atan2(lp.cd->imaginary, lp.cd->real);
	      mod2.d = exp(*rp.l * mod1.d);
	      arg2.d = *rp.l * arg1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    lp.cd++;
	    rp.l++;
	    tp.cd++;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    mod1.d = lp.cd->real*lp.cd->real
	      + lp.cd->imaginary*lp.cd->imaginary;
	    if (mod1.d) {
	      mod1.d = 0.5*log(mod1.d);
	      arg1.d = atan2(lp.cd->imaginary, lp.cd->real);
	      mod2.d = exp(*rp.f * mod1.d);
	      arg2.d = *rp.f * arg1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    lp.cd++;
	    rp.f++;
	    tp.cd++;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    mod1.d = lp.cd->real*lp.cd->real
	      + lp.cd->imaginary*lp.cd->imaginary;
	    if (mod1.d) {
	      mod1.d = 0.5*log(mod1.d);
	      arg1.d = atan2(lp.cd->imaginary, lp.cd->real);
	      mod2.d = exp(*rp.d * mod1.d);
	      arg2.d = *rp.d * arg1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    rp.d++;
	    lp.cd++;
	    tp.cd++;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    mod1.d = lp.cd->real*lp.cd->real
	      + lp.cd->imaginary*lp.cd->imaginary;
	    if (mod1.d) {
	      mod1.d = 0.5*log(mod1.d);
	      arg1.d = atan2(lp.cd->imaginary, lp.cd->real);
	      mod2.d = exp(rp.cf->real*mod1.d - rp.cf->imaginary*arg1.d);
	      arg2.d = rp.cf->real*arg1.d + rp.cf->imaginary*mod1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    lp.cd++;
	    rp.cf++;
	    tp.cd++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    mod1.d = lp.cd->real*lp.cd->real
	      + lp.cd->imaginary*lp.cd->imaginary;
	    if (mod1.d) {
	      mod1.d = 0.5*log(mod1.d);
	      arg1.d = atan2(lp.cd->imaginary, lp.cd->real);
	      mod2.d = exp(rp.cd->real*mod1.d - rp.cd->imaginary*arg1.d);
	      arg2.d = rp.cd->real*arg1.d + rp.cd->imaginary*mod1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    lp.cd++;
	    rp.cd++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
       cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_pow_as(void)
     /* power-taking with array LHS and scalar RHS */
{
  scalar	re, im, mod1, arg1, mod2, arg2;
  
  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  re.f = (double) *rp.b;
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.b++, re.f);
	  break;
	case ANA_WORD:
	  re.f = (double) *rp.w;
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.b++, re.f);
	  break;
	case ANA_LONG:
	  re.f = (double) *rp.l;
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.b++, re.f);
	  break;
	case ANA_FLOAT:
	  re.f = (double) *rp.f;
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.b++, re.f);
	  break;
	case ANA_DOUBLE:
	  re.d = *rp.d;
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.b++, re.d);
	  break;
	case ANA_CFLOAT:
	  re.f = rp.cf->real;
	  im.f = rp.cf->imaginary;
	  while (nRepeat--) {
	    if (*lp.b) {
	      mod1.f = log(*lp.b);
	      mod2.f = exp(re.f*mod1.f);
	      arg2.f = im.f*mod1.f;
	      tp.cf->real = mod2.f*cos(arg2.f);
	      tp.cf->imaginary = mod2.f*sin(arg2.f);
	    } else
	      tp.cf->real = tp.cf->imaginary = 0.0;
	    lp.b++;
	    tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  re.d = rp.cd->real;
	  im.d = rp.cd->imaginary;
	  while (nRepeat--) {
	    if (*lp.b) {
	      mod1.d = log(*lp.b);
	      mod2.d = exp(re.d*mod1.d);
	      arg2.d = im.d*mod1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    lp.b++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  re.f = *rp.b;
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.w++, re.f);
	  break;
	case ANA_WORD:
	  re.f = *rp.w;
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.w++, re.f);
	  break;
	case ANA_LONG:
	  re.f = *rp.l;
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.w++, re.f);
	  break;
	case ANA_FLOAT:
	  re.f = *rp.f;
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.w++, re.f);
	  break;
	case ANA_DOUBLE:
	  re.d = *rp.d;
	  while (nRepeat--)
	    *tp.d++ = pow((double) *lp.w++, re.d);
	  break;
	case ANA_CFLOAT:
	  re.f = rp.cf->real;
	  im.f = rp.cf->imaginary;
	  while (nRepeat--) {
	    if (*lp.w) {
	      mod1.f = log(fabs((double) *lp.w));
	      arg1.f = (*lp.w >= 0.0)? 0.0: M_PI;
	      mod2.f = exp(re.f*mod1.f - im.f*arg1.f);
	      arg2.f = im.f*mod1.f + re.f*arg1.f;
	      tp.cf->real = mod2.f*cos(arg2.f);
	      tp.cf->imaginary = mod2.f*sin(arg2.f);
	    } else
	      tp.cf->real = tp.cf->imaginary = 0.0;
	    lp.w++;
	    tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  re.d = rp.cd->real;
	  im.d = rp.cd->imaginary;
	  while (nRepeat--) {
	    if (*lp.w) {
	      mod1.d = log(fabs((double) *lp.w));
	      arg1.d = (*lp.w >= 0.0)? 0.0: M_PI;
	      mod2.d = exp(re.d*mod1.d - im.d*arg1.d);
	      arg2.d = im.d*mod1.d + re.d*arg1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    lp.w++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  re.f = *rp.b;
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.l++, re.f);
	  break;
	case ANA_WORD:
	  re.f = *rp.w;
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.l++, re.f);
	  break;
	case ANA_LONG:
	  re.f = *rp.l;
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.l++, re.f);
	  break;
	case ANA_FLOAT:
	  re.f = *rp.f;
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.l++, re.f);
	  break;
	case ANA_DOUBLE:
	  re.d = *rp.d;
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.l++, re.d);
	  break;
	case ANA_CFLOAT:
	  re.f = rp.cf->real;
	  im.f = rp.cf->imaginary;
	  while (nRepeat--) {
	    if (*lp.l) {
	      mod1.f = log(fabs((double) *lp.l));
	      arg1.f = (*lp.l >= 0.0)? 0.0: M_PI;
	      mod2.f = exp(re.f*mod1.f - im.f*arg1.f);
	      arg2.f = im.f*mod1.f + re.f*arg1.f;
	      tp.cf->real = mod2.f*cos(arg2.f);
	      tp.cf->imaginary = mod2.f*sin(arg2.f);
	    } else
	      tp.cf->real = tp.cf->imaginary = 0.0;
	    lp.l++;
	    tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  re.d = rp.cd->real;
	  im.d = rp.cd->imaginary;
	  while (nRepeat--) {
	    if (*lp.l) {
	      mod1.d = log(fabs((double) *lp.l));
	      arg1.d = (*lp.l >= 0.0)? 0.0: M_PI;
	      mod2.d = exp(re.d*mod1.d - im.d*arg1.d);
	      arg2.d = im.d*mod1.d + re.d*arg1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    lp.l++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  re.f = *rp.b;
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.f++, re.f);
	  break;
	case ANA_WORD:
	  re.f = *rp.w;
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.f++, re.f);
	  break;
	case ANA_LONG:
	  re.f = (double) *rp.l;
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.f++, re.f);
	  break;
	case ANA_FLOAT:
	  re.f = (double) *rp.f;
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.f++, re.f);
	  break;
	case ANA_DOUBLE:
	  re.f = *rp.d;
	  while (nRepeat--)
	    *tp.f++ = (float) pow((double) *lp.f++, re.f);
	  break;
	case ANA_CFLOAT:
	  re.f = rp.cf->real;
	  im.f = rp.cf->imaginary;
	  while (nRepeat--) {
	    if (*lp.f) {
	      mod1.f = log(fabs((double) *lp.f));
	      arg1.f = (*lp.f >= 0.0)? 0.0: M_PI;
	      mod2.f = exp(re.f*mod1.f - im.f*arg1.f);
	      arg2.f = im.f*mod1.f + re.f*arg1.f;
	      tp.cf->real = mod2.f*cos(arg2.f);
	      tp.cf->imaginary = mod2.f*sin(arg2.f);
	    } else
	      tp.cf->real = tp.cf->imaginary = 0.0;
	    lp.f++;
	    tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  re.d = rp.cd->real;
	  im.d = rp.cd->imaginary;
	  while (nRepeat--) {
	    if (*lp.f) {
	      mod1.d = log(fabs((double) *lp.f));
	      arg1.d = (*lp.f >= 0.0)? 0.0: M_PI;
	      mod2.d = exp(re.d*mod1.d - im.d*arg1.d);
	      arg2.d = im.d*mod1.d + re.d*arg1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    lp.f++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  re.d = *rp.b;
	  while (nRepeat--)
	    *tp.d++ = pow(*lp.d++, re.d);
	  break;
	case ANA_WORD:
	  re.d = *rp.w;
	  while (nRepeat--)
	    *tp.d++ = pow(*lp.d++, re.d);
	  break;
	case ANA_LONG:
	  re.d = *rp.l;
	  while (nRepeat--)
	    *tp.d++ = pow(*lp.d++, re.d);
	  break;
	case ANA_FLOAT:
	  re.d = *rp.f;
	  while (nRepeat--)
	    *tp.d++ = pow(*lp.d++, re.d);
	  break;
	case ANA_DOUBLE:
	  re.d = *rp.d;
	  while (nRepeat--)
	    *tp.f++ = pow(*lp.d++, re.d);
	  break;
	case ANA_CFLOAT:
	  re.d = rp.cf->real;
	  im.d = rp.cf->imaginary;
	  while (nRepeat--) {
	    if (*lp.d) {
	      mod1.d = log(fabs((double) *lp.d));
	      arg1.d = (*lp.d >= 0.0)? 0.0: M_PI;
	      mod2.d = exp(re.d*mod1.d - im.d*arg1.d);
	      arg2.d = im.d*mod1.d + re.d*arg1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    lp.d++;
	    tp.cd++;
	  }
	  break;
	case ANA_CDOUBLE:
	  re.d = rp.cd->real;
	  im.d = rp.cd->imaginary;
	  while (nRepeat--) {
	    if (*lp.d) {
	      mod1.d = log(fabs((double) *lp.d));
	      arg1.d = (*lp.d >= 0.0)? 0.0: M_PI;
	      mod2.d = exp(re.d*mod1.d - im.d*arg1.d);
	      arg2.d = im.d*mod1.d + re.d*arg1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    lp.d++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  re.f = *rp.b;
	  while (nRepeat--) {
	    mod1.f = lp.cf->real*lp.cf->real
	      + lp.cf->imaginary*lp.cf->imaginary;
	    if (mod1.f) {
	      mod1.f = 0.5*log(mod1.f);
	      arg1.f = atan2(lp.cf->imaginary, lp.cf->real);
	      mod2.f = exp(re.f*mod1.f);
	      arg2.f = re.f*arg1.f;
	      tp.cf->real = mod2.f*cos(arg2.f);
	      tp.cf->imaginary = mod2.f*sin(arg2.f);
	    } else
	      tp.cf->real = tp.cf->imaginary = 0.0;
	    tp.cf++;
	    lp.cf++;
	  }
	  break;
	case ANA_WORD:
	  re.f = *rp.w;
	  while (nRepeat--) {
	    mod1.f = lp.cf->real*lp.cf->real
	      + lp.cf->imaginary*lp.cf->imaginary;
	    if (mod1.f) {
	      mod1.f = 0.5*log(mod1.f);
	      arg1.f = atan2(lp.cf->imaginary, lp.cf->real);
	      mod2.f = exp(re.f*mod1.f);
	      arg2.f = re.f*arg1.f;
	      tp.cf->real = mod2.f*cos(arg2.f);
	      tp.cf->imaginary = mod2.f*sin(arg2.f);
	    } else
	      tp.cf->real = tp.cd->imaginary = 0.0;
	    tp.cf++;
	    lp.cf++;
	  }
	  break;
	case ANA_LONG:
	  re.f = *rp.l;
	  while (nRepeat--) {
	    mod1.f = lp.cf->real*lp.cf->real
	      + lp.cf->imaginary*lp.cf->imaginary;
	    if (mod1.f) {
	      mod1.f = 0.5*log(mod1.f);
	      arg1.f = atan2(lp.cf->imaginary, lp.cf->real);
	      mod2.f = exp(re.f*mod1.f);
	      arg2.f = re.f*arg1.f;
	      tp.cf->real = mod2.f*cos(arg2.f);
	      tp.cf->imaginary = mod2.f*sin(arg2.f);
	    } else 
	      tp.cf->real = tp.cf->imaginary = 0.0;
	    lp.cf++;
	    tp.cf++;
	  }
	  break;
	case ANA_FLOAT:
	  re.f = *rp.f;
	  while (nRepeat--) {
	    mod1.f = lp.cf->real*lp.cf->real
	      + lp.cf->imaginary*lp.cf->imaginary;
	    if (mod1.f) {
	      mod1.f = 0.5*log(mod1.f);
	      arg1.f = atan2(lp.cf->imaginary, lp.cf->real);
	      mod2.f = exp(re.f*mod1.f);
	      arg2.f = re.f*arg1.f;
	      tp.cf->real = mod2.f*cos(arg2.f);
	      tp.cf->imaginary = mod2.f*sin(arg2.f);
	    }
	    tp.cf++;
	    lp.cf++;
	  }
	  break;
	case ANA_DOUBLE:
	  re.d = *rp.d;
	  while (nRepeat--) {
	    mod1.d = lp.cf->real*lp.cf->real
	      + lp.cf->imaginary*lp.cf->imaginary;
	    if (mod1.d) {
	      mod1.d = 0.5*log(mod1.d);
	      arg1.d = atan2(lp.cf->imaginary, lp.cf->real);
	      mod2.d = exp(re.d*mod1.d);
	      arg2.d = re.d*arg1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    }
	    lp.cf++;
	    tp.cd++;
	  }
	  break;
	case ANA_CFLOAT:
	  re.f = rp.cf->real;
	  im.f = rp.cf->imaginary;
	  while (nRepeat--) {
	    mod1.f = lp.cf->real*lp.cf->real
	      + lp.cf->imaginary*lp.cf->imaginary;
	    if (mod1.f) {
	      mod1.f = 0.5*log(mod1.f);
	      arg1.f = atan2(lp.cf->imaginary, lp.cf->real);
	      mod2.f = exp(re.f*mod1.f - im.f*arg1.f);
	      arg2.f = re.f*arg1.f + im.f*mod1.f;
	      tp.cf->real = mod2.f*cos(arg2.f);
	      tp.cf->imaginary = mod2.f*sin(arg2.f);
	    } else
	      tp.cf->real = tp.cf->imaginary = 0.0;
	    tp.cf++;
	    lp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  re.d = rp.cd->real;
	  im.d = rp.cd->imaginary;
	  while (nRepeat--) {
	    mod1.d = lp.cf->real*lp.cf->real
	      + lp.cf->imaginary*lp.cf->imaginary;
	    if (mod1.d) {
	      mod1.d = 0.5*log(mod1.d);
	      arg1.d = atan2(lp.cf->imaginary, lp.cf->real);
	      mod2.d = exp(re.d*mod1.d - im.d*arg1.d);
	      arg2.d = re.d*arg1.d + im.d*mod1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    }
	    tp.cd++;
	    lp.cf++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  re.d = *rp.b;
	  while (nRepeat--) {
	    mod1.d = lp.cd->real*lp.cd->real
	      + lp.cd->imaginary*lp.cd->imaginary;
	    if (mod1.d) {
	      mod1.d = 0.5*log(mod1.d);
	      arg1.d = atan2(lp.cd->imaginary, lp.cd->real);
	      mod2.d = exp(re.d*mod1.d);
	      arg2.d = re.d*arg1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    tp.cd++;
	    lp.cd++;
	  }
	  break;
	case ANA_WORD:
	  re.d = *rp.w;
	  while (nRepeat--) {
	    mod1.d = lp.cd->real*lp.cd->real
	      + lp.cd->imaginary*lp.cd->imaginary;
	    if (mod1.d) {
	      mod1.d = 0.5*log(mod1.d);
	      arg1.d = atan2(lp.cd->imaginary, lp.cd->real);
	      mod2.d = exp(re.d*mod1.d);
	      arg2.d = re.d*arg1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    tp.cd++;
	    lp.cd++;
	  }
	  break;
	case ANA_LONG:
	  re.d = *rp.l;
	  while (nRepeat--) {
	    mod1.d = lp.cd->real*lp.cd->real
	      + lp.cd->imaginary*lp.cd->imaginary;
	    if (mod1.d) {
	      mod1.d = 0.5*log(mod1.d);
	      arg1.d = atan2(lp.cd->imaginary, lp.cd->real);
	      mod2.d = exp(re.d*mod1.d);
	      arg2.d = re.d*arg1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    tp.cd++;
	    lp.cd++;
	  }
	  break;
	case ANA_FLOAT:
	  re.d = *rp.f;
	  while (nRepeat--) {
	    mod1.d = lp.cd->real*lp.cd->real
	      + lp.cd->imaginary*lp.cd->imaginary;
	    if (mod1.d) {
	      mod1.d = 0.5*log(mod1.d);
	      arg1.d = atan2(lp.cd->imaginary, lp.cd->real);
	      mod2.d = exp(re.d*mod1.d);
	      arg2.d = re.d*arg1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    tp.cd++;
	    lp.cd++;
	  }
	  break;
	case ANA_DOUBLE:
	  re.d = *rp.d;
	  while (nRepeat--) {
	    mod1.d = lp.cd->real*lp.cd->real
	      + lp.cd->imaginary*lp.cd->imaginary;
	    if (mod1.d) {
	      mod1.d = 0.5*log(mod1.d);
	      arg1.d = atan2(lp.cd->imaginary, lp.cd->real);
	      mod2.d = exp(re.d*mod1.d);
	      arg2.d = re.d*arg1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    tp.cd++;
	    lp.cd++;
	  }
	  break;
	case ANA_CFLOAT:
	  re.d = rp.cf->real;
	  im.d = rp.cf->imaginary;
	  while (nRepeat--) {
	    mod1.d = lp.cd->real*lp.cd->real
	      + lp.cd->imaginary*lp.cd->imaginary;
	    if (mod1.d) {
	      mod1.d = 0.5*log(mod1.d);
	      arg1.d = atan2(lp.cd->imaginary, lp.cd->real);
	      mod2.d = exp(re.d*mod1.d - im.d*arg1.d);
	      arg2.d = re.d*arg1.d + im.d*mod1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    tp.cd++;
	    lp.cd++;
	  }
	  break;
	case ANA_CDOUBLE:
	  re.d = rp.cd->real;
	  im.d = rp.cd->imaginary;
	  while (nRepeat--) {
	    mod1.d = lp.cd->real*lp.cd->real
	      + lp.cd->imaginary*lp.cd->imaginary;
	    if (mod1.d) {
	      mod1.d = 0.5*log(mod1.d);
	      arg1.d = atan2(lp.cd->imaginary, lp.cd->real);
	      mod2.d = exp(re.d*mod1.d - im.d*arg1.d);
	      arg2.d = re.d*arg1.d + im.d*mod1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd->imaginary = mod2.d*sin(arg2.d);
	    } else
	      tp.cd->real = tp.cd->imaginary = 0.0;
	    tp.cd++;
	    lp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_pow_sa(void)
     /* power-taking with scalar LHS and array RHS */
{
  scalar	re, im, arg1, mod1, mod2, arg2;

  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  mod1.f = *lp.b;
	  while (nRepeat--)
	    *tp.f++ = pow(mod1.f, *rp.b++);
	  break;
	case ANA_WORD:
	  mod1.f = *lp.b;
	  while (nRepeat--)
	    *tp.f++ = pow(mod1.f, *rp.w++);
	  break;
	case ANA_LONG:
	  mod1.f = *lp.b;
	  while (nRepeat--)
	    *tp.f++ = pow(mod1.f, *rp.l++);
	  break;
	case ANA_FLOAT:
	  mod1.f = *lp.b;
	  while (nRepeat--)
	    *tp.f++ = pow(mod1.f, *rp.f++);
	  break;
	case ANA_DOUBLE:
	  mod1.d = *lp.b;
	  while (nRepeat--)
	    *tp.d++ = pow(mod1.d, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  if (*lp.b) {
	    mod1.f = log(*lp.b);
	    while (nRepeat--) {
	      re.f = rp.cf->real;
	      im.f = rp.cf++->imaginary;
	      mod2.f = exp(re.f*mod1.f);
	      arg2.f = im.f*log(mod1.f);
	      tp.cf->real = mod2.f*cos(arg2.f);
	      tp.cf++->imaginary = mod2.f*sin(arg2.f);
	    }
	  } else while (nRepeat--) {
	    tp.cf->real = tp.cf->imaginary = 0.0;
	    tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  if (*lp.b) {
	    mod1.d = log(*lp.b);
	    while (nRepeat--) {
	      re.d = rp.cd->real;
	      im.d = rp.cd++->imaginary;
	      mod2.d = exp(re.d*mod1.d);
	      arg2.d = im.d*log(mod1.d);
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd++->imaginary = mod2.d*sin(arg2.d);
	    }
	  } else while (nRepeat--) {
	    tp.cd->real = tp.cd->imaginary = 0.0;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  mod1.f = *lp.w;
	  while (nRepeat--)
	    *tp.f++ = pow(mod1.f, *rp.b++);
	  break;
	case ANA_WORD:
	  mod1.f = *lp.w;
	  while (nRepeat--)
	    *tp.f++ = pow(mod1.f, *rp.w++);
	  break;
	case ANA_LONG:
	  mod1.f = *lp.w;
	  while (nRepeat--)
	    *tp.f++ = pow(mod1.f, *rp.l++);
	  break;
	case ANA_FLOAT:
	  mod1.f = *lp.w;
	  while (nRepeat--)
	    *tp.f++ = pow(mod1.f, *rp.f++);
	  break;
	case ANA_DOUBLE:
	  mod1.d = *lp.w;
	  while (nRepeat--)
	    *tp.d++ = pow(mod1.d, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  if (*lp.w) {
	    mod1.f = log(fabs((double) *lp.w));
	    arg1.f = (*lp.w > 0.0)? 0.0: M_PI;
	    while (nRepeat--) {
	      re.f = rp.cf->real;
	      im.f = rp.cf++->imaginary;
	      mod2.f = exp(re.f*mod1.f - im.f*arg1.f);
	      arg2.f = im.f*mod1.f + re.f*arg1.f;
	      tp.cf->real = mod2.f*cos(arg2.f);
	      tp.cf++->imaginary = mod2.f*sin(arg2.f);
	    }
	  } else while (nRepeat--) {
	    tp.cf->real = tp.cf->imaginary = 0.0;
	    tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  if (*lp.w) {
	    mod1.d = log(fabs((double) *lp.w));
	    arg1.d = (*lp.w >= 0.0)? 0.0: M_PI;
	    while (nRepeat--) {
	      re.d = rp.cd->real;
	      im.d = rp.cd++->imaginary;
	      mod2.d = exp(re.d*mod1.d - im.d*arg1.d);
	      arg2.d = im.d*mod1.d + re.d*arg1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd++->imaginary = mod2.d*sin(arg2.d);
	    }
	  } else while (nRepeat--) {
	    tp.cd->real = tp.cd->imaginary = 0.0;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  mod1.f = *lp.l;
	  while (nRepeat--)
	    *tp.f++ = pow(mod1.f, *rp.b++);
	  break;
	case ANA_WORD:
	  mod1.f = *lp.l;
	  while (nRepeat--)
	    *tp.f++ = pow(mod1.f, *rp.w++);
	  break;
	case ANA_LONG:
	  mod1.f = *lp.l;
	  while (nRepeat--)
	    *tp.f++ = pow(mod1.f, *rp.l++);
	  break;
	case ANA_FLOAT:
	  mod1.f = *lp.l;
	  while (nRepeat--)
	    *tp.f++ = pow(mod1.f, *rp.f++);
	  break;
	case ANA_DOUBLE:
	  mod1.d = *lp.l;
	  while (nRepeat--)
	    *tp.d++ = pow(mod1.d, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  if (*lp.l) {
	    mod1.f = log(fabs((double) *lp.l));
	    arg1.f = (*lp.l > 0.0)? 0.0: M_PI;
	    while (nRepeat--) {
	      re.f = rp.cf->real;
	      im.f = rp.cf++->imaginary;
	      mod2.f = exp(re.f*mod1.f - im.f*arg1.f);
	      arg2.f = im.f*mod1.f + re.f*arg1.f;
	      tp.cf->real = mod2.f*cos(arg2.f);
	      tp.cf++->imaginary = mod2.f*sin(arg2.f);
	    }
	  } else while (nRepeat--) {
	    tp.cf->real = tp.cf->imaginary = 0.0;
	    tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  if (*lp.l) {
	    mod1.d = log(fabs((double) *lp.l));
	    arg1.f = (*lp.l > 0.0)? 0.0: M_PI;
	    while (nRepeat--) {
	      re.d = rp.cd->real;
	      im.d = rp.cd++->imaginary;
	      mod2.d = exp(re.d*mod1.d - im.d*arg1.d);
	      arg2.d = im.d*mod1.d + re.d*arg1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd++->imaginary = mod2.d*sin(arg2.d);
	    }
	  } else while (nRepeat--) {
	    tp.cd->real = tp.cd->imaginary = 0.0;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  mod1.f = *lp.f;
	  while (nRepeat--)
	    *tp.f++ = pow(mod1.f, *rp.b++);
	  break;
	case ANA_WORD:
	  mod1.f = *lp.f;
	  while (nRepeat--)
	    *tp.f++ = pow(mod1.f, *rp.w++);
	  break;
	case ANA_LONG:
	  mod1.f = *lp.f;
	  while (nRepeat--)
	    *tp.f++ = pow(mod1.f, *rp.l++);
	  break;
	case ANA_FLOAT:
	  mod1.f = *lp.f;
	  while (nRepeat--)
	    *tp.f++ = pow(mod1.f, *rp.f++);
	  break;
	case ANA_DOUBLE:
	  mod1.d = *lp.f;
	  while (nRepeat--)
	    *tp.d++ = pow(mod1.d, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  if (*lp.f) {
	    mod1.f = log(fabs((double) *lp.f));
	    arg1.f = (*lp.f > 0.0)? 0.0: M_PI;
	    while (nRepeat--) {
	      re.f = rp.cf->real;
	      im.f = rp.cf++->imaginary;
	      mod2.f = exp(re.f*mod1.f - im.f*arg1.f);
	      arg2.f = im.f*mod1.f + re.f*arg1.f;
	      tp.cf->real = mod2.f*cos(arg2.f);
	      tp.cf++->imaginary = mod2.f*sin(arg2.f);
	    }
	  } else while (nRepeat--) {
	    tp.cf->real = tp.cf->imaginary = 0.0;
	    tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  if (*lp.f) {
	    mod1.d = log(fabs((double) *lp.f));
	    arg1.d = (*lp.f > 0.0)? 0.0: M_PI;
	    while (nRepeat--) {
	      re.d = rp.cd->real;
	      im.d = rp.cd++->imaginary;
	      mod2.d = exp(re.d*mod1.d - im.d*arg1.d);
	      arg2.d = im.d*mod1.d + re.d*arg1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd++->imaginary = mod2.d*sin(arg2.d);
	    }
	  } else while (nRepeat--) {
	    tp.cd->real = tp.cd->imaginary = 0.0;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  mod1.f = *lp.d;
	  while (nRepeat--)
	    *tp.f++ = pow(mod1.f, *rp.b++);
	  break;
	case ANA_WORD:
	  mod1.f = *lp.d;
	  while (nRepeat--)
	    *tp.f++ = pow(mod1.f, *rp.w++);
	  break;
	case ANA_LONG:
	  mod1.f = *lp.d;
	  while (nRepeat--)
	    *tp.f++ = pow(mod1.f, *rp.l++);
	  break;
	case ANA_FLOAT:
	  mod1.f = *lp.d;
	  while (nRepeat--)
	    *tp.f++ = pow(mod1.f, *rp.f++);
	  break;
	case ANA_DOUBLE:
	  mod1.d = *lp.d;
	  while (nRepeat--)
	    *tp.d++ = pow(mod1.d, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  if (*lp.d) {
	    mod1.d = log(fabs((double) *lp.d));
	    arg1.d = (*lp.d > 0.0)? 0.0: M_PI;
	    while (nRepeat--) {
	      re.d = rp.cf->real;
	      im.d = rp.cf++->imaginary;
	      mod2.d = exp(re.d*mod1.d - im.d*arg1.d);
	      arg2.d = im.d*mod1.d + re.d*arg1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd++->imaginary = mod2.d*sin(arg2.d);
	    }
	  } else while (nRepeat--) {
	    tp.cd->real = tp.cd->imaginary = 0.0;
	    tp.cd++;
	  }
	  break;
	case ANA_CDOUBLE:
	  if (*lp.d) {
	    mod1.d = log(fabs((double) *lp.d));
	    arg1.d = (*lp.d >= 0.0)? 0.0: M_PI;
	    while (nRepeat--) {
	      re.d = rp.cd->real;
	      im.d = rp.cd++->imaginary;
	      mod2.d = exp(re.d*mod1.d - im.d*arg1.d);
	      arg2.d = im.d*mod1.d + re.d*arg1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd++->imaginary = mod2.d*sin(arg2.d);
	    }
	  } else while (nRepeat--) {
	    tp.cd->real = tp.cd->imaginary = 0.0;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  mod1.f = lp.cf->real*lp.cf->real
	    + lp.cf->imaginary*lp.cf->imaginary;
	  if (mod1.f) {
	    mod1.f = 0.5*log(mod1.f);
	    arg1.f = atan2(lp.cf->imaginary, lp.cf->real);
	    while (nRepeat--) {
	      mod2.f = exp(*rp.b*mod1.f);
	      arg2.f = arg1.f* *rp.b++;
	      tp.cf->real = mod2.f*cos(arg2.f);
	      tp.cf++->imaginary = mod2.f*sin(arg2.f);
	    }
	  } else while (nRepeat--) {
	    tp.cf->real = tp.cf->imaginary = 0.0;
	    tp.cf++;
	  }
	  break;
	case ANA_WORD:
	  mod1.f = lp.cf->real*lp.cf->real
	    + lp.cf->imaginary*lp.cf->imaginary;
	  if (mod1.f) {
	    mod1.f = 0.5*log(mod1.f);
	    arg1.f = atan2(lp.cf->imaginary, lp.cf->real);
	    while (nRepeat--) {
	      mod2.f = exp(*rp.w*mod1.f);
	      arg2.f = arg1.f* *rp.w++;
	      tp.cf->real = mod2.f*cos(arg2.f);
	      tp.cf++->imaginary = mod2.f*sin(arg2.f);
	    }
	  } else while (nRepeat--) {
	    tp.cf->real = tp.cf->imaginary = 0.0;
	    tp.cf++;
	  }
	  break;
	case ANA_LONG:
	  mod1.f = lp.cf->real*lp.cf->real
	    + lp.cf->imaginary*lp.cf->imaginary;
	  if (mod1.f) {
	    mod1.f = 0.5*log(mod1.f);
	    arg1.f = atan2(lp.cf->imaginary, lp.cf->real);
	    while (nRepeat--) {
	      mod2.f = exp(*rp.l*mod1.f);
	      arg2.f = arg1.f* *rp.l++;
	      tp.cf->real = mod2.f*cos(arg2.f);
	      tp.cf++->imaginary = mod2.f*sin(arg2.f);
	    }
	  } else while (nRepeat--) {
	    tp.cf->real = tp.cf->imaginary = 0.0;
	    tp.cf++;
	  }
	  break;
	case ANA_FLOAT:
	  mod1.f = lp.cf->real*lp.cf->real
	    + lp.cf->imaginary*lp.cf->imaginary;
	  if (mod1.f) {
	    mod1.f = 0.5*log(mod1.f);
	    arg1.f = atan2(lp.cf->imaginary, lp.cf->real);
	    while (nRepeat--) {
	      mod2.f = exp(*rp.f*mod1.f);
	      arg2.f = arg1.f* *rp.f++;
	      tp.cf->real = mod2.f*cos(arg2.f);
	      tp.cf++->imaginary = mod2.f*sin(arg2.f);
	    }
	  } else while (nRepeat--) {
	    tp.cf->real = tp.cf->imaginary = 0.0;
	    tp.cf++;
	  }
	  break;
	case ANA_DOUBLE:
	  mod1.d = lp.cf->real*lp.cf->real
	    + lp.cf->imaginary*lp.cf->imaginary;
	  if (mod1.d) {
	    mod1.d = 0.5*log(mod1.d);
	    arg1.d = atan2(lp.cf->imaginary, lp.cf->real);
	    while (nRepeat--) {
	      mod2.d = exp(*rp.d*mod1.d);
	      arg2.d = arg1.d* *rp.d++;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd++->imaginary = mod2.d*sin(arg2.d);
	    }
	  } else while (nRepeat--) {
	    tp.cd->real = tp.cd->imaginary = 0.0;
	    tp.cd++;
	  }
	  break;
	case ANA_CFLOAT:
	  mod1.f = lp.cf->real*lp.cf->real
	    + lp.cf->imaginary*lp.cf->imaginary;
	  if (mod1.f) {
	    mod1.f = 0.5*log(mod1.f);
	    arg1.f = atan2(lp.cf->imaginary, lp.cf->real);
	    while (nRepeat--) {
	      mod2.f = exp(rp.cf->real*mod1.f - rp.cf->imaginary*arg1.f);
	      arg2.f = arg1.f*rp.cf->real + rp.cf->imaginary*mod1.f;
	      tp.cf->real = mod2.f*cos(arg2.f);
	      tp.cf++->imaginary = mod2.f*sin(arg2.f);
	      rp.cf++;
	    }
	  } else while (nRepeat--) {
	    tp.cf->real = tp.cf->imaginary = 0.0;
	    tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  mod1.d = lp.cf->real*lp.cf->real
	    + lp.cf->imaginary*lp.cf->imaginary;
	  if (mod1.d) {
	    mod1.d = 0.5*log(mod1.d);
	    arg1.d = atan2(lp.cf->imaginary, lp.cf->real);
	    while (nRepeat--) {
	      mod2.d = exp(rp.cd->real*mod1.d - rp.cd->imaginary*arg1.d);
	      arg2.d = arg1.d*rp.cd->real + rp.cd->imaginary*log(mod1.d);
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd++->imaginary = mod2.d*sin(arg2.d);
	      rp.cd++;
	    }
	  } else while (nRepeat--) {
	    tp.cd->real = tp.cd->imaginary = 0.0;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  mod1.d = lp.cd->real*lp.cd->real
	    + lp.cd->imaginary*lp.cd->imaginary;
	  if (mod1.d) {
	    mod1.d = 0.5*log(mod1.d);
	    arg1.d = atan2(lp.cd->imaginary, lp.cd->real);
	    while (nRepeat--) {
	      mod2.d = exp(*rp.b*mod1.d);
	      arg2.d = arg1.d* *rp.b++;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd++->imaginary = mod2.d*sin(arg2.d);
	    }
	  } else while (nRepeat--) {
	    tp.cd->real = tp.cd->imaginary = 0.0;
	    tp.cd++;
	  }
	  break;
	case ANA_WORD:
	  mod1.d = lp.cd->real*lp.cd->real
	    + lp.cd->imaginary*lp.cd->imaginary;
	  if (mod1.d) {
	    mod1.d = 0.5*log(mod1.d);
	    arg1.d = atan2(lp.cd->imaginary, lp.cd->real);
	    while (nRepeat--) {
	      mod2.d = exp(*rp.w *mod1.d);
	      arg2.d = arg1.d* *rp.w++;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd++->imaginary = mod2.d*sin(arg2.d);
	    }
	  } else while (nRepeat--) {
	    tp.cd->real = tp.cd->imaginary = 0.0;
	    tp.cd++;
	  }
	  break;
	case ANA_LONG:
	  mod1.d = lp.cd->real*lp.cd->real
	    + lp.cd->imaginary*lp.cd->imaginary;
	  if (mod1.d) {
	    mod1.d = 0.5*log(mod1.d);
	    arg1.d = atan2(lp.cd->imaginary, lp.cd->real);
	    while (nRepeat--) {
	      mod2.d = exp(*rp.l*mod1.d);
	      arg2.d = arg1.d* *rp.l++;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd++->imaginary = mod2.d*sin(arg2.d);
	    }
	  } else while (nRepeat--) {
	    tp.cd->real = tp.cd->imaginary = 0.0;
	    tp.cd++;
	  }
	  break;
	case ANA_FLOAT:
	  mod1.d = lp.cd->real*lp.cd->real
	    + lp.cd->imaginary*lp.cd->imaginary;
	  if (mod1.d) {
	    mod1.d = 0.5*log(mod1.d);
	    arg1.d = atan2(lp.cd->imaginary, lp.cd->real);
	    while (nRepeat--) {
	      mod2.d = exp(*rp.f*mod1.d);
	      arg2.d = arg1.d* *rp.f++;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd++->imaginary = mod2.d*sin(arg2.d);
	    }
	  } else while (nRepeat--) {
	    tp.cd->real = tp.cd->imaginary = 0.0;
	    tp.cd++;
	  }
	  break;
	case ANA_DOUBLE:
	  mod1.d = lp.cd->real*lp.cd->real
	    + lp.cd->imaginary*lp.cd->imaginary;
	  if (mod1.d) {
	    mod1.d = 0.5*log(mod1.d);
	    arg1.d = atan2(lp.cd->imaginary, lp.cd->real);
	    while (nRepeat--) {
	      mod2.d = exp(*rp.d*mod1.d);
	      arg2.d = arg1.d* *rp.d++;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd++->imaginary = mod2.d*sin(arg2.d);
	    }
	  } else while (nRepeat--) {
	    tp.cd->real = tp.cd->imaginary = 0.0;
	    tp.cd++;
	  }
	  break;
	case ANA_CFLOAT:
	  mod1.d = lp.cd->real*lp.cd->real
	    + lp.cd->imaginary*lp.cd->imaginary;
	  if (mod1.d) {
	    mod1.d = 0.5*log(mod1.d);
	    arg1.d = atan2(lp.cd->imaginary, lp.cd->real);
	    while (nRepeat--) {
	      mod2.d = exp(rp.cf->real*mod1.d - rp.cf->imaginary*arg1.d);
	      arg2.d = arg1.d*rp.cf->real + rp.cf->imaginary*log(mod1.d);
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd++->imaginary = mod2.d*sin(arg2.d);
	      rp.cf++;
	    }
	  } else while (nRepeat--) {
	    tp.cd->real = tp.cd->imaginary = 0.0;
	    tp.cd++;
	  }
	  break;
	case ANA_CDOUBLE:
	  mod1.d = lp.cd->real*lp.cd->real
	    + lp.cd->imaginary*lp.cd->imaginary;
	  if (mod1.d) {
	    mod1.d = 0.5*log(mod1.d);
	    arg1.d = atan2(lp.cd->imaginary, lp.cd->real);
	    while (nRepeat--) {
	      mod2.d = exp(rp.cd->real*mod1.d - rp.cd->imaginary*arg1.d);
	      arg2.d = arg1.d*rp.cd->real + rp.cd->imaginary*mod1.d;
	      tp.cd->real = mod2.d*cos(arg2.d);
	      tp.cd++->imaginary = mod2.d*sin(arg2.d);
	      rp.cd++;
	    }
	  } else while (nRepeat--) {
	    tp.cd->real = tp.cd->imaginary = 0.0;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_add(void)
     /* addition with array operands */
{
  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.b++ = *lp.b++ + *rp.b++;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = *lp.b++ + *rp.w++;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = *lp.b++ + *rp.l++;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = *lp.b++ + *rp.f++;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = *lp.b++ + *rp.d++;
	  break; 
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cf->real = *lp.b++ + rp.cf->real;
	    tp.cf++->imaginary = rp.cf++->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = *lp.b++ + rp.cd->real;
	    tp.cd++->imaginary = rp.cd++->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
   case ANA_WORD:
     switch (rhsType) {
       case ANA_BYTE:
	 while (nRepeat--)
	   *tp.w++ = *lp.w++ + *rp.b++;
	 break;
       case ANA_WORD:
	 while (nRepeat--) 
	   *tp.w++ = *lp.w++ + *rp.w++;
	 break;
       case ANA_LONG:
	 while (nRepeat--)
	   *tp.l++ = *lp.w++ + *rp.l++;
	 break;
       case ANA_FLOAT:
	 while (nRepeat--)
	   *tp.f++ = *lp.w++ + *rp.f++;
	 break;
       case ANA_DOUBLE:
	 while (nRepeat--)
	   *tp.d++ = *lp.w++ + *rp.d++;
	 break; 
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cf->real = *lp.w++ + rp.cf->real;
	    tp.cf++->imaginary = rp.cf++->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = *lp.w++ + rp.cd->real;
	    tp.cd++->imaginary = rp.cd++->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
     }
     break;
   case ANA_LONG:
     switch (rhsType) {
       case ANA_BYTE: 
	 while (nRepeat--)
	   *tp.l++ = *lp.l++ + *rp.b++;
	 break;
       case ANA_WORD:
	 while (nRepeat--)
	   *tp.l++ = *lp.l++ + *rp.w++;
	 break;
       case ANA_LONG:
	 while (nRepeat--)
	   *tp.l++ = *lp.l++ + *rp.l++;
	 break;
       case ANA_FLOAT:
	 while (nRepeat--)
	   *tp.f++ = *lp.l++ + *rp.f++;
	 break;
       case ANA_DOUBLE:
	 while (nRepeat--)
	   *tp.d++ = *lp.l++ + *rp.d++;
	 break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cf->real = *lp.l++ + rp.cf->real;
	    tp.cf++->imaginary = rp.cf++->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = *lp.l++ + rp.cd->real;
	    tp.cd++->imaginary = rp.cd++->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
     }
     break;
   case ANA_FLOAT:
     switch (rhsType) {
       case ANA_BYTE:
	 while (nRepeat--)
	   *tp.f++ = *lp.f++ + *rp.b++;
	 break;
       case ANA_WORD:
	 while (nRepeat--)
	   *tp.f++ = *lp.f++ + *rp.w++;
	 break;
       case ANA_LONG:
	 while (nRepeat--)
	   *tp.f++ = *lp.f++ + *rp.l++;
	 break;
       case ANA_FLOAT:
	 while (nRepeat--)
	   *tp.f++ = *lp.f++ + *rp.f++;
	 break;
       case ANA_DOUBLE:
	 while (nRepeat--) 
	   *tp.d++ = *lp.f++ + *rp.d++;
	 break;
       case ANA_CFLOAT:
	 while (nRepeat--) {
	   tp.cf->real = *lp.f++ + rp.cf->real;
	   tp.cf++->imaginary = rp.cf++->imaginary;
	 }
	 break;
       case ANA_CDOUBLE:
	 while (nRepeat--) {
	   tp.cd->real = *lp.f++ + rp.cd->real;
	   tp.cd++->imaginary = rp.cd++->imaginary;
	 }
	 break;
       default:
	 cerror(ILL_TYPE, rhs, typeName(rhsType));
     }
     break;
   case ANA_DOUBLE:
     switch (rhsType) {
       case ANA_BYTE:
	 while (nRepeat--)
	   *tp.d++ = *lp.d++ + *rp.b++;
	 break;
       case ANA_WORD:
	 while (nRepeat--)
	   *tp.d++ = *lp.d++ + *rp.w++;
	 break;
       case ANA_LONG:
	 while (nRepeat--)
	   *tp.d++ = *lp.d++ + *rp.l++;
	 break;
       case ANA_FLOAT:
	 while (nRepeat--)
	   *tp.d++ = *lp.d++ + *rp.f++;
	 break;
       case ANA_DOUBLE:
	 while (nRepeat--)
	   *tp.d++ = *lp.d++ + *rp.d++;
	 break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cd->real = *lp.d++ + rp.cf->real;
	    tp.cd++->imaginary = rp.cf++->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = *lp.d++ + rp.cd->real;
	    tp.cd++->imaginary = rp.cd++->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
     }
     break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real + *rp.b++;
	    tp.cf++->imaginary = lp.cf++->imaginary;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real + *rp.w++;
	    tp.cf++->imaginary = lp.cf++->imaginary;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real + *rp.l++;
	    tp.cf++->imaginary = lp.cf++->imaginary;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real + *rp.f++;
	    tp.cf++->imaginary = lp.cf++->imaginary;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = lp.cf->real + *rp.d++;
	    tp.cd++->imaginary = lp.cf++->imaginary;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real + rp.cf->real;
	    tp.cf++->imaginary = lp.cf++->imaginary + rp.cf++->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = lp.cf->real + rp.cd->real;
	    tp.cd++->imaginary = lp.cf++->imaginary
	      + rp.cd++->imaginary;
	  }
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
	  break;
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real + *rp.b++;
	    tp.cd++->imaginary = lp.cd++->imaginary;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real + *rp.w++;
	    tp.cd++->imaginary = lp.cd++->imaginary;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real + *rp.l++;
	    tp.cd++->imaginary = lp.cd++->imaginary;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real + *rp.f++;
	    tp.cd++->imaginary = lp.cd++->imaginary;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real + *rp.d++;
	    tp.cd++->imaginary = lp.cd++->imaginary;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real + rp.cf->real;
	    tp.cd++->imaginary = lp.cd++->imaginary
	      + (double) rp.cf++->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real + rp.cd->real;
	    tp.cd++->imaginary = lp.cd++->imaginary + rp.cd++->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_add_as(void)
     /* addition with array LHS and scalar RHS */
{
  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.b++ = *lp.b++ + *rp.b;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = (word) *lp.b++ + *rp.w;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (int) *lp.b++ + *rp.l;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) *lp.b++ + *rp.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.b++ + *rp.d;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cf->real = (float) *lp.b++ + rp.cf->real;
	    tp.cf++->imaginary = rp.cf->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = (double) *lp.b++ + rp.cd->real;
	    tp.cd++->imaginary = rp.cd->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.w++ = *lp.w++ + (word) *rp.b;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = *lp.w++ + *rp.w;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (int) *lp.w++ + *rp.l;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) *lp.w++ + *rp.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.w++ + *rp.d;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cf->real = (float) *lp.w++ + rp.cf->real;
	    tp.cf++->imaginary = rp.cf->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = (double) *lp.w++ + rp.cd->real;
	    tp.cd++->imaginary = rp.cd->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = *lp.l++ + (int) *rp.b;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = *lp.l++ + (int) *rp.w;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = *lp.l++ + *rp.l;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) *lp.l++ + *rp.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.l++ + *rp.d;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cf->real = (float) *lp.l++ + rp.cf->real;
	    tp.cf++->imaginary = rp.cf->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = (double) *lp.l++ + rp.cd->real;
	    tp.cd++->imaginary = rp.cd->imaginary;
	  }
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
	  break;
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ + (float) *rp.b;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ + (float) *rp.w;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ + (float) *rp.l;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ + *rp.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.f++ + *rp.d;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cf->real = *lp.f++ + rp.cf->real;
	    tp.cf++->imaginary = rp.cf->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = (double) *lp.f++ + rp.cd->real;
	    tp.cd++->imaginary = rp.cd->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ + (double) *rp.b;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ + (double) *rp.w;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ + (double) *rp.l;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ + (double) *rp.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ + *rp.d;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cd->real = *lp.d++ + (double) rp.cf->real;
	    tp.cd++->imaginary = (double) rp.cf->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = *lp.d++ + rp.cd->real;
	    tp.cd++->imaginary = rp.cd->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real + (float) *rp.b;
	    tp.cf++->imaginary = lp.cf++->imaginary;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real + (float) *rp.w;
	    tp.cf++->imaginary = lp.cf++->imaginary;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real + (float) *rp.l;
	    tp.cf++->imaginary = lp.cf++->imaginary;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real + *rp.f;
	    tp.cf++->imaginary = lp.cf++->imaginary;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = (double) lp.cf->real + *rp.d;
	    tp.cd++->imaginary = (double) lp.cf++->imaginary;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real + rp.cf->real;
	    tp.cf++->imaginary = lp.cf++->imaginary + rp.cf->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = (double) lp.cf->real + rp.cd->real;
	    tp.cd++->imaginary = (double) lp.cf++->imaginary
	      + rp.cd->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real + (double) *rp.b;
	    tp.cd++->imaginary = lp.cd++->imaginary;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real + (double) *rp.w;
	    tp.cd++->imaginary = lp.cd++->imaginary;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real + (double) *rp.l;
	    tp.cd++->imaginary = lp.cd++->imaginary;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real + (double) *rp.f;
	    tp.cd++->imaginary = lp.cd++->imaginary;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real + *rp.d;
	    tp.cd++->imaginary = lp.cd++->imaginary;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real + (double) rp.cf->real;
	    tp.cd++->imaginary = lp.cd++->imaginary
	      + (double) rp.cf->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real + rp.cd->real;
	    tp.cd++->imaginary = lp.cd++->imaginary + rp.cd->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_add_sa(void)
     /* addition with scalar LHS and array RHS.  Since addition is */
     /* commutative, we just swap the LHS and RHS and pass on to */
     /* ana_add_as() */
{
 int		temp;
 pointer	tempp;

 temp = lhsType;
 lhsType = rhsType;
 rhsType = temp;
 tempp = lp;
 lp = rp;
 rp = tempp;
 ana_add_as();
 temp = lhsType;
 lhsType = rhsType;
 rhsType = temp;
 tempp = lp;
 lp = rp;
 rp = tempp;
}
/*----------------------------------------------------------*/
void ana_sub(void)
     /* subtraction with array operands */
{
  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.b++ = *lp.b++ - *rp.b++;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = (word) *lp.b++ - *rp.w++;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (int) *lp.b++ - *rp.l++;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) *lp.b++ - *rp.f++;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.b++ - *rp.d++;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cf->real = (float) *lp.b++ - rp.cf->real;
	    tp.cf++->imaginary = -rp.cf++->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = (double) *lp.b++ - rp.cd->real;
	    tp.cd++->imaginary = -rp.cd++->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.w++ = *lp.w++ - (word) *rp.b++;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = *lp.w++ - *rp.w++;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (int) *lp.w++ - *rp.l++;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) *lp.w++ - *rp.f++;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.w++ - *rp.d++;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cf->real = (float) *lp.w++ - rp.cf->real;
	    tp.cf++->imaginary = -rp.cf++->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = (double) *lp.w++ - rp.cd->real;
	    tp.cd++->imaginary = -rp.cd++->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = *lp.l++ - (int) *rp.b++;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = *lp.l++ - (int) *rp.w++;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = *lp.l++ - *rp.l++;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) *lp.l++ - *rp.f++;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.l++ - *rp.d++;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cf->real = (float) *lp.l++ - rp.cf->real;
	    tp.cf++->imaginary = -rp.cf++->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = (double) *lp.l++ - rp.cd->real;
	    tp.cd++->imaginary = -rp.cd++->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ - (float) *rp.b++;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ - (float) *rp.w++;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ - (float) *rp.l++;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ - *rp.f++;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.f++ - *rp.d++;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cf->real = *lp.f++ - rp.cf->real;
	    tp.cf++->imaginary = -rp.cf++->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = (double) *lp.f++ - rp.cd->real;
	    tp.cd++->imaginary = -rp.cd++->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ - (double) *rp.b++;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ - (double) *rp.w++;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ - (double) *rp.l++;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ - (double) *rp.f++;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ - *rp.d++;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cd->real = *lp.d++ - (double) rp.cf->real;
	    tp.cd++->imaginary = (double) -rp.cf++->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = *lp.d++ - rp.cd->real;
	    tp.cd++->imaginary = -rp.cd++->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real - (float) *rp.b++;
	    tp.cf++->imaginary = lp.cf++->imaginary;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real - (float) *rp.w++;
	    tp.cf++->imaginary = lp.cf++->imaginary;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real - (float) *rp.l++;
	    tp.cf++->imaginary = lp.cf++->imaginary;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real - *rp.f++;
	    tp.cf++->imaginary = lp.cf++->imaginary;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = (double) lp.cf->real - *rp.d++;
	    tp.cd++->imaginary = (double) lp.cf++->imaginary;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real - rp.cf->real;
	    tp.cf++->imaginary = lp.cf++->imaginary - rp.cf++->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = (double) lp.cf->real - rp.cd->real;
	    tp.cd++->imaginary = (double) lp.cf++->imaginary
	      - rp.cd++->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real - (float) *rp.b++;
	    tp.cd++->imaginary = lp.cd++->imaginary;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real - (float) *rp.w++;
	    tp.cd++->imaginary = lp.cd++->imaginary;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real - (float) *rp.l++;
	    tp.cd++->imaginary = lp.cd++->imaginary;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real - *rp.f++;
	    tp.cd++->imaginary = lp.cd++->imaginary;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = (double) lp.cd->real - *rp.d++;
	    tp.cd++->imaginary = (double) lp.cd++->imaginary;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real - rp.cf->real;
	    tp.cd++->imaginary = lp.cd++->imaginary - rp.cf++->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = (double) lp.cd->real - rp.cd->real;
	    tp.cd++->imaginary = (double) lp.cd++->imaginary
	      - rp.cd++->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_sub_as(void)
     /* subtraction with array LHS and scalar RHS */
{
  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.b++ = *lp.b++ - *rp.b;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = (word) *lp.b++ - *rp.w;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (int) *lp.b++ - *rp.l;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) *lp.b++ - *rp.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.b++ - *rp.d;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cf->real = (float) *lp.b++ - rp.cf->real;
	    tp.cf++->imaginary = -rp.cf->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = (double) *lp.b++ - rp.cd->real;
	    tp.cd++->imaginary = -rp.cd->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.w++ = *lp.w++ - (word) *rp.b;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = *lp.w++ - *rp.w;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (int) *lp.w++ - *rp.l;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) *lp.w++ - *rp.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.w++ - *rp.d;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cf->real = (float) *lp.w++ - rp.cf->real;
	    tp.cf++->imaginary = -rp.cf->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = (double) *lp.w++ - rp.cd->real;
	    tp.cd++->imaginary = -rp.cd->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = *lp.l++ - (int) *rp.b;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = *lp.l++ - (int) *rp.w;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = *lp.l++ - *rp.l;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) *lp.l++ - *rp.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.l++ - *rp.d;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cf->real = (float) *lp.l++ - rp.cf->real;
	    tp.cf++->imaginary = -rp.cf->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = (double) *lp.l++ - rp.cd->real;
	    tp.cd++->imaginary = -rp.cd->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ - (float) *rp.b;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ - (float) *rp.w;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ - (float) *rp.l;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ - *rp.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.f++ - *rp.d;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cf->real = *lp.f++ - rp.cf->real;
	    tp.cf++->imaginary = -rp.cf->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = (double) *lp.f++ - rp.cd->real;
	    tp.cd++->imaginary = -rp.cd->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ - (double) *rp.b;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ - (double) *rp.w;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ - (double) *rp.l;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ - (double) *rp.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ - *rp.d;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cd->real = *lp.d++ - (double) rp.cf->real;
	    tp.cd++->imaginary = (double) -rp.cf->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = *lp.d++ - rp.cd->real;
	    tp.cd++->imaginary = -rp.cd->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real - (float) *rp.b;
	    tp.cf++->imaginary = lp.cf++->imaginary;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real - (float) *rp.w;
	    tp.cf++->imaginary = lp.cf++->imaginary;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real - (float) *rp.l;
	    tp.cf++->imaginary = lp.cf++->imaginary;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real - *rp.f;
	    tp.cf++->imaginary = lp.cf++->imaginary;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = (double) lp.cf->real - *rp.d;
	    tp.cd++->imaginary = (double) lp.cf++->imaginary;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real - rp.cf->real;
	    tp.cf++->imaginary = lp.cf++->imaginary - rp.cf->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = (double) lp.cf->real - rp.cd->real;
	    tp.cd++->imaginary = (double) lp.cf++->imaginary
	      - rp.cd->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real - (double) *rp.b;
	    tp.cd++->imaginary = lp.cd++->imaginary;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real - (double) *rp.w;
	    tp.cd++->imaginary = lp.cd++->imaginary;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real - (double) *rp.l;
	    tp.cd++->imaginary = lp.cd++->imaginary;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real - (double) *rp.f;
	    tp.cd++->imaginary = lp.cd++->imaginary;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real - *rp.d;
	    tp.cd++->imaginary = lp.cd++->imaginary;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real - (double) rp.cf->real;
	    tp.cd++->imaginary = lp.cd++->imaginary
	      - (double) rp.cf->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real =  lp.cd->real - rp.cd->real;
	    tp.cd++->imaginary =  lp.cd++->imaginary - rp.cd->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_sub_sa(void)
     /* subtraction with scalar LHS and array RHS */
{
  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.b++ = *lp.b - *rp.b++;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = (word) *lp.b - *rp.w++;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (int) *lp.b - *rp.l++;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) *lp.b - *rp.f++;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.b - *rp.d++;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cf->real = (float) *lp.b - rp.cf->real;
	    tp.cf++->imaginary = -rp.cf++->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = (double) *lp.b - rp.cd->real;
	    tp.cd++->imaginary = -rp.cd++->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.w++ = *lp.w - (word) *rp.b++;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = *lp.w - *rp.w++;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (int) *lp.w - *rp.l++;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) *lp.w - *rp.f++;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.w - *rp.d++;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cf->real = (float) *lp.w - rp.cf->real;
	    tp.cf++->imaginary = -rp.cf++->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = (double) *lp.w - rp.cd->real;
	    tp.cd++->imaginary = -rp.cd++->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = *lp.l - (int) *rp.b++;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = *lp.l - (int) *rp.w++;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = *lp.l - *rp.l++;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) *lp.l - *rp.f++;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.l - *rp.d++;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cf->real = (float) *lp.l - rp.cf->real;
	    tp.cf++->imaginary = -rp.cf++->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = (double) *lp.l - rp.cd->real;
	    tp.cd++->imaginary = -rp.cd++->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.f++ = *lp.f - (float) *rp.b++;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.f++ = *lp.f - (float) *rp.w++;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.f++ = *lp.f - (float) *rp.l++;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = *lp.f - *rp.f++;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.f - *rp.d++;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cf->real = *lp.f - rp.cf->real;
	    tp.cf++->imaginary = -rp.cf++->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = (double) *lp.f - rp.cd->real;
	    tp.cd++->imaginary = -rp.cd++->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.d++ = *lp.d - (double) *rp.b++;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.d++ = *lp.d - (double) *rp.w++;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.d++ = *lp.d - (double) *rp.l++;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.d++ = *lp.d - (double) *rp.f++;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = *lp.d - *rp.d++;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cd->real = *lp.d - (double) rp.cf->real;
	    tp.cd++->imaginary = (double) -rp.cf++->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = *lp.d - rp.cd->real;
	    tp.cd++->imaginary = -rp.cd++->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real - (float) *rp.b++;
	    tp.cf++->imaginary = lp.cf->imaginary;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real - (float) *rp.w++;
	    tp.cf++->imaginary = lp.cf->imaginary;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real - (float) *rp.l++;
	    tp.cf++->imaginary = lp.cf->imaginary;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real - *rp.f++;
	    tp.cf++->imaginary = lp.cf->imaginary;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = (double) lp.cf->real - *rp.d++;
	    tp.cd++->imaginary = (double) lp.cf->imaginary;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real - rp.cf->real;
	    tp.cf++->imaginary = lp.cf->imaginary - rp.cf++->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = (double) lp.cf->real - rp.cd->real;
	    tp.cd++->imaginary = (double) lp.cf->imaginary
	      - rp.cd++->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real - (double) *rp.b++;
	    tp.cd++->imaginary = lp.cd->imaginary;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real - (double) *rp.w++;
	    tp.cd++->imaginary = lp.cd->imaginary;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real - (double) *rp.l++;
	    tp.cd++->imaginary = lp.cd->imaginary;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real - (double) *rp.f++;
	    tp.cd++->imaginary = lp.cd->imaginary;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real - *rp.d++;
	    tp.cd++->imaginary = lp.cd->imaginary;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real - (double) rp.cf->real;
	    tp.cd++->imaginary = lp.cd->imaginary
	      - (double) rp.cf++->imaginary;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    tp.cd->real =  lp.cd->real - rp.cd->real;
	    tp.cd++->imaginary =  lp.cd->imaginary - rp.cd++->imaginary;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_mul(void)
     /* multiplication with array operands */
{
  scalar	re, im;

  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.b++ = *lp.b++ * *rp.b++;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = (word) *lp.b++ * *rp.w++;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (int) *lp.b++ * *rp.l++;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) *lp.b++ * *rp.f++;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.b++ * *rp.d++;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    re.f = (float) *lp.b * rp.cf->real;
	    im.f = (float) *lp.b++ * rp.cf++->imaginary;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    re.d = (double) *lp.b * rp.cd->real;
	    im.d = (double) *lp.b++ * rp.cd++->imaginary;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.w++ = *lp.w++ * (word) *rp.b++;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = *lp.w++ * *rp.w++;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (int) *lp.w++ * *rp.l++;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) *lp.w++ * *rp.f++;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.w++ * *rp.d++;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    re.f = (float) *lp.w * rp.cf->real;
	    im.f = (float) *lp.w++ * rp.cf++->imaginary;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    re.d = (double) *lp.w * rp.cd->real;
	    im.d = (double) *lp.w++ * rp.cd++->imaginary;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = *lp.l++ * (int) *rp.b++;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = *lp.l++ * (int) *rp.w++;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = *lp.l++ * *rp.l++;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) *lp.l++ * *rp.f++;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.l++ * *rp.d++;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    re.f = (float) *lp.l * rp.cf->real;
	    im.f = (float) *lp.l++ * rp.cf++->imaginary;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    re.d = (double) *lp.l * rp.cd->real;
	    im.d = (double) *lp.l++ * rp.cd++->imaginary;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ * (float) *rp.b++;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ * (float) *rp.w++;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ * (float) *rp.l++;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ * *rp.f++;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.f++ * *rp.d++;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    re.f = *lp.f * rp.cf->real;
	    im.f = *lp.f++ * rp.cf++->imaginary;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    re.d = (double) *lp.f * rp.cd->real;
	    im.d = (double) *lp.f++ * rp.cd++->imaginary;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ * (double) *rp.b++;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ * (double) *rp.w++;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ * (double) *rp.l++;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ * (double) *rp.f++;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ * *rp.d++;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    re.d = *lp.d * (double) rp.cf->real;
	    im.d = *lp.d++ * (double) rp.cf++->imaginary;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    re.d = *lp.d * rp.cd->real;
	    im.d = *lp.d++ * rp.cd++->imaginary;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    re.f = lp.cf->real * (float) *rp.b;
	    im.f = lp.cf++->imaginary * (float) *rp.b++;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    re.f = lp.cf->real * (float) *rp.w;
	    im.f = lp.cf++->imaginary * (float) *rp.w++;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    re.f = lp.cf->real * (float) *rp.l;
	    im.f = lp.cf++->imaginary * (float) *rp.l++;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    re.f = lp.cf->real * *rp.f;
	    im.f = lp.cf++->imaginary * *rp.f++;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    re.d = (double) lp.cf->real * *rp.d;
	    im.d = (double) lp.cf++->imaginary * *rp.d++;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    re.f = lp.cf->real*rp.cf->real
	      - lp.cf->imaginary*rp.cf->imaginary;
	    im.f = lp.cf->real*rp.cf->imaginary
	      + lp.cf->imaginary*rp.cf->real;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	    lp.cf++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    re.d = (double) lp.cf->real*rp.cd->real
	      - (double) lp.cf->imaginary*rp.cd->imaginary;
	    im.d = (double) lp.cf->real*rp.cd->imaginary
	     + (double) lp.cf->imaginary*rp.cd->real;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	    lp.cf++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    re.d = lp.cd->real * (float) *rp.b;
	    im.d = lp.cd++->imaginary * (float) *rp.b++;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    re.d = lp.cd->real * (float) *rp.w;
	    im.d = lp.cd++->imaginary * (float) *rp.w++;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    re.d = lp.cd->real * (float) *rp.l;
	    im.d = lp.cd++->imaginary * (float) *rp.l++;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    re.d = lp.cd->real * (double) *rp.f;
	    im.d = lp.cd++->imaginary * (double) *rp.f++;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    re.d = lp.cd->real * *rp.d;
	    im.d = lp.cd++->imaginary * *rp.d++;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    re.d = lp.cd->real * (double) rp.cf->real
	      - lp.cd->imaginary * (double) rp.cf->imaginary;
	    im.d = lp.cd->real * (double) rp.cf->imaginary
	      + lp.cd->imaginary * (double) rp.cf->real;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	    lp.cd++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    re.d = lp.cd->real*rp.cd->real
	      - lp.cd->imaginary*rp.cd->imaginary;
	    im.d = lp.cd->real*rp.cd->imaginary
	      + lp.cd->imaginary*rp.cd->real;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	    lp.cd++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_mul_as(void)
     /* multiplication with array LHS and scalar RHS */
{
  scalar	re, im;

  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.b++ = *lp.b++ * *rp.b;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = (word) *lp.b++ * *rp.w;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (int) *lp.b++ * *rp.l;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) *lp.b++ * *rp.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.b++ * *rp.d;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    re.f = (float) *lp.b * rp.cf->real;
	    im.f = (float) *lp.b++ * rp.cf->imaginary;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    re.d = (double) *lp.b * rp.cd->real;
	    im.d = (double) *lp.b++ * rp.cd->imaginary;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.w++ = *lp.w++ * (word) *rp.b;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = *lp.w++ * *rp.w;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (int) *lp.w++ * *rp.l;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) *lp.w++ * *rp.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.w++ * *rp.d;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    re.f = (float) *lp.w * rp.cf->real;
	    im.f = (float) *lp.w++ * rp.cf->imaginary;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    re.d = (double) *lp.w * rp.cd->real;
	    im.d = (double) *lp.w++ * rp.cd->imaginary;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = *lp.l++ * (int) *rp.b;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = *lp.l++ * (int) *rp.w;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = *lp.l++ * *rp.l;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) *lp.l++ * *rp.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.l++ * *rp.d;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    re.f = (float) *lp.l * rp.cf->real;
	    im.f = (float) *lp.l++ * rp.cf->imaginary;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    re.d = (double) *lp.l * rp.cd->real;
	    im.d = (double) *lp.l++ * rp.cd->imaginary;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ * (float) *rp.b;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ * (float) *rp.w;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ * (float) *rp.l;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ * *rp.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.f++ * *rp.d;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    re.f = (float) *lp.f * rp.cf->real;
	    im.f = (float) *lp.f++ * rp.cf->imaginary;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    re.d = (double) *lp.f * rp.cd->real;
	    im.d = (double) *lp.f++ * rp.cd->imaginary;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ * (double) *rp.b;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ * (double) *rp.w;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ * (double) *rp.l;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ * (double) *rp.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ * *rp.d;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    re.d = *lp.d * (double) rp.cf->real;
	    im.d = *lp.d++ * (double) rp.cf->imaginary;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    re.d = *lp.d * rp.cd->real;
	    im.d = *lp.d++ * rp.cd->imaginary;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real*(float) *rp.b;
	    tp.cf++->imaginary = lp.cf++->imaginary*(float) *rp.b;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real*(float) *rp.w;
	    tp.cf++->imaginary = lp.cf++->imaginary*(float) *rp.w;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real*(float) *rp.l;
	    tp.cf++->imaginary = lp.cf++->imaginary*(float) *rp.l;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    tp.cf->real = lp.cf->real * *rp.f;
	    tp.cf++->imaginary = lp.cf++->imaginary * *rp.f;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = (double) lp.cf->real * *rp.d;
	    tp.cd++->imaginary = (double) lp.cf++->imaginary * *rp.d;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    re.f = lp.cf->real*rp.cf->real
	      - lp.cf->imaginary*rp.cf->imaginary;
	    im.f = lp.cf->real*rp.cf->imaginary
	      + lp.cf->imaginary*rp.cf->real;
	    tp.cf->real = re.f;
	    tp.cf->imaginary = im.f;
	    lp.cf++;
	    tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    re.d = lp.cf->real*rp.cd->real
	      - lp.cf->imaginary*rp.cd->imaginary;
	    im.d = lp.cf->real*rp.cd->imaginary
	      + lp.cf->imaginary*rp.cd->real;
	    tp.cd->real = re.d;
	    tp.cd->imaginary = im.d;
	    lp.cf++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real*(double) *rp.b;
	    tp.cd++->imaginary = lp.cd++->imaginary*(double) *rp.b;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real*(double) *rp.w;
	    tp.cd++->imaginary = lp.cd++->imaginary*(double) *rp.w;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real*(double) *rp.l;
	    tp.cd++->imaginary = lp.cd++->imaginary*(double) *rp.l;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real*(double) *rp.f;
	    tp.cd++->imaginary = lp.cd++->imaginary*(double) *rp.f;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    tp.cd->real = lp.cd->real * *rp.d;
	    tp.cd++->imaginary = lp.cd++->imaginary * *rp.d;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    re.d = lp.cd->real*rp.cf->real
	      - lp.cd->imaginary*rp.cf->imaginary;
	    im.d = lp.cd->real*rp.cf->imaginary
	      + lp.cd->imaginary*rp.cf->real;
	    tp.cd->real = re.d;
	    tp.cd->imaginary = im.d;
	    lp.cd++;
	    tp.cd++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    re.d = lp.cd->real*rp.cd->real
	      - lp.cd->imaginary*rp.cd->imaginary;
	    im.d = lp.cd->real*rp.cd->imaginary
	      + lp.cd->imaginary*rp.cd->real;
	    tp.cd->real = re.d;
	    tp.cd->imaginary = im.d;
	    lp.cd++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_mul_sa(void)
     /* multiplication with scalar LHS and array RHS.  Since multiplication */
     /* is commutative, we just swap the LHS and RHS and pass on to */
     /* ana_mul_as() */
{
  int		temp;
  pointer	tempp;
  
  temp = lhsType;
  lhsType = rhsType;
  rhsType = temp;
  tempp = lp;
  lp = rp;
  rp = tempp;
  ana_mul_as();
  temp = lhsType;
  lhsType = rhsType;
  rhsType = temp;
  tempp = lp;
  lp = rp;
  rp = tempp;
}
/*----------------------------------------------------------*/
void ana_div(void)
     /* division with array operands.
	NOTE: no checking for division by zero! */
{
  scalar	re, im, d;

  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.b++ = *lp.b++ / *rp.b++;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = (word) *lp.b++ / *rp.w++;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (int) *lp.b++ / *rp.l++;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) *lp.b++ / *rp.f++;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.b++ / *rp.d++;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    d.f = rp.cf->real*rp.cf->real
	      + rp.cf->imaginary*rp.cf->imaginary;
	    d.f = 1.0/d.f;
	    re.f = ((float) *lp.b * rp.cf->real)*d.f;
	    im.f = (-(float) *lp.b++ * rp.cf++->imaginary)*d.f;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    d.d = rp.cd->real*rp.cd->real
	      + rp.cd->imaginary*rp.cd->imaginary;
	    d.d = 1.0/d.d;
	    re.d = ((double) *lp.b * rp.cd->real)*d.d;
	    im.d = (-(double) *lp.b++ * rp.cd++->imaginary)*d.d;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.w++ = *lp.w++ / (word) *rp.b++;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = *lp.w++ / *rp.w++;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (int) *lp.w++ / *rp.l++;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) *lp.w++ / *rp.f++;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.w++ / *rp.d++;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    d.f = rp.cf->real*rp.cf->real
	      + rp.cf->imaginary*rp.cf->imaginary;
	    d.f = 1.0/d.f;
	    re.f = ((float) *lp.w * rp.cf->real)*d.f;
	    im.f = (-(float) *lp.w++ * rp.cf++->imaginary)*d.f;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    d.d = rp.cd->real*rp.cd->real
	      + rp.cd->imaginary*rp.cd->imaginary;
	    d.d = 1.0/d.d;
	    re.d = ((double) *lp.w * rp.cd->real)*d.d;
	    im.d = (-(double) *lp.w++ * rp.cd++->imaginary)*d.d;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = *lp.l++ / (int) *rp.b++;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = *lp.l++ / (int) *rp.w++;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = *lp.l++ / *rp.l++;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) *lp.l++ / *rp.f++;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.l++ / *rp.d++;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    d.f = rp.cf->real*rp.cf->real
	      + rp.cf->imaginary*rp.cf->imaginary;
	    d.f = 1.0/d.f;
	    re.f = ((float) *lp.l * rp.cf->real)*d.f;
	    im.f = (-(float) *lp.l++ * rp.cf++->imaginary)*d.f;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    d.d = rp.cd->real*rp.cd->real
	      + rp.cd->imaginary*rp.cd->imaginary;
	    d.d = 1.0/d.d;
	    re.d = ((double) *lp.l * rp.cd->real)*d.d;
	    im.d = (-(double) *lp.l++ * rp.cd++->imaginary)*d.d;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ / (float) *rp.b++;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ / (float) *rp.w++;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ / (float) *rp.l++;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ / *rp.f++;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.f++ / *rp.d++;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    d.f = rp.cf->real*rp.cf->real
	      + rp.cf->imaginary*rp.cf->imaginary;
	    d.f = 1.0/d.f;
	    re.f = ((float) *lp.f * rp.cf->real)*d.f;
	    im.f = (-(float) *lp.f++ * rp.cf++->imaginary)*d.f;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    d.d = rp.cd->real*rp.cd->real
	      + rp.cd->imaginary*rp.cd->imaginary;
	    d.d = 1.0/d.d;
	    re.d = ((double) *lp.f * rp.cd->real)*d.d;
	    im.d = (-(double) *lp.f++ * rp.cd++->imaginary)*d.d;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ / (double) *rp.b++;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ / (double) *rp.w++;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ / (double) *rp.l++;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ / (double) *rp.f++;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ / *rp.d++;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    d.d = (double) rp.cf->real*(double) rp.cf->real
	     + (double) rp.cf->imaginary*(double) rp.cf->imaginary;
	    d.d = 1.0/d.d;
	    re.d = (*lp.d * (double) rp.cf->real)*d.d;
	    im.d = (-*lp.d++ * (double) rp.cf++->imaginary)*d.d;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    d.d = rp.cd->real*rp.cd->real
	      + rp.cd->imaginary*rp.cd->imaginary;
	    d.d = 1.0/d.d;
	    re.d = (*lp.d * rp.cd->real)*d.d;
	    im.d = (-*lp.d++ * rp.cd++->imaginary)*d.d;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    d.f = 1.0/(float) *rp.b++;
	    tp.cf->real = lp.cf->real*d.f;
	    tp.cf++->imaginary = lp.cf++->imaginary*d.f;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    d.f = 1.0/(float) *rp.w++;
	    tp.cf->real = lp.cf->real*d.f;
	    tp.cf++->imaginary = lp.cf++->imaginary*d.f;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    d.f = 1.0/(float) *rp.l++;
	    tp.cf->real = lp.cf->real*d.f;
	    tp.cf++->imaginary = lp.cf++->imaginary*d.f;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    d.f = 1.0/ *rp.f++;
	    tp.cf->real = lp.cf->real*d.f;
	    tp.cf++->imaginary = lp.cf++->imaginary*d.f;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    d.d = 1.0/ *rp.d++;
	    tp.cd->real = (double) lp.cf->real*d.d;
	    tp.cd++->imaginary = (double) lp.cf++->imaginary*d.d;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    d.f = rp.cf->real*rp.cf->real
	      + rp.cf->imaginary*rp.cf->imaginary;
	    d.f = 1.0/d.f;
	    re.f = (lp.cf->real*rp.cf->real
		    + lp.cf->imaginary*rp.cf->imaginary)*d.f;
	    im.f = (lp.cf->imaginary*rp.cf->real
		    - lp.cf->real*rp.cf->imaginary)*d.f;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	    lp.cf++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    d.d = rp.cd->real*rp.cd->real
	      + rp.cd->imaginary*rp.cd->imaginary;
	    d.d = 1.0/d.d;
	    re.d = ((double) lp.cf->real*rp.cd->real
		    + (double) lp.cf->imaginary*rp.cd->imaginary)*d.d;
	    im.d = ((double) lp.cf->imaginary*rp.cd->real
		    - (double) lp.cf->real*rp.cd->imaginary)*d.d;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	    lp.cf++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    d.d = 1.0/(double) *rp.b++;
	    tp.cd->real = lp.cd->real*d.d;
	    tp.cd++->imaginary = lp.cd++->imaginary*d.d;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    d.d = 1.0/(double) *rp.w++;
	    tp.cd->real = lp.cd->real*d.d;
	    tp.cd++->imaginary = lp.cd++->imaginary*d.d;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    d.d = 1.0/(double) *rp.l++;
	    tp.cd->real = lp.cd->real*d.d;
	    tp.cd++->imaginary = lp.cd++->imaginary*d.d;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    d.d = 1.0/(double) *rp.f++;
	    tp.cd->real = lp.cd->real*d.d;
	    tp.cd++->imaginary = lp.cd++->imaginary*d.d;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    d.d = 1.0/ *rp.d++;
	    tp.cd->real = lp.cd->real*d.d;
	    tp.cd++->imaginary = lp.cd++->imaginary*d.d;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    d.d = (double) rp.cf->real*(double) rp.cf->real
	      + (double) rp.cf->imaginary*(double) rp.cf->imaginary;
	    d.d = 1.0/d.d;
	    re.d = (lp.cd->real*(double) rp.cf->real
		    + lp.cd->imaginary*(double) rp.cf->imaginary)*d.d;
	    im.d = (lp.cd->imaginary*(double) rp.cf->real
		    - lp.cd->real*(double) rp.cf->imaginary)*d.d;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	    lp.cd++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    d.d = rp.cd->real*rp.cd->real
	      + rp.cd->imaginary*rp.cd->imaginary;
	    d.d = 1.0/d.d;
	    re.d = (lp.cd->real*rp.cd->real
		    + lp.cd->imaginary*rp.cd->imaginary)*d.d;
	    im.d = (lp.cd->imaginary*rp.cd->real
		    - lp.cd->real*rp.cd->imaginary)*d.d;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	    lp.cd++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;      
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_div_as(void)
     /* division with an array LHS and scalar RHS */
{
  scalar	re, im, d;

  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.b++ = *lp.b++ / *rp.b;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = (word) *lp.b++ / *rp.w;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (int) *lp.b++ / *rp.l;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) *lp.b++ / *rp.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.b++ / *rp.d;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    d.f = rp.cf->real*rp.cf->real
	      + rp.cf->imaginary*rp.cf->imaginary;
	    d.f = 1.0/d.f;
	    re.f = ((float) *lp.b * rp.cf->real)*d.f;
	    im.f = (-(float) *lp.b++ * rp.cf->imaginary)*d.f;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    d.d = rp.cd->real*rp.cd->real
	      + rp.cd->imaginary*rp.cd->imaginary;
	    d.d = 1.0/d.d;
	    re.d = ((double) *lp.b * rp.cd->real)*d.d;
	    im.d = (-(double) *lp.b++ * rp.cd->imaginary)*d.d;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.w++ = *lp.w++ / (word) *rp.b;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = *lp.w++ / *rp.w;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (int) *lp.w++ / *rp.l;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) *lp.w++ / *rp.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.w++ / *rp.d;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    d.f = rp.cf->real*rp.cf->real
	      + rp.cf->imaginary*rp.cf->imaginary;
	    d.f = 1.0/d.f;
	    re.f = ((float) *lp.w * rp.cf->real)*d.f;
	    im.f = (-(float) *lp.w++ * rp.cf->imaginary)*d.f;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    d.d = rp.cd->real*rp.cd->real
	      + rp.cd->imaginary*rp.cd->imaginary;
	    d.d = 1.0/d.d;
	    re.d = ((double) *lp.w * rp.cd->real)*d.d;
	    im.d = (-(double) *lp.w++ * rp.cd->imaginary)*d.d;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = *lp.l++ / (int) *rp.b;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = *lp.l++ / (int) *rp.w;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = *lp.l++ / *rp.l;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) *lp.l++ / *rp.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.l++ / *rp.d;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    d.f = rp.cf->real*rp.cf->real
	      + rp.cf->imaginary*rp.cf->imaginary;
	    d.f = 1.0/d.f;
	    re.f = ((float) *lp.l * rp.cf->real)*d.f;
	    im.f = (-(float) *lp.l++ * rp.cf->imaginary)*d.f;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    d.d = rp.cd->real*rp.cd->real
	      + rp.cd->imaginary*rp.cd->imaginary;
	    d.d = 1.0/d.d;
	    re.d = ((double) *lp.l * rp.cd->real)*d.d;
	    im.d = (-(double) *lp.l++ * rp.cd->imaginary)*d.d;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ / (float) *rp.b;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ / (float) *rp.w;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ / (float) *rp.l;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = *lp.f++ / *rp.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.f++ / *rp.d;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    d.f = rp.cf->real*rp.cf->real
	      + rp.cd->imaginary*rp.cd->imaginary;
	    d.f = 1.0/d.f;
	    re.f = ((float) *lp.f * rp.cf->real)*d.f;
	    im.f = (-(float) *lp.f++ * rp.cf->imaginary)*d.f;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    d.d = rp.cd->real*rp.cd->real
	      + rp.cd->imaginary*rp.cd->imaginary;
	    d.d = 1.0/d.d;
	    re.d = ((double) *lp.f * rp.cd->real)*d.d;
	    im.d = (-(double) *lp.f++ * rp.cd->imaginary)*d.d;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ / (double) *rp.b;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ / (double) *rp.w;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ / (double) *rp.l;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ / (double) *rp.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = *lp.d++ / *rp.d;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    d.d = (double) rp.cf->real*(double) rp.cf->real
	     + (double) rp.cf->imaginary*(double) rp.cf->imaginary;
	    d.d = 1.0/d.d;
	    re.d = (*lp.d * (double) rp.cf->real)*d.d;
	    im.d = (-*lp.d++ * (double) rp.cf->imaginary)*d.d;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    d.d = rp.cd->real*rp.cd->real
	      + rp.cd->imaginary*rp.cd->imaginary;
	    d.d = 1.0/d.d;
	    re.d = (*lp.d * rp.cd->real)*d.d;
	    im.d = (-*lp.d++ * rp.cd->imaginary)*d.d;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    d.f = 1.0/(float) *rp.b;
	    tp.cf->real = lp.cf->real*d.f;
	    tp.cf++->imaginary = lp.cf++->imaginary*d.f;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    d.f = 1.0/(float) *rp.w;
	    tp.cf->real = lp.cf->real*d.f;
	    tp.cf++->imaginary = lp.cf++->imaginary*d.f;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    d.f = 1.0/(float) *rp.l;
	    tp.cf->real = lp.cf->real*d.f;
	    tp.cf++->imaginary = lp.cf++->imaginary*d.f;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    d.f = 1.0/ *rp.f;
	    tp.cf->real = lp.cf->real*d.f;
	    tp.cf++->imaginary = lp.cf++->imaginary*d.f;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    d.d = 1.0/ *rp.d;
	    tp.cd->real = (double) lp.cf->real*d.d;
	    tp.cd++->imaginary = (double) lp.cf++->imaginary*d.d;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    d.f = rp.cf->real*rp.cf->real
	      + rp.cf->imaginary*rp.cf->imaginary;
	    d.f = 1.0/d.f;
	    re.f = (lp.cf->real*rp.cf->real
		    + lp.cf->imaginary*rp.cf->imaginary)*d.f;
	    im.f = (lp.cf->imaginary*rp.cf->real
		    - lp.cf->real*rp.cf->imaginary)*d.f;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	    lp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    d.d = rp.cd->real*rp.cd->real
	      + rp.cd->imaginary*rp.cd->imaginary;
	    d.d = 1.0/d.d;
	    re.d = ((double) lp.cf->real*rp.cd->real
		    + (double) lp.cf->imaginary*rp.cd->imaginary)*d.d;
	    im.d = ((double) lp.cf->imaginary*rp.cd->real
		    - (double) lp.cf->real*rp.cd->imaginary)*d.d;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	    lp.cf++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    d.d = 1.0/(double) *rp.b;
	    tp.cd->real = lp.cd->real*d.d;
	    tp.cd++->imaginary = lp.cd++->imaginary*d.d;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    d.d = 1.0/(double) *rp.w;
	    tp.cd->real = lp.cd->real*d.d;
	    tp.cd++->imaginary = lp.cd++->imaginary*d.d;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    d.d = 1.0/(double) *rp.l;
	    tp.cd->real = lp.cd->real*d.d;
	    tp.cd++->imaginary = lp.cd++->imaginary*d.d;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    d.d = 1.0/(double) *rp.f;
	    tp.cd->real = lp.cd->real*d.d;
	    tp.cd++->imaginary = lp.cd++->imaginary*d.d;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    d.d = 1.0/ *rp.d;
	    tp.cd->real = lp.cd->real*d.d;
	    tp.cd++->imaginary = lp.cd++->imaginary*d.d;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    d.d = (double) rp.cf->real*(double) rp.cf->real
	      + (double) rp.cf->imaginary*(double) rp.cf->imaginary;
	    d.d = 1.0/d.d;
	    re.d = (lp.cd->real*(double) rp.cf->real
		    + lp.cd->imaginary*(double) rp.cf->imaginary)*d.d;
	    im.d = (lp.cd->imaginary*(double) rp.cf->real
		    - lp.cd->real*(double) rp.cf->imaginary)*d.d;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	    lp.cd++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    d.d = rp.cd->real*rp.cd->real
	      + rp.cd->imaginary*rp.cd->imaginary;
	    d.d = 1.0/d.d;
	    re.d = (lp.cd->real*rp.cd->real
		    + lp.cd->imaginary*rp.cd->imaginary)*d.d;
	    im.d = (lp.cd->imaginary*rp.cd->real
		    - lp.cd->real*rp.cd->imaginary)*d.d;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	    lp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;      
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_div_sa(void)
     /* division with scalar LHS and array RHS */
{
  scalar	re, im, d;

  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.b++ = *lp.b / *rp.b++;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = (word) *lp.b / *rp.w++;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (int) *lp.b / *rp.l++;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) *lp.b / *rp.f++;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.b / *rp.d++;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    d.f = rp.cf->real*rp.cf->real
	      + rp.cf->imaginary*rp.cf->imaginary;
	    d.f = 1.0/d.f;
	    re.f = ((float) *lp.b * rp.cf->real)*d.f;
	    im.f = (-(float) *lp.b * rp.cf++->imaginary)*d.f;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    d.d = rp.cd->real*rp.cd->real
	      + rp.cd->imaginary*rp.cd->imaginary;
	    d.d = 1.0/d.d;
	    re.d = ((double) *lp.b * rp.cd->real)*d.d;
	    im.d = (-(double) *lp.b * rp.cd++->imaginary)*d.d;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.w++ = *lp.w / (word) *rp.b++;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = *lp.w / *rp.w++;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (int) *lp.w / *rp.l++;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) *lp.w / *rp.f++;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.w / *rp.d++;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    d.f = rp.cf->real*rp.cf->real
	      + rp.cf->imaginary*rp.cf->imaginary;
	    d.f = 1.0/d.f;
	    re.f = ((float) *lp.w * rp.cf->real)*d.f;
	    im.f = (-(float) *lp.w * rp.cf++->imaginary)*d.f;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    d.d = rp.cd->real*rp.cd->real
	      + rp.cd->imaginary*rp.cd->imaginary;
	    d.d = 1.0/d.d;
	    re.d = ((double) *lp.w * rp.cd->real)*d.d;
	    im.d = (-(double) *lp.w * rp.cd++->imaginary)*d.d;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = *lp.l / (int) *rp.b++;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = *lp.l / (int) *rp.w++;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = *lp.l / *rp.l++;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = (float) *lp.l / *rp.f++;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.l / *rp.d++;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    d.f = rp.cf->real*rp.cf->real
	      + rp.cf->imaginary*rp.cf->imaginary;
	    d.f = 1.0/d.f;
	    re.f = ((float) *lp.l * rp.cf->real)*d.f;
	    im.f = (-(float) *lp.l * rp.cf++->imaginary)*d.f;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    d.d = rp.cd->real*rp.cd->real
	      + rp.cd->imaginary*rp.cd->imaginary;
	    d.d = 1.0/d.d;
	    re.d = ((double) *lp.l * rp.cd->real)*d.d;
	    im.d = (-(double) *lp.l * rp.cd++->imaginary)*d.d;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.f++ = *lp.f / (float) *rp.b++;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.f++ = *lp.f / (float) *rp.w++;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.f++ = *lp.f / (float) *rp.l++;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = *lp.f / *rp.f++;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = (double) *lp.f / *rp.d++;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    d.f = rp.cf->real*rp.cf->real
	      + rp.cf->imaginary*rp.cf->imaginary;
	    d.f = 1.0/d.f;
	    re.f = ((float) *lp.f * rp.cf->real)*d.f;
	    im.f = (-(float) *lp.f * rp.cf++->imaginary)*d.f;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    d.d = rp.cd->real*rp.cd->real
	      + rp.cd->imaginary*rp.cd->imaginary;
	    d.d = 1.0/d.d;
	    re.d = ((double) *lp.f * rp.cd->real)*d.d;
	    im.d = (-(double) *lp.f * rp.cd++->imaginary)*d.d;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.d++ = *lp.d / (double) *rp.b++;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.d++ = *lp.d / (double) *rp.w++;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.d++ = *lp.d / (double) *rp.l++;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.d++ = *lp.d / (double) *rp.f++;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = *lp.d / *rp.d++;
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    d.d = (double) rp.cf->real*(double) rp.cf->real
	     + (double) rp.cf->imaginary*(double) rp.cf->imaginary;
	    d.d = 1.0/d.d;
	    re.d = (*lp.d * (double) rp.cf->real)*d.d;
	    im.d = (-*lp.d * (double) rp.cf++->imaginary)*d.d;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    d.d = rp.cd->real*rp.cd->real
	      + rp.cd->imaginary*rp.cd->imaginary;
	    d.d = 1.0/d.d;
	    re.d = (*lp.d * rp.cd->real)*d.d;
	    im.d = (-*lp.d * rp.cd++->imaginary)*d.d;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    d.f = 1.0/(float) *rp.b++;
	    tp.cf->real = lp.cf->real*d.f;
	    tp.cf++->imaginary = lp.cf->imaginary*d.f;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    d.f = 1.0/(float) *rp.w++;
	    tp.cf->real = lp.cf->real*d.f;
	    tp.cf++->imaginary = lp.cf->imaginary*d.f;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    d.f = 1.0/(float) *rp.l++;
	    tp.cf->real = lp.cf->real*d.f;
	    tp.cf++->imaginary = lp.cf->imaginary*d.f;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    d.f = 1.0/ *rp.f++;
	    tp.cf->real = lp.cf->real*d.f;
	    tp.cf++->imaginary = lp.cf->imaginary*d.f;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    d.d = 1.0/ *rp.d++;
	    tp.cd->real = (double) lp.cf->real*d.d;
	    tp.cd++->imaginary = (double) lp.cf->imaginary*d.d;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    d.f = rp.cf->real*rp.cf->real
	      + rp.cf->imaginary*rp.cf->imaginary;
	    d.f = 1.0/d.f;
	    re.f = (lp.cf->real*rp.cf->real
		    + lp.cf->imaginary*rp.cf->imaginary)*d.f;
	    im.f = (lp.cf->imaginary*rp.cf->real
		    - lp.cf->real*rp.cf->imaginary)*d.f;
	    tp.cf->real = re.f;
	    tp.cf++->imaginary = im.f;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    d.d = rp.cd->real*rp.cd->real
	      + rp.cd->imaginary*rp.cd->imaginary;
	    d.d = 1.0/d.d;
	    re.d = ((double) lp.cf->real*rp.cd->real
		    + (double) lp.cf->imaginary*rp.cd->imaginary)*d.d;
	    im.d = ((double) lp.cf->imaginary*rp.cd->real
		    - (double) lp.cf->real*rp.cd->imaginary)*d.d;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    d.d = 1.0/(double) *rp.b++;
	    tp.cd->real = lp.cd->real*d.d;
	    tp.cd++->imaginary = lp.cd->imaginary*d.d;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    d.d = 1.0/(double) *rp.w++;
	    tp.cd->real = lp.cd->real*d.d;
	    tp.cd++->imaginary = lp.cd->imaginary*d.d;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    d.d = 1.0/(double) *rp.l++;
	    tp.cd->real = lp.cd->real*d.d;
	    tp.cd++->imaginary = lp.cd->imaginary*d.d;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    d.d = 1.0/(double) *rp.f++;
	    tp.cd->real = lp.cd->real*d.d;
	    tp.cd++->imaginary = lp.cd->imaginary*d.d;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    d.d = 1.0/ *rp.d++;
	    tp.cd->real = lp.cd->real*d.d;
	    tp.cd++->imaginary = lp.cd->imaginary*d.d;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    d.d = (double) rp.cf->real*(double) rp.cf->real
	      + (double) rp.cf->imaginary*(double) rp.cf->imaginary;
	    d.d = 1.0/d.d;
	    re.d = (lp.cd->real*(double) rp.cf->real
		    + lp.cd->imaginary*(double) rp.cf->imaginary)*d.d;
	    im.d = (lp.cd->imaginary*(double) rp.cf->real
		    - lp.cd->real*(double) rp.cf->imaginary)*d.d;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    d.d = rp.cd->real*rp.cd->real
	      + rp.cd->imaginary*rp.cd->imaginary;
	    d.d = 1.0/d.d;
	    re.d = (lp.cd->real*rp.cd->real
		    + lp.cd->imaginary*rp.cd->imaginary)*d.d;
	    im.d = (lp.cd->imaginary*rp.cd->real
		    - lp.cd->real*rp.cd->imaginary)*d.d;
	    tp.cd->real = re.d;
	    tp.cd++->imaginary = im.d;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;      
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_idiv(void)
/* division with array operands; returns integer part of result,
   rounded toward minus infinity */
/* NOTE: no checking for division by zero! */
{
  div_t qr;

  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    qr = div(*lp.b++, *rp.b++);
	    if (qr.rem < 0)
	      qr.quot--;
	    *tp.b++ = qr.quot;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    qr = div(*lp.b++, *rp.w++);
	    if (qr.rem < 0)
	      qr.quot--;
	    *tp.w++ = qr.quot;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    qr = div(*lp.b++, *rp.l++);
	    if (qr.rem < 0)
	      qr.quot--;
	    *tp.l++ = qr.quot;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = floor(*lp.b++ / *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = floor(*lp.b++ / *rp.d++);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    qr = div(*lp.w++, *rp.b++);
	    if (qr.rem < 0)
	      qr.quot--;
	    *tp.w++ = qr.quot;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    qr = div(*lp.w++, *rp.w++);
	    if (qr.rem < 0)
	      qr.quot--;
	    *tp.w++ = qr.quot;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    qr = div(*lp.w++, *rp.l++);
	    if (qr.rem < 0)
	      qr.quot--;
	    *tp.l++ = qr.quot;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = floor(*lp.w++ / *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = floor(*lp.w++ / *rp.d++);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    qr = div(*lp.l++, *rp.b++);
	    if (qr.rem < 0)
	      qr.quot--;
	    *tp.l++ = qr.quot;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    qr = div(*lp.l++, *rp.w++);
	    if (qr.rem < 0)
	      qr.quot--;
	    *tp.l++ = qr.quot;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    qr = div(*lp.l++, *rp.l++);
	    if (qr.rem < 0)
	      qr.quot--;
	    *tp.l++ = qr.quot;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = floor(*lp.l++ / *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = floor(*lp.l++ / *rp.d++);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.f++ = floor(*lp.f++ / *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.f++ = floor(*lp.f++ / *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.f++ = floor(*lp.f++ / *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = floor(*lp.f++ / *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = floor(*lp.f++ / *rp.d++);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.d++ = floor(*lp.d++ / *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.d++ = floor(*lp.d++ / *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.d++ = floor(*lp.d++ / *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.d++ = floor(*lp.d++ / *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = floor(*lp.d++ / *rp.d++);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_idiv_as(void)
/* division with array LHS and scalar RHS; returns integer part of result,
   rounded toward minus infinity */
/* NOTE: no checking for division by zero! */
{
  div_t qr;

  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    qr = div(*lp.b++, *rp.b);
	    if (qr.rem < 0)
	      qr.quot--;
	    *tp.b++ = qr.quot;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    qr = div(*lp.b++, *rp.w);
	    if (qr.rem < 0)
	      qr.quot--;
	    *tp.w++ = qr.quot;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    qr = div(*lp.b++, *rp.l);
	    if (qr.rem < 0)
	      qr.quot--;
	    *tp.l++ = qr.quot;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = floor(*lp.b++ / *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = floor(*lp.b++ / *rp.d);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    qr = div(*lp.w++, *rp.b);
	    if (qr.rem < 0)
	      qr.quot--;
	    *tp.w++ = qr.quot;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    qr = div(*lp.w++, *rp.w);
	    if (qr.rem < 0)
	      qr.quot--;
	    *tp.w++ = qr.quot;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    qr = div(*lp.w++, *rp.l);
	    if (qr.rem < 0)
	      qr.quot--;
	    *tp.l++ = qr.quot;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = floor(*lp.w++ / *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = floor(*lp.w++ / *rp.d);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    qr = div(*lp.l++, *rp.b);
	    if (qr.rem < 0)
	      qr.quot--;
	    *tp.l++ = qr.quot;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    qr = div(*lp.l++, *rp.w);
	    if (qr.rem < 0)
	      qr.quot--;
	    *tp.l++ = qr.quot;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    qr = div(*lp.l++, *rp.l);
	    if (qr.rem < 0)
	      qr.quot--;
	    *tp.l++ = qr.quot;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = floor(*lp.l++ / *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = floor(*lp.l++ / *rp.d);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.f++ = floor(*lp.f++ / *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.f++ = floor(*lp.f++ / *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.f++ = floor(*lp.f++ / *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = floor(*lp.f++ / *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = floor(*lp.f++ / *rp.d);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.d++ = floor(*lp.d++ / *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.d++ = floor(*lp.d++ / *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.d++ = floor(*lp.d++ / *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.d++ = floor(*lp.d++ / *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = floor(*lp.d++ / *rp.d);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_idiv_sa(void)
/* division with scalar LHS and array RHS; returns integer part of result,
   rounded toward minus infinity */
/* NOTE: no checking for division by zero! */
{
  div_t qr;

  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    qr = div(*lp.b, *rp.b++);
	    if (qr.rem < 0)
	      qr.quot--;
	    *tp.b++ = qr.quot;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    qr = div(*lp.b, *rp.w++);
	    if (qr.rem < 0)
	      qr.quot--;
	    *tp.w++ = qr.quot;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    qr = div(*lp.b, *rp.l++);
	    if (qr.rem < 0)
	      qr.quot--;
	    *tp.l++ = qr.quot;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = floor(*lp.b / *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = floor(*lp.b / *rp.d++);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    qr = div(*lp.w, *rp.b++);
	    if (qr.rem < 0)
	      qr.quot--;
	    *tp.w++ = qr.quot;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    qr = div(*lp.w, *rp.w++);
	    if (qr.rem < 0)
	      qr.quot--;
	    *tp.w++ = qr.quot;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    qr = div(*lp.w, *rp.l++);
	    if (qr.rem < 0)
	      qr.quot--;
	    *tp.l++ = qr.quot;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = floor(*lp.w / *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = floor(*lp.w / *rp.d++);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    qr = div(*lp.l, *rp.b++);
	    if (qr.rem < 0)
	      qr.quot--;
	    *tp.l++ = qr.quot;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    qr = div(*lp.l, *rp.w++);
	    if (qr.rem < 0)
	      qr.quot--;
	    *tp.l++ = qr.quot;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    qr = div(*lp.l, *rp.l++);
	    if (qr.rem < 0)
	      qr.quot--;
	    *tp.l++ = qr.quot;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = floor(*lp.l / *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = floor(*lp.l / *rp.d++);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.f++ = floor(*lp.f / *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.f++ = floor(*lp.f / *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.f++ = floor(*lp.f / *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = floor(*lp.f / *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = floor(*lp.f / *rp.d++);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.d++ = floor(*lp.d / *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.d++ = floor(*lp.d / *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.d++ = floor(*lp.d / *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.d++ = floor(*lp.d / *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = floor(*lp.d / *rp.d++);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
/* returns z = x mod y such that 0 <= z < |y| */
int iamod(int x, int y)
{
  int v;

  if (!y)
    return 0;
  if (y < 0)
    y = -y;
  v = x % y;
  if (v < 0)
    v += y;
  return v;
}
/*----------------------------------------------------------*/
/* returns z = x mod y such that -|y|/2 < z <= |y|/2 */
int iasmod(int x, int y)
{
  int v;

  if (!y)
    return 0;
  if (y < 0)
    y = -y;
  v = x % y;
  if (v < 0)
    v += y;
  if (2*v > y)
    v -= y;
  return v;  
}
/*----------------------------------------------------------*/
/* returns z = x mod y such that 0 <= z < |y| */
double famod(double x, double y)
{
  double v;

  if (!y)
    return 0;
  if (y < 0)
    y = -y;
  v = fmod(x, y);
  if (v < 0)
    v += y;
  return v;
}
/*----------------------------------------------------------*/
/* returns z = x mod y such that -|y|/2 < z <= |y|/2 */
double fasmod(double x, double y)
{
  double v;

  if (!y)
    return 0;
  if (y < 0)
    y = -y;
  v = fmod(x, y);
  if (v < 0)
    v += y;
  if (v > y/2)
    v -= y;
  return v;
}
/*----------------------------------------------------------*/
doubleComplex zamod(doubleComplex x, doubleComplex y)
{
  /* we formally define the modulus z1 amod z2 as
     z1 - n*z2 where n is the greatest integer not smaller
     than the real number closest to z2 */
  double rx, ry, ax, ay, d;
  int n;
  doubleComplex z;

  ry = hypot(y.real, y.imaginary);
  if (!ry)
    z.real = z.imaginary = 0;
  else {
    ay = atan2(y.imaginary, y.real);
    rx = hypot(x.real, x.imaginary);
    ax = atan2(x.imaginary, x.real);
  
    d = rx/ry*cos(ax - ay);
    n = (int) d;
    if (d < 0)
      n--;
    z.real = x.real - n*y.real;
    z.imaginary = x.imaginary - n*y.imaginary;
  }
  return z;
}
/*----------------------------------------------------------*/
doubleComplex zasmod(doubleComplex x, doubleComplex y)
{
  return zamod(x, y);
}
/*----------------------------------------------------------*/
void ana_smod(void)
     /* remainder-taking with array operands */
{ 
  doubleComplex l, r, t;

  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.b++ = iasmod(*lp.b++, *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = iasmod(*lp.b++, *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = iasmod(*lp.b++, *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = fasmod(*lp.b++, *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = fasmod(*lp.b++, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    l.real = *lp.b;
	    l.imaginary = 0;
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.b++; rp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    l.real = *lp.b;
	    l.imaginary = 0;
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.b++; rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.w++ = iasmod(*lp.w++, *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = iasmod(*lp.w++, *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = iasmod(*lp.w++, *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = fasmod(*lp.w++, *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = fasmod(*lp.w++, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    l.real = *lp.w;
	    l.imaginary = 0;
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.w++; rp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    l.real = *lp.w;
	    l.imaginary = 0;
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.w++; rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = iasmod(*lp.l++, *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = iasmod(*lp.l++, *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = iasmod(*lp.l++, *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = fasmod(*lp.l++, *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = fasmod(*lp.l++, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    l.real = *lp.l;
	    l.imaginary = 0;
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.l++; rp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    l.real = *lp.l;
	    l.imaginary = 0;
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.l++; rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.f++ = fasmod(*lp.f++, *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.f++ = fasmod(*lp.f++, *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.f++ = fasmod(*lp.f++, *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = fasmod(*lp.f++, *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = fasmod(*lp.f++, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    l.real = *lp.f;
	    l.imaginary = 0;
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.f++; rp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    l.real = *lp.f;
	    l.imaginary = 0;
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.f++; rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.d++ = fasmod(*lp.d++, *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.d++ = fasmod(*lp.d++, *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.d++ = fasmod(*lp.d++, *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.d++ = fasmod(*lp.d++, *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = fasmod(*lp.d++, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    l.real = *lp.d;
	    l.imaginary = 0;
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.d++; rp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    l.real = *lp.d;
	    l.imaginary = 0;
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.d++; rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    r.real = *rp.b;
	    r.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.cf++; rp.b++; tp.cf++;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    r.real = *rp.w;
	    r.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.cf++; rp.w++; tp.cf++;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    r.real = *rp.l;
	    r.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.cf++; rp.l++; tp.cf++;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    r.real = *rp.f;
	    r.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.cf++; rp.f++; tp.cf++;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    r.real = *rp.d;
	    r.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.cf++; rp.d++; tp.cf++;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.cf++; rp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cf++; rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    r.real = *rp.b;
	    r.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; rp.b++; tp.cd++;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    r.real = *rp.w;
	    r.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; rp.w++; tp.cd++;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    r.real = *rp.l;
	    r.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; rp.l++; tp.cd++;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    r.real = *rp.f;
	    r.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; rp.f++; tp.cd++;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    r.real = *rp.d;
	    r.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; rp.d++; tp.cd++;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; rp.cf++; tp.cd++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_smod_as(void)
     /* remainder-taking with array LHS and scalar RHS */
{
  doubleComplex l, r, t;

  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.b++ = iasmod(*lp.b++, *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = iasmod(*lp.b++, *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = iasmod(*lp.b++, *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = fasmod(*lp.b++, *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = fasmod(*lp.b++, *rp.d);
	  break;
	case ANA_CFLOAT:
	  r.real = rp.cf->real;
	  r.imaginary = rp.cf->imaginary;
	  while (nRepeat--) {
	    l.real = *lp.b;
	    l.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.b++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  r.real = rp.cd->real;
	  r.imaginary = rp.cd->imaginary;
	  while (nRepeat--) {
	    l.real = *lp.b;
	    l.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.b++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.w++ = iasmod(*lp.w++, *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = iasmod(*lp.w++, *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = iasmod(*lp.w++, *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = fasmod(*lp.w++, *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = fasmod(*lp.w++, *rp.d);
	  break;
	case ANA_CFLOAT:
	  r.real = rp.cf->real;
	  r.imaginary = rp.cf->imaginary;
	  while (nRepeat--) {
	    l.real = *lp.w;
	    l.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.w++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  r.real = rp.cd->real;
	  r.imaginary = rp.cd->imaginary;
	  while (nRepeat--) {
	    l.real = *lp.w;
	    l.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.w++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = iasmod(*lp.l++, *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = iasmod(*lp.l++, *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = iasmod(*lp.l++, *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = fasmod(*lp.l++, *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = fasmod(*lp.l++, *rp.d);
	  break;
	case ANA_CFLOAT:
	  r.real = rp.cf->real;
	  r.imaginary = rp.cf->imaginary;
	  while (nRepeat--) {
	    l.real = *lp.l;
	    l.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.l++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  r.real = rp.cd->real;
	  r.imaginary = rp.cd->imaginary;
	  while (nRepeat--) {
	    l.real = *lp.l;
	    l.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.l++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.f++ = fasmod(*lp.f++, *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.f++ = fasmod(*lp.f++, *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.f++ = fasmod(*lp.f++, *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = fasmod(*lp.f++, *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = fasmod(*lp.f++, *rp.d);
	  break;
	case ANA_CFLOAT:
	  r.real = rp.cf->real;
	  r.imaginary = rp.cf->imaginary;
	  while (nRepeat--) {
	    l.real = *lp.f;
	    l.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.f++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  r.real = rp.cd->real;
	  r.imaginary = rp.cd->imaginary;
	  while (nRepeat--) {
	    l.real = *lp.f;
	    l.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.f++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.d++ = fasmod(*lp.d++, *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.d++ = fasmod(*lp.d++, *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.d++ = fasmod(*lp.d++, *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.d++ = fasmod(*lp.d++, *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = fasmod(*lp.d++, *rp.d);
	  break;
	case ANA_CFLOAT:
	  r.real = rp.cf->real;
	  r.imaginary = rp.cf->imaginary;
	  while (nRepeat--) {
	    l.real = *lp.d;
	    l.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.d++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  r.real = rp.cd->real;
	  r.imaginary = rp.cd->imaginary;
	  while (nRepeat--) {
	    l.real = *lp.d;
	    l.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.d++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  r.real = *rp.b;
	  r.imaginary = 0;
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.cf++; tp.cf++;
	  }
	  break;
	case ANA_WORD:
	  r.real = *rp.w;
	  r.imaginary = 0;
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.cf++; tp.cf++;
	  }
	  break;
	case ANA_LONG:
	  r.real = *rp.l;
	  r.imaginary = 0;
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.cf++; tp.cf++;
	  }
	  break;
	case ANA_FLOAT:
	  r.real = *rp.f;
	  r.imaginary = 0;
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.cf++; tp.cf++;
	  }
	  break;
	case ANA_DOUBLE:
	  r.real = *rp.d;
	  r.imaginary = 0;
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CFLOAT:
	  r.real = rp.cf->real;
	  r.imaginary = rp.cf->imaginary;
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  r.real = rp.cd->real;
	  r.imaginary = rp.cd->imaginary;
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cf++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  r.real = *rp.b;
	  r.imaginary = 0;
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; tp.cd++;
	  }
	  break;
	case ANA_WORD:
	  r.real = *rp.w;
	  r.imaginary = 0;
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; tp.cd++;
	  }
	  break;
	case ANA_LONG:
	  r.real = *rp.l;
	  r.imaginary = 0;
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; tp.cd++;
	  }
	  break;
	case ANA_FLOAT:
	  r.real = *rp.f;
	  r.imaginary = 0;
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; tp.cd++;
	  }
	  break;
	case ANA_DOUBLE:
	  r.real = *rp.d;
	  r.imaginary = 0;
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; tp.cd++;
	  }
	  break;
	case ANA_CFLOAT:
	  r.real = rp.cf->real;
	  r.imaginary = rp.cf->imaginary;
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; tp.cd++;
	  }
	  break;
	case ANA_CDOUBLE:
	  r.real = rp.cd->real;
	  r.imaginary = rp.cd->imaginary;
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_smod_sa(void)
     /* remainder-taking with scalar LHS and array RHS */
{
  doubleComplex l, r, t;

  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.b++ = iasmod(*lp.b, *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = iasmod(*lp.b, *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = iasmod(*lp.b, *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = fasmod(*lp.b, *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = fasmod(*lp.b, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  l.real = *lp.b;
	  l.imaginary = 0;
	  while (nRepeat--) {
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    rp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  l.real = *lp.b;
	  l.imaginary = 0;
	  while (nRepeat--) {
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.w++ = iasmod(*lp.w, *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = iasmod(*lp.w, *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = iasmod(*lp.w, *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = fasmod(*lp.w, *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = fasmod(*lp.w, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  l.real = *lp.w;
	  l.imaginary = 0;
	  while (nRepeat--) {
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    rp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  l.real = *lp.w;
	  l.imaginary = 0;
	  while (nRepeat--) {
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = iasmod(*lp.l, *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = iasmod(*lp.l, *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = iasmod(*lp.l, *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = fasmod(*lp.l, *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = fasmod(*lp.l, *rp.d++);
	  break;
	case ANA_CFLOAT:
	    l.real = *lp.l;
	    l.imaginary = 0;
	  while (nRepeat--) {
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    rp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  l.real = *lp.l;
	  l.imaginary = 0;
	  while (nRepeat--) {
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.f++ = fasmod(*lp.f, *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.f++ = fasmod(*lp.f, *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.f++ = fasmod(*lp.f, *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = fasmod(*lp.f, *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = fasmod(*lp.f, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  l.real = *lp.f;
	  l.imaginary = 0;
	  while (nRepeat--) {
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    rp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  l.real = *lp.f;
	  l.imaginary = 0;
	  while (nRepeat--) {
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.d++ = fasmod(*lp.d, *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.d++ = fasmod(*lp.d, *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.d++ = fasmod(*lp.d, *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.d++ = fasmod(*lp.d, *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = fasmod(*lp.d, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  l.real = *lp.d;
	  l.imaginary = 0;
	  while (nRepeat--) {
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    rp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  l.real = *lp.d;
	  l.imaginary = 0;
	  while (nRepeat--) {
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  l.real = lp.cf->real;
	  l.imaginary = lp.cf->imaginary;
	  while (nRepeat--) {
	    r.real = *rp.b;
	    r.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    rp.b++; tp.cf++;
	  }
	  break;
	case ANA_WORD:
	  l.real = lp.cf->real;
	  l.imaginary = lp.cf->imaginary;
	  while (nRepeat--) {
	    r.real = *rp.w;
	    r.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    rp.w++; tp.cf++;
	  }
	  break;
	case ANA_LONG:
	  l.real = lp.cf->real;
	  l.imaginary = lp.cf->imaginary;
	  while (nRepeat--) {
	    r.real = *rp.l;
	    r.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    rp.l++; tp.cf++;
	  }
	  break;
	case ANA_FLOAT:
	  l.real = lp.cf->real;
	  l.imaginary = lp.cf->imaginary;
	  while (nRepeat--) {
	    r.real = *rp.f;
	    r.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    rp.f++; tp.cf++;
	  }
	  break;
	case ANA_DOUBLE:
	  l.real = lp.cf->real;
	  l.imaginary = lp.cf->imaginary;
	  while (nRepeat--) {
	    r.real = *rp.d;
	    r.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    rp.d++; tp.cf++;
	  }
	  break;
	case ANA_CFLOAT:
	  l.real = lp.cf->real;
	  l.imaginary = lp.cf->imaginary;
	  while (nRepeat--) {
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zasmod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    rp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  l.real = lp.cf->real;
	  l.imaginary = lp.cf->imaginary;
	  while (nRepeat--) {
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  l.real = lp.cd->real;
	  l.imaginary = lp.cd->imaginary;
	  while (nRepeat--) {
	    r.real = *rp.b;
	    r.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    rp.b++; tp.cd++;
	  }
	  break;
	case ANA_WORD:
	  l.real = lp.cd->real;
	  l.imaginary = lp.cd->imaginary;
	  while (nRepeat--) {
	    r.real = *rp.w;
	    r.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    rp.w++; tp.cd++;
	  }
	  break;
	case ANA_LONG:
	  l.real = lp.cd->real;
	  l.imaginary = lp.cd->imaginary;
	  while (nRepeat--) {
	    r.real = *rp.l;
	    r.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    rp.l++; tp.cd++;
	  }
	  break;
	case ANA_FLOAT:
	  l.real = lp.cd->real;
	  l.imaginary = lp.cd->imaginary;
	  while (nRepeat--) {
	    r.real = *rp.f;
	    r.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    rp.f++; tp.cd++;
	  }
	  break;
	case ANA_DOUBLE:
	  l.real = lp.cd->real;
	  l.imaginary = lp.cd->imaginary;
	  while (nRepeat--) {
	    r.real = *rp.d;
	    r.imaginary = 0;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    rp.d++; tp.cd++;
	  }
	  break;
	case ANA_CFLOAT:
	  l.real = lp.cd->real;
	  l.imaginary = lp.cd->imaginary;
	  while (nRepeat--) {
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    rp.cf++; tp.cd++;
	  }
	  break;
	case ANA_CDOUBLE:
	  l.real = lp.cd->real;
	  l.imaginary = lp.cd->imaginary;
	  while (nRepeat--) {
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zasmod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_mod(void)
     /* remainder-taking with array operands */
{ 
  doubleComplex l, r, t;

  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.b++ = iamod(*lp.b++, *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = iamod(*lp.b++, *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = iamod(*lp.b++, *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = famod(*lp.b++, *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = famod(*lp.b++, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    l.real = *lp.b;
	    l.imaginary = 0;
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.b++; rp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    l.real = *lp.b;
	    l.imaginary = 0;
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.b++; rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.w++ = iamod(*lp.w++, *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = iamod(*lp.w++, *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = iamod(*lp.w++, *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = famod(*lp.w++, *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = famod(*lp.w++, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    l.real = *lp.w;
	    l.imaginary = 0;
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.w++; rp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    l.real = *lp.w;
	    l.imaginary = 0;
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.w++; rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = iamod(*lp.l++, *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = iamod(*lp.l++, *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = iamod(*lp.l++, *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = famod(*lp.l++, *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = famod(*lp.l++, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    l.real = *lp.l;
	    l.imaginary = 0;
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.l++; rp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    l.real = *lp.l;
	    l.imaginary = 0;
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.l++; rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.f++ = famod(*lp.f++, *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.f++ = famod(*lp.f++, *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.f++ = famod(*lp.f++, *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = famod(*lp.f++, *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = famod(*lp.f++, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    l.real = *lp.f;
	    l.imaginary = 0;
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.f++; rp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    l.real = *lp.f;
	    l.imaginary = 0;
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.f++; rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.d++ = famod(*lp.d++, *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.d++ = famod(*lp.d++, *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.d++ = famod(*lp.d++, *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.d++ = famod(*lp.d++, *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = famod(*lp.d++, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    l.real = *lp.d;
	    l.imaginary = 0;
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.d++; rp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    l.real = *lp.d;
	    l.imaginary = 0;
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.d++; rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    r.real = *rp.b;
	    r.imaginary = 0;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.cf++; rp.b++; tp.cf++;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    r.real = *rp.w;
	    r.imaginary = 0;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.cf++; rp.w++; tp.cf++;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    r.real = *rp.l;
	    r.imaginary = 0;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.cf++; rp.l++; tp.cf++;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    r.real = *rp.f;
	    r.imaginary = 0;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.cf++; rp.f++; tp.cf++;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    r.real = *rp.d;
	    r.imaginary = 0;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.cf++; rp.d++; tp.cf++;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.cf++; rp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cf++; rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    r.real = *rp.b;
	    r.imaginary = 0;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; rp.b++; tp.cd++;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    r.real = *rp.w;
	    r.imaginary = 0;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; rp.w++; tp.cd++;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    r.real = *rp.l;
	    r.imaginary = 0;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; rp.l++; tp.cd++;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    r.real = *rp.f;
	    r.imaginary = 0;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; rp.f++; tp.cd++;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    r.real = *rp.d;
	    r.imaginary = 0;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; rp.d++; tp.cd++;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; rp.cf++; tp.cd++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_mod_as(void)
     /* remainder-taking with array LHS and scalar RHS */
{
  doubleComplex l, r, t;

  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.b++ = iamod(*lp.b++, *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = iamod(*lp.b++, *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = iamod(*lp.b++, *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = famod(*lp.b++, *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = famod(*lp.b++, *rp.d);
	  break;
	case ANA_CFLOAT:
	  r.real = rp.cf->real;
	  r.imaginary = rp.cf->imaginary;
	  while (nRepeat--) {
	    l.real = *lp.b;
	    l.imaginary = 0;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.b++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  r.real = rp.cd->real;
	  r.imaginary = rp.cd->imaginary;
	  while (nRepeat--) {
	    l.real = *lp.b;
	    l.imaginary = 0;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.b++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.w++ = iamod(*lp.w++, *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = iamod(*lp.w++, *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = iamod(*lp.w++, *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = famod(*lp.w++, *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = famod(*lp.w++, *rp.d);
	  break;
	case ANA_CFLOAT:
	  r.real = rp.cf->real;
	  r.imaginary = rp.cf->imaginary;
	  while (nRepeat--) {
	    l.real = *lp.w;
	    l.imaginary = 0;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.w++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  r.real = rp.cd->real;
	  r.imaginary = rp.cd->imaginary;
	  while (nRepeat--) {
	    l.real = *lp.w;
	    l.imaginary = 0;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.w++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = iamod(*lp.l++, *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = iamod(*lp.l++, *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = iamod(*lp.l++, *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = famod(*lp.l++, *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = famod(*lp.l++, *rp.d);
	  break;
	case ANA_CFLOAT:
	  r.real = rp.cf->real;
	  r.imaginary = rp.cf->imaginary;
	  while (nRepeat--) {
	    l.real = *lp.l;
	    l.imaginary = 0;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.l++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  r.real = rp.cd->real;
	  r.imaginary = rp.cd->imaginary;
	  while (nRepeat--) {
	    l.real = *lp.l;
	    l.imaginary = 0;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.l++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.f++ = famod(*lp.f++, *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.f++ = famod(*lp.f++, *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.f++ = famod(*lp.f++, *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = famod(*lp.f++, *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = famod(*lp.f++, *rp.d);
	  break;
	case ANA_CFLOAT:
	  r.real = rp.cf->real;
	  r.imaginary = rp.cf->imaginary;
	  while (nRepeat--) {
	    l.real = *lp.f;
	    l.imaginary = 0;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.f++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  r.real = rp.cd->real;
	  r.imaginary = rp.cd->imaginary;
	  while (nRepeat--) {
	    l.real = *lp.f;
	    l.imaginary = 0;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.f++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.d++ = famod(*lp.d++, *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.d++ = famod(*lp.d++, *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.d++ = famod(*lp.d++, *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.d++ = famod(*lp.d++, *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = famod(*lp.d++, *rp.d);
	  break;
	case ANA_CFLOAT:
	  r.real = rp.cf->real;
	  r.imaginary = rp.cf->imaginary;
	  while (nRepeat--) {
	    l.real = *lp.d;
	    l.imaginary = 0;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.d++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  r.real = rp.cd->real;
	  r.imaginary = rp.cd->imaginary;
	  while (nRepeat--) {
	    l.real = *lp.d;
	    l.imaginary = 0;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.d++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  r.real = *rp.b;
	  r.imaginary = 0;
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.cf++; tp.cf++;
	  }
	  break;
	case ANA_WORD:
	  r.real = *rp.w;
	  r.imaginary = 0;
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.cf++; tp.cf++;
	  }
	  break;
	case ANA_LONG:
	  r.real = *rp.l;
	  r.imaginary = 0;
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.cf++; tp.cf++;
	  }
	  break;
	case ANA_FLOAT:
	  r.real = *rp.f;
	  r.imaginary = 0;
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.cf++; tp.cf++;
	  }
	  break;
	case ANA_DOUBLE:
	  r.real = *rp.d;
	  r.imaginary = 0;
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CFLOAT:
	  r.real = rp.cf->real;
	  r.imaginary = rp.cf->imaginary;
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    lp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  r.real = rp.cd->real;
	  r.imaginary = rp.cd->imaginary;
	  while (nRepeat--) {
	    l.real = lp.cf->real;
	    l.imaginary = lp.cf->imaginary;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cf++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  r.real = *rp.b;
	  r.imaginary = 0;
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; tp.cd++;
	  }
	  break;
	case ANA_WORD:
	  r.real = *rp.w;
	  r.imaginary = 0;
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; tp.cd++;
	  }
	  break;
	case ANA_LONG:
	  r.real = *rp.l;
	  r.imaginary = 0;
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; tp.cd++;
	  }
	  break;
	case ANA_FLOAT:
	  r.real = *rp.f;
	  r.imaginary = 0;
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; tp.cd++;
	  }
	  break;
	case ANA_DOUBLE:
	  r.real = *rp.d;
	  r.imaginary = 0;
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; tp.cd++;
	  }
	  break;
	case ANA_CFLOAT:
	  r.real = rp.cf->real;
	  r.imaginary = rp.cf->imaginary;
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; tp.cd++;
	  }
	  break;
	case ANA_CDOUBLE:
	  r.real = rp.cd->real;
	  r.imaginary = rp.cd->imaginary;
	  while (nRepeat--) {
	    l.real = lp.cd->real;
	    l.imaginary = lp.cd->imaginary;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    lp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_mod_sa(void)
     /* remainder-taking with scalar LHS and array RHS */
{
  doubleComplex l, r, t;

  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.b++ = iamod(*lp.b, *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = iamod(*lp.b, *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = iamod(*lp.b, *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = famod(*lp.b, *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = famod(*lp.b, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  l.real = *lp.b;
	  l.imaginary = 0;
	  while (nRepeat--) {
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    rp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  l.real = *lp.b;
	  l.imaginary = 0;
	  while (nRepeat--) {
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.w++ = iamod(*lp.w, *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = iamod(*lp.w, *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = iamod(*lp.w, *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = famod(*lp.w, *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = famod(*lp.w, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  l.real = *lp.w;
	  l.imaginary = 0;
	  while (nRepeat--) {
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    rp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  l.real = *lp.w;
	  l.imaginary = 0;
	  while (nRepeat--) {
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = iamod(*lp.l, *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = iamod(*lp.l, *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = iamod(*lp.l, *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = famod(*lp.l, *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = famod(*lp.l, *rp.d++);
	  break;
	case ANA_CFLOAT:
	    l.real = *lp.l;
	    l.imaginary = 0;
	  while (nRepeat--) {
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    rp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  l.real = *lp.l;
	  l.imaginary = 0;
	  while (nRepeat--) {
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.f++ = famod(*lp.f, *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.f++ = famod(*lp.f, *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.f++ = famod(*lp.f, *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = famod(*lp.f, *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = famod(*lp.f, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  l.real = *lp.f;
	  l.imaginary = 0;
	  while (nRepeat--) {
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    rp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  l.real = *lp.f;
	  l.imaginary = 0;
	  while (nRepeat--) {
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.d++ = famod(*lp.d, *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.d++ = famod(*lp.d, *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.d++ = famod(*lp.d, *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.d++ = famod(*lp.d, *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = famod(*lp.d, *rp.d++);
	  break;
	case ANA_CFLOAT:
	  l.real = *lp.d;
	  l.imaginary = 0;
	  while (nRepeat--) {
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    rp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  l.real = *lp.d;
	  l.imaginary = 0;
	  while (nRepeat--) {
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  l.real = lp.cf->real;
	  l.imaginary = lp.cf->imaginary;
	  while (nRepeat--) {
	    r.real = *rp.b;
	    r.imaginary = 0;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    rp.b++; tp.cf++;
	  }
	  break;
	case ANA_WORD:
	  l.real = lp.cf->real;
	  l.imaginary = lp.cf->imaginary;
	  while (nRepeat--) {
	    r.real = *rp.w;
	    r.imaginary = 0;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    rp.w++; tp.cf++;
	  }
	  break;
	case ANA_LONG:
	  l.real = lp.cf->real;
	  l.imaginary = lp.cf->imaginary;
	  while (nRepeat--) {
	    r.real = *rp.l;
	    r.imaginary = 0;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    rp.l++; tp.cf++;
	  }
	  break;
	case ANA_FLOAT:
	  l.real = lp.cf->real;
	  l.imaginary = lp.cf->imaginary;
	  while (nRepeat--) {
	    r.real = *rp.f;
	    r.imaginary = 0;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    rp.f++; tp.cf++;
	  }
	  break;
	case ANA_DOUBLE:
	  l.real = lp.cf->real;
	  l.imaginary = lp.cf->imaginary;
	  while (nRepeat--) {
	    r.real = *rp.d;
	    r.imaginary = 0;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    rp.d++; tp.cf++;
	  }
	  break;
	case ANA_CFLOAT:
	  l.real = lp.cf->real;
	  l.imaginary = lp.cf->imaginary;
	  while (nRepeat--) {
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zamod(l, r);
	    tp.cf->real = t.real;
	    tp.cf->imaginary = t.imaginary;
	    rp.cf++; tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  l.real = lp.cf->real;
	  l.imaginary = lp.cf->imaginary;
	  while (nRepeat--) {
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  l.real = lp.cd->real;
	  l.imaginary = lp.cd->imaginary;
	  while (nRepeat--) {
	    r.real = *rp.b;
	    r.imaginary = 0;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    rp.b++; tp.cd++;
	  }
	  break;
	case ANA_WORD:
	  l.real = lp.cd->real;
	  l.imaginary = lp.cd->imaginary;
	  while (nRepeat--) {
	    r.real = *rp.w;
	    r.imaginary = 0;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    rp.w++; tp.cd++;
	  }
	  break;
	case ANA_LONG:
	  l.real = lp.cd->real;
	  l.imaginary = lp.cd->imaginary;
	  while (nRepeat--) {
	    r.real = *rp.l;
	    r.imaginary = 0;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    rp.l++; tp.cd++;
	  }
	  break;
	case ANA_FLOAT:
	  l.real = lp.cd->real;
	  l.imaginary = lp.cd->imaginary;
	  while (nRepeat--) {
	    r.real = *rp.f;
	    r.imaginary = 0;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    rp.f++; tp.cd++;
	  }
	  break;
	case ANA_DOUBLE:
	  l.real = lp.cd->real;
	  l.imaginary = lp.cd->imaginary;
	  while (nRepeat--) {
	    r.real = *rp.d;
	    r.imaginary = 0;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    rp.d++; tp.cd++;
	  }
	  break;
	case ANA_CFLOAT:
	  l.real = lp.cd->real;
	  l.imaginary = lp.cd->imaginary;
	  while (nRepeat--) {
	    r.real = rp.cf->real;
	    r.imaginary = rp.cf->imaginary;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    rp.cf++; tp.cd++;
	  }
	  break;
	case ANA_CDOUBLE:
	  l.real = lp.cd->real;
	  l.imaginary = lp.cd->imaginary;
	  while (nRepeat--) {
	    r.real = rp.cd->real;
	    r.imaginary = rp.cd->imaginary;
	    t = zamod(l, r);
	    tp.cd->real = t.real;
	    tp.cd->imaginary = t.imaginary;
	    rp.cd++; tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_max(void)
     /* largest-taking with array operands */
{
  scalar	value1, value2;
  
  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.b++ = ((value1.b = *lp.b++) > (value2.b = *rp.b++))?
	      value1.b:
	      value2.b;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = ((value1.w = (word) *lp.b++) > (value2.w = *rp.w++))?
	      value1.w:
	      value2.w;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((value1.l = (int) *lp.b++) > (value2.l = *rp.l++))?
	      value1.l:
	      value2.l;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = ((value1.f = (float) *lp.b++) > (value2.f = *rp.f++))?
	      value1.f:
	      value2.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = ((value1.d = (double) *lp.b++) > (value2.d = *rp.d++))?
	      value1.d:
	      value2.d;
	  break;
	case ANA_CFLOAT:
	  /* if complex numbers are involved, then we compare the absolute
	     values.  LS 8dec98 */
	  while (nRepeat--) {
	    if (*lp.b**lp.b > rp.cf->real*rp.cf->real
		+ rp.cf->imaginary*rp.cf->imaginary) {
	      tp.cf->real = *lp.b;
	      tp.cf->imaginary = 0;
	    } else
	      memcpy(tp.cf, rp.cf, sizeof(floatComplex));
	    tp.cf++;
	    lp.b++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    if (*lp.b**lp.b > rp.cd->real*rp.cd->real
		+ rp.cd->imaginary*rp.cd->imaginary) {
	      tp.cd->real = *lp.b;
	      tp.cd->imaginary = 0;
	    } else
	      memcpy(tp.cd, rp.cd, sizeof(doubleComplex));
	    tp.cd++;
	    lp.b++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.w++ = ((value1.w = *lp.w++) > (value2.w = (word) *rp.b++))?
	      value1.w:
	      value2.w;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = ((value1.w = *lp.w++) > (value2.w = *rp.w++))?
	      value1.w:
	      value2.w;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((value1.l = (int) *lp.w++) > (value2.l = *rp.l++))?
	      value1.l:
	      value2.l;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = ((value1.f = (float) *lp.w++) > (value2.f = *rp.f++))?
	      value1.f:
	      value2.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = ((value1.d = (double) *lp.w++) > (value2.d = *rp.d++))?
	      value1.d:
	      value2.d;
	  break;
	case ANA_CFLOAT:
	  /* if complex numbers are involved, then we compare the absolute
	     values.  LS 8dec98 */
	  while (nRepeat--) {
	    if (*lp.w**lp.w > rp.cf->real*rp.cf->real
		+ rp.cf->imaginary*rp.cf->imaginary) {
	      tp.cf->real = *lp.w;
	      tp.cf->imaginary = 0;
	    } else
	      memcpy(tp.cf, rp.cf, sizeof(floatComplex));
	    tp.cf++;
	    lp.w++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    if (*lp.w**lp.w > rp.cd->real*rp.cd->real
		+ rp.cd->imaginary*rp.cd->imaginary) {
	      tp.cd->real = *lp.w;
	      tp.cd->imaginary = 0;
	    } else
	      memcpy(tp.cd, rp.cd, sizeof(doubleComplex));
	    tp.cd++;
	    lp.w++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = ((value1.l = *lp.l++) > (value2.l = (int) *rp.b++))?
	      value1.l:
	      value2.l;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = ((value1.l = *lp.l++) > (value2.l = (int) *rp.w++))?
	      value1.l:
	      value2.l;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((value1.l = *lp.l++) > (value2.l = *rp.l++))?
	      value1.l:
	      value2.l;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = ((value1.f = (float) *lp.l++) > (value2.f = *rp.f++))?
	      value1.f:
	      value2.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = ((value1.d = (double) *lp.l++) > (value2.d = *rp.d++))?
	      value1.d:
	      value2.d;
	  break;
	case ANA_CFLOAT:
	  /* if complex numbers are involved, then we compare the absolute
	     values.  LS 8dec98 */
	  while (nRepeat--) {
	    if (*lp.l**lp.l > rp.cf->real*rp.cf->real
		+ rp.cf->imaginary*rp.cf->imaginary) {
	      tp.cf->real = *lp.l;
	      tp.cf->imaginary = 0;
	    } else
	      memcpy(tp.cf, rp.cf, sizeof(floatComplex));
	    tp.cf++;
	    lp.l++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    if (*lp.l**lp.l > rp.cd->real*rp.cd->real
		+ rp.cd->imaginary*rp.cd->imaginary) {
	      tp.cd->real = *lp.l;
	      tp.cd->imaginary = 0;
	    } else
	      memcpy(tp.cd, rp.cd, sizeof(doubleComplex));
	    tp.cd++;
	    lp.l++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.f++ = ((value1.f = *lp.f++) > (value2.f = (float) *rp.b++))?
	      value1.f:
	      value2.f;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.f++ = ((value1.f = *lp.f++) > (value2.f = (float) *rp.w++))?
	      value1.f:
	      value2.f;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.f++ = ((value1.f = *lp.f++) > (value2.f = (float) *rp.l++))?
	      value1.f:
	      value2.f;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = ((value1.f = *lp.f++) > (value2.f = *rp.f++))?
	      value1.f:
	      value2.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = ((value1.d = (double) *lp.f++) > (value2.d = *rp.d++))?
	      value1.d:
	      value2.d;
	  break;
	case ANA_CFLOAT:
	  /* if complex numbers are involved, then we compare the absolute
	     values.  LS 8dec98 */
	  while (nRepeat--) {
	    if (*lp.f**lp.f > rp.cf->real*rp.cf->real
		+ rp.cf->imaginary*rp.cf->imaginary) {
	      tp.cf->real = *lp.f;
	      tp.cf->imaginary = 0;
	    } else
	      memcpy(tp.cf, rp.cf, sizeof(floatComplex));
	    tp.cf++;
	    lp.f++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    if (*lp.f**lp.f > rp.cd->real*rp.cd->real
		+ rp.cd->imaginary*rp.cd->imaginary) {
	      tp.cd->real = *lp.f;
	      tp.cd->imaginary = 0;
	    } else
	      memcpy(tp.cd, rp.cd, sizeof(doubleComplex));
	    tp.cd++;
	    lp.f++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.d++ = ((value1.d = *lp.d++) > (value2.d = (double) *rp.b++))?
	      value1.d:
	      value2.d;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.d++ = ((value1.d = *lp.d++) > (value2.d = (double) *rp.w++))?
	      value1.d:
	      value2.d;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.d++ = ((value1.d = *lp.d++) > (value2.d = (double) *rp.l++))?
	      value1.d:
	      value2.d;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.d++ = ((value1.d = *lp.d++) > (value2.d = (double) *rp.f++))?
	      value1.d:
	      value2.d;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = ((value1.d = *lp.d++) > (value2.d = *rp.d++))?
	      value1.d:
	      value2.d;
	  break;
	case ANA_CFLOAT:
	  /* if complex numbers are involved, then we compare the absolute
	     values.  LS 8dec98 */
	  while (nRepeat--) {
	    if (*lp.d**lp.d > rp.cf->real*rp.cf->real
		+ rp.cf->imaginary*rp.cf->imaginary) {
	      tp.cf->real = *lp.d;
	      tp.cf->imaginary = 0;
	    } else
	      memcpy(tp.cf, rp.cf, sizeof(floatComplex));
	    tp.cf++;
	    lp.d++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    if (*lp.d**lp.d > rp.cd->real*rp.cd->real
		+ rp.cd->imaginary*rp.cd->imaginary) {
	      tp.cd->real = *lp.d;
	      tp.cd->imaginary = 0;
	    } else
	      memcpy(tp.cd, rp.cd, sizeof(doubleComplex));
	    tp.cd++;
	    lp.d++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      /* if complex numbers are involved, then we compare absolute values */
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		> *rp.b**rp.b)
	      memcpy(tp.cf, lp.cf, sizeof(floatComplex));
	    else {
	      tp.cf->real = *rp.b;
	      tp.cf->imaginary = 0;
	    }
	    tp.cf++;
	    lp.cf++;
	    rp.b++;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		> *rp.w**rp.w)
	      memcpy(tp.cf, lp.cf, sizeof(floatComplex));
	    else {
	      tp.cf->real = *rp.w;
	      tp.cf->imaginary = 0;
	    }
	    tp.cf++;
	    lp.cf++;
	    rp.w++;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		> *rp.l**rp.l)
	      memcpy(tp.cf, lp.cf, sizeof(floatComplex));
	    else {
	      tp.cf->real = *rp.l;
	      tp.cf->imaginary = 0;
	    }
	    tp.cf++;
	    lp.cf++;
	    rp.l++;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		> *rp.f**rp.f)
	      memcpy(tp.cf, lp.cf, sizeof(floatComplex));
	    else {
	      tp.cf->real = *rp.f;
	      tp.cf->imaginary = 0;
	    }
	    tp.cf++;
	    lp.cf++;
	    rp.f++;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		> *rp.d**rp.d)
	      memcpy(tp.cf, lp.cf, sizeof(floatComplex));
	    else {
	      tp.cf->real = *rp.d;
	      tp.cf->imaginary = 0;
	    }
	    tp.cf++;
	    lp.cf++;
	    rp.d++;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		> rp.cf->real*rp.cf->real + rp.cf->imaginary*rp.cf->imaginary){
	      tp.cf->real = lp.cf->real;
	      tp.cf->imaginary = lp.cf->imaginary;
	    } else {
	      tp.cf->real = rp.cf->real;
	      tp.cf->imaginary = rp.cf->imaginary;
	    }
	    tp.cf++;
	    lp.cf++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		> rp.cd->real*rp.cd->real + rp.cd->imaginary*rp.cd->imaginary){
	      tp.cd->real = lp.cf->real;
	      tp.cd->imaginary = lp.cf->imaginary;
	    } else {
	      tp.cd->real = rp.cd->real;
	      tp.cd->imaginary = rp.cd->imaginary;
	    }
	    tp.cd++;
	    lp.cf++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      /* if complex numbers are involved, then we compare absolute values */
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		> *rp.b**rp.b)
	      memcpy(tp.cf, lp.cd, sizeof(floatComplex));
	    else {
	      tp.cf->real = *rp.b;
	      tp.cf->imaginary = 0;
	    }
	    tp.cf++;
	    lp.cd++;
	    rp.b++;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		> *rp.w**rp.w)
	      memcpy(tp.cf, lp.cd, sizeof(floatComplex));
	    else {
	      tp.cf->real = *rp.w;
	      tp.cf->imaginary = 0;
	    }
	    tp.cf++;
	    lp.cd++;
	    rp.w++;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		> *rp.l**rp.l)
	      memcpy(tp.cf, lp.cd, sizeof(floatComplex));
	    else {
	      tp.cf->real = *rp.l;
	      tp.cf->imaginary = 0;
	    }
	    tp.cf++;
	    lp.cd++;
	    rp.l++;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		> *rp.f**rp.f)
	      memcpy(tp.cf, lp.cd, sizeof(floatComplex));
	    else {
	      tp.cf->real = *rp.f;
	      tp.cf->imaginary = 0;
	    }
	    tp.cf++;
	    lp.cd++;
	    rp.f++;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		> *rp.d**rp.d)
	      memcpy(tp.cf, lp.cd, sizeof(floatComplex));
	    else {
	      tp.cf->real = *rp.d;
	      tp.cf->imaginary = 0;
	    }
	    tp.cf++;
	    lp.cd++;
	    rp.d++;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		> rp.cf->real*rp.cf->real + rp.cf->imaginary*rp.cf->imaginary){
	      tp.cd->real = lp.cd->real;
	      tp.cd->imaginary = lp.cd->imaginary;
	    } else {
	      tp.cd->real = rp.cf->real;
	      tp.cd->imaginary = rp.cf->imaginary;
	    }
	    tp.cd++;
	    lp.cd++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		> rp.cd->real*rp.cd->real + rp.cd->imaginary*rp.cd->imaginary){
	      tp.cd->real = lp.cd->real;
	      tp.cd->imaginary = lp.cd->imaginary;
	    } else {
	      tp.cd->real = rp.cd->real;
	      tp.cd->imaginary = rp.cd->imaginary;
	    }
	    tp.cd++;
	    lp.cd++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
      break;
  }
}
/*----------------------------------------------------------*/
void ana_max_as(void)
     /* largest-taking with array LHS and scalar RHS */
{
  scalar	value1, value2;
  
  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  value2.b = *rp.b;
	  while (nRepeat--)
	    *tp.b++ = ((value1.b = *lp.b++) > value2.b)?
	      value1.b:
		value2.b;
	  break;
	case ANA_WORD:
	  value2.w = *rp.w;
	  while (nRepeat--)
	    *tp.w++ = ((value1.w = *lp.b++) > value2.w)?
	      value1.w:
		value2.w;
	  break;
	case ANA_LONG:
	  value2.l = *rp.l;
	  while (nRepeat--)
	    *tp.l++ = ((value1.l = *lp.b++) > value2.l)?
	      value1.l:
		value2.l;
	  break;
	case ANA_FLOAT:
	  value2.f = *rp.f;
	  while (nRepeat--)
	    *tp.f++ = ((value1.f = *lp.b++) > value2.f)?
	      value1.f:
		value2.f;
	  break;
	case ANA_DOUBLE:
	  value2.d = *rp.d;
	  while (nRepeat--)
	    *tp.d++ = ((value1.d = *lp.b++) > value2.d)?
	      value1.d:
		value2.d;
	  break;
	case ANA_CFLOAT:
	  value2.d = rp.cf->real*rp.cf->real
	    + rp.cf->imaginary*rp.cf->imaginary;
	  while (nRepeat--) {
	    if (*lp.b**lp.b > value2.d) {
	      tp.cf->real = *lp.b;
	      tp.cf->imaginary = 0;
	    } else {
	      tp.cf->real = rp.cf->real;
	      tp.cf->imaginary = rp.cf->imaginary;
	    }
	    lp.b++;
	    tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value2.d = rp.cd->real*rp.cd->real
	    + rp.cd->imaginary*rp.cd->imaginary;
	  while (nRepeat--) {
	    if (*lp.b**lp.b > value2.d) {
	      tp.cd->real = *lp.b;
	      tp.cd->imaginary = 0;
	    } else {
	      tp.cd->real = rp.cd->real;
	      tp.cd->imaginary = rp.cd->imaginary;
	    }
	    lp.b++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  value2.w = (word) *rp.b;  while (nRepeat--)
	    *tp.w++ = ((value1.w = *lp.w++) > value2.w)?
	      value1.w:
	      value2.w;
	  break;
	case ANA_WORD:
	  value2.w = *rp.w;  while (nRepeat--)
	    *tp.w++ = ((value1.w = *lp.w++) > value2.w)?
	      value1.w:
	      value2.w;
	  break;
	case ANA_LONG:
	  value2.l = *rp.l;  while (nRepeat--)
	    *tp.l++ = ((value1.l = (int) *lp.w++) > value2.l)?
	      value1.l:
	      value2.l;
	  break;
	case ANA_FLOAT:
	  value2.f = *rp.f;  while (nRepeat--)
	    *tp.f++ = ((value1.f = (float) *lp.w++) > value2.f)?
	      value1.f:
	      value2.f;
	  break;
	case ANA_DOUBLE:
	  value2.d = *rp.d;  while (nRepeat--)
	    *tp.d++ = ((value1.d = (double) *lp.w++) > value2.d)?
	      value1.d:
	      value2.d;
	  break;
	case ANA_CFLOAT:
	  value2.d = rp.cf->real*rp.cf->real
	    + rp.cf->imaginary*rp.cf->imaginary;
	  while (nRepeat--) {
	    if (*lp.w**lp.w > value2.d) {
	      tp.cf->real = *lp.w;
	      tp.cf->imaginary = 0;
	    } else {
	      tp.cf->real = rp.cf->real;
	      tp.cf->imaginary = rp.cf->imaginary;
	    }
	    lp.w++;
	    tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value2.d = rp.cd->real*rp.cd->real
	    + rp.cd->imaginary*rp.cd->imaginary;
	  while (nRepeat--) {
	    if (*lp.w**lp.w > value2.d) {
	      tp.cd->real = *lp.w;
	      tp.cd->imaginary = 0;
	    } else {
	      tp.cd->real = rp.cd->real;
	      tp.cd->imaginary = rp.cd->imaginary;
	    }
	    lp.w++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  value2.l = (int) *rp.b;  while (nRepeat--)
	    *tp.l++ = ((value1.l = *lp.l++) > value2.l)?
	      value1.l:
	      value2.l;
	  break;
	case ANA_WORD:
	  value2.l = (int) *rp.w;  while (nRepeat--)
	    *tp.l++ = ((value1.l = *lp.l++) > value2.l)?
	      value1.l:
	      value2.l;
	  break;
	case ANA_LONG:
	  value2.l = *rp.l;  while (nRepeat--)
	    *tp.l++ = ((value1.l = *lp.l++) > value2.l)?
	      value1.l:
	      value2.l;
	  break;
	case ANA_FLOAT:
	  value2.f = *rp.f;  while (nRepeat--)
	    *tp.f++ = ((value1.f = (float) *lp.l++) > value2.f)?
	      value1.f:
	      value2.f;
	  break;
	case ANA_DOUBLE:
	  value2.d = *rp.d;  while (nRepeat--)
	    *tp.d++ = ((value1.d = (double) *lp.l++) > value2.d)?
	      value1.d:
	      value2.d;
	  break;
	case ANA_CFLOAT:
	  value2.d = rp.cf->real*rp.cf->real
	    + rp.cf->imaginary*rp.cf->imaginary;
	  while (nRepeat--) {
	    if (*lp.l**lp.l > value2.d) {
	      tp.cf->real = *lp.l;
	      tp.cf->imaginary = 0;
	    } else {
	      tp.cf->real = rp.cf->real;
	      tp.cf->imaginary = rp.cf->imaginary;
	    }
	    lp.l++;
	    tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value2.d = rp.cd->real*rp.cd->real
	    + rp.cd->imaginary*rp.cd->imaginary;
	  while (nRepeat--) {
	    if (*lp.l**lp.l > value2.d) {
	      tp.cd->real = *lp.l;
	      tp.cd->imaginary = 0;
	    } else {
	      tp.cd->real = rp.cd->real;
	      tp.cd->imaginary = rp.cd->imaginary;
	    }
	    lp.l++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  value2.f = (float) *rp.b;  while (nRepeat--)
	    *tp.f++ = ((value1.f = *lp.f++) > value2.f)?
	      value1.f:
	      value2.f;
	  break;
	case ANA_WORD:
	  value2.f = (float) *rp.w;  while (nRepeat--)
	    *tp.f++ = ((value1.f = *lp.f++) > value2.f)?
	      value1.f:
	      value2.f;
	  break;
	case ANA_LONG:
	  value2.f = (float) *rp.l;  while (nRepeat--)
	    *tp.f++ = ((value1.f = *lp.f++) > value2.f)?
	      value1.f:
	      value2.f;
	  break;
	case ANA_FLOAT:
	  value2.f = *rp.f;  while (nRepeat--)
	    *tp.f++ = ((value1.f = *lp.f++) > value2.f)?
	      value1.f:
	      value2.f;
	  break;
	case ANA_DOUBLE:
	  value2.d = *rp.d;  while (nRepeat--)
	    *tp.d++ = ((value1.d = (double) *lp.f++) > value2.d)?
	      value1.d:
	      value2.d;
	  break;
	case ANA_CFLOAT:
	  value2.d = rp.cf->real*rp.cf->real
	    + rp.cf->imaginary*rp.cf->imaginary;
	  while (nRepeat--) {
	    if (*lp.f**lp.f > value2.d) {
	      tp.cf->real = *lp.f;
	      tp.cf->imaginary = 0;
	    } else {
	      tp.cf->real = rp.cf->real;
	      tp.cf->imaginary = rp.cf->imaginary;
	    }
	    lp.f++;
	    tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value2.d = rp.cd->real*rp.cd->real
	    + rp.cd->imaginary*rp.cd->imaginary;
	  while (nRepeat--) {
	    if (*lp.f**lp.f > value2.d) {
	      tp.cd->real = *lp.f;
	      tp.cd->imaginary = 0;
	    } else {
	      tp.cd->real = rp.cd->real;
	      tp.cd->imaginary = rp.cd->imaginary;
	    }
	    lp.f++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  value2.d = (double) *rp.b;  while (nRepeat--)
	    *tp.d++ = ((value1.d = *lp.d++) > value2.d)?
	      value1.d:
	      value2.d;
	  break;
	case ANA_WORD:
	  value2.d = (double) *rp.w;  while (nRepeat--)
	    *tp.d++ = ((value1.d = *lp.d++) > value2.d)?
	      value1.d:
	      value2.d;
	  break;
	case ANA_LONG:
	  value2.d = (double) *rp.l;  while (nRepeat--)
	    *tp.d++ = ((value1.d = *lp.d++) > value2.d)?
	      value1.d:
	      value2.d;
	  break;
	case ANA_FLOAT:
	  value2.d = (double) *rp.f;  while (nRepeat--)
	    *tp.d++ = ((value1.d = *lp.d++) > value2.d)?
	      value1.d:
	      value2.d;
	  break;
	case ANA_DOUBLE:
	  value2.d = *rp.d;  while (nRepeat--)
	    *tp.d++ = ((value1.d = *lp.d++) > value2.d)?
	      value1.d:
	      value2.d;
	  break;
	case ANA_CFLOAT:
	  value2.d = rp.cf->real*rp.cf->real
	    + rp.cf->imaginary*rp.cf->imaginary;
	  while (nRepeat--) {
	    if (*lp.d**lp.d > value2.d) {
	      tp.cf->real = *lp.d;
	      tp.cf->imaginary = 0;
	    } else {
	      tp.cf->real = rp.cf->real;
	      tp.cf->imaginary = rp.cf->imaginary;
	    }
	    lp.d++;
	    tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value2.d = rp.cd->real*rp.cd->real
	    + rp.cd->imaginary*rp.cd->imaginary;
	  while (nRepeat--) {
	    if (*lp.d**lp.d > value2.d) {
	      tp.cd->real = *lp.d;
	      tp.cd->imaginary = 0;
	    } else {
	      tp.cd->real = rp.cd->real;
	      tp.cd->imaginary = rp.cd->imaginary;
	    }
	    lp.d++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  value2.d = *rp.b**rp.b;
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		> value2.d) {
	      tp.cf->real = lp.cf->real;
	      tp.cf->imaginary = lp.cf->imaginary;
	    } else {
	      tp.cf->real = *rp.b;
	      tp.cf->imaginary = 0;
	    }
	    lp.cf++;
	    tp.cf++;
	  }
	  break;
	case ANA_WORD:
	  value2.d = *rp.w**rp.w;
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		> value2.d) {
	      tp.cf->real = lp.cf->real;
	      tp.cf->imaginary = lp.cf->imaginary;
	    } else {
	      tp.cf->real = *rp.w;
	      tp.cf->imaginary = 0;
	    }
	    lp.cf++;
	    tp.cf++;
	  }
	  break;
	case ANA_LONG:
	  value2.d = *rp.l**rp.l;
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		> value2.d) {
	      tp.cf->real = lp.cf->real;
	      tp.cf->imaginary = lp.cf->imaginary;
	    } else {
	      tp.cf->real = *rp.l;
	      tp.cf->imaginary = 0;
	    }
	    lp.cf++;
	    tp.cf++;
	  }
	  break;
	case ANA_FLOAT:
	  value2.d = *rp.f**rp.f;
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		> value2.d) {
	      tp.cf->real = lp.cf->real;
	      tp.cf->imaginary = lp.cf->imaginary;
	    } else {
	      tp.cf->real = *rp.f;
	      tp.cf->imaginary = 0;
	    }
	    lp.cf++;
	    tp.cf++;
	  }
	  break;
	case ANA_DOUBLE:
	  value2.d = *rp.d**rp.d;
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		> value2.d) {
	      tp.cf->real = lp.cf->real;
	      tp.cf->imaginary = lp.cf->imaginary;
	    } else {
	      tp.cf->real = *rp.d;
	      tp.cf->imaginary = 0;
	    }
	    lp.cf++;
	    tp.cf++;
	  }
	  break;
	case ANA_CFLOAT:
	  value2.d = rp.cf->real*rp.cf->real
	    + rp.cf->imaginary*rp.cf->imaginary;
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		> value2.d) {
	      tp.cf->real = lp.cf->real;
	      tp.cf->imaginary = lp.cf->imaginary;
	    } else {
	      tp.cf->real = rp.cf->real;
	      tp.cf->imaginary = rp.cf->imaginary;
	    }
	    lp.cf++;
	    tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value2.d = rp.cd->real*rp.cd->real
	    + rp.cd->imaginary*rp.cd->imaginary;
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		> value2.d) {
	      tp.cd->real = lp.cf->real;
	      tp.cd->imaginary = lp.cf->imaginary;
	    } else {
	      tp.cd->real = rp.cd->real;
	      tp.cd->imaginary = rp.cd->imaginary;
	    }
	    lp.cf++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  value2.d = *rp.b**rp.b;
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		> value2.d) {
	      tp.cd->real = lp.cd->real;
	      tp.cd->imaginary = lp.cd->imaginary;
	    } else {
	      tp.cd->real = *rp.b;
	      tp.cd->imaginary = 0;
	    }
	    lp.cd++;
	    tp.cd++;
	  }
	  break;
	case ANA_WORD:
	  value2.d = *rp.w**rp.w;
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		> value2.d) {
	      tp.cd->real = lp.cd->real;
	      tp.cd->imaginary = lp.cd->imaginary;
	    } else {
	      tp.cd->real = *rp.w;
	      tp.cd->imaginary = 0;
	    }
	    lp.cd++;
	    tp.cd++;
	  }
	  break;
	case ANA_LONG:
	  value2.d = *rp.l**rp.l;
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		> value2.d) {
	      tp.cd->real = lp.cd->real;
	      tp.cd->imaginary = lp.cd->imaginary;
	    } else {
	      tp.cd->real = *rp.l;
	      tp.cd->imaginary = 0;
	    }
	    lp.cd++;
	    tp.cd++;
	  }
	  break;
	case ANA_FLOAT:
	  value2.d = *rp.f**rp.f;
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		> value2.d) {
	      tp.cd->real = lp.cd->real;
	      tp.cd->imaginary = lp.cd->imaginary;
	    } else {
	      tp.cd->real = *rp.f;
	      tp.cd->imaginary = 0;
	    }
	    lp.cd++;
	    tp.cd++;
	  }
	  break;
	case ANA_DOUBLE:
	  value2.d = *rp.d**rp.d;
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		> value2.d) {
	      tp.cd->real = lp.cd->real;
	      tp.cd->imaginary = lp.cd->imaginary;
	    } else {
	      tp.cd->real = *rp.d;
	      tp.cd->imaginary = 0;
	    }
	    lp.cd++;
	    tp.cd++;
	  }
	  break;
	case ANA_CFLOAT:
	  value2.d = rp.cf->real*rp.cf->real
	    + rp.cf->imaginary*rp.cf->imaginary;
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		> value2.d) {
	      tp.cd->real = lp.cd->real;
	      tp.cd->imaginary = lp.cd->imaginary;
	    } else {
	      tp.cd->real = rp.cf->real;
	      tp.cd->imaginary = rp.cf->imaginary;
	    }
	    lp.cd++;
	    tp.cd++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value2.d = rp.cd->real*rp.cd->real
	    + rp.cd->imaginary*rp.cd->imaginary;
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		> value2.d) {
	      tp.cd->real = lp.cd->real;
	      tp.cd->imaginary = lp.cd->imaginary;
	    } else {
	      tp.cd->real = rp.cd->real;
	      tp.cd->imaginary = rp.cd->imaginary;
	    }
	    lp.cd++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_max_sa(void)
     /* largest-taking with scalar LHS and array RHS.  Since this operator */
     /* is commutative, we just swap LHS and RHS and pass on to */
     /* ana_max_as() */
{
  int		temp;
  pointer	tempp;
  
  temp = lhsType;
  lhsType = rhsType;
  rhsType = temp;
  tempp = lp;
  lp = rp;
  rp = tempp;
  ana_max_as();
  temp = lhsType;
  lhsType = rhsType;
  rhsType = temp;
  tempp = lp;
  lp = rp;
  rp = tempp;
}
/*----------------------------------------------------------*/
void ana_min(void)
     /* smallest-taking with array operands */
{
  scalar	value1, value2;
  
  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.b++ = ((value1.b = *lp.b++) < (value2.b = *rp.b++))?
	      value1.b:
	      value2.b;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = ((value1.w = (word) *lp.b++) < (value2.w = *rp.w++))?
	      value1.w:
	      value2.w;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((value1.l = (int) *lp.b++) < (value2.l = *rp.l++))?
	      value1.l:
	      value2.l;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = ((value1.f = (float) *lp.b++) < (value2.f = *rp.f++))?
	      value1.f:
	      value2.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = ((value1.d = (double) *lp.b++) < (value2.d = *rp.d++))?
	      value1.d:
	      value2.d;
	  break;
	case ANA_CFLOAT:
	  /* if complex numbers are involved, then we compare the absolute
	     values.  LS 8dec98 */
	  while (nRepeat--) {
	    if (*lp.b**lp.b < rp.cf->real*rp.cf->real
		+ rp.cf->imaginary*rp.cf->imaginary) {
	      tp.cf->real = *lp.b;
	      tp.cf->imaginary = 0;
	    } else
	      memcpy(tp.cf, rp.cf, sizeof(floatComplex));
	    tp.cf++;
	    lp.b++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    if (*lp.b**lp.b < rp.cd->real*rp.cd->real
		+ rp.cd->imaginary*rp.cd->imaginary) {
	      tp.cd->real = *lp.b;
	      tp.cd->imaginary = 0;
	    } else
	      memcpy(tp.cd, rp.cd, sizeof(doubleComplex));
	    tp.cd++;
	    lp.b++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.w++ = ((value1.w = *lp.w++) < (value2.w = (word) *rp.b++))?
	      value1.w:
	      value2.w;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.w++ = ((value1.w = *lp.w++) < (value2.w = *rp.w++))?
	      value1.w:
	      value2.w;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((value1.l = (int) *lp.w++) < (value2.l = *rp.l++))?
	      value1.l:
	      value2.l;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = ((value1.f = (float) *lp.w++) < (value2.f = *rp.f++))?
	      value1.f:
	      value2.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = ((value1.d = (double) *lp.w++) < (value2.d = *rp.d++))?
	      value1.d:
	      value2.d;
	  break;
	case ANA_CFLOAT:
	  /* if complex numbers are involved, then we compare the absolute
	     values.  LS 8dec98 */
	  while (nRepeat--) {
	    if (*lp.w**lp.w < rp.cf->real*rp.cf->real
		+ rp.cf->imaginary*rp.cf->imaginary) {
	      tp.cf->real = *lp.w;
	      tp.cf->imaginary = 0;
	    } else
	      memcpy(tp.cf, rp.cf, sizeof(floatComplex));
	    tp.cf++;
	    lp.w++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    if (*lp.w**lp.w < rp.cd->real*rp.cd->real
		+ rp.cd->imaginary*rp.cd->imaginary) {
	      tp.cd->real = *lp.w;
	      tp.cd->imaginary = 0;
	    } else
	      memcpy(tp.cd, rp.cd, sizeof(doubleComplex));
	    tp.cd++;
	    lp.w++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = ((value1.l = *lp.l++) < (value2.l = (int) *rp.b++))?
	      value1.l:
	      value2.l;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = ((value1.l = *lp.l++) < (value2.l = (int) *rp.w++))?
	      value1.l:
	      value2.l;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((value1.l = *lp.l++) < (value2.l = *rp.l++))?
	      value1.l:
	      value2.l;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = ((value1.f = (float) *lp.l++) < (value2.f = *rp.f++))?
	      value1.f:
	      value2.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = ((value1.d = (double) *lp.l++) < (value2.d = *rp.d++))?
	      value1.d:
	      value2.d;
	  break;
	case ANA_CFLOAT:
	  /* if complex numbers are involved, then we compare the absolute
	     values.  LS 8dec98 */
	  while (nRepeat--) {
	    if (*lp.l**lp.l < rp.cf->real*rp.cf->real
		+ rp.cf->imaginary*rp.cf->imaginary) {
	      tp.cf->real = *lp.l;
	      tp.cf->imaginary = 0;
	    } else
	      memcpy(tp.cf, rp.cf, sizeof(floatComplex));
	    tp.cf++;
	    lp.l++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    if (*lp.l**lp.l < rp.cd->real*rp.cd->real
		+ rp.cd->imaginary*rp.cd->imaginary) {
	      tp.cd->real = *lp.l;
	      tp.cd->imaginary = 0;
	    } else
	      memcpy(tp.cd, rp.cd, sizeof(doubleComplex));
	    tp.cd++;
	    lp.l++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.f++ = ((value1.f = *lp.f++) < (value2.f = (float) *rp.b++))?
	      value1.f:
	      value2.f;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.f++ = ((value1.f = *lp.f++) < (value2.f = (float) *rp.w++))?
	      value1.f:
	      value2.f;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.f++ = ((value1.f = *lp.f++) < (value2.f = (float) *rp.l++))?
	      value1.f:
	      value2.f;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.f++ = ((value1.f = *lp.f++) < (value2.f = *rp.f++))?
	      value1.f:
	      value2.f;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = ((value1.d = (double) *lp.f++) < (value2.d = *rp.d++))?
	      value1.d:
	      value2.d;
	  break;
	case ANA_CFLOAT:
	  /* if complex numbers are involved, then we compare the absolute
	     values.  LS 8dec98 */
	  while (nRepeat--) {
	    if (*lp.f**lp.f < rp.cf->real*rp.cf->real
		+ rp.cf->imaginary*rp.cf->imaginary) {
	      tp.cf->real = *lp.f;
	      tp.cf->imaginary = 0;
	    } else
	      memcpy(tp.cf, rp.cf, sizeof(floatComplex));
	    tp.cf++;
	    lp.f++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    if (*lp.f**lp.f < rp.cd->real*rp.cd->real
		+ rp.cd->imaginary*rp.cd->imaginary) {
	      tp.cd->real = *lp.f;
	      tp.cd->imaginary = 0;
	    } else
	      memcpy(tp.cd, rp.cd, sizeof(doubleComplex));
	    tp.cd++;
	    lp.f++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.d++ = ((value1.d = *lp.d++) < (value2.d = (double) *rp.b++))?
	      value1.d:
	      value2.d;
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.d++ = ((value1.d = *lp.d++) < (value2.d = (double) *rp.w++))?
	      value1.d:
	      value2.d;
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.d++ = ((value1.d = *lp.d++) < (value2.d = (double) *rp.l++))?
	      value1.d:
	      value2.d;
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.d++ = ((value1.d = *lp.d++) < (value2.d = (double) *rp.f++))?
	      value1.d:
	      value2.d;
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.d++ = ((value1.d = *lp.d++) < (value2.d = *rp.d++))?
	      value1.d:
	      value2.d;
	  break;
	case ANA_CFLOAT:
	  /* if complex numbers are involved, then we compare the absolute
	     values.  LS 8dec98 */
	  while (nRepeat--) {
	    if (*lp.d**lp.d < rp.cf->real*rp.cf->real
		+ rp.cf->imaginary*rp.cf->imaginary) {
	      tp.cf->real = *lp.d;
	      tp.cf->imaginary = 0;
	    } else
	      memcpy(tp.cf, rp.cf, sizeof(floatComplex));
	    tp.cf++;
	    lp.d++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    if (*lp.d**lp.d < rp.cd->real*rp.cd->real
		+ rp.cd->imaginary*rp.cd->imaginary) {
	      tp.cd->real = *lp.d;
	      tp.cd->imaginary = 0;
	    } else
	      memcpy(tp.cd, rp.cd, sizeof(doubleComplex));
	    tp.cd++;
	    lp.d++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      /* if complex numbers are involved, then we compare absolute values */
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		< *rp.b**rp.b)
	      memcpy(tp.cf, lp.cf, sizeof(floatComplex));
	    else {
	      tp.cf->real = *rp.b;
	      tp.cf->imaginary = 0;
	    }
	    tp.cf++;
	    lp.cf++;
	    rp.b++;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		< *rp.w**rp.w)
	      memcpy(tp.cf, lp.cf, sizeof(floatComplex));
	    else {
	      tp.cf->real = *rp.w;
	      tp.cf->imaginary = 0;
	    }
	    tp.cf++;
	    lp.cf++;
	    rp.w++;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		< *rp.l**rp.l)
	      memcpy(tp.cf, lp.cf, sizeof(floatComplex));
	    else {
	      tp.cf->real = *rp.l;
	      tp.cf->imaginary = 0;
	    }
	    tp.cf++;
	    lp.cf++;
	    rp.l++;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		< *rp.f**rp.f)
	      memcpy(tp.cf, lp.cf, sizeof(floatComplex));
	    else {
	      tp.cf->real = *rp.f;
	      tp.cf->imaginary = 0;
	    }
	    tp.cf++;
	    lp.cf++;
	    rp.f++;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		< *rp.d**rp.d)
	      memcpy(tp.cf, lp.cf, sizeof(floatComplex));
	    else {
	      tp.cf->real = *rp.d;
	      tp.cf->imaginary = 0;
	    }
	    tp.cf++;
	    lp.cf++;
	    rp.d++;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		< rp.cf->real*rp.cf->real + rp.cf->imaginary*rp.cf->imaginary){
	      tp.cf->real = lp.cf->real;
	      tp.cf->imaginary = lp.cf->imaginary;
	    } else {
	      tp.cf->real = rp.cf->real;
	      tp.cf->imaginary = rp.cf->imaginary;
	    }
	    tp.cf++;
	    lp.cf++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		< rp.cd->real*rp.cd->real + rp.cd->imaginary*rp.cd->imaginary){
	      tp.cd->real = lp.cf->real;
	      tp.cd->imaginary = lp.cf->imaginary;
	    } else {
	      tp.cd->real = rp.cd->real;
	      tp.cd->imaginary = rp.cd->imaginary;
	    }
	    tp.cd++;
	    lp.cf++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      /* if complex numbers are involved, then we compare absolute values */
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		< *rp.b**rp.b)
	      memcpy(tp.cf, lp.cd, sizeof(floatComplex));
	    else {
	      tp.cf->real = *rp.b;
	      tp.cf->imaginary = 0;
	    }
	    tp.cf++;
	    lp.cd++;
	    rp.b++;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		< *rp.w**rp.w)
	      memcpy(tp.cf, lp.cd, sizeof(floatComplex));
	    else {
	      tp.cf->real = *rp.w;
	      tp.cf->imaginary = 0;
	    }
	    tp.cf++;
	    lp.cd++;
	    rp.w++;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		< *rp.l**rp.l)
	      memcpy(tp.cf, lp.cd, sizeof(floatComplex));
	    else {
	      tp.cf->real = *rp.l;
	      tp.cf->imaginary = 0;
	    }
	    tp.cf++;
	    lp.cd++;
	    rp.l++;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		< *rp.f**rp.f)
	      memcpy(tp.cf, lp.cd, sizeof(floatComplex));
	    else {
	      tp.cf->real = *rp.f;
	      tp.cf->imaginary = 0;
	    }
	    tp.cf++;
	    lp.cd++;
	    rp.f++;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		< *rp.d**rp.d)
	      memcpy(tp.cf, lp.cd, sizeof(floatComplex));
	    else {
	      tp.cf->real = *rp.d;
	      tp.cf->imaginary = 0;
	    }
	    tp.cf++;
	    lp.cd++;
	    rp.d++;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		< rp.cf->real*rp.cf->real + rp.cf->imaginary*rp.cf->imaginary){
	      tp.cd->real = lp.cd->real;
	      tp.cd->imaginary = lp.cd->imaginary;
	    } else {
	      tp.cd->real = rp.cf->real;
	      tp.cd->imaginary = rp.cf->imaginary;
	    }
	    tp.cd++;
	    lp.cd++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		< rp.cd->real*rp.cd->real + rp.cd->imaginary*rp.cd->imaginary){
	      tp.cd->real = lp.cd->real;
	      tp.cd->imaginary = lp.cd->imaginary;
	    } else {
	      tp.cd->real = rp.cd->real;
	      tp.cd->imaginary = rp.cd->imaginary;
	    }
	    tp.cd++;
	    lp.cd++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_min_as(void)
     /* smallest-taking with array LHS and scalar RHS */
{
  scalar	value1, value2;

  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  value2.b = *rp.b;
	  while (nRepeat--)
	    *tp.b++ = ((value1.b = *lp.b++) < value2.b)?
	      value1.b:
		value2.b;
	  break;
	case ANA_WORD:
	  value2.w = *rp.w;
	  while (nRepeat--)
	    *tp.w++ = ((value1.w = *lp.b++) < value2.w)?
	      value1.w:
		value2.w;
	  break;
	case ANA_LONG:
	  value2.l = *rp.l;
	  while (nRepeat--)
	    *tp.l++ = ((value1.l = *lp.b++) < value2.l)?
	      value1.l:
		value2.l;
	  break;
	case ANA_FLOAT:
	  value2.f = *rp.f;
	  while (nRepeat--)
	    *tp.f++ = ((value1.f = *lp.b++) < value2.f)?
	      value1.f:
		value2.f;
	  break;
	case ANA_DOUBLE:
	  value2.d = *rp.d;
	  while (nRepeat--)
	    *tp.d++ = ((value1.d = *lp.b++) < value2.d)?
	      value1.d:
		value2.d;
	  break;
	case ANA_CFLOAT:
	  value2.d = rp.cf->real*rp.cf->real
	    + rp.cf->imaginary*rp.cf->imaginary;
	  while (nRepeat--) {
	    if (*lp.b**lp.b < value2.d) {
	      tp.cf->real = *lp.b;
	      tp.cf->imaginary = 0;
	    } else {
	      tp.cf->real = rp.cf->real;
	      tp.cf->imaginary = rp.cf->imaginary;
	    }
	    lp.b++;
	    tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value2.d = rp.cd->real*rp.cd->real
	    + rp.cd->imaginary*rp.cd->imaginary;
	  while (nRepeat--) {
	    if (*lp.b**lp.b < value2.d) {
	      tp.cd->real = *lp.b;
	      tp.cd->imaginary = 0;
	    } else {
	      tp.cd->real = rp.cd->real;
	      tp.cd->imaginary = rp.cd->imaginary;
	    }
	    lp.b++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  value2.w = (word) *rp.b;  while (nRepeat--)
	    *tp.w++ = ((value1.w = *lp.w++) < value2.w)?
	      value1.w:
	      value2.w;
	  break;
	case ANA_WORD:
	  value2.w = *rp.w;  while (nRepeat--)
	    *tp.w++ = ((value1.w = *lp.w++) < value2.w)?
	      value1.w:
	      value2.w;
	  break;
	case ANA_LONG:
	  value2.l = *rp.l;  while (nRepeat--)
	    *tp.l++ = ((value1.l = (int) *lp.w++) < value2.l)?
	      value1.l:
	      value2.l;
	  break;
	case ANA_FLOAT:
	  value2.f = *rp.f;  while (nRepeat--)
	    *tp.f++ = ((value1.f = (float) *lp.w++) < value2.f)?
	      value1.f:
	      value2.f;
	  break;
	case ANA_DOUBLE:
	  value2.d = *rp.d;  while (nRepeat--)
	    *tp.d++ = ((value1.d = (double) *lp.w++) < value2.d)?
	      value1.d:
	      value2.d;
	  break;
	case ANA_CFLOAT:
	  value2.d = rp.cf->real*rp.cf->real
	    + rp.cf->imaginary*rp.cf->imaginary;
	  while (nRepeat--) {
	    if (*lp.w**lp.w < value2.d) {
	      tp.cf->real = *lp.w;
	      tp.cf->imaginary = 0;
	    } else {
	      tp.cf->real = rp.cf->real;
	      tp.cf->imaginary = rp.cf->imaginary;
	    }
	    lp.w++;
	    tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value2.d = rp.cd->real*rp.cd->real
	    + rp.cd->imaginary*rp.cd->imaginary;
	  while (nRepeat--) {
	    if (*lp.w**lp.w < value2.d) {
	      tp.cd->real = *lp.w;
	      tp.cd->imaginary = 0;
	    } else {
	      tp.cd->real = rp.cd->real;
	      tp.cd->imaginary = rp.cd->imaginary;
	    }
	    lp.w++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  value2.l = (int) *rp.b;  while (nRepeat--)
	    *tp.l++ = ((value1.l = *lp.l++) < value2.l)?
	      value1.l:
	      value2.l;
	  break;
	case ANA_WORD:
	  value2.l = (int) *rp.w;  while (nRepeat--)
	    *tp.l++ = ((value1.l = *lp.l++) < value2.l)?
	      value1.l:
	      value2.l;
	  break;
	case ANA_LONG:
	  value2.l = *rp.l;  while (nRepeat--)
	    *tp.l++ = ((value1.l = *lp.l++) < value2.l)?
	      value1.l:
	      value2.l;
	  break;
	case ANA_FLOAT:
	  value2.f = *rp.f;  while (nRepeat--)
	    *tp.f++ = ((value1.f = (float) *lp.l++) < value2.f)?
	      value1.f:
	      value2.f;
	  break;
	case ANA_DOUBLE:
	  value2.d = *rp.d;  while (nRepeat--)
	    *tp.d++ = ((value1.d = (double) *lp.l++) < value2.d)?
	      value1.d:
	      value2.d;
	  break;
	case ANA_CFLOAT:
	  value2.d = rp.cf->real*rp.cf->real
	    + rp.cf->imaginary*rp.cf->imaginary;
	  while (nRepeat--) {
	    if (*lp.l**lp.l < value2.d) {
	      tp.cf->real = *lp.l;
	      tp.cf->imaginary = 0;
	    } else {
	      tp.cf->real = rp.cf->real;
	      tp.cf->imaginary = rp.cf->imaginary;
	    }
	    lp.l++;
	    tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value2.d = rp.cd->real*rp.cd->real
	    + rp.cd->imaginary*rp.cd->imaginary;
	  while (nRepeat--) {
	    if (*lp.l**lp.l < value2.d) {
	      tp.cd->real = *lp.l;
	      tp.cd->imaginary = 0;
	    } else {
	      tp.cd->real = rp.cd->real;
	      tp.cd->imaginary = rp.cd->imaginary;
	    }
	    lp.l++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  value2.f = (float) *rp.b;  while (nRepeat--)
	    *tp.f++ = ((value1.f = *lp.f++) < value2.f)?
	      value1.f:
	      value2.f;
	  break;
	case ANA_WORD:
	  value2.f = (float) *rp.w;  while (nRepeat--)
	    *tp.f++ = ((value1.f = *lp.f++) < value2.f)?
	      value1.f:
	      value2.f;
	  break;
	case ANA_LONG:
	  value2.f = (float) *rp.l;  while (nRepeat--)
	    *tp.f++ = ((value1.f = *lp.f++) < value2.f)?
	      value1.f:
	      value2.f;
	  break;
	case ANA_FLOAT:
	  value2.f = *rp.f;  while (nRepeat--)
	    *tp.f++ = ((value1.f = *lp.f++) < value2.f)?
	      value1.f:
	      value2.f;
	  break;
	case ANA_DOUBLE:
	  value2.d = *rp.d;  while (nRepeat--)
	    *tp.d++ = ((value1.d = (double) *lp.f++) < value2.d)?
	      value1.d:
	      value2.d;
	  break;
	case ANA_CFLOAT:
	  value2.d = rp.cf->real*rp.cf->real
	    + rp.cf->imaginary*rp.cf->imaginary;
	  while (nRepeat--) {
	    if (*lp.f**lp.f < value2.d) {
	      tp.cf->real = *lp.f;
	      tp.cf->imaginary = 0;
	    } else {
	      tp.cf->real = rp.cf->real;
	      tp.cf->imaginary = rp.cf->imaginary;
	    }
	    lp.f++;
	    tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value2.d = rp.cd->real*rp.cd->real
	    + rp.cd->imaginary*rp.cd->imaginary;
	  while (nRepeat--) {
	    if (*lp.f**lp.f < value2.d) {
	      tp.cd->real = *lp.f;
	      tp.cd->imaginary = 0;
	    } else {
	      tp.cd->real = rp.cd->real;
	      tp.cd->imaginary = rp.cd->imaginary;
	    }
	    lp.f++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  value2.d = (double) *rp.b;  while (nRepeat--)
	    *tp.d++ = ((value1.d = *lp.d++) < value2.d)?
	      value1.d:
	      value2.d;
	  break;
	case ANA_WORD:
	  value2.d = (double) *rp.w;  while (nRepeat--)
	    *tp.d++ = ((value1.d = *lp.d++) < value2.d)?
	      value1.d:
	      value2.d;
	  break;
	case ANA_LONG:
	  value2.d = (double) *rp.l;  while (nRepeat--)
	    *tp.d++ = ((value1.d = *lp.d++) < value2.d)?
	      value1.d:
	      value2.d;
	  break;
	case ANA_FLOAT:
	  value2.d = (double) *rp.f;  while (nRepeat--)
	    *tp.d++ = ((value1.d = *lp.d++) < value2.d)?
	      value1.d:
	      value2.d;
	  break;
	case ANA_DOUBLE:
	  value2.d = *rp.d;  while (nRepeat--)
	    *tp.d++ = ((value1.d = *lp.d++) < value2.d)?
	      value1.d:
	      value2.d;
	  break;
	case ANA_CFLOAT:
	  value2.d = rp.cf->real*rp.cf->real
	    + rp.cf->imaginary*rp.cf->imaginary;
	  while (nRepeat--) {
	    if (*lp.d**lp.d < value2.d) {
	      tp.cf->real = *lp.d;
	      tp.cf->imaginary = 0;
	    } else {
	      tp.cf->real = rp.cf->real;
	      tp.cf->imaginary = rp.cf->imaginary;
	    }
	    lp.d++;
	    tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value2.d = rp.cd->real*rp.cd->real
	    + rp.cd->imaginary*rp.cd->imaginary;
	  while (nRepeat--) {
	    if (*lp.d**lp.d < value2.d) {
	      tp.cd->real = *lp.d;
	      tp.cd->imaginary = 0;
	    } else {
	      tp.cd->real = rp.cd->real;
	      tp.cd->imaginary = rp.cd->imaginary;
	    }
	    lp.d++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  value2.d = *rp.b**rp.b;
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		< value2.d) {
	      tp.cf->real = lp.cf->real;
	      tp.cf->imaginary = lp.cf->imaginary;
	    } else {
	      tp.cf->real = *rp.b;
	      tp.cf->imaginary = 0;
	    }
	    lp.cf++;
	    tp.cf++;
	  }
	  break;
	case ANA_WORD:
	  value2.d = *rp.w**rp.w;
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		< value2.d) {
	      tp.cf->real = lp.cf->real;
	      tp.cf->imaginary = lp.cf->imaginary;
	    } else {
	      tp.cf->real = *rp.w;
	      tp.cf->imaginary = 0;
	    }
	    lp.cf++;
	    tp.cf++;
	  }
	  break;
	case ANA_LONG:
	  value2.d = *rp.l**rp.l;
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		< value2.d) {
	      tp.cf->real = lp.cf->real;
	      tp.cf->imaginary = lp.cf->imaginary;
	    } else {
	      tp.cf->real = *rp.l;
	      tp.cf->imaginary = 0;
	    }
	    lp.cf++;
	    tp.cf++;
	  }
	  break;
	case ANA_FLOAT:
	  value2.d = *rp.f**rp.f;
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		< value2.d) {
	      tp.cf->real = lp.cf->real;
	      tp.cf->imaginary = lp.cf->imaginary;
	    } else {
	      tp.cf->real = *rp.f;
	      tp.cf->imaginary = 0;
	    }
	    lp.cf++;
	    tp.cf++;
	  }
	  break;
	case ANA_DOUBLE:
	  value2.d = *rp.d**rp.d;
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		< value2.d) {
	      tp.cf->real = lp.cf->real;
	      tp.cf->imaginary = lp.cf->imaginary;
	    } else {
	      tp.cf->real = *rp.d;
	      tp.cf->imaginary = 0;
	    }
	    lp.cf++;
	    tp.cf++;
	  }
	  break;
	case ANA_CFLOAT:
	  value2.d = rp.cf->real*rp.cf->real
	    + rp.cf->imaginary*rp.cf->imaginary;
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		< value2.d) {
	      tp.cf->real = lp.cf->real;
	      tp.cf->imaginary = lp.cf->imaginary;
	    } else {
	      tp.cf->real = rp.cf->real;
	      tp.cf->imaginary = rp.cf->imaginary;
	    }
	    lp.cf++;
	    tp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value2.d = rp.cd->real*rp.cd->real
	    + rp.cd->imaginary*rp.cd->imaginary;
	  while (nRepeat--) {
	    if (lp.cf->real*lp.cf->real + lp.cf->imaginary*lp.cf->imaginary
		< value2.d) {
	      tp.cd->real = lp.cf->real;
	      tp.cd->imaginary = lp.cf->imaginary;
	    } else {
	      tp.cd->real = rp.cd->real;
	      tp.cd->imaginary = rp.cd->imaginary;
	    }
	    lp.cf++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  value2.d = *rp.b**rp.b;
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		< value2.d) {
	      tp.cd->real = lp.cd->real;
	      tp.cd->imaginary = lp.cd->imaginary;
	    } else {
	      tp.cd->real = *rp.b;
	      tp.cd->imaginary = 0;
	    }
	    lp.cd++;
	    tp.cd++;
	  }
	  break;
	case ANA_WORD:
	  value2.d = *rp.w**rp.w;
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		< value2.d) {
	      tp.cd->real = lp.cd->real;
	      tp.cd->imaginary = lp.cd->imaginary;
	    } else {
	      tp.cd->real = *rp.w;
	      tp.cd->imaginary = 0;
	    }
	    lp.cd++;
	    tp.cd++;
	  }
	  break;
	case ANA_LONG:
	  value2.d = *rp.l**rp.l;
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		< value2.d) {
	      tp.cd->real = lp.cd->real;
	      tp.cd->imaginary = lp.cd->imaginary;
	    } else {
	      tp.cd->real = *rp.l;
	      tp.cd->imaginary = 0;
	    }
	    lp.cd++;
	    tp.cd++;
	  }
	  break;
	case ANA_FLOAT:
	  value2.d = *rp.f**rp.f;
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		< value2.d) {
	      tp.cd->real = lp.cd->real;
	      tp.cd->imaginary = lp.cd->imaginary;
	    } else {
	      tp.cd->real = *rp.f;
	      tp.cd->imaginary = 0;
	    }
	    lp.cd++;
	    tp.cd++;
	  }
	  break;
	case ANA_DOUBLE:
	  value2.d = *rp.d**rp.d;
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		< value2.d) {
	      tp.cd->real = lp.cd->real;
	      tp.cd->imaginary = lp.cd->imaginary;
	    } else {
	      tp.cd->real = *rp.d;
	      tp.cd->imaginary = 0;
	    }
	    lp.cd++;
	    tp.cd++;
	  }
	  break;
	case ANA_CFLOAT:
	  value2.d = rp.cf->real*rp.cf->real
	    + rp.cf->imaginary*rp.cf->imaginary;
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		< value2.d) {
	      tp.cd->real = lp.cd->real;
	      tp.cd->imaginary = lp.cd->imaginary;
	    } else {
	      tp.cd->real = rp.cf->real;
	      tp.cd->imaginary = rp.cf->imaginary;
	    }
	    lp.cd++;
	    tp.cd++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value2.d = rp.cd->real*rp.cd->real
	    + rp.cd->imaginary*rp.cd->imaginary;
	  while (nRepeat--) {
	    if (lp.cd->real*lp.cd->real + lp.cd->imaginary*lp.cd->imaginary
		< value2.d) {
	      tp.cd->real = lp.cd->real;
	      tp.cd->imaginary = lp.cd->imaginary;
	    } else {
	      tp.cd->real = rp.cd->real;
	      tp.cd->imaginary = rp.cd->imaginary;
	    }
	    lp.cd++;
	    tp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_min_sa(void)
     /* smallest-taking with scalar LHS and array RHS.  Since this operator */
     /* is commutative we just swap LHS and RHS and pass on to */
     /* ana_min_as() */
{
  int		temp;
  pointer	tempp;
  
  temp = lhsType;
  lhsType = rhsType;
  rhsType = temp;
  tempp = lp;
  lp = rp;
  rp = tempp;
  ana_min_as();
  temp = lhsType;
  lhsType = rhsType;
  rhsType = temp;
  tempp = lp;
  lp = rp;
  rp = tempp;
}
/*----------------------------------------------------------*/
void ana_eq(void)
     /* equal-to with array operands */
{
  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b++ == *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b++ == *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b++ == *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b++ == *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b++ == *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.b == rp.cf->real && rp.cf->imaginary == 0);
	    lp.b++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.b == rp.cd->real && rp.cd->imaginary == 0);
	    lp.b++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ == (word) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ == (word) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ == (word) *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ == (word) *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ == (word) *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.w == rp.cf->real && rp.cf->imaginary == 0);
	    lp.w++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.w == rp.cd->real && rp.cd->imaginary == 0);
	    lp.w++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ == (int) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ == (int) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ == *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = ((float) *lp.l++ == *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.l++ == *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.l == rp.cf->real && rp.cf->imaginary == 0);
	    lp.l++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.l == rp.cd->real && rp.cd->imaginary == 0);
	    lp.l++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ == (float) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ == (float) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ == (float) *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ == *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.f++ == *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.f == rp.cf->real && rp.cf->imaginary == 0);
	    lp.f++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.f == rp.cd->real && rp.cd->imaginary == 0);
	    lp.f++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ == (double) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ == (double) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ == (double) *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ == (double) *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ == *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.d == rp.cf->real && rp.cf->imaginary == 0);
	    lp.d++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.d == rp.cd->real && rp.cd->imaginary == 0);
	    lp.d++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real == *rp.b && lp.cf->imaginary == 0);
	    lp.cf++;
	    rp.b++;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real == *rp.w && lp.cf->imaginary == 0);
	    lp.cf++;
	    rp.w++;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real == *rp.l && lp.cf->imaginary == 0);
	    lp.cf++;
	    rp.l++;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real == *rp.f && lp.cf->imaginary == 0);
	    lp.cf++;
	    rp.f++;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real == *rp.d && lp.cf->imaginary == 0);
	    lp.cf++;
	    rp.d++;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real == rp.cf->real
		       && lp.cf->imaginary == rp.cf->imaginary);
	    lp.cf++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real == rp.cd->real
		       && lp.cf->imaginary == rp.cd->imaginary);
	    lp.cf++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real == *rp.b && lp.cd->imaginary == 0);
	    lp.cd++;
	    rp.b++;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real == *rp.w && lp.cd->imaginary == 0);
	    lp.cd++;
	    rp.w++;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real == *rp.l && lp.cd->imaginary == 0);
	    lp.cd++;
	    rp.l++;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real == *rp.f && lp.cd->imaginary == 0);
	    lp.cd++;
	    rp.f++;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real == *rp.d && lp.cd->imaginary == 0);
	    lp.cd++;
	    rp.d++;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real == rp.cf->real
		       && lp.cd->imaginary == rp.cf->imaginary);
	    lp.cd++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real == rp.cd->real
		       && lp.cd->imaginary == rp.cd->imaginary);
	    lp.cd++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_STRING_ARRAY:
      if (rhsType == ANA_STRING_ARRAY) {
	while (nRepeat--)
	  *tp.l++ = strcmp(*lp.sp++, *rp.sp++) == 0;
	break;
      }
      /* else fall through to default */
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_eq_as(void)
     /* equal-to with array LHS and scalar RHS */
{
  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b++ == *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = ((word) *lp.b++ == *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.b++ == *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = ((float) *lp.b++ == *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.b++ == *rp.d);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b++ == rp.cf->real && rp.cf->imaginary == 0);
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b++ == rp.cd->real && rp.cd->imaginary == 0);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ == (word) *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ == *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.w++ == *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = ((float) *lp.w++ == *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.w++ == *rp.d);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ == rp.cf->real && rp.cf->imaginary == 0);
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ == rp.cd->real && rp.cd->imaginary == 0);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ == (int) *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ == (int) *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ == *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = ((float) *lp.l++ == *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.l++ == *rp.d);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ == rp.cf->real && rp.cf->imaginary == 0);
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ == rp.cd->real && rp.cd->imaginary == 0);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ == (float) *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ == (float) *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ == (float) *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ == *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.f++ == *rp.d);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ == rp.cf->real && rp.cf->imaginary == 0);
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ == rp.cd->real && rp.cd->imaginary == 0);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ == (double) *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ == (double) *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ == (double) *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ == (double) *rp.f); 
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ == *rp.d);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ == rp.cf->real && rp.cf->imaginary == 0);
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ == rp.cd->real && rp.cd->imaginary == 0);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (lp.cf->real == *rp.b && lp.cf->imaginary == 0);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (lp.cf->real == *rp.w && lp.cf->imaginary == 0);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (lp.cf->real == *rp.l && lp.cf->imaginary == 0);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (lp.cf->real == *rp.f && lp.cf->imaginary == 0);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (lp.cf->real == *rp.d && lp.cf->imaginary == 0);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--)
	    *tp.l++ = (lp.cf->real == rp.cf->real
		       && lp.cf->imaginary == rp.cf->imaginary);
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (lp.cd->real == rp.cf->real
		       && lp.cd->imaginary == rp.cf->imaginary);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (lp.cd->real == *rp.b && lp.cd->imaginary == 0);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (lp.cd->real == *rp.w && lp.cd->imaginary == 0);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (lp.cd->real == *rp.l && lp.cd->imaginary == 0);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (lp.cd->real == *rp.f && lp.cd->imaginary == 0);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (lp.cd->real == *rp.d && lp.cd->imaginary == 0);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--)
	    *tp.l++ = (lp.cd->real == rp.cf->real
		       && lp.cd->imaginary == rp.cf->imaginary);
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (lp.cd->real == rp.cf->real
		       && lp.cd->imaginary == rp.cf->imaginary);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_eq_sa(void)
     /* equal-to with scalar LHS and array RHS; a commutative operation, */
     /* so we swap LHS and RHS and pass on to ana_eq_as() */
{
  int		temp;
  pointer	tempp;
  
  temp = lhsType;
  lhsType = rhsType;
  rhsType = temp;
  tempp = lp;
  lp = rp;
  rp = tempp;
  ana_eq_as();
  temp = lhsType;
  lhsType = rhsType;
  rhsType = temp;
  tempp = lp;
  lp = rp;
  rp = tempp;
}
/*----------------------------------------------------------*/
void ana_gt(void)
     /* greater-than with two array operands */
{
  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b++ > *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = ((word) *lp.b++ > *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.b++ > *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = ((float) *lp.b++ > *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.b++ > *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.b**lp.b > rp.cf->real*rp.cf->real);
	    lp.b++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.b**lp.b > rp.cd->real*rp.cd->real);
	    lp.b++;
	    rp.cd++;
	  }
	  break;	  
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ > (word) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ > *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.w++ > *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = ((float) *lp.w++ > *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.w++ > *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.w**lp.w > rp.cf->real*rp.cf->real);
	    lp.w++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.w**lp.w > rp.cd->real*rp.cd->real);
	    lp.w++;
	    rp.cd++;
	  }
	  break;	  
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ > (int) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ > (int) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ > *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = ((float) *lp.l++ > *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.l++ > *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.l**lp.l > rp.cf->real*rp.cf->real);
	    lp.l++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.l**lp.l > rp.cd->real*rp.cd->real);
	    lp.l++;
	    rp.cd++;
	  }
	  break;	  
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ > (float) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ > (float) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ > (float) *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ > *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.f++ > *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.f**lp.f > rp.cf->real*rp.cf->real);
	    lp.f++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.f**lp.f > rp.cd->real*rp.cd->real);
	    lp.f++;
	    rp.cd++;
	  }
	  break;	  
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ > (double) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ > (double) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ > (double) *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ > (double) *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ > *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.d**lp.d > rp.cf->real*rp.cf->real);
	    lp.d++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.d**lp.d > rp.cd->real*rp.cd->real);
	    lp.d++;
	    rp.cd++;
	  }
	  break;	  
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary > *rp.b**rp.b);
	    lp.cf++;
	    rp.b++;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary > *rp.w**rp.w);
	    lp.cf++;
	    rp.w++;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary > *rp.l**rp.l);
	    lp.cf++;
	    rp.l++;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary > *rp.f**rp.f);
	    lp.cf++;
	    rp.f++;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary > *rp.d**rp.d);
	    lp.cf++;
	    rp.d++;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary >
		       rp.cf->real*rp.cf->real
		       + rp.cf->imaginary*rp.cf->imaginary);
	    lp.cf++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary >
		       rp.cd->real*rp.cd->real
		       + rp.cd->imaginary*rp.cd->imaginary);
	    lp.cf++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary > *rp.b**rp.b);
	    lp.cd++;
	    rp.b++;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary > *rp.w**rp.w);
	    lp.cd++;
	    rp.w++;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary > *rp.l**rp.l);
	    lp.cd++;
	    rp.l++;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary > *rp.f**rp.f);
	    lp.cd++;
	    rp.f++;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary > *rp.d**rp.d);
	    lp.cd++;
	    rp.d++;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary
		       > rp.cf->real*rp.cf->real
		       + rp.cf->imaginary*rp.cf->imaginary);
	    lp.cd++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary
		       > rp.cd->real*rp.cd->real
		       + rp.cd->imaginary*rp.cd->imaginary);
	    lp.cd++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_STRING_ARRAY:
      if (rhsType == ANA_STRING_ARRAY) {
	while (nRepeat--)
	  *tp.l++ = strcmp(*lp.sp++, *rp.sp++) > 0;
	break;
      }
      /* else fall through to default */
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_gt_as(void)
     /* greater-than with array LHS and scalar RHS */
{
  double	value;

  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b++ > *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = ((word) *lp.b++ > *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.b++ > *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = ((float) *lp.b++ > *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.b++ > *rp.d);
	  break;
	case ANA_CFLOAT:
	  value = rp.cf->real*rp.cf->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.b**lp.b > value);
	    lp.b++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value = rp.cd->real*rp.cd->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.b**lp.b > value);
	    lp.b++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ > (word) *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ > *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.w++ > *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = ((float) *lp.w++ > *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.w++ > *rp.d);
	  break;
	case ANA_CFLOAT:
	  value = rp.cf->real*rp.cf->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.w**lp.w > value);
	    lp.w++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value = rp.cd->real*rp.cd->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.w**lp.w > value);
	    lp.w++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ > (int) *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ > (int) *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ > *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = ((float) *lp.l++ > *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.l++ > *rp.d);
	  break;
	case ANA_CFLOAT:
	  value = rp.cf->real*rp.cf->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.l**lp.l > value);
	    lp.l++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value = rp.cd->real*rp.cd->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.l**lp.l > value);
	    lp.l++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ > (float) *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ > (float) *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ > (float) *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ > *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.f++ > *rp.d);
	  break;
	case ANA_CFLOAT:
	  value = rp.cf->real*rp.cf->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.f**lp.f > value);
	    lp.f++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value = rp.cd->real*rp.cd->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.f**lp.f > value);
	    lp.f++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ > (double) *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ > (double) *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ > (double) *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ > (double) *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ > *rp.d);
	  break;
	case ANA_CFLOAT:
	  value = rp.cf->real*rp.cf->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.d**lp.d > value);
	    lp.d++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value = rp.cd->real*rp.cd->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.d**lp.d > value);
	    lp.d++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary > *rp.b**rp.b);
	    lp.cf++;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary > *rp.w**rp.w);
	    lp.cf++;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary > *rp.l**rp.l);
	    lp.cf++;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary > *rp.f**rp.f);
	    lp.cf++;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary > *rp.d**rp.d);
	    lp.cf++;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary >
		       rp.cf->real*rp.cf->real
		       + rp.cf->imaginary*rp.cf->imaginary);
	    lp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary >
		       rp.cd->real*rp.cd->real
		       + rp.cd->imaginary*rp.cd->imaginary);
	    lp.cf++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary > *rp.b**rp.b);
	    lp.cd++;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary > *rp.w**rp.w);
	    lp.cd++;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary > *rp.l**rp.l);
	    lp.cd++;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary > *rp.f**rp.f);
	    lp.cd++;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary > *rp.d**rp.d);
	    lp.cd++;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary
		       > rp.cf->real*rp.cf->real
		       + rp.cf->imaginary*rp.cf->imaginary);
	    lp.cd++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary
		       > rp.cd->real*rp.cd->real
		       + rp.cd->imaginary*rp.cd->imaginary);
	    lp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_gt_sa(void)
     /* greater-than with scalar LHS and array RHS */
{
  double	value;

  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b > *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = ((word) *lp.b > *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.b > *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = ((float) *lp.b > *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.b > *rp.d++);
	  break;
	case ANA_CFLOAT:
	  value = rp.cf->real*rp.cf->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.b**lp.b > value);
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value = rp.cd->real*rp.cd->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.b**lp.b > value);
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w > (word) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w > *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.w > *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = ((float) *lp.w > *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.w > *rp.d++);
	  break;
	case ANA_CFLOAT:
	  value = rp.cf->real*rp.cf->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.w**lp.w > value);
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value = rp.cd->real*rp.cd->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.w**lp.w > value);
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l > (int) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l > (int) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l > *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = ((float) *lp.l > *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.l > *rp.d++);
	  break;
	case ANA_CFLOAT:
	  value = rp.cf->real*rp.cf->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.l**lp.l > value);
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value = rp.cd->real*rp.cd->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.l**lp.l > value);
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f > (float) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f > (float) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f > (float) *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f > *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.f > *rp.d++);
	  break;
	case ANA_CFLOAT:
	  value = rp.cf->real*rp.cf->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.f**lp.f > value);
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value = rp.cd->real*rp.cd->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.f**lp.f > value);
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d > (double) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d > (double) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d > (double) *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d > (double) *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d > *rp.d++);
	  break;
	case ANA_CFLOAT:
	  value = rp.cf->real*rp.cf->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.d**lp.d > value);
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value = rp.cd->real*rp.cd->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.d**lp.d > value);
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary > *rp.b**rp.b);
	    rp.b++;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary > *rp.w**rp.w);
	    rp.w++;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary > *rp.l**rp.l);
	    rp.l++;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary > *rp.f**rp.f);
	    rp.f++;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary > *rp.d**rp.d);
	    rp.d++;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary >
		       rp.cf->real*rp.cf->real
		       + rp.cf->imaginary*rp.cf->imaginary);
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary >
		       rp.cd->real*rp.cd->real
		       + rp.cd->imaginary*rp.cd->imaginary);
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary > *rp.b**rp.b);
	    rp.b++;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary > *rp.w**rp.w);
	    rp.w++;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary > *rp.l**rp.l);
	    rp.l++;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary > *rp.f**rp.f);
	    rp.f++;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary > *rp.d**rp.d);
	    rp.d++;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary
		       > rp.cf->real*rp.cf->real
		       + rp.cf->imaginary*rp.cf->imaginary);
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary
		       > rp.cd->real*rp.cd->real
		       + rp.cd->imaginary*rp.cd->imaginary);
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_ge(void)
     /* greater-than-or-equal-to with array operands */
{
  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b++ >= *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = ((word) *lp.b++ >= *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.b++ >= *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = ((float) *lp.b++ >= *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.b++ >= *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.b**lp.b >= rp.cf->real*rp.cf->real);
	    lp.b++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.b**lp.b >= rp.cd->real*rp.cd->real);
	    lp.b++;
	    rp.cd++;
	  }
	  break;	  
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ >= (word) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ >= *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.w++ >= *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = ((float) *lp.w++ >= *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.w++ >= *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.w**lp.w >= rp.cf->real*rp.cf->real);
	    lp.w++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.w**lp.w >= rp.cd->real*rp.cd->real);
	    lp.w++;
	    rp.cd++;
	  }
	  break;	  
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ >= (int) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ >= (int) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ >= *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = ((float) *lp.l++ >= *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.l++ >= *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.l**lp.l >= rp.cf->real*rp.cf->real);
	    lp.l++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.l**lp.l >= rp.cd->real*rp.cd->real);
	    lp.l++;
	    rp.cd++;
	  }
	  break;	  
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ >= (float) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ >= (float) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ >= (float) *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ >= *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.f++ >= *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.f**lp.f >= rp.cf->real*rp.cf->real);
	    lp.f++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.f**lp.f >= rp.cd->real*rp.cd->real);
	    lp.f++;
	    rp.cd++;
	  }
	  break;	  
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ >= (double) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ >= (double) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ >= (double) *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ >= (double) *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ >= *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.d**lp.d >= rp.cf->real*rp.cf->real);
	    lp.d++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.d**lp.d >= rp.cd->real*rp.cd->real);
	    lp.d++;
	    rp.cd++;
	  }
	  break;	  
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary >= *rp.b**rp.b);
	    lp.cf++;
	    rp.b++;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary >= *rp.w**rp.w);
	    lp.cf++;
	    rp.w++;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary >= *rp.l**rp.l);
	    lp.cf++;
	    rp.l++;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary >= *rp.f**rp.f);
	    lp.cf++;
	    rp.f++;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary >= *rp.d**rp.d);
	    lp.cf++;
	    rp.d++;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary >
		       rp.cf->real*rp.cf->real
		       + rp.cf->imaginary*rp.cf->imaginary);
	    lp.cf++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary >
		       rp.cd->real*rp.cd->real
		       + rp.cd->imaginary*rp.cd->imaginary);
	    lp.cf++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary >= *rp.b**rp.b);
	    lp.cd++;
	    rp.b++;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary >= *rp.w**rp.w);
	    lp.cd++;
	    rp.w++;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary >= *rp.l**rp.l);
	    lp.cd++;
	    rp.l++;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary >= *rp.f**rp.f);
	    lp.cd++;
	    rp.f++;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary >= *rp.d**rp.d);
	    lp.cd++;
	    rp.d++;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary
		       >= rp.cf->real*rp.cf->real
		       + rp.cf->imaginary*rp.cf->imaginary);
	    lp.cd++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary
		       >= rp.cd->real*rp.cd->real
		       + rp.cd->imaginary*rp.cd->imaginary);
	    lp.cd++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_STRING_ARRAY:
      if (rhsType == ANA_STRING_ARRAY) {
	while (nRepeat--)
	  *tp.l++ = strcmp(*lp.sp++, *rp.sp++) >= 0;
	break;
      }
      /* else fall through to default */
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_ge_as(void)
     /* greater-than-or-equal-to with array LHS and scalar RHS */
{
  double	value;

  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b++ >= *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = ((word) *lp.b++ >= *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.b++ >= *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = ((float) *lp.b++ >= *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.b++ >= *rp.d);
	  break;
	case ANA_CFLOAT:
	  value = rp.cf->real*rp.cf->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.b**lp.b >= value);
	    lp.b++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value = rp.cd->real*rp.cd->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.b**lp.b >= value);
	    lp.b++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ >= (word) *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ >= *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.w++ >= *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = ((float) *lp.w++ >= *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.w++ >= *rp.d);
	  break;
	case ANA_CFLOAT:
	  value = rp.cf->real*rp.cf->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.w**lp.w >= value);
	    lp.w++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value = rp.cd->real*rp.cd->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.w**lp.w >= value);
	    lp.w++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ >= (int) *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ >= (int) *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ >= *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = ((float) *lp.l++ >= *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.l++ >= *rp.d);
	  break;
	case ANA_CFLOAT:
	  value = rp.cf->real*rp.cf->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.l**lp.l >= value);
	    lp.l++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value = rp.cd->real*rp.cd->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.l**lp.l >= value);
	    lp.l++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ >= (float) *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ >= (float) *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ >= (float) *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ >= *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.f++ >= *rp.d);
	  break;
	case ANA_CFLOAT:
	  value = rp.cf->real*rp.cf->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.f**lp.f >= value);
	    lp.f++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value = rp.cd->real*rp.cd->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.f**lp.f >= value);
	    lp.f++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ >= (double) *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ >= (double) *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ >= (double) *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ >= (double) *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ >= *rp.d);
	  break;
	case ANA_CFLOAT:
	  value = rp.cf->real*rp.cf->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.d**lp.d >= value);
	    lp.d++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value = rp.cd->real*rp.cd->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.d**lp.d >= value);
	    lp.d++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary >= *rp.b**rp.b);
	    lp.cf++;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary >= *rp.w**rp.w);
	    lp.cf++;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary >= *rp.l**rp.l);
	    lp.cf++;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary >= *rp.f**rp.f);
	    lp.cf++;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary >= *rp.d**rp.d);
	    lp.cf++;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary >
		       rp.cf->real*rp.cf->real
		       + rp.cf->imaginary*rp.cf->imaginary);
	    lp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary >
		       rp.cd->real*rp.cd->real
		       + rp.cd->imaginary*rp.cd->imaginary);
	    lp.cf++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary >= *rp.b**rp.b);
	    lp.cd++;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary >= *rp.w**rp.w);
	    lp.cd++;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary >= *rp.l**rp.l);
	    lp.cd++;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary >= *rp.f**rp.f);
	    lp.cd++;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary >= *rp.d**rp.d);
	    lp.cd++;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary
		       >= rp.cf->real*rp.cf->real
		       + rp.cf->imaginary*rp.cf->imaginary);
	    lp.cd++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary
		       >= rp.cd->real*rp.cd->real
		       + rp.cd->imaginary*rp.cd->imaginary);
	    lp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_ge_sa(void)
     /* greater-than-or-equal-to with scalar LHS and array RHS */
{
  double	value;

  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b >= *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = ((word) *lp.b >= *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.b >= *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = ((float) *lp.b >= *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.b >= *rp.d++);
	  break;
	case ANA_CFLOAT:
	  value = rp.cf->real*rp.cf->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.b**lp.b >= value);
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value = rp.cd->real*rp.cd->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.b**lp.b >= value);
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w >= (word) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w >= *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.w >= *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = ((float) *lp.w >= *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.w >= *rp.d++);
	  break;
	case ANA_CFLOAT:
	  value = rp.cf->real*rp.cf->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.w**lp.w >= value);
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value = rp.cd->real*rp.cd->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.w**lp.w >= value);
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l >= (int) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l >= (int) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l >= *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = ((float) *lp.l >= *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.l >= *rp.d++);
	  break;
	case ANA_CFLOAT:
	  value = rp.cf->real*rp.cf->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.l**lp.l >= value);
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value = rp.cd->real*rp.cd->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.l**lp.l >= value);
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f >= (float) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f >= (float) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f >= (float) *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f >= *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.f >= *rp.d++);
	  break;
	case ANA_CFLOAT:
	  value = rp.cf->real*rp.cf->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.f**lp.f >= value);
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value = rp.cd->real*rp.cd->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.f**lp.f >= value);
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d >= (double) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d >= (double) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d >= (double) *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d >= (double) *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d >= *rp.d++);
	  break;
	case ANA_CFLOAT:
	  value = rp.cf->real*rp.cf->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.d**lp.d >= value);
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  value = rp.cd->real*rp.cd->real;
	  while (nRepeat--) {
	    *tp.l++ = (*lp.d**lp.d >= value);
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary >= *rp.b**rp.b);
	    rp.b++;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary >= *rp.w**rp.w);
	    rp.w++;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary >= *rp.l**rp.l);
	    rp.l++;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary >= *rp.f**rp.f);
	    rp.f++;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary >= *rp.d**rp.d);
	    rp.d++;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary >
		       rp.cf->real*rp.cf->real
		       + rp.cf->imaginary*rp.cf->imaginary);
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real*lp.cf->real
		       + lp.cf->imaginary*lp.cf->imaginary >
		       rp.cd->real*rp.cd->real
		       + rp.cd->imaginary*rp.cd->imaginary);
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary >= *rp.b**rp.b);
	    rp.b++;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary >= *rp.w**rp.w);
	    rp.w++;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary >= *rp.l**rp.l);
	    rp.l++;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary >= *rp.f**rp.f);
	    rp.f++;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary >= *rp.d**rp.d);
	    rp.d++;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary
		       >= rp.cf->real*rp.cf->real
		       + rp.cf->imaginary*rp.cf->imaginary);
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real*lp.cd->real
		       + lp.cd->imaginary*lp.cd->imaginary
		       >= rp.cd->real*rp.cd->real
		       + rp.cd->imaginary*rp.cd->imaginary);
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_lt(void)
     /* less-than with array operands.  This operator is the mirror image */
     /* of greater-than, so we swap LHS and RHS and pass on to ana_gt() */
{
  int	temp;
  pointer	tempp;
  
  temp = lhsType;  lhsType = rhsType;  rhsType = temp;
  tempp = lp;  lp = rp;  rp = tempp;
  ana_gt();
  temp = lhsType;  lhsType = rhsType;  rhsType = temp;
  tempp = lp;  lp = rp;  rp = tempp;
}
/*----------------------------------------------------------*/
void ana_lt_as(void)
     /* less-than with array LHS and scalar RHS.  This operand is the */
     /* mirror image of ana_gt_as(), so swap LHS and RHS and use that */
{
  int		temp;
  pointer	tempp;
  
  temp = lhsType;  lhsType = rhsType;  rhsType = temp;
  tempp = lp;  lp = rp;  rp = tempp;
  ana_gt_sa();
  temp = lhsType;  lhsType = rhsType;  rhsType = temp;
  tempp = lp;  lp = rp;  rp = tempp;
}
/*----------------------------------------------------------*/
void ana_lt_sa(void)
     /* less-than with scalar LHS and array RHS.  This operand is the mirror */
     /* image of ana_gt_sa() so we swap LHS and RHS and use that */
{
  int		temp;
  pointer	tempp;
  
  temp = lhsType;  lhsType = rhsType;  rhsType = temp;
  tempp = lp;  lp = rp;  rp = tempp;
  ana_gt_as();
  temp = lhsType;  lhsType = rhsType;  rhsType = temp;
  tempp = lp;  lp = rp;  rp = tempp;
}
/*----------------------------------------------------------*/
void ana_le(void)
     /* less-than with array operands.  Mirror image of ana_ge(), so */
     /* swap operands and use that */
{
  int		temp;
  pointer	tempp;
  
  temp = lhsType;  lhsType = rhsType;  rhsType = temp;
  tempp = lp;  lp = rp;  rp = tempp;
  ana_ge();
  temp = lhsType;  lhsType = rhsType;  rhsType = temp;
  tempp = lp;  lp = rp;  rp = tempp;
}
/*----------------------------------------------------------*/
void ana_le_as(void)
     /* less-than with array LHS and scalar RHS.  Mirror image of */
     /* ana_ge_as() so swap operands and use that */
{
  int		temp;
  pointer	tempp;
  
  temp = lhsType;  lhsType = rhsType;  rhsType = temp;
  tempp = lp;  lp = rp;  rp = tempp;
  ana_ge_sa();
  temp = lhsType;  lhsType = rhsType;  rhsType = temp;
  tempp = lp;  lp = rp;  rp = tempp;
}
/*----------------------------------------------------------*/
void ana_le_sa(void)
     /* less-than with scalar LHS and array RHS.  Mirror image of */
     /* ana_ge_sa() so swap operands and use that */
{
  int		temp;
  pointer	tempp;
  
  temp = lhsType;  lhsType = rhsType;  rhsType = temp;
  tempp = lp;  lp = rp;  rp = tempp;
  ana_ge_as();
  temp = lhsType;  lhsType = rhsType;  rhsType = temp;
  tempp = lp;  lp = rp;  rp = tempp;
}
/*----------------------------------------------------------*/
void ana_ne(void)
     /* not-equal-to with array operands */
{
  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b++ != *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b++ != *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b++ != *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b++ != *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b++ != *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.b != rp.cf->real || rp.cf->imaginary != 0);
	    lp.b++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.b != rp.cd->real || rp.cd->imaginary != 0);
	    lp.b++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ != (word) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ != (word) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ != (word) *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ != (word) *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ != (word) *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.w != rp.cf->real || rp.cf->imaginary != 0);
	    lp.w++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.w != rp.cd->real || rp.cd->imaginary != 0);
	    lp.w++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ != (int) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ != (int) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ != *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = ((float) *lp.l++ != *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.l++ != *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.l != rp.cf->real || rp.cf->imaginary != 0);
	    lp.l++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.l != rp.cd->real || rp.cd->imaginary != 0);
	    lp.l++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ != (float) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ != (float) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ != (float) *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ != *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.f++ != *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.f != rp.cf->real || rp.cf->imaginary != 0);
	    lp.f++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.f != rp.cd->real || rp.cd->imaginary != 0);
	    lp.f++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ != (double) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ != (double) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ != (double) *rp.l++);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ != (double) *rp.f++);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ != *rp.d++);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.d != rp.cf->real || rp.cf->imaginary != 0);
	    lp.d++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (*lp.d != rp.cd->real || rp.cd->imaginary != 0);
	    lp.d++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real != *rp.b || lp.cf->imaginary != 0);
	    lp.cf++;
	    rp.b++;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real != *rp.w || lp.cf->imaginary != 0);
	    lp.cf++;
	    rp.w++;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real != *rp.l || lp.cf->imaginary != 0);
	    lp.cf++;
	    rp.l++;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real != *rp.f || lp.cf->imaginary != 0);
	    lp.cf++;
	    rp.f++;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real != *rp.d || lp.cf->imaginary != 0);
	    lp.cf++;
	    rp.d++;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real != rp.cf->real
		       || lp.cf->imaginary != rp.cf->imaginary);
	    lp.cf++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cf->real != rp.cd->real
		       || lp.cf->imaginary != rp.cd->imaginary);
	    lp.cf++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real != *rp.b || lp.cd->imaginary != 0);
	    lp.cd++;
	    rp.b++;
	  }
	  break;
	case ANA_WORD:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real != *rp.w || lp.cd->imaginary != 0);
	    lp.cd++;
	    rp.w++;
	  }
	  break;
	case ANA_LONG:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real != *rp.l || lp.cd->imaginary != 0);
	    lp.cd++;
	    rp.l++;
	  }
	  break;
	case ANA_FLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real != *rp.f || lp.cd->imaginary != 0);
	    lp.cd++;
	    rp.f++;
	  }
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real != *rp.d || lp.cd->imaginary != 0);
	    lp.cd++;
	    rp.d++;
	  }
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real != rp.cf->real
		       || lp.cd->imaginary != rp.cf->imaginary);
	    lp.cd++;
	    rp.cf++;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--) {
	    *tp.l++ = (lp.cd->real != rp.cd->real
		       || lp.cd->imaginary != rp.cd->imaginary);
	    lp.cd++;
	    rp.cd++;
	  }
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_STRING_ARRAY:
      if (rhsType == ANA_STRING_ARRAY) {
	while (nRepeat--)
	  *tp.l++ = strcmp(*lp.sp++, *rp.sp++) != 0;
	break;
      }
      /* else fall through to default */
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_ne_as(void)
     /* not-equal-to with array LHS and scalar RHS */
{
  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b++ != *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = ((word) *lp.b++ != *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.b++ != *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = ((float) *lp.b++ != *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.b++ != *rp.d);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b++ != rp.cf->real || rp.cf->imaginary != 0);
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b++ != rp.cd->real || rp.cd->imaginary != 0);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ != (word) *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ != *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.w++ != *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = ((float) *lp.w++ != *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.w++ != *rp.d);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ != rp.cf->real || rp.cf->imaginary != 0);
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ != rp.cd->real || rp.cd->imaginary != 0);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ != (int) *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ != (int) *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ != *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = ((float) *lp.l++ != *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.l++ != *rp.d);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ != rp.cf->real || rp.cf->imaginary != 0);
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ != rp.cd->real || rp.cd->imaginary != 0);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_FLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ != (float) *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ != (float) *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ != (float) *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ != *rp.f);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = ((double) *lp.f++ != *rp.d);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ != rp.cf->real || rp.cf->imaginary != 0);
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.f++ != rp.cd->real || rp.cd->imaginary != 0);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_DOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ != (double) *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ != (double) *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ != (double) *rp.l);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ != (double) *rp.f); 
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ != *rp.d);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ != rp.cf->real || rp.cf->imaginary != 0);
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.d++ != rp.cd->real || rp.cd->imaginary != 0);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CFLOAT:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (lp.cf->real != *rp.b || lp.cf->imaginary != 0);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (lp.cf->real != *rp.w || lp.cf->imaginary != 0);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (lp.cf->real != *rp.l || lp.cf->imaginary != 0);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (lp.cf->real != *rp.f || lp.cf->imaginary != 0);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (lp.cf->real != *rp.d || lp.cf->imaginary != 0);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--)
	    *tp.l++ = (lp.cf->real != rp.cf->real
		       || lp.cf->imaginary != rp.cf->imaginary);
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (lp.cd->real != rp.cf->real
		       || lp.cd->imaginary != rp.cf->imaginary);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_CDOUBLE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (lp.cd->real != *rp.b || lp.cd->imaginary != 0);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (lp.cd->real != *rp.w || lp.cd->imaginary != 0);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (lp.cd->real != *rp.l || lp.cd->imaginary != 0);
	  break;
	case ANA_FLOAT:
	  while (nRepeat--)
	    *tp.l++ = (lp.cd->real != *rp.f || lp.cd->imaginary != 0);
	  break;
	case ANA_DOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (lp.cd->real != *rp.d || lp.cd->imaginary != 0);
	  break;
	case ANA_CFLOAT:
	  while (nRepeat--)
	    *tp.l++ = (lp.cd->real != rp.cf->real
		       || lp.cd->imaginary != rp.cf->imaginary);
	  break;
	case ANA_CDOUBLE:
	  while (nRepeat--)
	    *tp.l++ = (lp.cd->real != rp.cf->real
		       || lp.cd->imaginary != rp.cf->imaginary);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_ne_sa(void)
     /* not-equal-to with scalar LHS and array RHS.  Operator is */
     /* commutative, so swap operands and use ana_ne_as() */
{
  int		temp;
  pointer	tempp;
  
  temp = lhsType;  lhsType = rhsType;  rhsType = temp;
  tempp = lp;  lp = rp;  rp = tempp;
  ana_ne_as();
  temp = lhsType;  lhsType = rhsType;  rhsType = temp;
  tempp = lp;  lp = rp;  rp = tempp;
}
/*----------------------------------------------------------*/
void ana_and(void)
     /* logical-and with array operands */
{
  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b++ & *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = ((word) *lp.b++ & *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.b++ & *rp.l++);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ & (word) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ & *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.w++ & *rp.l++);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ & (int) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ & (int) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ & *rp.l++);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_and_as(void)
     /* logical-and with array LHS and scalar RHS */
{
  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b++ & *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = ((word) *lp.b++ & *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.b++ & *rp.l);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ & (word) *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ & *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.w++ & *rp.l);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ & (int) *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ & (int) *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ & *rp.l);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_and_sa(void)
     /* logical-and with scalar LHS and array RHS.  Operator is commutative, */
     /* so swap operands and use ana_and_as() */
{
  int		temp;
  pointer	tempp;
  
  temp = lhsType;  lhsType = rhsType;  rhsType = temp;
  tempp = lp;  lp = rp;  rp = tempp;
  ana_and_as();
  temp = lhsType;  lhsType = rhsType;  rhsType = temp;
  tempp = lp;  lp = rp;  rp = tempp;
}
/*----------------------------------------------------------*/
void ana_or(void)
     /* logical-or with array operands */
{
  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b++ | *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = ((word) *lp.b++ | *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.b++ | *rp.l++);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ | (word) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ | *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.w++ | *rp.l++);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ | (int) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ | (int) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ | *rp.l++);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_or_as(void)
     /* logical-or with array LHS and scalar RHS */
{
  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b++ | *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = ((word) *lp.b++ | *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.b++ | *rp.l);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ | (word) *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ | *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.w++ | *rp.l);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ | (int) *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ | (int) *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ | *rp.l);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_or_sa(void)
     /* logical-or with scalar LHS and array RHS.  Operator is */
     /* commutative, so swap operands and use ana_or_as() */
{
  int		temp;
  pointer	tempp;
  
  temp = lhsType;  lhsType = rhsType;  rhsType = temp;
  tempp = lp;  lp = rp;  rp = tempp;
  ana_or_as();
  temp = lhsType;  lhsType = rhsType;  rhsType = temp;
  tempp = lp;  lp = rp;  rp = tempp;
}
/*----------------------------------------------------------*/
void ana_xor(void)
     /* logical-exclusive-or with array operands */
{
  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b++ ^ *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = ((word) *lp.b++ ^ *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.b++ ^ *rp.l++);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ ^ (word) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ ^ *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.w++ ^ *rp.l++);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ ^ (int) *rp.b++);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ ^ (int) *rp.w++);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ ^ *rp.l++);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_xor_as(void)
     /* logical-exclusive-or with array LHS and scalar RHS */
{
  switch (lhsType) {
    case ANA_BYTE:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.b++ ^ *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = ((word) *lp.b++ ^ *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.b++ ^ *rp.l);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_WORD:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ ^ (word) *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.w++ ^ *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = ((int) *lp.w++ ^ *rp.l);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    case ANA_LONG:
      switch (rhsType) {
	case ANA_BYTE:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ ^ (int) *rp.b);
	  break;
	case ANA_WORD:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ ^ (int) *rp.w);
	  break;
	case ANA_LONG:
	  while (nRepeat--)
	    *tp.l++ = (*lp.l++ ^ *rp.l);
	  break;
	default:
	  cerror(ILL_TYPE, rhs, typeName(rhsType));
      }
      break;
    default:
      cerror(ILL_TYPE, lhs, typeName(lhsType));
  }
}
/*----------------------------------------------------------*/
void ana_xor_sa(void)
     /* logical-exlusive-or with scalar LHS and array RHS.  Operator is */
     /* commutative, so swap operands and use ana_xor_as() */
{
  int		temp;
  pointer	tempp;
  
  temp = lhsType;  lhsType = rhsType;  rhsType = temp;
  tempp = lp;  lp = rp;  rp = tempp;
  ana_xor_as();
  temp = lhsType;  lhsType = rhsType;  rhsType = temp;
  tempp = lp;  lp = rp;  rp = tempp;
}
/*----------------------------------------------------------*/
int ana_string_add(void)
     /* add (i.e. concatenate) two strings */
{
  int	result, i;
  
  i = string_size(lhs) + string_size(rhs); /* size of result */
  result = string_scratch(i);	/* get result symbol */
  tp.s = string_value(result);	/* result data */
  strcpy(tp.s, string_value(lhs)); /* copy LHS */
  strcat(tp.s, string_value(rhs)); /* append RHS */
  return result;
}
/*----------------------------------------------------------*/
/* array-array binary operations */
static void (*binFunc[])(void) = {
  ana_add, ana_sub, ana_mul, ana_div, ana_idiv, ana_mod, ana_smod, ana_max,
  ana_min, ana_eq, ana_gt, ana_ge, ana_lt, ana_le, ana_ne, ana_or,
  ana_and, ana_xor, ana_bin_pow
};

/* scalar-array binary operations */
static void (*binFunc_sa[])(void) = {
  ana_add_sa, ana_sub_sa, ana_mul_sa, ana_div_sa, ana_idiv_sa, ana_mod_sa,
  ana_smod_sa, ana_max_sa, ana_min_sa, ana_eq_sa, ana_gt_sa, ana_ge_sa,
  ana_lt_sa, ana_le_sa, ana_ne_sa, ana_or_sa, ana_and_sa, ana_xor_sa,
  ana_pow_sa
};

/* array-scalar binary operations */
static void (*binFunc_as[])(void) = {
  ana_add_as, ana_sub_as, ana_mul_as, ana_div_as, ana_idiv_as, ana_mod_as,
  ana_smod_as, ana_max_as, ana_min_as, ana_eq_as, ana_gt_as,
  ana_ge_as, ana_lt_as, ana_le_as, ana_ne_as, ana_or_as,
  ana_and_as, ana_xor_as, ana_pow_as
};

/*----------------------------------------------------------*/
int evalScalarBinOp(void)
/* evaluate binary operation with scalar operands.
 return value: symbol number of newly created output symbol
*/
{
  int	result;
  
  result = scalar_scratch(topType); /* get symbol for result */
  if (isComplexType(lhsType))
    lp.cf = complex_scalar_data(lhs).cf;
  else
    lp.b = &scalar_value(lhs).b; /* pointer to LHS value */
  if (isComplexType(rhsType))
    rp.cf = complex_scalar_data(rhs).cf;
  else
    rp.b = &scalar_value(rhs).b;
  if (isComplexType(topType)) 	/* a complex scalar */
    tp.cf = complex_scalar_data(result).cf;
  else
    tp.b = &scalar_value(result).b;
  nRepeat = 1;			/* only one value to calculate */
  (*binFunc[binOp])();		/* calculate */
  return result;			/* done */
}
/*----------------------------------------------------------*/
int evalScalarArrayBinOp(void)
/* evaluate binary operation with scalar as left-hand operand and
   array as right-hand operand
   return value: number of newly created output symbol */
{
  int	result;
  
  if (array_type(rhs) == topType && isFreeTemp(rhs))
    /* we can overwrite the array with the results */
    result = rhs;
  else if ((result = array_clone(rhs, topType)) < 0)
    /* could not create an output symbol */
    return ANA_ERROR;
  if (isComplexType(scalar_type(lhs)))
    lp.cf = complex_scalar_data(lhs).cf;
  else
    lp.b = &scalar_value(lhs).b;
  rp.l = array_data(rhs);
  tp.l = array_data(result);
  nRepeat = array_size(rhs);
  (*binFunc_sa[binOp])();
  return result;
}
/*----------------------------------------------------------*/
int evalArrayScalarBinOp(void)
/* evaluate binary operation with array as left-hand operand and
 scalar as right-hand operand.  Return value: number of newly created
 output symbol */
{
  int	result;
  
  if (array_type(lhs) == topType && (isFreeTemp(lhs)))
    /* we can overwrite the array with the results */
    result = lhs; 
  else if ((result = array_clone(lhs, topType)) < 0)
    /* could not creat an output symbol */
    return ANA_ERROR;
  if (isComplexType(scalar_type(rhs)))
    rp.cf = complex_scalar_data(rhs).cf;
  else
    rp.b = &scalar_value(rhs).b;
  lp.l = array_data(lhs);
  tp.l = array_data(result);
  nRepeat = array_size(lhs);
  (*binFunc_as[binOp])();
  return result;
}
/*----------------------------------------------------------*/
#define ORDINARY	1
#define SCALAR_LEFT	2
#define SCALAR_RIGHT	3
int evalArrayBinOp(void)
/* supports "implicit dimensions", i.e. dimensions which are 1 in one of */
/* the operands and non-1 in the other.  The smaller operand is repeated */
/* as needed to service all elements of the larger operand. */
{
  int	result, i, i0, nRepeats[MAX_DIMS], action[MAX_DIMS], nAction = 0,
    tally[MAX_DIMS], nCumulR[MAX_DIMS], nCumulL[MAX_DIMS], ndim,
    bigOne;
  extern int	pipeSym, pipeExec;
  
  /* the arrays must have an equal number of dimensions, except for */
  /* possible trailing dimensions of one element */
  if (array_num_dims(rhs) > array_num_dims(lhs)) {/* rhs has more dims */
    ndim = array_num_dims(lhs);	/* number of dimensions in output */
    bigOne = rhs;		/* rhs has more dims than lhs */
    for (i = ndim; i < array_num_dims(rhs); i++)
      if (array_dims(rhs)[i] != 1)
	return cerror(INCMP_DIMS, rhs);
  } else {
    ndim = array_num_dims(rhs);
    bigOne = (array_num_dims(rhs) < array_num_dims(lhs))? lhs: 0;
				/* 0 indicates lhs has same #dims as rhs */
    for (i = array_num_dims(rhs); i < array_num_dims(lhs); i++)
      if (array_dims(lhs)[i] != 1)
	return cerror(INCMP_DIMS, rhs);
  }

  /* now we figure out how to treat the operands */
  /* nRepeats[] will contain the number of repeats of each type of action */
  /* action[] will contain the type of action to take:
     ORDINARY for a block of dimensions that are all unequal to 1 and
              equal in the lhs to what they are in the rhs
     SCALAR_LEFT for a dimension that is equal to 1 in the lhs and
              unequal to 1 in the rhs
     SCALAR_RIGHT for a dimension that is equal to 1 in the rhs and
              unequal to 1 in the lhs */
  nRepeat = 1;			/* default number of loops */
  lp.l = array_data(lhs);	/* lhs data */
  rp.l = array_data(rhs);	/* rhs data */
  for (i0 = i = 0; i < ndim; i++) {
    if ((array_dims(rhs)[i] == 1) ^ (array_dims(lhs)[i] == 1)) {
      /* one is 1 and the other is not: implicit dimension */
      if (nRepeat > 1) {	/* already had some ordinary dimensions */
	nRepeats[nAction] = nRepeat; /* store combined repeat count */
	action[nAction++] = ORDINARY;
      }
      if (array_dims(rhs)[i] == 1) { /* rhs has dimension equal to 1 */
	action[nAction] = SCALAR_RIGHT;
	nRepeats[nAction] = array_dims(lhs)[i];
      } else {			/* lhs has dimension equal to 1 */
	action[nAction] = SCALAR_LEFT;
	nRepeats[nAction] = array_dims(rhs)[i];
      }
      nAction++;
      nRepeat = 1;		/* reset for ORDINARY count */
    } else if (array_dims(rhs)[i] != array_dims(lhs)[i])
      /* unequal and neither equal to 1 -> error */
      return cerror(INCMP_DIMS, rhs);
    else
      nRepeat *= array_dims(rhs)[i]; /* both equal but not to 1 */
  }
  if (nAction && nRepeat > 1) {	/* some ordinary dimensions at the end */
    nRepeats[nAction] = nRepeat;
    action[nAction++] = ORDINARY;
  }
  if (!nAction) {		/* plain binary operation, no implicit dims */
    if (ana_type_size[array_type(lhs)] == ana_type_size[topType] /* lhs type OK */
	&& (lhs == bigOne || !bigOne) /* lhs is big enough */
	&& (isFreeTemp(lhs) || (!pipeExec && pipeSym == lhs))) /* and free */
      result = lhs;		/* use lhs to store result */
    else if (ana_type_size[array_type(rhs)] == ana_type_size[topType]
	     && (rhs == bigOne || !bigOne)
	     && (isFreeTemp(rhs) || (!pipeExec && pipeSym == rhs)))
      result = rhs;		/* use rhs to store result */
    else if ((result = array_clone(bigOne? bigOne: lhs, topType)) < 0)
      return ANA_ERROR;		/* could not generate output symbol */
    tp.l = array_data(result);	/* output data */
    array_type(result) = topType;
    (*binFunc[binOp])();
    return result;
  } else {				/* implicit dimensions */
    int	lStride, rStride;
    char	done = 0;
    
    /* create result array: first calculate its number of elements */
    nRepeat = 1;
    for (i = 0; i < nAction; i++)
      nRepeat *= nRepeats[i];
    if (ana_type_size[array_type(lhs)] == ana_type_size[topType] /* lhs type OK */
	&& array_size(lhs) == nRepeat /* and has correct size */
	&& (isFreeTemp(lhs) || (!pipeExec && pipeSym == lhs))) /* and free */
      result = lhs;		/* use lhs to store the result */
    else if (ana_type_size[array_type(rhs)] == ana_type_size[topType]
	     && array_size(rhs) == nRepeat
	     && (isFreeTemp(rhs) || (!pipeExec && pipeSym == rhs)))
      result = rhs;		/* use rhs to store the result */
    else if ((result = array_scratch(topType, 1, &nRepeat)) < 0)
      return ANA_ERROR;		/* could not create output symbol */

    /* if the result symbol was created from scratch, then it has
     only a single dimension, which may not be correct.  We put in
     the correct dimensions */
    /* first those up to the smaller of the number of dimensions of
     the lhs and rhs */
    for (i = 0; i < ndim; i++)
      array_dims(result)[i] = MAX(array_dims(lhs)[i], array_dims(rhs)[i]);
    /* and any remaining ones are set equal to 1 */
    for (i = ndim;
	 i < ((bigOne == lhs)? array_num_dims(lhs): array_num_dims(rhs)); i++)
      array_dims(result)[i] = 1;
    array_num_dims(result) = (bigOne == lhs)? array_num_dims(lhs):
      array_num_dims(rhs);

    /* the result data pointer */
    tp.l = array_data(result);
    /* now deduce step sizes */
    *nCumulR = rStride = ana_type_size[rhsType];
    *nCumulL = lStride = ana_type_size[lhsType];

    for (i = 1; i < nAction; i++) { /* cumulative sizes */
      switch (action[i - 1]) {
	case ORDINARY:
	  nCumulR[i] = nCumulR[i - 1]*nRepeats[i - 1];
	  nCumulL[i] = nCumulL[i - 1]*nRepeats[i - 1];
	  break;
	case SCALAR_RIGHT:
	  nCumulL[i] = nCumulL[i - 1]*nRepeats[i - 1];
	  nCumulR[i] = nCumulR[i - 1];
	  break;
	case SCALAR_LEFT:
	  nCumulR[i] = nCumulR[i - 1]*nRepeats[i - 1];
	  nCumulL[i] = nCumulL[i - 1];
	  break;
      }
    }
    /* the binary operation routines (binOp...) do pointer advancement. */
    /* here we only need to reset pointers if we need to repeat certain */
    /* stretches of the lhs or rhs, i.e. when an implicit dimensions is */
    /* encountered.  the nCumuls should indicate by how much to reset the */
    /* pointers, so set them to zero for explicit dimensions. */
    for (i = 0; i < nAction; i++) {
      tally[i] = 1;
      switch (action[i]) {
	case SCALAR_RIGHT:
	  nCumulL[i] = 0;
	  break;
	case SCALAR_LEFT:
	  nCumulR[i] = 0;
	  break;
	case ORDINARY:
	  nCumulR[i] = nCumulL[i] = 0;
	  break;
      }
    }
    /* now the real action */
    do {
      nRepeat = *nRepeats;	/* #elements for subroutine */
      switch (*action) {
	case ORDINARY:
	  (*binFunc[binOp])();
	  break;
	case SCALAR_LEFT:
	  (*binFunc_sa[binOp])();
	  break;
	case SCALAR_RIGHT:
	  (*binFunc_as[binOp])();
	  break;
      }
      rp.b += *nCumulR;		/* if this is an implicit dimension, then */
      /* pointer didn't get advanced in subroutine */
      lp.b += *nCumulL;
      done = 1;
      for (i = 1; i < nAction; i++) {
	if (tally[i]++ != nRepeats[i]) {
	  rp.b -= nCumulR[i]; 	/* adjust pointers for next go */
	  lp.b -= nCumulL[i];
	  done = 0;
	  break;
	}
	tally[i] = 1;
      }
    } while (!done);
    array_type(result) = topType; /* in case we use one of the operands */
    /* for result and the type of the operand */
    /* is different from the type of the result */
    /* (e.g. ANA_LONG -> ANA_FLOAT) */
    return result;
  }
}
/*----------------------------------------------------------*/
int evalStringBinOp(void)
     /* binary operation with two string arguments */
{
  int	result, i;
  
  lp.s = string_value(lhs);
  rp.s = string_value(rhs);
  if (binOp == ANA_ADD)
    return ana_string_add();
  i = strcmp(lp.s, rp.s);
  switch (binOp) {
    case ANA_EQ:
      i = (i == 0);
      break;
    case ANA_GE:
      i = (i >= 0);
      break;
    case ANA_GT:
      i = (i > 0);
      break;
    case ANA_NE:
      i = (i != 0);
      break;
    case ANA_LE:
      i = (i <= 0);
      break;
    case ANA_LT:
      i = (i <  0);
      break;
    default:
      return cerror(ILL_W_STR, lhs);
  }
  result = scalar_scratch(ANA_LONG);
  scalar_value(result).l = i;
  return result;
}
/*----------------------------------------------------------*/
int evalSArrayStringBinOp(void)
/* binary operation with a string and a string array */
{
  int	n, result;

  lp.sp = array_data(lhs);
  rp.s = string_value(rhs);
  n = array_size(lhs);
  switch (binOp) {
    case ANA_EQ: case ANA_GE: case ANA_GT: case ANA_NE: case ANA_LE:
    case ANA_LT:
      break;			/* these are OK */
    default:
      return cerror(ILL_W_STR, lhs);
  }
  result = array_clone(lhs, ANA_LONG);
  tp.l = array_data(result);

  switch (binOp) {
    case ANA_EQ:
      while (n--)
	*tp.l++ = strcmp(*lp.sp++, rp.s) == 0;
      break;
    case ANA_GE:
      while (n--)
	*tp.l++ = strcmp(*lp.sp++, rp.s) >= 0;
      break;
    case ANA_GT:
      while (n--)
	*tp.l++ = strcmp(*lp.sp++, rp.s) > 0;
      break;
    case ANA_NE:
      while (n--)
	*tp.l++ = strcmp(*lp.sp++, rp.s) != 0;
      break;
    case ANA_LE:
      while (n--)
	*tp.l++ = strcmp(*lp.sp++, rp.s) <= 0;
      break;
    case ANA_LT:
      while (n--)
	*tp.l++ = strcmp(*lp.sp++, rp.s) < 0;
      break;
  }
  return result;
}
/*----------------------------------------------------------*/
int evalStringSArrayBinOp(void)
/* binary operation with a string and a string array */
{
  int	n, result;

  rp.sp = array_data(rhs);
  lp.s = string_value(lhs);
  n = array_size(rhs);
  switch (binOp) {
    case ANA_EQ: case ANA_GE: case ANA_GT: case ANA_NE: case ANA_LE:
    case ANA_LT:
      break;			/* these are OK */
    default:
      return cerror(ILL_W_STR, lhs);
  }
  result = array_clone(rhs, ANA_LONG);
  tp.l = array_data(result);

  switch (binOp) {
    case ANA_EQ:
      while (n--)
	*tp.l++ = strcmp(lp.s, *rp.sp++) == 0;
      break;
    case ANA_GE:
      while (n--)
	*tp.l++ = strcmp(lp.s, *rp.sp++) >= 0;
      break;
    case ANA_GT:
      while (n--)
	*tp.l++ = strcmp(lp.s, *rp.sp++) > 0;
      break;
    case ANA_NE:
      while (n--)
	*tp.l++ = strcmp(lp.s, *rp.sp++) != 0;
      break;
    case ANA_LE:
      while (n--)
	*tp.l++ = strcmp(lp.s, *rp.sp++) <= 0;
      break;
    case ANA_LT:
      while (n--)
	*tp.l++ = strcmp(lp.s, *rp.sp++) < 0;
      break;
  }
  return result;
}
/*----------------------------------------------------------*/
int evalScalarRangeBinOp(void)
/* binary operation on a scalar and a scalar ANA_RANGE symbol */
/* if range start or end has * - expr notation, then must apply */
/* operation to minus its value, so that, say, (1:*-10) + 3 yields */
/* (4:*-7) rather than (4:*-13). */
{
  int	range, result;
  
  range = rhs;
  result = newSymbol(ANA_RANGE, 0, 0);
  rhs = range_start(range);
  if (rhs < 0) {		/* * - expr notation */
    rhs = -rhs;		/* get proper symbol number */
    rhs = ana_neg_func(1, &rhs);
  }
  topType = lhsType = scalar_type(lhs);
  rhsType = symbol_type(rhs);
  if (rhsType > topType)
    topType = rhsType;
  range_start(result) = evalScalarBinOp();
  if (range_start(range) < 0) {	/* restore * - expr notation */
    rhs = (int) range_start(result);
    rhs = ana_neg_func(1, &rhs);
    range_start(result) = (word) -rhs;
    embed(-range_start(result), result);
  } else
    embed(range_start(result), result);
  rhs = range_end(range);
  if (rhs < 0) {
    rhs = -rhs;		/* get proper symbol number */
    rhs = ana_neg_func(1, &rhs);
  }
  topType = lhsType;
  rhsType = symbol_type(rhs);
  if (rhsType > topType)
    topType = rhsType;
  range_end(result) = evalScalarBinOp();
  if (range_end(range) < 0) {	/* restore * - expr notation */
    rhs = (int) range_end(result);
    rhs = ana_neg_func(1, &rhs);
    range_end(result) = (word) -rhs;
    embed(-range_end(result), result);
  } else
    embed(range_end(result), result);
  return result;
}
/*----------------------------------------------------------*/
int evalRangeScalarBinOp(void)
     /* binary operation on a scalar and a scalar ANA_RANGE symbol */
{
  int	range, result;
  int	newSymbol(int, ...);
  
  range = lhs;
  result = newSymbol(ANA_RANGE, 0, 0);
  lhs = range_start(range);
  if (lhs < 0) {		/* * - expr notation */
    lhs = -lhs;		/* get proper symbol number */
    lhs = ana_neg_func(1, &lhs);
  }
  topType = rhsType = scalar_type(rhs);
  lhsType = symbol_type(lhs);
  if (lhsType > topType)
    topType = lhsType;
  range_start(result) = evalScalarBinOp();
  if (range_start(range) < 0) {	/* restore * - expr notation */
    lhs = (int) range_start(result);
    lhs = ana_neg_func(1, &lhs);
    range_start(result) = (word) -lhs;
    embed(-range_start(result), result);
  } else
    embed(range_start(result), result);
  lhs = range_end(range);
  if (lhs < 0) {
    lhs = -lhs;		/* get proper symbol number */
    lhs = ana_neg_func(1, &lhs);
  }
  topType = rhsType;
  lhsType = symbol_type(lhs);
  if (lhsType > topType)
    topType = lhsType;
  range_end(result) = evalScalarBinOp();
  if (range_end(range) < 0) {	/* restore * - expr notation */
    lhs = (int) range_end(result);
    lhs = ana_neg_func(1, &lhs);
    range_end(result) = (word) -lhs;
    embed(-range_end(result), result);
  } else
    embed(range_end(result), result);
  return result;
}
/*----------------------------------------------------------*/
int extractListElem(int base, int index, char *key, int write)
/* returns the number of the symbol that <arg> points at in
   the CLIST or LIST <base>.  If <base> is negative, then
   <index> is the numerical tag; otherwise <key> is the string tag.
   LS 14sep98 */
{
  int	i, n;
  int	eval(int), copySym(int), installString(char *);
  
  if (base < 0)			/* numerical tag */
    base = -base;
  else				/* string tag */
    index = -1;

  if (base >= NSYM || base <= 0) /* illegal symbol number */
    return ANA_ERROR;

  switch (symbol_class(base)) {	/* what kind of envelope? */
    case ANA_RANGE:
      if (index < 0 || index > 1) /* bad label */
	return ANA_ERROR;
      n = index? range_end(base): range_start(base);
      if (n < 0)
	n = -n;
      return write? n: copySym(n);
    case ANA_CLIST:
      if (index < 0 || index >= clist_num_symbols(base))
	return ANA_ERROR;
      n = clist_symbols(base)[index];
      return write? n: copySym(n);
    case ANA_LIST:
      n = list_num_symbols(base);
      if (index < 0) {		/* need to match the key */
	for (i = 0; i < n; i++) {
	  if (!strcmp(key, list_key(base,i)))
	    break;
	}
	if (i < n)
	  index = i;
      }
      if (index < 0 || index >= n) /* index points outside list */
	return ANA_ERROR;
      n = list_symbol(base,index);
      return write? n: copySym(n);
    case ANA_ENUM:
      if (write)
	return cerror(ILL_CLASS, base);
      if (index < 0) {		/* need to match the key */
	for (i = 0; i < (n = enum_num_elements(base)); i++) {
	  if (!strcmp(key, enum_key(base,i)))
	    break;
	}
	if (i < n)
	  index = i;
      }
      if (index < 0 || index >= n)
	return ANA_ERROR;
      if ((n = nextFreeTempVariable()) < 0)
	return ANA_ERROR;
      symbol_class(n) = ANA_SCALAR;
      scalar_type(n) = ANA_LONG;
      scalar_value(n).l = enum_value(base,index);
      return n;
    case ANA_SUBROUTINE:
    case ANA_FUNCTION:
      if (write)
	return anaerror("Cannot modify local variables from outside their scope",
		     base);
      if (index < 0) {		/* need to match the key */
	i = installString(key);
	n = findVar(i, base);
	freeString(i);
      }
      if (index >= 0 || n < 0)
	return ANA_ERROR;
      return n;
    default:
      return anaerror("Pointer to non-embedding variable.", 0);
  }
}
/*----------------------------------------------------------*/
int evalListPtr(int symbol)
     /* evaluates pointers to elements of structures, lists, */
     /* ranges, and enums, and also pointers to local variables */
     /* in user functions, routines, and block-routines */
     /* Use copySym to return a copy of the pointed-at element */
     /* (for LISTs, and RANGEs) because the same */
     /* element may be pointed at more than once. */
{
  int	base, index = -1, n;
  char	*key;
  
  base = list_ptr_target(symbol); /* the enveloping structure */
  if (base < 0) {		/* numerical label */
    index = list_ptr_tag_number(symbol);
    base = -eval(-base);
  } else {
    key = list_ptr_tag_string(symbol);
    base = eval(base);
  }
  n = extractListElem(base, index, key, 0);
  if (n == ANA_ERROR)
    return cerror(BAD_STRUCT_KEY, symbol);
  return n;
}
/*----------------------------------------------------------*/
int evalStructPtr(int symbol)
/* evaluates <symbol> as a STRUCT_PTR */
{
  int	target, result, n, i, nout, one = 1, outdims[MAX_DIMS], outndim = 0,
    *dims, ndim, nms, i1, i2, j, k, nelem, *p, ne, type, total_ndim;
  structElem	*se;
  structPtr	*spe;
  pointer	trgt;
  
  target = struct_ptr_target(symbol); /* target is assumed to be a STRUCT */
  se = struct_elements(target);
  nms = struct_ptr_n_elements(symbol); /* number of member specifications */
  spe = struct_ptr_elements(symbol);
  /* we figure out what dimensions the result has, and we check that
     the subscripts are in range */
  total_ndim = 0;
  for (i = 0; i < nms; i++) {	/* all subscripts */
    total_ndim += spe[i].n_subsc; /* accumulate total number of subscripts */
    switch (se[spe[i].desc].u.regular.type) { /* the type of the structure
						 element */
      case ANA_STRUCT:
	anaerror("Sorry, not yet implemented", symbol);
	goto evalStructPtr_1;
      default:			/* an array or scalar or string */
	/* get the dimensions of the structure element */
	if (spe[i].desc) {	/* a part of the structure */
	  dims = se[spe[i].desc].u.regular.spec.singular.dims;
	  ndim = se[spe[i].desc].u.regular.spec.singular.ndim;
	} else {		/* the structure as a whole */
	  dims = se[0].u.first.dims;
	  ndim = se[0].u.first.ndim;
	}
	if (!ndim) {		/* a scalar; mimic an array with one element */
	  ndim = 1;
	  dims = &one;
	}
	/* the number of subscripts must either be 1 or be equal to the
	   number of dimensions in the corresponding structure part */
	if (spe[i].n_subsc != 1 && spe[i].n_subsc != ndim) {
	  cerror(ILL_N_SUBSC, symbol);
	  goto evalStructPtr_1;
	}
	if (spe[i].n_subsc == 1 && ndim > 1) {
	  /* we mimic a 1D array, so we must calculate the total
	     number of elements */
	  nelem = 1;
	  for (k = 0; k < ndim; k++)
	    nelem *= dims[k];
	} else
	  nelem = 0;
	for (j = 0; j < spe[i].n_subsc; j++) { /* all subscripts */
	  n = nelem? nelem: dims[j];
	  switch (spe[i].member[j].type) { /* subscript type */
	    case ANA_SCALAR:	/* this indexes an array as if the array
				   is one-dimensional */
	      i1 = spe[i].member[j].data.scalar.value;
	      if (i1 < 0 || i1 >= n) {	/* subscript value out of range */
		cerror(SUBSC_RANGE, symbol);
		goto evalStructPtr_1;
	      }
	      break;
	    case ANA_RANGE:
	      i1 = spe[i].member[j].data.range.start;
	      i2 = spe[i].member[j].data.range.end;
	      if (i1 < 0)
		i1 += n;
	      if (i2 < 0)
		i2 += n;
	      if (i1 < i2) {
		cerror(ILL_SUBSC, symbol);
		goto evalStructPtr_1;
	      }
	      if (i2 != i1) {	/* doesn't reduce to a single element */
		if (outndim == MAX_DIMS - 1) { /* we're exceeding the maximum
						 number of dimensions */
		  cerror(ILL_NUM_DIM, symbol);
		  goto evalStructPtr_1;
		}
		outdims[++outndim] = i2 - i1 + 1;
	      }
	      break;
	    case ANA_ARRAY:
	      p = spe[i].member[j].data.array.ptr;
	      ne = spe[i].member[j].data.array.n_elem;
	      for (k = 0; k < ne; k++)
		if (p[k] < 0 || p[k] >= n) {
		  cerror(SUBSC_RANGE, symbol);
		  goto evalStructPtr_1;
		}
	      if (ne > 1) {	/* doesn't reduce to a single element */
		if (outndim == MAX_DIMS - 1) { /* exceeding maximum number */
		  cerror(ILL_NUM_DIM, symbol);
		  goto evalStructPtr_1;
		}
		outdims[++outndim] = ne;
	      }
	      break;
	  } /* end of switch (spe[i].member[j].type) */
	} /* end of for (j = 0; j < spe[i].n_subsc; j++) */
    } /* end of switch (se[spe[i].desc].u.regular.type) */
  } /* end of for (i = 0; i < n; i++) */

  /* the last element defines the kind of output we get */
  type = se[spe[nms - 1].desc].u.regular.type;
  switch (type) {
    case ANA_STRUCT:		/* a structure */
      anaerror("Sorry, not yet implemented", 0);
      goto evalStructPtr_1;
    case ANA_TEMP_STRING:	/* a string or string array */
      if (outndim > 1) {	/* a string array */
	result = array_scratch(type, outndim - 1, outdims + 1);
	trgt.sp = array_data(result);
      } else {
	result = string_scratch(0);
	trgt.sp = &string_value(result);
      }
      break;
    default:			/* everything else */
      if (outndim) { 		/* an array */
	result = array_scratch(type, outndim, outdims);
	trgt.v = array_data(result);
      } else {			/* a scalar */
	result = scalar_scratch(type);
	trgt.v = &scalar_value(result).b;
      }
      break;
  } /* end of switch (type) */

  


  return result;

  evalStructPtr_1:
  return ANA_ERROR;
}
/*----------------------------------------------------------*/
int evalLhs(symbol)
/* evaluate <symbol> as a left-hand side of an assignment: resolves
   TRANSFERs and finds the member of LISTs that is pointed at.
   Returns an ANA_EXTRACT symbol wherein the head is a simple variable.
   LS 7jan99 */
{
  int	target, kind, depth, nitem, class, modified, result, n, special, j;
  extractSec	*eptr, *tptr;
  char	**sptr, *name, *p;
  int	findTarget(char *, int *, int);
  void	*v;
  extern int	eval_func,  /* function number of EVAL function */
    d_r_sym, r_d_sym;		/* symbol numbers of #D.R and #R.D */

  switch (symbol_class(symbol)) {
    case ANA_SCALAR: case ANA_SCAL_PTR: case ANA_ARRAY: case ANA_CARRAY:
    case ANA_UNDEFINED: case ANA_STRING: case ANA_CPLIST: case ANA_CLIST:
    case ANA_RANGE: case ANA_STRUCT: case ANA_FILEMAP: case ANA_LIST:
    case ANA_CSCALAR:
      return symbol;
    case ANA_TRANSFER: case ANA_POINTER:
      symbol = transfer(symbol);
      return evalLhs(symbol);
    default:
      return anaerror("Sorry, not implemented in evalLhs()", symbol);
    case ANA_EXTRACT:
      break;
    case ANA_PRE_EXTRACT:
      /* #D.R and #R.D are widely used as "degrees-to-radians" and
	 "radians-to-degrees" conversion factors, but now clash with our
	 notation for structure tags; i.e., they are now interpreted
	 as "member R of list/structure #D" and "member D of list/structure
	 #R", respectively.  We must deal with these as special cases:
	 we do not allow any new ones like it.  LS 9jan99 */
      special = 0;
      if (!strcmp(pre_extract_name(symbol), "#D")
	  && pre_extract_num_sec(symbol) >= 1) {
	eptr = pre_extract_ptr(symbol);
	if (eptr->type == ANA_LIST
	    && !strcmp(*eptr->ptr.sp, "R")) /* it is #D.R */
	  special = d_r_sym;
      } else if (!strcmp(pre_extract_name(symbol), "#R")
		 && pre_extract_num_sec(symbol) >= 1) {
	eptr = pre_extract_ptr(symbol);
	if (eptr->type == ANA_LIST
	    && !strcmp(*eptr->ptr.sp, "D")) /* it is #R.D */
	  special = r_d_sym;
      }
      if (special) {
	/* we replace #D .R... with (#D.R)... */
	symbol_class(symbol) = ANA_EXTRACT;
	free(pre_extract_name(symbol));
	extract_ptr(symbol) = pre_extract_ptr(symbol);
	extract_target(symbol) = special;
	if (eptr->number > 1) { /* still more tags in this section */
	  eptr->number = eptr->number - 1;
	  memmove(*eptr->ptr.sp, *eptr->ptr.sp + 1,
		  eptr->number*sizeof(char *));
	  eptr->ptr.sp = realloc(eptr->ptr.sp, eptr->number*sizeof(char *));
	} else {		/* we must get rid of this whole section */
	  if (extract_num_sec(symbol) > 1) {
	    /* still have more sections */
	    symbol_memory(symbol) =
	      (extract_num_sec(symbol) - 1)*sizeof(extractSec);
	    memmove(eptr, eptr + 1, symbol_memory(symbol));
	  } else { 		/* just the target: no more extraction */
	    symbol_class(symbol) = ANA_TRANSFER;
	    transfer_target(symbol) = special;
	    transfer_is_parameter(symbol) = 0; /* not a routine parameter */
	    return symbol;	/* all done */
	  }
	}
      } else {
	target = findTarget(pre_extract_name(symbol), &kind, 1);
	if (kind == ANA_ERROR)
	  return anaerror("No variable or function with name %s", 0,
		       pre_extract_name(symbol));
	if (kind == ANA_INT_FUNC)
	  target = -target;
	/* we change the ANA_PRE_EXTRACT symbol into an ANA_EXTRACT symbol */
	symbol_class(symbol) = ANA_EXTRACT;
	free(pre_extract_name(symbol));
	v = pre_extract_ptr(symbol);
	free(symbol_data(symbol));
	extract_ptr(symbol) = v;
	extract_num_sec(symbol) = pre_extract_num_sec(symbol);
	symbol_memory(symbol) = extract_num_sec(symbol)*sizeof(extractSec);
	extract_target(symbol) = target;

      }
      break;
  }

  /* if we get here then it is an ANA_EXTRACT */
  target = extract_target(symbol);
  if (target > 0)
    target = transfer(target);
  modified = (target != extract_target(symbol));
  
  depth = extract_num_sec(symbol);
  if (!depth)			/* empty parentheses */
    return anaerror("No empty parentheses allowed here", symbol);

  eptr = extract_ptr(symbol);
  if (target <= 0) {		/* insertion into an internal function */
    /* we want to allow EVAL(string) on the left-hand side: the string
       is interpreted as the name of a target symbol.  If that symbol
       is found, then it is used instead.  LS 7jan99 */
    kind = 0;
    if (target == -eval_func) {	/* EVAL function at left-hand side */
      if (eptr->type == ANA_RANGE /* parenthesized */
	  && eptr->number == 1)	{ /* and a single subscript */
	n = eval(*eptr->ptr.w);	/* the subscript */
	if (symbol_class(n) == ANA_STRING) { /* it's a string */
	  name = p = strsave(string_value(n)); /* the name */
	  while (*p) {		/* make all uppercase */
	    *p = toupper(*p);
	    p++;
	  }
	  target = findTarget(name, &kind, 1); /* seek the name */
	  if (target == ANA_ERROR) /* none found */
	    target = findVarName(name, curContext);
	  if (target == ANA_ERROR) { /* some error */
	    anaerror("No variable or function with name %s", 0,
		  name);
	    free(name);
	    return ANA_ERROR;
	  }
	  free(name);		/* don't need this one anymore */
	  if (kind == ANA_INT_FUNC)
	    return anaerror("Cannot insert into an internal function", target);
	  depth--;
	  if (!depth)		/* all done */
	    return target;
	  eptr++;
	}
      }
    }
    if (!kind)
      return anaerror("Cannot insert into an internal function", symbol);
  }
  class = symbol_class(target);
  
  while (depth--) {
    if (eptr->type == ANA_LIST)	{ /* tags */
      nitem = eptr->number;
      sptr = eptr->ptr.sp;
    } else
      nitem = 1;
    while (nitem--) {
      switch (class) {
	case ANA_ARRAY:	case ANA_CARRAY: /* inserting into an array */
	case ANA_FILEMAP:	/* or a file map */
	  switch (eptr->type) {
	    case ANA_RANGE:	/* subscripts */
	      if (modified) {	/* not the original symbol anymore */
		if (depth)	/* a multiply subscripted array */
		  return anaerror("Sorry, multiple subscripts on LISTs are not yet implemented", symbol);
		/* we construct an ANA_EXTRACT symbol */
		result = nextFreeTempVariable();
		if (result == ANA_ERROR)
		  return ANA_ERROR;
		symbol_class(result) = ANA_EXTRACT;
		extract_target(result) = target;
		extract_num_sec(result) = depth + 1;
		symbol_memory(result) = (depth + 1)*sizeof(extractSec);
		extract_ptr(result) = tptr = malloc(symbol_memory(result));
		if (!tptr)
		  return cerror(ALLOC_ERR, 0);
		for (j = 0; j <= depth; j++) {
		  tptr[j].type = eptr[j].type;
		  tptr[j].number = eptr[j].number;
		  tptr[j].ptr.w = malloc(tptr[j].number*sizeof(word));
		  if (!tptr[j].ptr.w)
		    return cerror(ALLOC_ERR, 0);
		  memcpy(tptr[j].ptr.w, eptr[j].ptr.w,
			 tptr[j].number*sizeof(word));
		}
		return result;
	      } else
		return symbol;
	    case ANA_LIST:
	      return cerror(ILL_SUBSC, symbol);
	  }
	  break;
	case ANA_LIST:
	  if (depth)
	    return anaerror("Sorry, multiple subscripts on LISTs are not yet implemented", symbol);
	  switch (eptr->type) {
	    case ANA_RANGE:
	      n = eval(*eptr->ptr.w); /* the subscript symbol */
	      if (n == ANA_ERROR) /* some error */
		return ANA_ERROR; /* pass on */
	      switch (symbol_class(n)) {
		case ANA_SCALAR:
		  n = int_arg(n); /* the integer subscript */
		  if (n < 0 || n >= list_num_symbols(target))
		    return cerror(SUBSC_RANGE, symbol);
		  return list_symbol(target, n);
		default:
		  return cerror(ILL_SUBSC, n);
	      }
	      break;
	    case ANA_LIST:
	    { int	i;

	      for (i = 0; i < list_num_symbols(target); i++)
		if (!strcmp(list_key(target, i), *sptr))
		  break;
	      if (i == list_num_symbols(target)) /* not found */
		return cerror(BAD_STRUCT_KEY, symbol);
	      return list_symbol(target, i);
	    }
	  };
	  break;
	default:
	  return anaerror("Cannot insert into a %s [%1d]", symbol,
		       className(class), class);
      }	/* end of switch (class) */
    } /* end of while (nitem--) */
    if (!depth)
      return target;
    class = symbol_class(target);
    eptr++;
  } /* end of while (depth--) */
  return anaerror("Unexpected exit from evalLhs()", symbol);
} /* end of evalLhs() */
/*----------------------------------------------------------*/
int evalExtractRhs(int symbol)
/* evaluate ANA_EXTRACT symbol as rhs */
{
  int	target, class, depth, result, n, nitem, i, kind, special, allowSubr,
    j, k, *ip;
  int	findTarget(char *, int *, int), getBody(int);
  word	*wptr;
  extractSec	*eptr;
  char	**sptr;
  pointer	p, q, r;
  extern int	d_r_sym, r_d_sym;
  structElem	*se;
  structPtr	*spe;
  structPtrMember	*spm;

  if (symbol_class(symbol) == ANA_PRE_EXTRACT) {
    /* #D.R and #R.D are widely used as "degrees-to-radians" and
       "radians-to-degrees" conversion factors, but now clash with our
       notation for structure tags; i.e., they are now interpreted
       as "member R of list/structure #D" and "member D of list/structure
       #R", respectively.  We must deal with these as special cases:
       we do not allow any new ones like it.  LS 9jan99 */
    special = 0;
    eptr = pre_extract_ptr(symbol);
    if (!strcmp(pre_extract_name(symbol), "#D")
	&& pre_extract_num_sec(symbol) >= 1) {
      if (eptr->type == ANA_LIST
	  && !strcmp(*eptr->ptr.sp, "R")) /* it is #D.R */
	special = d_r_sym;
    } else if (!strcmp(pre_extract_name(symbol), "#R")
	       && pre_extract_num_sec(symbol) >= 1) {
      if (eptr->type == ANA_LIST
	  && !strcmp(*eptr->ptr.sp, "D")) /* it is #R.D */
	special = r_d_sym;
    }
    if (special) {		/* we found #D.R or #R.D */
      /* we replace #D .R... with (#D.R)... */
      symbol_class(symbol) = ANA_EXTRACT;
      free(pre_extract_name(symbol));
      extract_ptr(symbol) = pre_extract_ptr(symbol);
      extract_target(symbol) = special;
      if (eptr->number > 1) { /* still more tags in this section */
	eptr->number = eptr->number - 1;
	memmove(*eptr->ptr.sp, *eptr->ptr.sp + 1,
		eptr->number*sizeof(char *));
	eptr->ptr.sp = realloc(eptr->ptr.sp, eptr->number*sizeof(char *));
      } else {		/* we must get rid of this whole section */
	if (extract_num_sec(symbol) > 1) {
	  /* still have more sections */
	  symbol_memory(symbol) =
	    (extract_num_sec(symbol) - 1)*sizeof(extractSec);
	  memmove(eptr, eptr + 1, symbol_memory(symbol));
	} else { 		/* just the target: no more extraction */
	  undefine(symbol);
	  symbol_class(symbol) = ANA_TRANSFER; /* unconditional transfer */
	  transfer_target(symbol) = special;
	  transfer_is_parameter(symbol) = 0; /* not a routine parameter */
	  return special;	/* all done */
	} /* end of if (extract_num_sec(symbol) > 1) else */
      }	/* end of if (eptr->number > 1) else */
    } else {			/* no #D.R or #R.D */
      /* we must allow an ANA_SUBR as a target if we are extracting using
	 a structure tag: then we may be referring to a variable local to
	 the subroutine; if the extraction is through parentheses, then
	 ANA_SUBRs are not allowed. */
      if (pre_extract_num_sec(symbol) >= 1 /* have arguments */
	  && eptr->type == ANA_LIST) /* it's a tag */
	allowSubr = 1;
      else
	allowSubr = 0;
      target = findTarget(pre_extract_name(symbol), &kind, allowSubr);
      if (kind == ANA_ERROR)
	return anaerror("No variable or function with name %s", 0,
		     pre_extract_name(symbol));
      if (kind == ANA_INT_FUNC)
	target = -target;
      /* now we change the ANA_PRE_EXTRACT symbol into an ANA_EXTRACT symbol */
      symbol_class(symbol) = ANA_EXTRACT;
      free(pre_extract_name(symbol));
      p.v = pre_extract_ptr(symbol);
      free(pre_extract_data(symbol));
      extract_ptr(symbol) = p.v;
      extract_target(symbol) = target;
    }
  } else			/* we assume it's an ANA_EXTRACT symbol */
    target = extract_target(symbol);
  if (target > 0) {		/* extraction from a symbol */
    target = eval(target);
    class = symbol_class(target);
  } else {			/* call to an internal function */
    target = -target;
    class = ANA_INT_FUNC;
  }
  depth = extract_num_sec(symbol);
  eptr = extract_ptr(symbol);

  /* we extract from the target according to the first extraction item,
     and then repeatedly take the extracted data as the new target and
     extract according to the next extraction item, until we have no
     more extraction items left, or an illegal extraction is attempted. */
  if (!depth) {			/* a function is called without any
				   arguments */
    switch (class) {
      case ANA_INT_FUNC:
	/* we construct an internal function call symbol */
	result = nextFreeTempVariable();
	if (result == ANA_ERROR)
	  return ANA_ERROR;
	symbol_class(result) = ANA_INT_FUNC;
	int_func_number(result) = target;
	int_func_arguments(result) = NULL;
	symbol_memory(result) = 0;
	target = eval(result);
	zap(result);		/* because it was a temp */
	break;
      case ANA_FUNCTION:
	result = nextFreeTempVariable();
	if (result == ANA_ERROR)
	  return ANA_ERROR;
	symbol_class(result) = ANA_USR_FUNC;
	usr_func_number(result) = target;
	symbol_memory(result) = 0;
	usr_func_arguments(result) = NULL;
	target = eval(result);
	zap(result);
	break;
      default:
	return anaerror("Illegal target class for empty subscripts", symbol);
    }
    return target;
  } else while (depth--) {
    if (eptr->type == ANA_LIST)	{ /* tags */
      nitem = eptr->number;
      sptr = eptr->ptr.sp;
    } else
      nitem = 1;
    while (nitem--) {
      switch (class) {
	case ANA_INT_FUNC:	/* call to an internal function */
	  switch (eptr->type) {
	    case ANA_RANGE:	/* subscripts */
	      /* we construct an internal function call symbol */
	      result = nextFreeTempVariable();
	      if (result == ANA_ERROR)
		return ANA_ERROR;
	      symbol_class(result) = ANA_INT_FUNC;
	      int_func_number(result) = target;
	      int_func_arguments(result) = malloc(eptr->number*sizeof(word));
	      if (!int_func_arguments(result))
		return cerror(ALLOC_ERR, 0);
	      symbol_memory(result) = eptr->number*sizeof(word);
	      memcpy(int_func_arguments(result), eptr->ptr.w,
		     eptr->number*sizeof(word));
	      target = eval(result);
	      zap(result);	/*  it was a temp */
	      break;
	    default:		/* tags */
	      return anaerror("Impossible error: cannot apply tags to functions",
			   symbol);
	  }
	  break;
	case ANA_ARRAY:	case ANA_CARRAY: /* extracting from an array */
	case ANA_FILEMAP:	/* or a file map */
	  switch (eptr->type) {
	    case ANA_RANGE:	/* subscripts */
	      /* we construct an internal function call to ana_subsc_fun */
	      result = nextFreeTempVariable();
	      if (result == ANA_ERROR)
		return ANA_ERROR;
	      symbol_class(result) = ANA_INT_FUNC;
	      int_func_number(result) = ANA_SUBSC_FUN;
	      n = eptr->number;
	      symbol_memory(result) = (n + 1)*sizeof(word);
	      int_func_arguments(result) = malloc(symbol_memory(result));
	      memcpy(int_func_arguments(result), eptr->ptr.w, n*sizeof(word));
	      int_func_arguments(result)[n] = target;
	      target = eval(result);
	      unMark(result);
	      zap(result);
	      break;
	    default:
	      return cerror(ILL_CLASS, symbol);
	  }
	  break;
	case ANA_DEFERRED_FUNC:	/* calling a deferred function */
	  /* we must compile the function body */
	  if (getBody(target) == ANA_ERROR)
	    return ANA_ERROR;
	  /* now it's a regular function; fall-thru to ANA_FUNCTION case. */
	case ANA_FUNCTION:	/* calling a user-defined function */
	  switch (eptr->type) {
	    case ANA_RANGE:
	      result = nextFreeTempVariable();
	      if (result == ANA_ERROR)
		return ANA_ERROR;
	      symbol_class(result) = ANA_USR_FUNC;
	      usr_func_number(result) = target;
	      symbol_memory(result) = eptr->number*sizeof(word);
	      usr_func_arguments(result) = malloc(symbol_memory(result));
	      memcpy(usr_func_arguments(result), eptr->ptr.w,
		     symbol_memory(result));
	      target = eval(result);
	      zap(result);
	      break;
	    case ANA_LIST:	/* get a local variable from the function */
	      target = lookForVarName(*sptr, target);
	      if (target == -1)
		return anaerror("No such local variable: %s", symbol,
			     *sptr);
	      sptr++;
	      break;
	  }
	  break;
	case ANA_SUBROUTINE:
	  switch (eptr->type) {
	    case ANA_RANGE:
	      return cerror(ILL_CLASS, symbol);
	    case ANA_LIST:	/* get a local variable from the function */
	      target = lookForVarName(*sptr, target);
	      if (target == -1)
		return anaerror("No such local variable: %s", symbol,
			     *sptr);
	      sptr++;
	      break;
	  }
	  break;
	case ANA_CLIST: case ANA_CPLIST:
	  switch (eptr->type) {
	    case ANA_RANGE:
	      if (eptr->number > 1)
		return anaerror("Only one subscript allowed on CLISTs", symbol);
	      result = eval(eptr->ptr.w[0]); /* the single subscript */
	      switch (symbol_class(result)) {
		case ANA_SCALAR:
		  i = int_arg(result);
		  if (i < 0 || i >= clist_num_symbols(target))
		    return cerror(SUBSC_RANGE, symbol);
		  target = clist_symbols(target)[i];
		  break;
		case ANA_ARRAY:
		  result = ana_long(1, &result); /* ensure LONG */
		  p.l = array_data(result);
		  n = array_size(result);
		  for (i = 0; i < n; i++)
		    if (p.l[i] < 0 || p.l[i] >= clist_num_symbols(target))
		      return cerror(SUBSC_RANGE, symbol);
		  target = nextFreeTempVariable();
		  symbol_class(target) = ANA_LIST;
		  symbol_memory(target) = n*sizeof(word);
		  q.w = clist_symbols(target) = malloc(symbol_memory(target));
		  if (!q.w)
		    return cerror(ALLOC_ERR, symbol);
		  for (i = 0; i < n; i++)
		    *q.w++ = copySym(clist_symbols(target)[*p.l++]);
		  break;
		default:
		  return anaerror("Not implemented", symbol);
	      }
	      break;
	    case ANA_LIST:
	      return cerror(ILL_CLASS, symbol);
	  }
	  break;
	case ANA_LIST:
	  switch (eptr->type) {
	    case ANA_RANGE:
	      if (eptr->number > 1)
		return anaerror("Only one subscript allowed on LISTs", symbol);
	      result = eval(eptr->ptr.w[0]); /* the single subscript */
	      switch (symbol_class(result)) {
		case ANA_SCALAR:
		  i = int_arg(result);
		  if (i < 0 || i >= list_num_symbols(target))
		    return cerror(SUBSC_RANGE, symbol);
		  target = list_symbol(target, i);
		  break;
		case ANA_ARRAY:
		  /* NOTE: currently, the result is a CLIST; should be a
		     LIST.  LS 31dec98 */
		  result = ana_long(1, &result); /* ensure LONG */
		  p.l = array_data(result);
		  n = array_size(result);
		  for (i = 0; i < n; i++)
		    if (p.l[i] < 0 || p.l[i] >= list_num_symbols(target))
		      return cerror(SUBSC_RANGE, symbol);
		  target = nextFreeTempVariable();
		  symbol_class(target) = ANA_CLIST;
		  symbol_memory(target) = n*sizeof(word);
		  q.w = clist_symbols(target) = malloc(symbol_memory(target));
		  if (!q.w)
		    return cerror(ALLOC_ERR, symbol);
		  for (i = 0; i < n; i++)
		    *q.w++ = copySym(list_symbol(target, *p.l++));
		  break;
		default:
		  return anaerror("Not implemented", symbol);
	      }
	      break;
	    case ANA_LIST:
	      n = list_num_symbols(target);
	      for (i = 0; i < n; i++)
		if (!strcmp(*sptr, list_key(target, i)))
		  break;
	      if (i == n)	/* none found */
		return cerror(BAD_STRUCT_KEY, symbol);
	      target = list_symbol(target, i);
	      sptr++;
	      break;
	  }
	  break;
	case ANA_ENUM:
	  switch (eptr->type) {
	    case ANA_RANGE:
	      return cerror(ILL_CLASS, symbol);
	    case ANA_LIST:
	      n = enum_num_elements(target);
	      for (i = 0; i < n; i++)
		if (!strcmp(*sptr, enum_key(target, i)))
		  break;
	      if (i == n)	/* none found */
		return cerror(BAD_STRUCT_KEY, symbol);
	      result = scalar_scratch(ANA_LONG);
	      scalar_value(result).l = enum_value(target, i);
	      target = result;
	      sptr++;
	      break;
	  }
	  break;
	case ANA_SCALAR:	/* regarded as a one-element array */
	  switch (eptr->type) {
	    case ANA_RANGE:
	      if (eptr->number > 1)
		return anaerror("Only one subscript allowed on SCALARs", symbol);
	      result = eval(eptr->ptr.w[0]); /* the single subscript */
	      switch (symbol_class(result)) {
		case ANA_SCALAR:
		  i = int_arg(result);
		  if (i != 0)
		    return cerror(SUBSC_RANGE, symbol);
		  break;
		case ANA_ARRAY:
		  if (symbolIsStringArray(result))
		    return anaerror("No string array allowed here", result);
		  n = nextFreeTempVariable();
		  if (n == ANA_ERROR)
		    return ANA_ERROR;
		  symbol_class(n) = ANA_INT_FUNC;
		  int_func_number(n) = ANA_SUBSC_FUN;
		  symbol_memory(n) = 2*sizeof(word);
		  int_func_arguments(n) = malloc(symbol_memory(result));
		  *int_func_arguments(n) = result;
		  int_func_arguments(n)[1] = target;
		  target = eval(n);
		  zap(n);
		  break;
		default:
		  return anaerror("Not implemented", symbol);
	      }
	      break;
	    case ANA_LIST:
	      return cerror(ILL_CLASS, symbol);
	  }
	  break;
	case ANA_RANGE:
	  switch (eptr->type) {
	    case ANA_RANGE:
	      if (eptr->number > 1)
		return anaerror("Only one subscript allowed on RANGEs", symbol);
	      result = eval(eptr->ptr.w[0]); /* the single subscript */
	      switch (symbol_class(result)) {
		case ANA_SCALAR:
		  i = int_arg(result);
		  if (i < 0 || i >= 2)
		    return cerror(SUBSC_RANGE, symbol);
		  target = i? range_end(target): range_start(target);
		  /* we silently remove any "*-" part */
		  if (target < 0)
		    target = -target;
		  break;
		case ANA_ARRAY:
		  result = ana_long(1, &result); /* ensure LONG */
		  p.l = array_data(result);
		  n = array_size(result);
		  for (i = 0; i < n; i++)
		    if (p.l[i] < 0 || p.l[i] >= 2)
		      return cerror(SUBSC_RANGE, symbol);
		  target = nextFreeTempVariable();
		  symbol_class(target) = ANA_CLIST;
		  symbol_memory(target) = n*sizeof(word);
		  q.w = clist_symbols(target) = malloc(symbol_memory(target));
		  if (!q.w)
		    return cerror(ALLOC_ERR, symbol);
		  for (i = 0; i < n; i++) {
		    kind = *p.l? range_end(target): range_start(target);
		    /* we silently remove any "*-" part */
		    if (kind < 0)
		      kind = -kind;
		    *q.w++ = copySym(kind);
		  }
		  break;
		default:
		  return anaerror("Not implemented", symbol);
	      }
	      break;
	    case ANA_LIST:
	      return cerror(ILL_CLASS, target);
	  }
	  break;
	case ANA_STRING:
	  switch (eptr->type) {
	    case ANA_RANGE:
	      if (eptr->number > 1)
		return anaerror("Only one subscript allowed on RANGEs", symbol);
	      result = eval(eptr->ptr.w[0]); /* the single subscript */
	      switch (symbol_class(result)) {
		case ANA_SCALAR:
		  result = ana_long(1, &result);
		  p.l = &scalar_value(result).l;
		  i = 0;
		  n = 1;
		  if (*p.l < 0 || *p.l >= string_size(target))
		    return cerror(SUBSC_RANGE, symbol);
		  q.s = string_value(target);
		  target = string_scratch(1);
		  r.s = string_value(target);
		  *r.s++ = q.s[*p.l++];
		  *r.s = '\0';	/* terminate the string */
		  break;
		case ANA_RANGE:
		  if (!range_scalar(result))
		    return cerror(SUBSC_RANGE, symbol);
		  i = range_start(result);
		  if (i < 0)
		    i = string_size(target) + int_arg(-i);
		  else
		    i = int_arg(i);
		  if (i < 0 || i >= string_size(target))
		    return cerror(ILL_SUBSC, symbol);
		  n = range_end(result);
		  if (n < 0)
		    n = string_size(target) - int_arg(-n);
		  else
		    n = int_arg(n);
		  n = n - i + 1;
		  if (n < 1 || i + n > string_size(target))
		    return cerror(SUBSC_RANGE, result);
		  q.s = string_value(target) + i;
		  i = target;
		  target = string_scratch(n);
		  r.s = string_value(target);
		  memcpy(r.s, q.s, n);
		  r.s[n] = '\0'; /* terminate the string */
		  zapTemp(i);	/* may not need this one anymore */
		  break;
		case ANA_ARRAY:
		  result = ana_long(1, &result); /* ensure LONG */
		  p.l = array_data(result);
		  n = array_size(result);
		  for (i = 0; i < n; i++)
		    if (p.l[i] < 0 || p.l[i] >= string_size(target))
		      return cerror(SUBSC_RANGE, target);
		  q.s = string_value(target);
		  target = nextFreeTempVariable();
		  symbol_class(target) = ANA_STRING;
		  symbol_memory(target) = n + 1;
		  r.s = string_value(target) = malloc(symbol_memory(target));
		  if (!r.s)
		    return cerror(ALLOC_ERR, symbol);
		  while (n--)
		    *r.s++ = q.s[*p.l++];
		  *r.s = '\0';	/* terminate the string */
		  break;
		default:
		  return anaerror("Not implemented", symbol);
	      }
	      zapTemp(result);	/* done with it - if it is a temp */
	      break;
	    case ANA_LIST:
	      return cerror(ILL_CLASS, target);
	  }
	  break;
	case ANA_STRUCT: case ANA_STRUCT_PTR:
	  /* the first time we get here it's a STRUCT and we generate a
	     STRUCT_PTR; following times (in the same loop) it's the
	     STRUCT_PTR. */
	  /* The STRUCT_PTR contains one element for each tag, plus one
	     for the top structure.  Each element contains items:
	        spe->desc: the index to the descriptor in the structure
		spe->n_subsc: the number of subscripts to the current tag
		spm = spe->member: pointer to the list of subscripts
		spm->type: the type of the subscript; SCALAR for a scalar,
		           RANGE for a range, ARRAY for an array */
	  if (class == ANA_STRUCT) {
	    result = newSymbol(ANA_STRUCT_PTR);
	    struct_ptr_target(result) = target;
	    target = result;
	    spe = struct_ptr_elements(target);
	    spe->desc = 0; /* first one always points at top element */
	    spe->n_subsc = 0;
	  } /* otherwise spe already points at the current item */
	  switch (eptr->type) {	/* subscript type */
	    case ANA_RANGE:	/* subscripts */
	      if (spe->n_subsc) { /* we already have subscripts on this
				     one -- illegal! */
		zapTemp(result);
		return anaerror("No double subscripts allowed here!", symbol);
	      }
	      n = spe->n_subsc = eptr->number; /* number of subscripts */
	      spm = spe->member = malloc(spe->n_subsc*sizeof(structPtrMember));
	      wptr = eptr->ptr.w;
	      while (n--) {	/* treat all subscripts */
		i = *wptr++; /* subscript symbol number */
		switch (symbol_class(i)) { /* what kind of subscript? */
		  case ANA_SCALAR:
		    spm->type = ANA_SCALAR;
		    j = int_arg(i); /* integer subscript value */
		    spm->data.scalar.value = j;
		    break;
		  case ANA_PRE_RANGE:
		    j = eval(i);
		    embed(j, symbol_context(i));
		    zap(wptr[-1]); /* remove PRE_RANGE */
		    wptr[-1] = j; /* substitute RANGE */
		    i = j;
		    /* fall through to ANA_RANGE */
		  case ANA_RANGE:
		    spm->type = ANA_RANGE;
		    j = int_arg(range_start(i));
		    spm->data.range.start = j;
		    j = int_arg(range_end(i));
		    spm->data.range.end = j;
		    break;
		  case ANA_ARRAY:
		    /* assume the data type is real! */
		    spm->type = ANA_ARRAY;
		    k = spm->data.array.n_elem = array_size(i);
		    ip = spm->data.array.ptr = malloc(k*sizeof(int));
		    p.v = array_data(i);
		    switch (array_type(i)) {
		      case ANA_BYTE:
			while (k--)
			  *ip++ = (int) *p.b++;
			break;
		      case ANA_WORD:
			while (k--)
			  *ip++ = (int) *p.w++;
			break;
		      case ANA_LONG:
			memcpy(ip, p.l, k*sizeof(int));
			break;
		      case ANA_FLOAT:
			while (k--)
			  *ip++ = (int) *p.f++;
			break;
		      case ANA_DOUBLE:
			while (k--)
			  *ip++ = (int) *p.d++;
			break;
		    }
		    break;
		  default:
		    zap(target);
		    return cerror(ILL_CLASS, i);
		}
		spm++;
	      }
	      break;
	    case ANA_LIST:	/* a tag */
	      se = struct_elements(struct_ptr_target(result));
	      for (i = 1; i < struct_num_top_elements(target) + 1; i++)
		if (se[i].u.regular.tag
		    && !strcmp(se[i].u.regular.tag, *sptr)) {
		  /* found the tag in the top-level list for this structure */
		  break;
		}
	      if (i == struct_num_top_elements(target) + 1) { /* not found */
		zapTemp(result);
		return anaerror("Didn't find tag \"%s\" in the structure",
			     symbol, *sptr);
	      }
	      /* add an entry to the STRUCT_PTR list */
	      n = struct_ptr_n_elements(target);
	      symbol_memory(target) += sizeof(structPtr);
	      struct_ptr_elements(target) =
		realloc(struct_ptr_elements(target), symbol_memory(target));
	      spe = struct_ptr_elements(target) + n; /* point at the new one */
	      spe->desc = i;
	      spe->n_subsc = 0;	/* zero indicates a tag but no subscripts
				   (yet) */
	      break;
	    default:
	      return anaerror("Illegal subscript type", 0);
	  }
	  break;
	default:
	  return anaerror("Class %s not yet implemented in evalExtractRhs",
		       target, className(symbol_class(target)));
      }
    }
    if (!depth)
      return target;
    class = symbol_class(target);
    eptr++;
  }

  return anaerror("Unexpected exit from extractRhsSymbol", symbol);
}
/*----------------------------------------------------------*/
int eval(int symbol)
     /* evaluates the symbol.  classes PRE_XXX */
     /* contain unevaluated member symbols.  classes XXX */
     /* only contain evaluated member symbols. */
{
  int	n, thisLhs, thisRhs, result, i;
  char	isScalarRange, offsetEnd;
  extern int	tempSym,	/* symbol number of !TEMP */
	tempVariableIndex,
	pipeExec,	/* pipe executable */
	pipeSym;	/* pipe symbol */
  int	namevar(int, int), transfer(int);
  void	updateIndices(void);
  
  if (symbol == ANA_ERROR)		/* some error */
    return symbol;
  /* we must resolve transfer symbols, but only if they are
     named variables -- otherwise we cannot turn symbols into transfer
     symbols.  E.g., A=&B -> do not resolve &B or else A gets the value
     of whatever B happens to point at.  Later C=A -> resolve A.
     LS 23nov98 */
  if ((symbol_class(symbol) == ANA_POINTER
       && symbolIsNamed(symbol))
      || symbol_class(symbol) == ANA_TRANSFER)
    symbol = transfer(symbol);	/* gets target of ANA_POINTER symbol */
  if (symbol == ANA_ERROR)
    return ANA_ERROR;
  if (symbol_class(symbol) == ANA_UNUSED)
    return 0;			/* unused variable, return zero */
  pegMark();
  if ((n = symbol_class(symbol)) == ANA_SCAL_PTR && evalScalPtr) {
    symbol = dereferenceScalPointer(symbol); /* ANA_SCAL_PTR -> ANA_SCALAR */
    unMark(symbol);
  }
  switch (n) {
    case ANA_SCALAR: case ANA_ARRAY: case ANA_STRING: case ANA_UNDEFINED:
    case ANA_SUBSC_PTR: case ANA_RANGE: case ANA_LIST: case ANA_CLIST:
    case ANA_KEYWORD: case ANA_SCAL_PTR: case ANA_POINTER: case ANA_FILEMAP:
    case ANA_ASSOC: case ANA_ENUM: case ANA_CSCALAR: case ANA_CARRAY:
    case ANA_STRUCT: case ANA_SUBROUTINE: case ANA_FUNCTION: case ANA_CPLIST:
    case ANA_DEFERRED_FUNC:
      /* check if piping is allowed */
      if (symbol == pipeExec)
	pipeExec = 0;
      zapMarked();
      return symbol;
  }
  /* not a "simple" evaluation, so temporary variables may be created. */
  /* store current tempVariableIndex so we can dispose properly of the */
  /* temps after the evaluation. */
  /* pushTempVariableIndex(); */
  switch (n) {
    case ANA_META:
      /* a SYMBOL() instance; return indicated */
      /* variable */
      result = namevar(eval(meta_target(symbol)), 1);
      break;
    case ANA_PRE_EXTRACT: case ANA_EXTRACT:
      result = evalExtractRhs(symbol);
      break;
    case ANA_FUNC_PTR:
      if (symbol >= tempSym) {
	unMark(symbol);
	zapMarked();
	return symbol;
      }
      getFreeTempVariable(n);	/* must be a !FUNC */
      symbol_class(n) = ANA_INT_FUNC; /* generate an ANA_INT_FUNC symbol */
      int_func_number(n) = -func_ptr_routine_num(symbol); /* negative flags
							     internal */
      symbol_memory(n) = 0;	/* no arguments, so no allocated memory */
      result = eval(n);		/* evaluate the function */
      break;
    case ANA_PRE_LIST:
      /* unevaluated structure */
      if ((result = nextFreeTempVariable()) < 0)/* no temps available */
	break;
      symbol_class(result) = ANA_LIST; /* generate ANA_LIST symbol */
      n = pre_list_num_symbols(symbol); /* number of elements */
      allocate(list_symbols(result), n, listElem); /* get memory */
      symbol_memory(result) = symbol_memory(symbol); /* is same size */
      for (i = 0; i < n; i++) {	/* all elements */
	list_key(result,i) = pre_list_key(symbol,i)?
	  strsave(pre_list_key(symbol,i)): NULL; /* store tag, if any */
	/* evaluate into temp and store in struct symbol */
	if ((list_symbol(result, i)
	     = copyEvalSym(pre_list_symbol(symbol,i))) == ANA_ERROR) {
	  result = ANA_ERROR;	/* some error */
	  break;
	}
	embed(list_symbol(result,i),result);
      }
      break;
    case ANA_PRE_CLIST:
      /* unevaluated list */
      if ((result = nextFreeTempVariable()) < 0) { /* no temps available */
	result = ANA_ERROR;
	break;
      }
      symbol_class(result) = ANA_CLIST; /* generate ANA_CLIST symbol */
      symbol_context(result) = -compileLevel; /* main level */
      n = pre_clist_num_symbols(symbol); /* number of elements */
      allocate(clist_symbols(result), n, word); /* get memory */
      symbol_memory(result) = symbol_memory(symbol); /* same size */
      for (i = 0; i < n; i++) {	/* all elements */
	if ((clist_symbols(result)[i]
	     = copyEvalSym(pre_clist_symbols(symbol)[i])) == ANA_ERROR) {
	  result = ANA_ERROR;	/* some error */
	  break;
	}
	symbol_context(clist_symbols(result)[i]) = result; /* embed */
      }
      break;
    case ANA_PRE_RANGE:
      /* unevaluated range expression */
      isScalarRange = 1;	/* default: scalar limits; modify if needed. */
      if ((result = nextFreeTempVariable()) < 0) /* no temp available */
	break;
      symbol_class(result) = ANA_RANGE;	/* generate ANA_RANGE symbol */
      symbol_context(result) = -compileLevel; /* main level
						 (see ANA_PRE_LIST) */
      if ((n = pre_range_start(symbol)) == -ANA_ONE) {	/* (*) range */
	range_start(result) = -ANA_ONE;
	range_end(result) = ANA_ZERO;
      } else {			/* not a (*) range */
	if (n < 0) {		/* (*-expr: ...) range */
	  n = -n;		/* get positive symbol number */
	  offsetEnd = 1;	/* and flag counting from end */
	} else
	  offsetEnd = 0;	/* flag counting from start */
	if ((range_start(result) = copyEvalSym(n)) < 0) {
	  result = ANA_ERROR;
	  break;
	} /* end of if (n < 0) */
	if (symbol_class(range_start(result)) != ANA_SCALAR)
	  /* start non-scalar */
	  isScalarRange = 0;	/* unset flag */
	symbol_context(range_start(result)) = result; /* put start symbol in
							 range's context */
	if (offsetEnd)		/* counting from end */
	  range_start(result) = -range_start(result); /* -> negative symbol */
	
	if ((n = pre_range_end(symbol)) < 0) { /* (...: *-expr) range */
	  n = -n;		/* get positive symbol number */
	  offsetEnd = 1;	/* and flag counting from end */
	} else
	  offsetEnd = 0;	/* flag counting from start */
	if (n == ANA_ZERO)		/* (...: *) range */
	  range_end(result) = n;	/* just copy */
	else {			/* (...: expr) range */
	  if ((range_end(result) = copyEvalSym(n)) < 0) {
	    result = ANA_ERROR;
	    break;
	  }
	  if (symbol_class(range_end(result)) != ANA_SCALAR)
	    /* non-scalar end */
	    isScalarRange = 0;	/* unset flag */
	  symbol_context(range_end(result)) = result; /* put end symbol in
							 range's context */
	  if (offsetEnd)		/* counting from the end */
	    range_end(result) = -range_end(result); /* -> negative symbol */
	} /* end of if (n == ANA_ZERO) */
      } /* end of ((n = pre_range_start(symbol)) == -ANA_ONE) */

      /* note:
	 the parser definition ensures that the summation flag and */
      /* redirection cannot be specified simultaneously.  LS 6aug96 */
      range_sum(result) = pre_range_sum(symbol);/* summation flag */
      if ((n = pre_range_redirect(symbol)) >= 0) { /* redirection */
	if ((range_redirect(result) = copyEvalSym(n)) < 0) {
	  result = ANA_ERROR;
	  break;
	} /* if ((range_redirect(result) ... */
	symbol_context(range_redirect(result)) = result; /* proper context */
      } else
	range_redirect(result) = n; /* no redirection; just copy */
      range_scalar(result) = isScalarRange; /* store is-scalar flag */
      break;
    case ANA_LIST_PTR:
	  /* a structure element */
      result = evalListPtr(symbol);
      break;
    case ANA_INT_FUNC:
      /* an internal function call */
      result = internal_routine(symbol, function);
      break;
    case ANA_USR_FUNC:
      result = usr_routine(symbol);
      break;
    case ANA_IF_OP:
      /* ANDIF or ORIF */
      if ((thisLhs = eval(bin_op_lhs(symbol))) < 0) {/* evaluate LHS */
	result = ANA_ERROR;
	thisLhs = thisRhs = 0;
	break;
      }
      switch (symbol_class(thisLhs)) {
	default:
	  /* expression did not evaluate to a scalar */
	  result = cerror(COND_NO_SCAL, bin_op_lhs(symbol));
	  thisRhs = 0;
	  break;
	case ANA_SCAL_PTR:
	  /* scalar pointer, transform to scalar */
	  thisLhs = dereferenceScalPointer(thisLhs);
	  /* FALL-THRU to ANA_SCALAR */
	case ANA_SCALAR:
	  switch (scalar_type(thisLhs)) { /* is LHS nonzero? */
	    case ANA_BYTE:
	      n = (scalar_value(thisLhs).b)? 1: 0;
	      break;
	    case ANA_WORD:
	      n = (scalar_value(thisLhs).w)? 1: 0;
	      break;
	    case ANA_LONG:
	      n = (scalar_value(thisLhs).l)? 1: 0;
	      break;
	    case ANA_FLOAT:
	      n = (scalar_value(thisLhs).f)? 1: 0;
	      break;
	    case ANA_DOUBLE:
	      n = (scalar_value(thisLhs).d)? 1: 0;
	      break;
	  }
	  result = ANA_ERROR;
	  switch (bin_op_type(symbol)) { /* which operator? */
	    case ANA_ANDIF:
	      if (n == 0)	/* LHS is zero, so result is too */
		result= ANA_ZERO;
	      break;
	    case ANA_ORIF:
	      if (n == 1)	/* LHS is one, so result is too */
		result = ANA_ONE;
	      break;
	  }
	  if (result >= 0) {	/* result has been determined */
	    thisRhs = 1;
	    break;
	  }
	  /* result is not yet decided, need RHS */
	  if ((thisRhs = eval(bin_op_rhs(symbol))) == ANA_ERROR) {
	    result = ANA_ERROR;
	    thisRhs = 0;
	    break;
	  }
	  switch (symbol_class(thisRhs)) {
	    default:
	      /* RHS is non-scalar expression */
	      result = cerror(COND_NO_SCAL, bin_op_rhs(symbol));
	      break;
	    case ANA_SCAL_PTR:
	      /* transform scalar pointer to scalar */
	      thisRhs = dereferenceScalPointer(thisRhs);
	      /* FALL-THRU to ANA_SCALAR */
	    case ANA_SCALAR:
	      switch (scalar_type(thisRhs)) { /* is RHS non-zero? */
		case ANA_BYTE:
		  n = (scalar_value(thisRhs).b)? 1: 0;
		  break;
		case ANA_WORD:
		  n = (scalar_value(thisRhs).w)? 1: 0;
		  break;
		case ANA_LONG:
		  n = (scalar_value(thisRhs).l)? 1: 0;
		  break;
		case ANA_FLOAT:
		  n = (scalar_value(thisRhs).f)? 1: 0;
		  break;
		case ANA_DOUBLE:
		  n = (scalar_value(thisRhs).d)? 1: 0;
		  break; }
	      result = n? ANA_ONE: ANA_ZERO;
	      break;
	  }
      }
      if (symbol_context(thisLhs) == -compileLevel)
	zapTemp(thisLhs);
      if (symbol_context(thisRhs) == -compileLevel)
	zapTemp(thisRhs);
      while (tempVariableIndex > TEMPS_START
	     && symbol_class(tempVariableIndex - 1) == ANA_UNUSED)
	tempVariableIndex--;
      break;
    case ANA_BIN_OP:
      /* rhs, lhs, and binOp are static variables:
	 they are changed by
	 deeper nested evaluations.  So, we evaluate rhs and lhs first
	 into our own private dynamics variables, and store the
	 binOp for this evaluation only after evaluation of the operands */
      if ((thisLhs = eval(bin_op_lhs(symbol))) < 0) { /* get LHS */
	result = ANA_ERROR;
	break;
      }
      mark(thisLhs);		/* mark for deletion (if it is a temp) */
      if (symbol_class(thisLhs) == ANA_SCAL_PTR) /* scalar pointer */
	thisLhs = dereferenceScalPointer(thisLhs);
      if ((thisRhs = eval(bin_op_rhs(symbol))) < 0) { /* get RHS */
	result = ANA_ERROR;
	break;
      }
      mark(thisRhs);
      if (symbol_class(thisRhs) == ANA_SCAL_PTR)
	thisRhs = dereferenceScalPointer(thisRhs);
      lhs = thisLhs;
      rhs = thisRhs;
      if ((binOp = bin_op_type(symbol)) < 0
	  || binOp >= NUM_BIN_OP) {
	result = anaerror("Impossible binary operation (number %d)\n", 0, binOp);
	break;
      }
      topType = lhsType = symbol_type(lhs);
      if (symbol == pipeExec || pipeExec == 1) { /* piping may be possible */
	if (thisLhs == pipeSym || thisRhs == pipeSym)
	  /* but only if one of the operands is the pipe symbol */
	  pipeExec = 0;
	else			/* disallow - for now */
	  pipeExec = 1;
      } else
	if (!pipeExec)
	  pipeExec = 1;		/* disallow - for now */
      rhsType = symbol_type(rhs);
      if (rhsType > topType)
	topType = rhsType;
      if ((lhsType >= ANA_CFLOAT) ^ (rhsType >= ANA_CFLOAT)) {
	/* one argument is complex but the other one is not */
	if (lhsType == ANA_DOUBLE
	    || rhsType == ANA_DOUBLE
	    || lhsType == ANA_CDOUBLE
	    || rhsType == ANA_CDOUBLE)
	  topType = ANA_CDOUBLE;
	else
	  topType = ANA_CFLOAT;
      }
      /* power function returns at least ANA_FLOAT */
      if (binOp == ANA_POW) {
	if (topType > ANA_CDOUBLE)
	  return cerror(ILL_TYPE, (lhsType >= ANA_CFLOAT)? lhs: rhs);
	if (topType < ANA_FLOAT)
	  topType = ANA_FLOAT;
      }
      /* all logical function return ANA_LONG */
      if (binOp >= ANA_EQ && binOp < ANA_POW)
	topType = ANA_LONG;
      if (binOp >= ANA_OR && binOp < ANA_POW) {
	if (lhsType > ANA_LONG) {
	  result = cerror(NO_FLT_COND, lhs, binOpName[binOp],
			  typeName(lhsType));
	  break;
	}
	if (rhsType > ANA_LONG) {
	  result = cerror(NO_FLT_COND, rhs, binOpName[binOp],
			  typeName(rhsType));
	  break;
	}
      } 
      switch (symbol_class(lhs)) {
	case ANA_RANGE:
	  if (range_scalar(lhs) && symbol_class(rhs) == ANA_SCALAR) {
	    result = evalRangeScalarBinOp();
	  } else
	    result = -ILL_COMB-1;
	  break;
	case ANA_SCALAR: case ANA_CSCALAR:
	  switch (symbol_class(rhs)) {
	    case ANA_SCALAR: case ANA_CSCALAR:
	      result = evalScalarBinOp();
	      break;
	    case ANA_ARRAY: case ANA_CARRAY:
	      if (isNumericalType(topType)) {
		result = evalScalarArrayBinOp();
	      } else
		result = -ILL_COMB-1;
	      break;
	    case ANA_RANGE:
	      if (range_scalar(rhs)) { /* only allow binary operations
					on ranges if they are scalar ranges */
		result = evalScalarRangeBinOp();
		break;
	      }
	      /* else fall-thru */
	    default:
	      result = -ILL_COMB-1;
	      break;
	  }
	  break;
	case ANA_ARRAY: case ANA_CARRAY:
	  switch (symbol_class(rhs)) {
	    case ANA_ARRAY: case ANA_CARRAY:
	      result = evalArrayBinOp();
	      break;
	    case ANA_SCALAR: case ANA_CSCALAR:
	      if (isNumericalType(topType)) {
		result = evalArrayScalarBinOp();
		break;
	      } else
		result = -ILL_COMB-1;
	      break;
	    case ANA_STRING:
	      if (symbol_type(lhs) == ANA_STRING_ARRAY) {
		result = evalSArrayStringBinOp();
		break;
	      } else
		result = -ILL_COMB-1;
	      break;
	    default:
	      result = -ILL_COMB-1;
	  }
	  break;
	case ANA_STRING:
	  switch (symbol_class(rhs)) {
	    case ANA_STRING:
	      result = evalStringBinOp();
	      break;
	    case ANA_ARRAY:
	      if (array_type(rhs) == ANA_STRING_ARRAY) {
		result = evalStringSArrayBinOp();
		break;
	      }
	      /* else fall through to default */
	    default:
	      result = -ILL_COMB-1;
	  }
	  break;
	default:
	  result = -ILL_COMB-1;
	  break;
      }
      updateIndices();
      break;
    case ANA_STRUCT_PTR:
      return evalStructPtr(symbol);
    default:
      result =
	anaerror("Sorry, class %d symbols are not implemented in eval()\n",
	      symbol, n);
      break;
  }
  if (result > 0)
    unMark(result);
  zapMarked();
  if (result == -ILL_COMB-1)
    return cerror(ILL_COMB, symbol,
		  (symbol_class(lhs) != ANA_UNUSED)?
		  typeName(symbol_type(lhs)): "",
		  className(symbol_class(lhs)), binOpSign[binOp],
		  (symbol_class(rhs) != ANA_UNUSED)?
		  typeName(symbol_type(rhs)): "",
		  className(symbol_class(rhs)));
  if (result > 0)
    return eval(result);
  else
    return cerror(-1, symbol);
}
/*----------------------------------------------------------*/
int evals(int nsym)
     /* always evaluate ANA_SCAL_PTR to ANA_SCALAR or ANA_TEMP_STRING */
{ int	temp;
  
  temp = evalScalPtr;
  evalScalPtr = 1;
  nsym = eval(nsym);
  evalScalPtr = temp;
  return nsym;
}
/*----------------------------------------------------------*/
#define UNKNOWN	-1
#define NEXT -2
branchInfo checkBranch(int lhs, int rhs)
     /* determines at what level in the assignment <lhs> = <rhs> the <lhs> can */
     /* be used for storage of intermediate results */
{
  branchInfo	result = { 0, 0, 0, 0}, branch1, branch2;
  static int	depth, lhsSize;
  word	*args;
  int	n;
  
  if (!rhs)			/* initialization */
  { depth = 0;
    if (sym[lhs].class == ANA_ARRAY)
      lhsSize = symbol_memory(lhs); /* piping only for arrays */
    else lhsSize = 0;
    return result; }
  depth++;
  switch (sym[rhs].class)
  { case ANA_ARRAY:
      
      result.size = symbol_memory(rhs);
      if (result.size == lhsSize) /* this array does not need to be */
	/* evaluated, so start piping (if any) */
	/* at NEXT level up */
      { result.depth = depth;
	result.symbol = NEXT; }
      else			/* wrong size, no pipe */
      { result.depth = -1;
	result.symbol = 0; }
      result.containLHS = (rhs == lhs)? 1:
      0;
      break;
    case ANA_SCALAR:
    case ANA_SCAL_PTR:
      
      result.depth = -1;	/* no piping for scalars. */
      result.symbol = 0;
      result.containLHS = (rhs == lhs)? 1:
      0;
      result.size = 0;		/* no extra allocated memory */
      break;
    default:
      /* can be anything */
      result.depth = depth;
      result.symbol = rhs;
      result.containLHS = 1;	/* worst-case assumption */
      result.size = UNKNOWN;	/* unknown size, so no piping */
      break;
    case ANA_INT_FUNC:
      
      /* functions that allow piping have:
	 a keyList structure with */
      /* the pipe member unequal to zero */
      if (!function[int_func_number(rhs)].keys
	  || !((keyList *) function[int_func_number(rhs)].keys)->pipe)
	/* no piping */
      { result.depth = depth;
	result.symbol = rhs;
	result.size = UNKNOWN; } /* unknown size, so no piping */
      else result.size = 0;	/* flags that piping is possible */
      result.containLHS = 0;
      n = int_func_num_arguments(rhs); /* # arguments */
      if (n)
      { args = int_func_arguments(rhs);	/* pointer to start of list */
	branch2 = branch1 = checkBranch(lhs, *args); /* check first arg */
	if (!result.size)	/* piping is OK */
	  result.size = branch1.size;
	result.containLHS = branch1.containLHS;
	/* if LHS was not in first argument, check other arguments */
	/* until LHS is found in argument or until all arguments have */
	/* been checked. */
	while ((!branch2.containLHS || branch2.symbol == NEXT) && --n)
	{ branch2 = checkBranch(lhs, *++args);
	  if (branch2.containLHS)
	    result.containLHS = 1; }
      }
      else result.size = UNKNOWN; /* no arguments, result size unknown */
      if (result.size == lhsSize) /* try piping */
      { if (branch1.containLHS)
	{ if (branch1.symbol == NEXT)
	  { if (branch2.containLHS && branch2.symbol != NEXT)
				/* pipe branch2 symbol */
	    { result.symbol = branch2.symbol;
	      result.depth = branch2.depth; }
	    else		/* no piping here */
	    { result.symbol = rhs;
	      result.depth = depth; }
	  } else		/* pipe 1st arg */
	  { result.symbol = branch1.symbol;
	    result.depth = branch1.depth; }
	} else			/* no piping */
	{ result.symbol = rhs;
	  result.depth = depth; }
      } else			/* possible pipe only after argument eval */
      { result.symbol = rhs;
	result.depth = depth; }
      break;
    case ANA_BIN_OP:
      branch1 = checkBranch(lhs, bin_op_lhs(rhs));
      branch2 = checkBranch(lhs, bin_op_rhs(rhs));
      if (branch1.containLHS && branch2.containLHS) /* both contain LHS */
      { result.containLHS = 1;
	result.symbol = rhs;	/* only pipe after branches have been eval'd */
	result.depth = depth; }
      else if (branch1.containLHS) /* only first operand contains LHS */
      { result.containLHS = 1;
	if (branch1.size == symbol_memory(lhs))	/* proper size */
	{ result.symbol = branch1.symbol == NEXT? rhs:
	    branch1.symbol;
	  result.depth = branch1.depth; }
	else			/* wrong size */
	{ result.symbol = rhs;
	  result.depth = depth; }
      }
      else if (branch2.containLHS) /* only 2nd operand contains LHS */
      { result.containLHS = 1;
	if (branch2.size == sym[lhs].spec.array.bstore)
	{ result.symbol = branch2.symbol == NEXT? rhs:
	    branch2.symbol;
	  result.depth = branch2.depth; }
	else
	{ result.symbol = rhs;
	  result.depth = depth; }
      }
      else			/* LHS in neither branch */
      { result.containLHS = 0;
	if (branch1.depth >= branch2.depth) /* take longest branch */
	{ result.symbol = branch1.symbol;
	  result.depth = branch1.depth; }
	else
	{ result.symbol = branch2.symbol;
	  result.depth = branch1.depth; }
      }
      if (branch1.size == UNKNOWN || branch2.size == UNKNOWN)
	result.size = UNKNOWN;	/* unknown size, no piping */
      else result.size = (branch1.size > branch2.size)?
	branch1.size:
	branch2.size;
      break;
    }
  depth--;
  return result;
}
/*----------------------------------------------------------*/
int	pipeExec = 0, pipeSym = 0;
branchInfo checkTree(int lhs, int rhs)
{
  branchInfo	result;
  
  checkBranch(lhs, 0);		/* initialize */
  result = checkBranch(lhs, rhs);
  return result;
}      
/*----------------------------------------------------------*/
