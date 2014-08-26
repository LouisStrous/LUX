/* This is file idl.c.

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
/* LUX routines for reading IDL Save files
   Louis Strous 19sep98 */
#include <stdio.h>
#include "action.hh"

int32_t lux_idlrestore(int32_t narg, int32_t ps[])
/* IDLRESTORE,<filename> restores all variables from the IDL Save file
   with name <filename>.  Supports scalars, strings, and numerical arrays
   LS 18sep98 */
{
  uint8_t	bytes[8];
  char	*p;
  int32_t	ints[3], dims[MAX_DIMS], n, var, ndim, nread;
  Symboltype type;
  FILE	*fp;
  void	endian(void *, int32_t, int32_t);
  int32_t	installString(char const*);
  Scalar	value;
  pointer	pp, data;

  if (!symbol_class(ps[0]) == LUX_STRING)
    return cerror(NEED_STR, ps[0]);

  /* open the file */
  fp = fopen(expand_name(string_value(ps[0]), NULL), "r");
  if (!fp)
    return cerror(ERR_OPEN, ps[0]);

  fread(bytes, 1, 4, fp);
  if (ferror(fp) || feof(fp)) {
    fclose(fp);
    return luxerror("Could not check file %s for IDL Save format", ps[0],
		 expname);
  }

  if (bytes[0] != 83
      || bytes[1] != 82
      || bytes[2] != 0
      || bytes[3] != 4) {
    fclose(fp);
    return luxerror("File %s does not appear to be an IDL Save file", ps[0],
		 expname);
  }

  /* we now assume that the file is in fact an IDL save file and do not
     check for I/O errors anymore */

  pp.b = &value.b;

  fseek(fp, 4, SEEK_CUR);	/* skip one int32_t */
  fread(ints, 4, 1, fp);	/* read one int32_t */
#if !LITTLEENDIAN
  endian(ints, 4, LUX_INT32);
#endif
  fseek(fp, ints[0], SEEK_SET);	/* go to the indicated offset */

  nread = 0;

  do {
    fread(ints, 4, 2, fp);	/* read 2 ints */
#if !LITTLEENDIAN
    endian(ints, 8, LUX_INT32);
#endif
    if (ints[0] == 14) {
      fseek(fp, ints[1], SEEK_SET);
      continue;
    } else if (ints[0] != 2) {	/* all done */
      fclose(fp);
      if (!nread)
	printf("IDLRESTORE - No variables found in IDL Save file \"%s\"??\n",
	       expname);
      return LUX_OK;
    }

    nread++;

    fread(ints, 4, 3, fp);	/* read 3 ints */
#if !LITTLEENDIAN
    endian(ints, 12, LUX_INT32);
#endif

    n = ints[2];		/* size of name */
    fread(curScrat, 1, n, fp);
    curScrat[n] = '\0';

    printf("restoring %s\n", curScrat);
    n = installString(curScrat);
    var = findVar(n, curContext);	/* get the variable */

    /* align on next 4-Byte boundary */
    n = 3 - (ints[2] - 1) % 4;
    if (n)
      fseek(fp, n, SEEK_CUR);

    fread(ints, 4, 3, fp);
#if !LITTLEENDIAN
    endian(ints, 12, LUX_INT32);
#endif
    if (ints[1] == 20) {	/* array */
      type = (Symboltype) ints[0];

      fseek(fp, 12, SEEK_CUR);
      fread(ints, 4, 1, fp);
#if !LITTLEENDIAN
      endian(ints, 4, LUX_INT32);
#endif
      ndim = ints[0];		/* number of dimensions */

      fseek(fp, 12, SEEK_CUR);	/* skip 3 ints */

      fread(dims, 4, 8, fp);	/* read dimensions */
#if !LITTLEENDIAN
      endian(dims, 4*ndim, LUX_INT32);
#endif

      redef_array(var, (Symboltype) (type - 1), ndim, dims);
      fseek(fp, 4, SEEK_CUR);

      n = array_size(var);
      data.b = (uint8_t*) array_data(var);
      switch (type) {
	case 1:			/* bytes stored as longs (!) */
	  fseek(fp, 4, SEEK_CUR); /* skip extra int32_t */
	  fread(data.b, 1, n, fp);
	  n = 3 - (n - 1) % 4;
	  if (n)		/* align on int32_t boundary */
	    fseek(fp, n, SEEK_CUR);
	  break;
	case 2:			/* int16_t */
	  /* words are stored as longs (!) so we have to read them one by
	     one and Byte-swap if necessary */
	  while (n--) {
	    fread(pp.b, 4, 1, fp);
#if !LITTLEENDIAN
	    endian(pp.b, 4, LUX_INT32);
#endif
	    *data.w++ = *pp.l;
	  }
	  break;
	case 3:	case 4: case 5:	/* long, float, double */
	  fread(data.b, lux_type_size[type - 1], n, fp);
#if !LITTLEENDIAN
	  endian(data.b, lux_type_size[type - 1]*n, LUX_INT32);
#endif
	  break;
	default:
	  fclose(fp);
	  return luxerror("Unsupported data type %d in IDL Save file %s\n", ps[0],
		       type, expname);
      }
    } else {			/* assume scalar or string */
      switch (ints[0]) {
	case 1:			/* Byte */
	  fread(pp.b, 1, 4, fp);
#if !LITTLEENDIAN
	  endian(pp.b, 4, LUX_INT32);
#endif	
	  value.b = value.l;
	  redef_scalar(var, LUX_INT8, &value.b);
	  break;
	case 2:			/* int16_t */
	  fread(pp.b, 1, 4, fp); /* words are stored as ints */
#if !LITTLEENDIAN
	  endian(pp.b, 4, LUX_INT32);
#endif
	  value.w = value.l;
	  redef_scalar(var, LUX_INT16, &value.w);
	  break;
	case 3:			/* long */
	  fread(pp.b, 1, 4, fp);
#if !LITTLEENDIAN
	  endian(pp.b, 4, LUX_INT32);
#endif
	  redef_scalar(var, LUX_INT32, &value.l);
	  break;
	case 4:			/* float */
	  fread(pp.b, 1, 4, fp);
#if !LITTLEENDIAN
	  endian(pp.b, 4, LUX_FLOAT);
#endif
	  redef_scalar(var, LUX_FLOAT, &value.f);
	  break;
	case 5:			/* double */
	  fread(pp.b, 1, 8, fp);
#if !LITTLEENDIAN
	  endian(pp.b, 8, LUX_DOUBLE);
#endif
	  redef_scalar(var, LUX_DOUBLE, &value.d);
	  break;
	case 7:			/* string */
	  fread(ints, 4, 2, fp);
#if !LITTLEENDIAN
	  endian(ints, 4, LUX_INT32);
#endif
	  redef_string(var, ints[0]);
	  p = string_value(var);
	  fread(p, 1, ints[0], fp);
	  p[ints[0]] = '\0';	/* terminate string */
	  n = 3 - (ints[0] - 1) % 4;
	  if (n)
	    fseek(fp, n, SEEK_CUR); /* align on int32_t */
	  break;
	default:
	  fclose(fp);
	  return luxerror("Unsupported data type %d in IDL Save file %s\n", ps[0],
		       type, expname);
      }
    }	  
  } while (1);
}
/*-----------------------------------------------------------------------*/
int32_t lux_idlread_f(int32_t narg, int32_t ps[])
/* IDLREAD(<var>, <filename>) restores the first variable from the IDL
   Save file with name <filename> into <var>.  Supports scalars, strings,
   and numerical arrays.  Returns LUX_ONE on success, LUX_ZERO on failure.
   LS 18sep98 */
{
  uint8_t	bytes[8];
  char	*p;
  int32_t	ints[3], dims[MAX_DIMS], n, var, ndim, type;
  FILE	*fp;
  void	endian(void *, int32_t, int32_t);
  Scalar	value;
  pointer	pp, data;

  if (!symbol_class(ps[1]) == LUX_STRING)
    return cerror(NEED_STR, ps[1]);

  /* open the file */
  fp = fopen(expand_name(string_value(ps[1]), NULL), "r");
  if (!fp)
    return LUX_ZERO;

  fread(bytes, 1, 4, fp);
  if (ferror(fp) || feof(fp)) {
    fclose(fp);
    return LUX_ZERO;
  }

  if (bytes[0] != 83
      || bytes[1] != 82
      || bytes[2] != 0
      || bytes[3] != 4) {
    fclose(fp);
    return LUX_ZERO;
  }

  /* we now assume that the file is in fact an IDL save file and do not
     check for I/O errors anymore */

  pp.b = &value.b;

  fseek(fp, 4, SEEK_CUR);	/* skip one int32_t */
  fread(ints, 4, 1, fp);	/* read one int32_t */
#if LITTLEENDIAN
  endian(ints, 4, LUX_INT32);
#endif
  fseek(fp, ints[0], SEEK_SET);	/* go to the indicated offset */
  
  do {
    fread(ints, 4, 2, fp);	/* read 2 ints */
#if LITTLEENDIAN
    endian(ints, 8, LUX_INT32);
#endif
    if (ints[0] == 14) {
      fseek(fp, ints[1], SEEK_SET);
      continue;
    } else if (ints[0] != 2) {	/* all done, but we didn't read anything */
      fclose(fp);
      return LUX_ZERO;		/* so some error must have occurred */
    }
    break;
  } while (1);
    
  fread(ints, 4, 3, fp);	/* read 3 ints */
#if LITTLEENDIAN
  endian(ints, 12, LUX_INT32);
#endif
    
  n = ints[2];			/* size of name */
  fseek(fp, n, SEEK_CUR);	/* skip name */

  var = ps[0];			/* get the variable */

  /* align on next 4-Byte boundary */
  n = 3 - (ints[2] - 1) % 4;
  if (n)
    fseek(fp, n, SEEK_CUR);

  fread(ints, 4, 3, fp);
#if LITTLEENDIAN
  endian(ints, 12, LUX_INT32);
#endif
  if (ints[1] == 20) {		/* array */
    type = ints[0];
    
    fseek(fp, 12, SEEK_CUR);
    fread(ints, 4, 1, fp);
#if LITTLEENDIAN
    endian(ints, 4, LUX_INT32);
#endif
    ndim = ints[0];		/* number of dimensions */

    fseek(fp, 12, SEEK_CUR);	/* skip 3 ints */

    fread(dims, 4, 8, fp);	/* read dimensions */
#if LITTLEENDIAN
    endian(dims, 4*ndim, LUX_INT32);
#endif

    redef_array(var, (Symboltype) (type - 1), ndim, dims);
    fseek(fp, 4, SEEK_CUR);

    n = array_size(var);
    data.b = (uint8_t*) array_data(var);
    switch (type) {
      case 1:			/* bytes stored as longs (!) */
	fseek(fp, 4, SEEK_CUR); /* skip extra int32_t */
	fread(data.b, 1, n, fp);
	break;
      case 2:			/* int16_t */
	/* words are stored as longs (!) so we have to read them one by
	   one and Byte-swap if necessary */
	while (n--) {
	  fread(pp.b, 4, 1, fp);
#if LITTLEENDIAN
	  endian(pp.b, 4, LUX_INT32);
#endif
	  *data.w++ = *pp.l;
	}
	break;
      case 3: case 4: case 5:	/* long, float, double */
	fread(data.b, lux_type_size[type - 1], n, fp);
#if LITTLEENDIAN
	endian(data.b, lux_type_size[type - 1]*n, LUX_INT32);
#endif
	break;
      default:
	fclose(fp);
	return LUX_ZERO;
    }
  } else {			/* assume scalar or string */
    switch (ints[0]) {
      case 1:			/* Byte */
	fread(pp.b, 1, 4, fp);
#if LITTLEENDIAN
	endian(pp.b, 4, LUX_INT32);
#endif	
	value.b = value.l;
	redef_scalar(var, LUX_INT8, &value.b);
	break;
      case 2:			/* int16_t */
	fread(pp.b, 1, 4, fp);	/* words are stored as ints */
#if LITTLEENDIAN
	endian(pp.b, 4, LUX_INT32);
#endif
	value.w = value.l;
	redef_scalar(var, LUX_INT16, &value.w);
	break;
      case 3:			/* long */
	fread(pp.b, 1, 4, fp);
#if LITTLEENDIAN
	endian(pp.b, 4, LUX_INT32);
#endif
	redef_scalar(var, LUX_INT32, &value.l);
	break;
      case 4:			/* float */
	fread(pp.b, 1, 4, fp);
#if LITTLEENDIAN
	endian(pp.b, 4, LUX_FLOAT);
#endif
	redef_scalar(var, LUX_FLOAT, &value.f);
	break;
      case 5:			/* double */
	fread(pp.b, 1, 8, fp);
#if LITTLEENDIAN
	endian(pp.b, 8, LUX_DOUBLE);
#endif
	redef_scalar(var, LUX_DOUBLE, &value.d);
	break;
      case 7:			/* string */
	redef_string(var, ints[2]);
	p = string_value(var);
	fread(ints, 4, 2, fp);
#if LITTLEENDIAN
	endian(ints + 1, 4, LUX_INT32);
#endif
	fread(p, 1, ints[1], fp);
	p[ints[1]] = '\0';	/* terminate string */
	break;
      default:
	fclose(fp);
	return LUX_ZERO;
    }
  }	  

  fclose(fp);
  return LUX_ONE;
}
/*-----------------------------------------------------------------------*/
