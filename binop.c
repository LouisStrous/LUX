#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "action.h"

#define ORDINARY	1
#define SCALAR_LEFT	2
#define SCALAR_RIGHT	3
int evalBinOp(int lhs, int rhs,
	      int (*function)(pointer, pointer, pointer))
/* supports "implicit dimensions", i.e. dimensions which are 1 in one of */
/* the operands and non-1 in the other.  The smaller operand is repeated */
/* as needed to service all elements of the larger operand. */
{
  int	result, i, i0, nRepeats[MAX_DIMS], action[MAX_DIMS], nAction = 0,
    tally[MAX_DIMS], nCumulR[MAX_DIMS], nCumulL[MAX_DIMS], ndim,
    bigOne;
  extern int	pipeSym, pipeExec;
  int ldims[MAX_DIMS], rdims[MAX_DIMS], lndims, rndims;
  int topType, ltype, rtype;

  if (numerical(lhs, ldims, &lndims, NULL, &lp) == ANA_ERROR)
    return ANA_ERROR;
  if (numerical(rhs, rdims, &rndims, NULL, &rp) == ANA_ERROR)
    return ANA_ERROR;
  ltype = symbol_type(lhs);
  rtype = symbol_type(rhs);
  topType = (ltype > rtype)? ltype: rtype;

  /* the arrays must have an equal number of dimensions, except for */
  /* possible trailing dimensions of one element */
  if (rndims > lndims) {/* rhs has more dims */
    ndim = lndims;	/* number of dimensions in output */
    bigOne = rhs;		/* rhs has more dims than lhs */
    for (i = ndim; i < rndims; i++)
      if (rdims[i] != 1)
	return cerror(INCMP_DIMS, rhs);
  } else {
    ndim = rndims;
    bigOne = (rndims < lndims)? lhs: 0;
				/* 0 indicates lhs has same #dims as rhs */
    for (i = rndims; i < lndims; i++)
      if (ldims[i] != 1)
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
  for (i0 = i = 0; i < ndim; i++) {
    if ((rdims[i] == 1) ^ (ldims[i] == 1)) {
      /* one is 1 and the other is not: implicit dimension */
      if (nRepeat > 1) {	/* already had some ordinary dimensions */
	nRepeats[nAction] = nRepeat; /* store combined repeat count */
	action[nAction++] = ORDINARY;
      }
      if (rdims[i] == 1) { /* rhs has dimension equal to 1 */
	action[nAction] = SCALAR_RIGHT;
	nRepeats[nAction] = ldims[i];
      } else {			/* lhs has dimension equal to 1 */
	action[nAction] = SCALAR_LEFT;
	nRepeats[nAction] = rdims[i];
      }
      nAction++;
      nRepeat = 1;		/* reset for ORDINARY count */
    } else if (rdims[i] != ldims[i])
      /* unequal and neither equal to 1 -> error */
      return cerror(INCMP_DIMS, rhs);
    else
      nRepeat *= rdims[i]; /* both equal but not to 1 */
  }
  if (nAction && nRepeat > 1) {	/* some ordinary dimensions at the end */
    nRepeats[nAction] = nRepeat;
    action[nAction++] = ORDINARY;
  }
  if (!nAction) {		/* plain binary operation, no implicit dims */
    if (ana_type_size[ltype] == ana_type_size[topType] /* lhs type OK */
	&& (lhs == bigOne || !bigOne) /* lhs is big enough */
	&& (isFreeTemp(lhs) || (!pipeExec && pipeSym == lhs))) /* and free */
      result = lhs;		/* use lhs to store result */
    else if (ana_type_size[rtype] == ana_type_size[topType]
	     && (rhs == bigOne || !bigOne)
	     && (isFreeTemp(rhs) || (!pipeExec && pipeSym == rhs)))
      result = rhs;		/* use rhs to store result */
    else if ((result = array_clone(bigOne? bigOne: lhs, topType)) < 0)
      return ANA_ERROR;		/* could not generate output symbol */
    tp.l = array_data(result);	/* output data */
    array_type(result) = topType;
    {
      int repeat = nRepeat;

      while (repeat--) {
	function(&lp, &rp, &tp, lhsType, rhsType, topType);
	lp.b += 
    }
    return result;
  } else {				/* implicit dimensions */
    int	lStride, rStride;
    char	done = 0;
    
    /* create result array: first calculate its number of elements */
    nRepeat = 1;
    for (i = 0; i < nAction; i++)
      nRepeat *= nRepeats[i];
    if (ana_type_size[ltype] == ana_type_size[topType] /* lhs type OK */
	&& array_size(lhs) == nRepeat /* and has correct size */
	&& (isFreeTemp(lhs) || (!pipeExec && pipeSym == lhs))) /* and free */
      result = lhs;		/* use lhs to store the result */
    else if (ana_type_size[rtype] == ana_type_size[topType]
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
      array_dims(result)[i] = MAX(ldims[i], rdims[i]);
    /* and any remaining ones are set equal to 1 */
    for (i = ndim;
	 i < ((bigOne == lhs)? lndims: rndims); i++)
      array_dims(result)[i] = 1;
    array_num_dims(result) = (bigOne == lhs)? lndims:
      rndims;

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
	  (*function)();
	  break;
	case SCALAR_LEFT:
	  (*function_sa)();
	  break;
	case SCALAR_RIGHT:
	  (*function_as)();
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
