/*------------------------------------------------------------------------- */
Int ana_cubic_spline(Int narg, Int ps[])
/* Cubic spline interpolation along the first dimension of an array.
   ynew = CSPLINE(xtab, ytab [, xnew][, d1, d2][,/keep])
     interpolates in the table of <ytab> versus <xtab> (must be in
     ascending order) at positions <xnew>.  <d1> and <d2> are (optional)
     values for the first derivatives at either end of the interpolation
     interval.  If they are not specified, then the second derivatives
     at either end are set to zero (i.e., the so-called natural cubic
     spline).  /KEEP signals retention of <xtab>, <ytab>, and the derived
     second derivatives for use in subsequent interpolations from the
     same table.
   ynew = CSPLINE(xnew)
     Interpolates at positions <xnew> in the table specified last in a
     call to CSPLINE, in which the /KEEP switch must have been specified.
   ynew = CSPLINE()
     Clear the table that was retained in a call to CSPLINE employing the
     /KEEP switch.  (In case the table is real big and you want to get
     rid of it.)
   LS 29apr96 */
{
  static char	haveTable = '\0';
  Int	xNewSym = 0, xTabSym = 0, yTabSym, d1Sym = 0, d2Sym, size,
	oldType, n;
  pointer	src, trgt, p, q;
  Float	a, b, c, x;
  static Int	nPoints, type;
  static pointer	xTable, yTable, der2;

  /* first check on all the arguments */
  if (!narg)			/* CSPLINE() */
  { if (haveTable)
    { Free(xTable.b);
      Free(yTable.b);
      Free(der2.b); }
    return ANA_ONE; }
  if (narg >= 2)		/* have <xtab> and <ytab> */
  { xTabSym = *ps++;
    yTabSym = *ps++; }
  if (narg % 2 == 1)		/* have <xnew> */
    xNewSym = *ps++;
  if (narg >= 4)		/* have <d1> and <d2> */
  { d1Sym = *ps++;
    d2Sym = *ps++; }

  if (xNewSym && !haveTable && !xTabSym)
    return anaerror("Need table to interpolate from", xNewSym);
  if (xTabSym && symbol_class(xTabSym) != ANA_ARRAY)
    return cerror(NEED_ARR, xTabSym);
  if (yTabSym && symbol_class(yTabSym) != ANA_ARRAY)
    return cerror(NEED_ARR, yTabSym);
  if (xTabSym && array_size(xTabSym) != array_size(yTabSym))
    return cerror(INCMP_ARR, yTabSym);

  if (haveTable && xTabSym)	/* want new table but still have old */
  { Free(xTable.b);
    Free(yTable.b);
    Free(der2.b);
    haveTable = '\0'; }
  
  if (xTabSym)			/* install new table */
  { nPoints = array_size(xTabSym);
    oldType = (array_type(xTabSym) > array_type(yTabSym))?
      array_type(xTabSym): array_type(yTabSym);
    type = (oldType < ANA_FLOAT)? ANA_FLOAT: oldType;
    size = ana_type_size[type]*nPoints;
    xTable.b = (Byte *) Malloc(size);
    yTable.b = (Byte *) Malloc(size);
    der2.b = (Byte *) Malloc(size);
    if (oldType == type)
    { memcpy(xTable.b, array_data(xTabSym), size);
      memcpy(yTable.b, array_data(yTabSym), size); }
    else
    { n = nPoints;
      src.b = array_data(xTabSym);
      trgt = xTable;
      switch (type)
      { case ANA_FLOAT:
	  switch (oldType)
	  { case ANA_BYTE:
	      while (n--)
		*trgt.f++ = (Float) *src.b++;
	      src.b = array_data(yTabSym);
	      trgt = yTable;
	      while (n--)
		*trgt.f++ = (Float) *src.b++;
	      break;
	    case ANA_WORD:
	      while (n--)
		*trgt.f++ = (Float) *src.w++;
	      src.b = array_data(yTabSym);
	      trgt = yTable;
	      while (n--)
		*trgt.f++ = (Float) *src.w++;
	      break;
	    case ANA_LONG:
	      while (n--)
		*trgt.f++ = (Float) *src.l++;
	      src.b = array_data(yTabSym);
	      trgt = yTable;
	      while (n--)
		*trgt.f++ = (Float) *src.l++;
	      break; }
	  break;
	case ANA_DOUBLE:
	  switch (oldType)
	  { case ANA_BYTE:
	      while (n--)
		*trgt.d++ = (Double) *src.b++;
	      src.b = array_data(yTabSym);
	      trgt = yTable;
	      while (n--)
		*trgt.d++ = (Double) *src.b++;
	      break;
	    case ANA_WORD:
	      while (n--)
		*trgt.d++ = (Double) *src.w++;
	      src.b = array_data(yTabSym);
	      trgt = yTable;
	      while (n--)
		*trgt.d++ = (Double) *src.w++;
	      break;
	    case ANA_LONG:
	      while (n--)
		*trgt.d++ = (Double) *src.l++;
	      src.b = array_data(yTabSym);
	      trgt = yTable;
	      while (n--)
		*trgt.d++ = (Double) *src.l++;
	      break; 
	    case ANA_FLOAT:
	      while (n--)
		*trgt.d++ = (Double) *src.f++;
	      src.b = array_data(yTabSym);
	      trgt = yTable;
	      while (n--)
		*trgt.d++ = (Double) *src.f++;
	      break; }
	  break; }
    }
    /* for now we force natural cubic splines */
    if (type == ANA_FLOAT)
      der2.f[0] = der2.f[nPoints - 1] = 0.0;
    else
      der2.d[0] = der2.d[nPoints - 1] = 0.0;
    temp.b = (Byte *) Malloc(size*nPoints);
    src = xTable;
    trgt = yTable;
    switch (type)
    { case ANA_FLOAT:
	*p.f++ = 0.0;		/*  point zero */
	*q.f++ = 1.0;
	a = (src.f[1] - src.f[0])/6.0;
	c = (src.f[1] - src.f[0])/6.0;
	b = 2*(c - a);
	*p.f++ = *trgt.f++/c;	/*  point one */
	*q.f++ = -b/c;
	for (i = 2; i < nPoints - 1; i++)
	{ a = c;
	  src.f++;
	  c = (src.f[1] - src.f[0])/6.0;
	  b = 2*(c - a);
	  *p.f++ = (*trgt.f++ - b*p.f[-1] - a*p.f[-2])/c;
	  *q.f++ = (-b*q.f[-1] - a*q.f[-2])/c; }
	x = (*trgt.f - a*p.f[-2] - b*p.f[-1])/(b*q.f[-1] + a*q.f[-2]);
	for (i = nPoints - 1; i; i--)
	{ p.f--;
	  der2.f[i] = p.f[0] + q.f[0]*x; }
	break;
      }
    haveTable = 1; }
  
  return ANA_ONE;
}