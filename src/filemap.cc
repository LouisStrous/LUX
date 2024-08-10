/* This is file filemap.cc.

Copyright 2013-2014 Louis Strous

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
// File filemap.c
// LUX routines implementing file arrays (file maps).
// Routines to handle file maps through file arrays
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "action.hh"

//-------------------------------------------------------------------------
int32_t filemap(Symboltype type, ArgumentCount narg, Symbol ps[])
/* Create a file map symbol (file array) and stores array structure,
   file name and offset */
{
 int32_t        iq, ndim, dims[8], mq, get_dims(int32_t *, int32_t *, int32_t *);
 char        *p;

 iq = ps[1];                                // file name
 if (symbol_class(iq) != LUX_STRING)
   return cerror(NEED_STR, iq);
 p = expand_name(string_value(iq), "");
 iq = ps[2];                                // dimensions
 ndim = narg - 2;
 if (ndim > MAX_DIMS)
   return cerror(N_DIMS_OVR, iq);        // too many dims
 if (get_dims(&ndim, &ps[2], dims) != 1)
   return -1;                        // some error in dims
 getFreeTempVariable(iq);
 symbol_class(iq) = LUX_FILEMAP;
 file_map_type(iq) = type;
 symbol_context(iq) = -1;        // ?
 symbol_line(iq) = curLineNumber;
                                        // memory requirement
 mq = sizeof(Array) + sizeof(int32_t)*(*ps != 0) + strlen(p) + 1;
 symbol_memory(iq) = mq;
 file_map_header(iq) = (Array*) malloc(mq);
 if (!file_map_header(iq))
   return luxerror("Memory allocation error\n", 0);
 if (internalMode & 1)
   set_file_map_readonly(iq);        // /READONLY
 else
   set_file_map_readwrite(iq);  // read-write
 if (internalMode & 2)                // /SWAP
   set_file_map_swap(iq);
 file_map_num_dims(iq) = ndim;
 memcpy(file_map_dims(iq), dims, ndim*sizeof(int32_t)); // copy dimensions
 if (*ps) {
   set_file_map_has_offset(iq);
   file_map_offset(iq) = int_arg(*ps);
 }
 strcpy(file_map_file_name(iq), p); // copy file name
 return iq;
}
//-------------------------------------------------------------------------
int32_t lux_bytfarr(ArgumentCount narg, Symbol ps[])
// Create a LUX_INT8 file array (file map)
{ return filemap(LUX_INT8, narg, ps); }
REGISTER(bytfarr, f, bytfarr, 3, MAX_DIMS + 1, "%1%offset:1readonly:2swap");
REGISTER(bytfarr, f, uint8farr, 3, MAX_DIMS + 1, "%1%offset:1readonly:2swap");
//-------------------------------------------------------------------------
int32_t lux_intfarr(ArgumentCount narg, Symbol ps[])
// Create a LUX_INT16 file array (file map)
{ return filemap(LUX_INT16, narg, ps); }
REGISTER(intfarr, f, intfarr, 3, MAX_DIMS + 1, "%1%offset:1readonly:2swap");
REGISTER(intfarr, f, int16farr, 3, MAX_DIMS + 1, "%1%offset:1readonly:2swap");
//-------------------------------------------------------------------------
int32_t lux_lonfarr(ArgumentCount narg, Symbol ps[])
// Create a LUX_INT32 file array (file map)
{ return filemap(LUX_INT32, narg, ps); }
REGISTER(lonfarr, f, lonfarr, 3, MAX_DIMS + 1, "%1%offset:1readonly:2swap");
REGISTER(lonfarr, f, int32farr, 3, MAX_DIMS + 1, "%1%offset:1readonly:2swap");
//-------------------------------------------------------------------------
int32_t lux_int64farr(ArgumentCount narg, Symbol ps[])
// Create a LUX_INT64 file array (file map)
{ return filemap(LUX_INT64, narg, ps); }
REGISTER(int64farr, f, int64farr, 3, MAX_DIMS + 1, "%1%offset:1readonly:2swap");
//-------------------------------------------------------------------------
int32_t fltfarr(ArgumentCount narg, Symbol ps[])
// Create a LUX_FLOAT file array (file map)
{ return filemap(LUX_FLOAT, narg, ps); }
//-------------------------------------------------------------------------
int32_t dblfarr(ArgumentCount narg, Symbol ps[])
// Create a LUX_DOUBLE file array (file map)
{ return filemap(LUX_DOUBLE, narg, ps); }
//-------------------------------------------------------------------------
int32_t cfltfarr(ArgumentCount narg, Symbol ps[])
// Create a LUX_CFLOAT file array (file map)
{ return filemap(LUX_CFLOAT, narg, ps); }
//-------------------------------------------------------------------------
int32_t cdblfarr(ArgumentCount narg, Symbol ps[])
// Create a LUX_CDOUBLE file array (file map)
{ return filemap(LUX_CDOUBLE, narg, ps); }
//-------------------------------------------------------------------------
int32_t lux_i_file_output(FILE *fp, Pointer q, int32_t assoctype,
 int32_t offsym, int32_t dsize, int32_t fsize, int32_t baseOffset)
/* use a file array as a guide to writing into a file, and an
 index array as a guide to the positions where to write */
{
  int32_t        *qi, error = 0, size, dindx, i;
  Array        *h;

  if (sym[offsym].type != LUX_INT32)
    offsym = lux_long(1, &offsym);
  switch (symbol_class(offsym))
  { case LUX_SCAL_PTR:
      offsym = dereferenceScalPointer(offsym);
    case LUX_SCALAR:
      qi = &sym[offsym].spec.scalar.i32;
      dindx = 1;
      break;
    case LUX_ARRAY:
      h = HEAD(offsym);
      GET_SIZE(dindx, h->dims, h->ndim);
      qi = LPTR(h);
      break;
    default:
      fclose(fp);
      return cerror(ILL_CLASS, offsym); }
  if (dsize != dindx)
  { fclose(fp);
    printf("Need as many indices as data elements\n");
    return cerror(INCMP_ARR, offsym); }
  size = lux_type_size[assoctype];
  while (dindx--)
  { if (*qi < 0 || *qi >= fsize)
      error++;
    else if (fseek(fp, *qi*size + baseOffset, SEEK_SET))
    { fclose(fp);
      return cerror(POS_ERR, -1); }
    if (fwrite(q.ui8, size, 1, fp) != 1)
    { fclose(fp);
      return cerror(WRITE_ERR, -1); }
    qi++;
    q.ui8 += size; }
  fclose(fp);
  if (error)
    printf("INSERT - %1d indices out of range\n", error);
  return 1;
}
//-------------------------------------------------------------------------
int32_t lux_file_output(int32_t iq, int32_t jq, int32_t offsym, int32_t axsym)
     // use a file array as a guide to writing into a file
     // iq is sym # of file array, jq is rhs sym with data
{
 int32_t    ddat, *dat, dfile, *file, daxes, *axes, doff, *off, offset;
 int32_t    i, dattype, assoctype, ystep[MAX_DIMS], rstep[MAX_DIMS],
        tally[MAX_DIMS], done;
 int32_t    efile, n, *step, baseOffset;
 Pointer       q;
 FILE   *fp;
 char        *fname;

 if (file_map_readonly(iq))
   return luxerror("File array is read-only", iq);
 step = rstep;
 assoctype = file_map_type(iq);
 dattype = symbol_type(jq);
 if (assoctype != dattype)         // make proper type
   jq = lux_converts[assoctype](1, &jq);
 switch (symbol_class(jq)) {
   case LUX_SCAL_PTR: case LUX_SCALAR:
     done = ddat = 1;
     dat = &done;
     q.i32 = &scalar_value(iq).i32;
     break;
   case LUX_ARRAY:
     ddat = array_num_dims(jq);
     dat = array_dims(jq);
     q.i32 = (int32_t*) array_data(jq);
     break;
   default:
     return cerror(ILL_CLASS, jq);
 }
 dfile = file_map_num_dims(iq);
 file = file_map_dims(iq);
 fname = file_map_file_name(iq);
 /* Note:  We want to open the file for updating at any position in that file.
    If it doesn't exist yet, then it should be created.  DEC Ultrix (Utrecht)
    does all this with the a+ file access mode.  SGI Irix (Palo Alto) doesn't
    allow overwriting the existing contents of a file with a+ mode, and
    complains if a file doesn't yet exist when opened with r+.
    Solution:  catch r+ complaint and do a w+ if necessary */
 if ((fp = fopen(fname, "r+")) == NULL
     && (fp = fopen(fname, "w+")) == NULL) {
   printf("File %s; ", fname);
   return cerror(ERR_OPEN, iq);
 }
 if (ddat > dfile) {
   printf("Source has too many dimensions for file array");
   return cerror(INCMP_DIMS, jq);
 }
 baseOffset = file_map_has_offset(iq)? file_map_offset(iq): 0;
 if (axsym == -2) {
   GET_SIZE(n, dat, ddat);
   ddat = n;
   GET_SIZE(n, file, dfile);
   dfile = n;
   return lux_i_file_output(fp, q, assoctype, offsym, ddat, dfile,
                             baseOffset);
 }
 if (!offsym) {
   offset = 0;
   doff = 0;
 } else {
   if (symbolIsNumericalArray(offsym)) { // coordinate offset
     doff = array_size(offsym);
     if (doff != dfile) {
       puts("Offset has wrong # dimensions");
       return cerror(INCMP_DIMS, offsym);
     }
     off = (int32_t*) array_data(offsym);
     CK_SGN(off, doff, 1, iq);
   } else {
     offset = int_arg(offsym);
     doff = 0;
   }
 }
 if (axsym == -1) {                // assoc(off) = .. quick insert
   GET_SIZE(done, dat, ddat);
   ddat = 1;                                    // fake 1D insert
   daxes = 0;
   dat = &done;
 } else if (axsym) {                // redirection axes
   daxes = array_size(axsym);
   if (daxes != ddat) {
     puts("Wrong number of axes");
     return cerror(INCMP_DIMS, axsym);
   }
   axes = (int32_t*) array_data(axsym);
   CK_SGN(axes, daxes, 2, iq);                  // nonnegative?
   CK_MAG(dfile, axes, daxes, 2, iq);          // within rearrange bounds?
 } else
   daxes = 0;
 for (i = 0; i < ddat; i++)
   tally[i] = 1;
 n = *ystep = 1;
 for (i = 1; i < dfile; i++)
   ystep[i] = (n *= file[i - 1]);
 n = lux_type_size[assoctype];
 if (doff) {
   offset = 0;
   for (i = 0; i < doff; i++)
     offset += off[i]*ystep[i];
 } else {
   GET_SIZE(efile, dat, ddat);
   offset *= efile;
 }
 offset *= n;
 if (axsym != -1)
   for (i = 0; i < ddat; i++)
     if (off[(daxes)? axes[i]: i] + dat[i] > file[(daxes)? axes[i]: i]) {
       printf("%d + %d > %d\n", off[(daxes)? axes[i]: i], dat[i],
              file[(daxes)? axes[i]: i]);
       puts("Specified coordinates reach outside array");
       return cerror(INCMP_DIMS, iq);
     }
 for (i = 0; i < ddat; i++)
   step[i] = ystep[(daxes)? axes[i]: i]*n;
 for (i = ddat - 1; i; i--)
   step[i] -= step[i - 1]*dat[i - 1];
 *step -= n;                    // because writing advances file pointer
 if (fseek(fp, offset + baseOffset, SEEK_SET))
   return cerror(POS_ERR, iq);
 while (!*step && ddat) {                         // can write in batches
   n *= *dat;                                   // write a row at a time
   step++;                                      // one level higher
   dat++;
   ddat--;
 }
 if (!ddat)
   done = 1;
 do {
   if (fwrite(q.ui8, n, 1, fp) != 1)
     return cerror(WRITE_ERR, iq);
   q.ui8 += n;
   for (i = 0; i < ddat; i++) {
     if (tally[i]++ != dat[i]) {
       done = 0;
       break;
     }
     tally[i] = 1;
     done = 1;
     fseek(fp, step[i + 1], SEEK_CUR);
   }
  if (!done && fseek(fp, *step, SEEK_CUR))
    return cerror(POS_ERR, iq);
 } while (!done);
 fclose(fp);
 return 1;
}
//-------------------------------------------------------------------------
int32_t lux_fzarr(ArgumentCount narg, Symbol ps[])
// FZARR(name) returns a file array symbol appropriate for an
// uncompressed FZ file.
{
  char        *name;
  int32_t        wwflag, iq, mq;
  FILE        *fp;
  FzHead        *fh;
  int32_t        ck_synch_hd(FILE *, FzHead *, int32_t *);

  if (symbol_class(*ps) != LUX_STRING)
    return cerror(NEED_STR, *ps);
  name = string_value(*ps);
  fp = Fopen(expand_name(name, NULL), "r");
  if (!fp)                        // could not open file for reading
    return cerror(ERR_OPEN, 0);
  fh = (FzHead *) scrat;
  if (ck_synch_hd(fp, fh, &wwflag) < 0)
    return LUX_ERROR;
  fclose(fp);
  if (fh->subf % 128 == 1)        // compressed data
    return luxerror("FZARR cannot deal with compressed data", 0);

  getFreeTempVariable(iq);
  symbol_class(iq) = LUX_FILEMAP;
  file_map_type(iq) = (Symboltype) fh->datyp;
  symbol_context(iq) = -1;        // ?
  symbol_line(iq) = curLineNumber;
                                        // memory requirement
  mq = sizeof(Array) + sizeof(int32_t) + strlen(name) + 1;
  symbol_memory(iq) = mq;
  file_map_header(iq) = (Array*) malloc(mq);
  if (!file_map_header(iq))
    return luxerror("Memory allocation error\n", 0);
  if (internalMode & 1)
    set_file_map_readonly(iq);        // /READONLY
  file_map_num_dims(iq) = fh->ndim;
  set_file_map_has_offset(iq);        // offset will be specified

   // copy dimensions
  memcpy(file_map_dims(iq), fh->dim, fh->ndim*sizeof(int32_t));
  file_map_offset(iq) = 512*fh->nhb; // offset
  strcpy(file_map_file_name(iq), name); // copy file name
  return iq;
}
//-------------------------------------------------------------------------
