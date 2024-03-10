/* This is file strous.cc.

Copyright 2013-2024 Louis Strous

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
// File strous.c
// Various LUX routines by L. Strous.
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <algorithm>            // for std::binary_search
#include <iterator>             // for std::distance
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
//#include <malloc.h>
#include <ctype.h>
#include <float.h>
#include <sys/stat.h>
#include <limits.h>
#include <stdarg.h>
#include <time.h>               // for difftime

#include "action.hh"
#include "install.hh"
#include "format.hh"
#include "editor.hh"            // for BUFSIZE
#include "readline.h"
#include "FloatingPointAccumulator.hh"

int16_t         stack[STACKSIZE], *stackPointer = &stack[STACKSIZE];
extern int32_t  stackSym;
int32_t         lux_convert(int32_t, int32_t [], Symboltype, int32_t), copyToSym(int32_t, int32_t),
  lux_replace(int32_t, int32_t), format_check(char *, char **, int32_t),
  f_decomp(float *, int32_t, int32_t), f_solve(float *, float *, int32_t, int32_t);
void    symdumpswitch(int32_t, int32_t);
//-------------------------------------------------------------------------
int32_t lux_distr(ArgumentCount narg, Symbol ps[])
/* DISTR,target,bins,values  puts each <value> in the corresponding
   <bin> of <target>.  LS */
{
 int32_t        iq, ntarget, i, n, *bin, nout = 0;
 Symboltype targettype;
 Array *h, *h2;
 Pointer        target, value;

 iq = *ps++;                            // target
 CK_ARR(iq, 1);
 h = HEAD(iq);
 GET_SIZE(ntarget, h->dims, h->ndim);
 target.i32 = LPTR(h);
 targettype = sym[iq].type;
 iq = lux_long(1, ps++);                // bins
 CK_ARR(iq, 2);
 h = HEAD(iq);
 GET_SIZE(n, h->dims, h->ndim);
 bin = LPTR(h);
 iq = lux_convert(1, ps++, targettype, 1); // value
 if (iq < 0)
   return LUX_ERROR;            // some error
 h2 = HEAD(iq);
 value.i32 = LPTR(h2);
 if (h2->ndim != h->ndim)
   return cerror(INCMP_ARR, iq);
 for (i = 0; i < (int32_t) h->ndim; i++)
   if (h->dims[i] != h2->dims[i])
     return cerror(INCMP_DIMS, iq);
 switch (targettype) {
 case LUX_INT8:
   while (n--) {
     if (*bin >= 0 && *bin < ntarget)
       target.ui8[*bin] += *value.ui8++;
     else
       nout++;
     bin++;
   }
   break;
 case LUX_INT16:
   while (n--) {
     if (*bin >= 0 && *bin < ntarget)
       target.i16[*bin] += *value.i16++;
     else
       nout++;
     bin++;
   }
   break;
 case LUX_INT32:
   while (n--) {
     if (*bin >= 0 && *bin < ntarget)
       target.i32[*bin] += *value.i32++;
     else
       nout++;
     bin++;
   }
   break;
 case LUX_INT64:
   while (n--) {
     if (*bin >= 0 && *bin < ntarget)
       target.i64[*bin] += *value.i64++;
     else
       nout++;
     bin++;
   }
   break;
 case LUX_FLOAT:
   while (n--) {
     if (*bin >= 0 && *bin < ntarget)
       target.f[*bin] += *value.f++;
     else
       nout++;
     bin++;
   }
   break;
 case LUX_DOUBLE:
   while (n--) {
     if (*bin >= 0 && *bin < ntarget)
       target.d[*bin] += *value.d++;
     else
       nout++;
     bin++;
   }
   break;
 }
 if (nout)
   printf("DISTR - %d elements were out of range\n", nout);
 return 1;
}
//-------------------------------------------------------------------------
int32_t lux_distr_f(ArgumentCount narg, Symbol ps[])
/* y=DISTR(bins,values)  puts each <value> in the corresponding <bin> and
   returns an array containing bins 0 through max(values). LS */
{
  extern int32_t histmin, histmax;
  extern Scalar         lastmin, lastmax;
  int32_t       iq, i, n, nd, nd2, range, result_sym,
        minmax(int32_t *, int32_t, int32_t);
  Symboltype type, type2;
  Pointer arg1, arg2, res;
  int32_t       lux_zero(int32_t, int32_t []);

  iq = ps[0];
  if (symbol_class(iq) != LUX_ARRAY)
    return cerror(NEED_ARR, iq);
  type = array_type(iq);
  arg1.i32 = (int32_t*) array_data(iq);
  nd = array_num_dims(iq);
  n = array_size(iq);

  // second argument
  iq = ps[1];
  if (symbol_class(iq) != LUX_ARRAY)
    return cerror(NEED_ARR, iq);
  type2 = array_type(iq);
  nd2 = array_num_dims(iq);
  if (nd != nd2 || n != array_size(iq))
    return cerror(INCMP_ARR, iq);
  arg2.i32 = (int32_t*) array_data(iq);
  // always need the range
  minmax(arg1.i32, n, type);
  // get long (int32_t) versions of min and max
  convertPointer(&lastmin, type, LUX_INT32);
  convertPointer(&lastmax, type, LUX_INT32);
  // create an array for results
  histmin = lastmin.i32;
  histmax = lastmax.i32;
  // make the min 0 if it is greater
  histmin = histmin > 0 ? 0 : histmin;
  if (histmin < 0 ) {
    printf("WARNING - DISTR argument #1 (%s) contains negative entries,\n",
           varName(ps[0]));
    printf("see !histmin and !histmax to find range included\n"); }
  range = histmax - histmin + 1;
  result_sym = array_scratch(type2, 1, &range);
  res.i32 = (int32_t*) array_data(result_sym);
  lux_zero(1, &result_sym);             // need to zero initially
  // now accumulate the distribution
  switch (type2) {                      // type of result
  case LUX_INT8:
    switch (type) {
    case LUX_INT8:
      while (n--) {
        i = *arg1.ui8++ - histmin;
        res.ui8[i] += *arg2.ui8++;
      }
      break;
    case LUX_INT16:
      while (n--) {
        i = *arg1.i16++ - histmin;
        res.ui8[i] += *arg2.ui8++;
      }
      break;
    case LUX_INT32:
      while (n--) {
        i = *arg1.i32++ - histmin;
        res.ui8[i] += *arg2.ui8++;
      }
      break;
    case LUX_INT64:
      while (n--) {
        i = *arg1.i64++ - histmin;
        res.ui8[i] += *arg2.ui8++;
      }
      break;
    case LUX_FLOAT:
      while (n--) {
        i = *arg1.f++ - histmin;
        res.ui8[i] += *arg2.ui8++;
      }
      break;
    case LUX_DOUBLE:
      while (n--) {
        i = *arg1.d++ - histmin;
        res.ui8[i] += *arg2.ui8++;
      }
      break;
    }
    break;
  case LUX_INT16:
    switch (type) {
    case LUX_INT8:
      while (n--) {
        i = *arg1.ui8++ - histmin;
        res.i16[i] += *arg2.i16++;
      }
      break;
    case LUX_INT16:
      while (n--) {
        i = *arg1.i16++ - histmin;
        res.i16[i] += *arg2.i16++;
      }
      break;
    case LUX_INT32:
      while (n--) {
        i = *arg1.i32++ - histmin;
        res.i16[i] += *arg2.i16++;
      }
      break;
    case LUX_INT64:
      while (n--) {
        i = *arg1.i64++ - histmin;
        res.i16[i] += *arg2.i16++;
      }
      break;
    case LUX_FLOAT:
      while (n--) {
        i = *arg1.f++ - histmin;
        res.i16[i] += *arg2.i16++;
      }
      break;
    case LUX_DOUBLE:
      while (n--) {
        i = *arg1.d++ - histmin;
        res.i16[i] += *arg2.i16++;
      }
      break;
    }
    break;
  case LUX_INT32:
    switch (type) {
    case LUX_INT8:
      while (n--) {
        i = *arg1.ui8++ - histmin;
        res.i32[i] += *arg2.i32++;
      }
      break;
    case LUX_INT16:
      while (n--) {
        i = *arg1.i16++ - histmin;
        res.i32[i] += *arg2.i32++;
      }
      break;
    case LUX_INT32:
      while (n--) {
        i = *arg1.i32++ - histmin;
        res.i32[i] += *arg2.i32++;
      }
      break;
    case LUX_INT64:
      while (n--) {
        i = *arg1.i64++ - histmin;
        res.i32[i] += *arg2.i32++;
      }
      break;
    case LUX_FLOAT:
      while (n--) {
        i = *arg1.f++ - histmin;
        res.i32[i] += *arg2.i32++;
      }
      break;
    case LUX_DOUBLE:
      while (n--) {
        i = *arg1.d++ - histmin;
        res.i32[i] += *arg2.i32++;
      }
      break;
    }
    break;
  case LUX_INT64:
    switch (type) {
    case LUX_INT8:
      while (n--) {
        i = *arg1.ui8++ - histmin;
        res.i64[i] += *arg2.i64++;
      }
      break;
    case LUX_INT16:
      while (n--) {
        i = *arg1.i16++ - histmin;
        res.i64[i] += *arg2.i64++;
      }
      break;
    case LUX_INT32:
      while (n--) {
        i = *arg1.i32++ - histmin;
        res.i64[i] += *arg2.i64++;
      }
      break;
    case LUX_INT64:
      while (n--) {
        i = *arg1.i64++ - histmin;
        res.i64[i] += *arg2.i64++;
      }
      break;
    case LUX_FLOAT:
      while (n--) {
        i = *arg1.f++ - histmin;
        res.i64[i] += *arg2.i64++;
      }
      break;
    case LUX_DOUBLE:
      while (n--) {
        i = *arg1.d++ - histmin;
        res.i64[i] += *arg2.i64++;
      }
      break;
    }
    break;
  case LUX_FLOAT:
    switch (type) {
    case LUX_INT8:
      while (n--) {
        i = *arg1.ui8++ - histmin;
        res.f[i] += *arg2.f++;
      }
      break;
    case LUX_INT16:
      while (n--) {
        i = *arg1.i16++ - histmin;
        res.f[i] += *arg2.f++;
      }
      break;
    case LUX_INT32:
      while (n--) {
        i = *arg1.i32++ - histmin;
        res.f[i] += *arg2.f++;
      }
      break;
    case LUX_INT64:
      while (n--) {
        i = *arg1.i64++ - histmin;
        res.f[i] += *arg2.f++;
      }
      break;
    case LUX_FLOAT:
      while (n--) {
        i = *arg1.f++ - histmin;
        res.f[i] += *arg2.f++;
      }
      break;
    case LUX_DOUBLE:
      while (n--) {
        i = *arg1.d++ - histmin;
        res.f[i] += *arg2.f++;
      }
      break;
    }
    break;
  case LUX_DOUBLE:
    switch (type) {
    case LUX_INT8:
      while (n--) {
        i = *arg1.ui8++ - histmin;
        res.d[i] += *arg2.d++;
      }
      break;
    case LUX_INT16:
      while (n--) {
        i = *arg1.i16++ - histmin;
        res.d[i] += *arg2.d++;
      }
      break;
    case LUX_INT32:
      while (n--) {
        i = *arg1.i32++ - histmin;
        res.d[i] += *arg2.d++;
      }
      break;
    case LUX_INT64:
      while (n--) {
        i = *arg1.i64++ - histmin;
        res.d[i] += *arg2.d++;
      }
      break;
    case LUX_FLOAT:
      while (n--) {
        i = *arg1.f++ - histmin;
        res.d[i] += *arg2.d++;
      }
      break;
    case LUX_DOUBLE:
      while (n--) {
        i = *arg1.d++ - histmin;
        res.d[i] += *arg2.d++;
      }
      break;
    }
    break;
  }
  return result_sym;
}
// ignorelimit and increaselimit are ignored
REGISTER(distr, f, distr, 2, 2, "2ignorelimit:4increaselimit" );
//-------------------------------------------------------------------------
int32_t readkey(int32_t mode)
{
 int32_t        ch, result_sym;

 ch = rl_getc(stdin);
 result_sym = scalar_scratch(LUX_INT32);
 sym[result_sym].spec.scalar.i32 = ch;
 if (mode) { putchar(ch);  putchar('\n'); }
 return result_sym;
}
//-------------------------------------------------------------------------
int32_t lux_readkey(ArgumentCount narg, Symbol ps[])
// returns the code for the next pressed key and echos that key
{
 return readkey(1);
}
//-------------------------------------------------------------------------
int32_t lux_readkeyne(ArgumentCount narg, Symbol ps[])
// returns the code for the next pressed key but does not echo that key
// see lux_readkey for more info
{
 return readkey(0);
}
//-------------------------------------------------------------------------
int32_t lux_readarr(ArgumentCount narg, Symbol ps[])
// reads array elements until they are exhausted and returns read elements
// in an array.  Louis Strous 24may92
{
 extern FILE    *inputStream;
 FILE   *tp, *is;
 char   *p, *pt, lastchar, token[32], line[BUFSIZE];
 int32_t        arrs=0, i, iq, *iptr,
   getNewLine(char *, size_t, char const *, char);
 Symboltype maxtype = LUX_INT32;
 float  *fptr;

 iq = ps[0];                                            // target
 tp = Tmpfile();                                        // temporary storage
 is = inputStream;
 inputStream = stdin;
 i = getNewLine((char *) line, BUFSIZE, "arr>", 0);  // read line
 inputStream = is;
 if (i > 0)
 { p = line; lastchar = *p;
   while (lastchar)                                     // more chars
   { while (isspace((uint8_t) *p) || *p == ',') p++; // skip space & ,
     pt = p;                                            // token start
     while (isspace((uint8_t) *p) == 0 && *p != ',' && *p != 0) // scan value
     { if (*p == 'e' || *p == 'E' || *p == '.')                 // float?
         maxtype = LUX_FLOAT; p++; }
     lastchar = *p;  *p++ = '\0';
     if (*pt) {
       strcpy(token, pt);
       arrs++;
       fprintf(tp, "%s\n", token);
     }
   }
   rewind(tp);
   if (redef_array(iq, maxtype, 1, &arrs) != 1)                 // target
   { printf("(redef_array): "); fclose(tp); return cerror(ALLOC_ERR, iq); }
   if (maxtype == LUX_FLOAT)
   { fptr = (float *) ((char *) sym[iq].spec.array.ptr + sizeof(Array));
     for (i = arrs; i > 0; i--) fscanf(tp, "%f", fptr++); }
   else
   { iptr = (int32_t *) ((char *) sym[iq].spec.array.ptr + sizeof(Array));
     for (i = arrs; i > 0; i--) fscanf(tp, "%d", iptr++); }
 }
 fclose(tp);
 return 1;
}
//-------------------------------------------------------------------------
template<typename T>
void
find_action(StandardLoopIterator<T> datait,
             StandardLoopIterator<T> keysit,
             StandardLoopIterator<int32_t> offsetit,
             StandardLoopIterator<int32_t> targetit,
             int32_t flags)
{
  auto data = datait.pointer();
  auto offsets = offsetit.pointer();
  auto target = targetit.pointer();
  auto data_count = datait.element_count();
  auto offsets_count = offsetit.element_count();

  bool data_monotonic = (flags & (1 << 0));
  bool at_or_past     = (flags & (1 << 1));

  bool data_going_up = true;
  if (data_monotonic)
  {
    // figure out if the data increases or decreases
    auto base = data[0];
    for (auto ix = 1; ix < data_count; ++ix)
    {
      auto x = data[ix];
      if (x > base)
        break;                  // data_going_up is already true
      else if (x < base)
      {
        data_going_up = false;
        break;
      }
    }
  }

  int32_t offset;
  if (offsets_count >= 1)
  {
    offset = *offsets;
    --offsets_count;
  }
  else
    offset = 0;
  while (!keysit.done())
  {
    T key = *keysit++;          // value to search for

    if (offset < 0 || offset >= data_count)
      *target = -1;
    else if (data_monotonic)
    {
      decltype(data) it;
      if (data_going_up)
        it = std::lower_bound(data + offset, data + data_count, key);
      else
        it = std::lower_bound(data + offset, data + data_count, key,
                              [](const T& lhs, const T& rhs)
                              {
                                return lhs > rhs;
                              });
      if ((it == data + data_count)
          || (!at_or_past && *it != key))
        *target = -1;
      else
        *target = it - data;
    }
    else // brute force search
    {
      int32_t ix;
      for (ix = offset; ix < data_count; ++ix)
      {
        if (data[ix] == key)    // found it
        {
          *target = ix;
          break;
        }
      }
      if (ix == data_count)     // not found
        *target = -1;
    }
    ++target;
    if (offsets_count)
    {
      offset = *++offsets;
      --offsets_count;
    }
  }
}

Symbol
lux_find(ArgumentCount narg, Symbol ps[])
// FIND(data, keys [, offsets] [, /data_monotonic, /at_or_past])
//
// Locate the keys in the data, starting at offset, which defaults to 0 (the
// index of the first data element).
//
// /data_monotonic is a promise by the caller that the data values are sorted in
// ascending or descending order.  Then a faster search algorithm can be used.
//
// /at_or_past says to return the index to the first data value that is equal to
// or past the key.  If /data_monotonic is also specified, then "past the key"
// means in the direction in which the data is monotonic, otherwise "past the
// key". means greater than the key.  If /at_or_past is not specifid then only
// an exact match between key and data is accepted.
{
  StandardArguments
    sa(narg, ps,
       "i*;"       // data: input argument with arbitrary dimensions
       "iZ*;" // key: input argument with arbitrary dimensions, converted to the
              // type of the reference parameter (here the data)
       "iL[1]#?;"  // offset: optional input argument, converted to LUX_INT32,
                   // the element count must be equal to 1 or to that of the key
       "rL[1]@&"); // return symbol: LUX_INT32, dimensions equal to that of the
                   // key, omitting dimensions equal to 1 but leaving one if
                   // there would otherwise be no dimensions at all
  if (sa.result() < 0) {
    return sa.result();
  }

  switch (sa.datainfo(0).type) {
  case LUX_INT8:
    find_action(StandardLoopIterator<uint8_t>(sa, 0),
                 StandardLoopIterator<uint8_t>(sa, 1),
                 StandardLoopIterator<int32_t>(sa, 2),
                 StandardLoopIterator<int32_t>(sa, 3),
                 internalMode);
    break;
  case LUX_INT16:
    find_action(StandardLoopIterator<int16_t>(sa, 0),
                StandardLoopIterator<int16_t>(sa, 1),
                StandardLoopIterator<int32_t>(sa, 2),
                StandardLoopIterator<int32_t>(sa, 3),
                internalMode);
    break;
  case LUX_INT32:
    find_action(StandardLoopIterator<int32_t>(sa, 0),
                StandardLoopIterator<int32_t>(sa, 1),
                StandardLoopIterator<int32_t>(sa, 2),
                StandardLoopIterator<int32_t>(sa, 3),
                internalMode);
    break;
  case LUX_INT64:
    find_action(StandardLoopIterator<int64_t>(sa, 0),
                StandardLoopIterator<int64_t>(sa, 1),
                StandardLoopIterator<int32_t>(sa, 2),
                StandardLoopIterator<int32_t>(sa, 3),
                internalMode);
    break;
  case LUX_FLOAT:
    find_action(StandardLoopIterator<float>(sa, 0),
                StandardLoopIterator<float>(sa, 1),
                StandardLoopIterator<int32_t>(sa, 2),
                StandardLoopIterator<int32_t>(sa, 3),
                internalMode);
    break;
  case LUX_DOUBLE:
    find_action(StandardLoopIterator<double>(sa, 0),
                 StandardLoopIterator<double>(sa, 1),
                StandardLoopIterator<int32_t>(sa, 2),
                StandardLoopIterator<int32_t>(sa, 3),
                internalMode);
    break;
  default:
    return luxerror("Illegal type in arguments to FIND function", 0);
  }
  return sa.result();
}
REGISTER(find, f, find, 2, 3, "1data_monotonic:2at_or_past");
//-------------------------------------------------------------------------
int timespec_diff(struct timespec* two, struct timespec* one)
{
  if (two->tv_nsec >= one->tv_nsec) {
    return ((double) (two->tv_nsec - one->tv_nsec))/1e9
      + difftime(two->tv_sec, one->tv_sec);
  } else {
    return -((double) (one->tv_nsec - two->tv_nsec))/1e9
      + difftime(two->tv_sec, one->tv_sec);
  }
}
//-------------------------------------------------------------------------
#include <errno.h>
int32_t lux_help(ArgumentCount narg, Symbol ps[])
{
  char const* topic = narg? string_arg(*ps): "Top";
  char cmd[300];
  if (strlen(topic) > 270)
    return luxerror("Help topic is too long: '%s'", 0, topic);

  // Unfortunately, GNU Info does not provide a way to tell if the
  // sought node has been found, so we first look for a user-defined
  // subroutine or function with the given name, and only if that
  // cannot be found do we look for the name as a topic in the LUX
  // manual.

  FILE *fp;

  if ((internalMode & 1) == 0) { // no /manual
    // look for a user-defined subroutine or function
    fp = openPathFile(topic, FIND_SUBR | FIND_LOWER); // subroutine
    if (!fp)
      fp = openPathFile(topic, FIND_FUNC | FIND_LOWER); // function
  }
  if (fp) {
    setPager(0);
    fgets(line, 256, fp);
    printw(line);
    while (fgets(line, 256, fp) && *line == ';')
      printw(line + 1);
    resetPager();
    fclose(fp);
  } else {                      // look in the manual
    sprintf(cmd, "info --file lux --node \"%s\" 2>/dev/null", topic);
    system(cmd);
  }
  return LUX_OK;
}
//-------------------------------------------------------------------------
void endian(void *pp, int32_t n, int32_t type)
/* swap bytes according to data type, starting at p, spanning n bytes.
   goes from bigendian to littleendian or back.
   uint8_t -> do nothing
   int16_t -> swap 1 2 to 2 1
   long, float -> swap 1 2 3 4 to 4 3 2 1
   double -> swap 1 2 3 4 5 6 7 8 to 8 7 6 5 4 3 2 1
   Works between Ultrix and Irix.   LS 10/12/92         */
{
 int32_t        size, i, n2, n3;
 uint8_t        temp, *p2, *p;

 p = (uint8_t *) pp;
 size = lux_type_size[type];
 if (size <= 1)
   return;
 n /= size;                     // number of elements
 n2 = size/2;
 n3 = size + n2;
 p2 = p + size;
 while (n--)
 { for (i = 0; i < n2; i++)
   { temp = *p;
     *p++ = *--p2;
     *p2 = temp; }
   p += n2;
   p2 += n3; }
 return;
}
//-------------------------------------------------------------------------
int32_t lux_endian(ArgumentCount narg, Symbol ps[])
/* Switches an array between littleendian and bigendian or vice versa.
   Works between Ultrix and Irix.  LS 10/12/92 */
// added support for scalars.  LS 10oct97
{
 int32_t        iq, n, type;
 Pointer        q;

 iq = ps[0];
 switch (symbol_class(iq)) {
   case LUX_ARRAY:
     q.i32 = (int32_t*) array_data(iq);
     type = array_type(iq);
     n = array_size(iq)*lux_type_size[type];
     break;
   case LUX_SCALAR:
     q.i32 = &scalar_value(iq).i32;
     type = scalar_type(iq);
     n = lux_type_size[type];
     break;
   default:
     return cerror(ILL_CLASS, iq);
 }
 endian(q.ui8, n, type);
 return 1;
}
//-------------------------------------------------------------------------
int32_t lux_differ(ArgumentCount narg, Symbol ps[])
/* running difference; reverse of running sum
  syntax:  y = differ(x [[,axis] ,order])
  axis is the dimension along which differencing is performed; for each of the
  remaining coordinates differencing is started over.  If axis is not
  specified, or negative, then an uninterrupted differ is performed as if
  the array were 1D.  |<order>| is the order of the difference (e.g. 2nd order
  -> difference between element and two elements back).  If <order> is less
  than zero, then the elements for which there isn't a comparison
  element in the array are copied from the original (-> reverse of RUNSUM),
  otherwise they are zeroed.  For pseudo-1D differencing with order = 1,
  the first element is omitted (compatible with the previous version of
  DIFFER).
  LS 24nov92 */
{
  Pointer       src, trgt, order;
  int32_t       result, nOrder, loop, o, ww, stride, offset1, offset3,
    w1, one = 1, iq, n, i, old, circular;
  Symboltype type;
  LoopInfo      srcinfo, trgtinfo;

  if (standardLoop(ps[0], narg > 2? ps[1]: 0,
                   SL_SAMEDIMS | SL_UPGRADE | SL_EACHROW, LUX_INT8,
                   &srcinfo, &src, &result, &trgtinfo, &trgt) < 0)
    return LUX_ERROR;

  if (narg > 1) {               // <order>
    // check that it is numerical and ensure that it is LUX_INT32
    if (getNumerical(ps[narg - 1], LUX_INT32, &nOrder, &order,
                     GN_UPGRADE | GN_UPDATE, NULL, NULL) < 0)
      return LUX_ERROR;
    if (nOrder > 1 && nOrder != srcinfo.naxes)
      return luxerror("Number of orders must be 1 or equal to number of axes",
                   ps[2]);
  } else {
    order.i32 = &one;
    nOrder = 1;
  }
  iq = ps[0];                   // data
  type = symbol_type(iq);

  if (narg == 1) {              // old style
    if (symbol_class(result) == LUX_SCALAR)
      return luxerror("Cannot take difference of only one number", ps[0]);
    ww = array_size(result) - 1;
    unMark(result);
    zap(result);
    if (ww == 1) {              // change to scalar
      result = scalar_scratch(type);
      trgt.i32 = &scalar_value(result).i32;
    } else {                    // reduce size by one
      result = array_scratch(type, 1, &ww);
      trgt.i32 = (int32_t*) array_data(result);
    }
    old = 1;
  } else
    old = 0;

  if (!srcinfo.naxes)
    srcinfo.naxes++;
  for (loop = 0; loop < srcinfo.naxes; loop++) {
    o = *order.i32;
    if (nOrder > 1)
      order.i32++;
    if (internalMode & 1) {
      if (o < 0) {
        o = -o;                         // cannot have /CENTRAL and negative
                                // order: use positive order instead
        printf("Need positive order with /CENTRAL - use %1d\n", o);
      }
      if (o % 2 == 1) {
        o -= 1;
        printf("Need even order for central difference - use %1d\n", o);
      }
    }
    circular = (internalMode & 2) != 0;
    ww = abs(o);
    if (!ww) {
      n = result;
      result = iq;
      iq = result;
      continue;
    }
    if (ww > srcinfo.rdims[0])
      ww = srcinfo.rdims[0];
    stride = srcinfo.step[0];
    offset1 = -ww*stride;
    offset3 = offset1 + srcinfo.rdims[0]*stride;
    w1 = (internalMode & 1)? ww/2: ww;
    switch (symbol_type(iq)) {
      default:
        return cerror(ILL_TYPE, ps[0]);
      case LUX_INT8:
        do {
          // do the left edge
          if (o > 0 || old) {
            if (circular)
              for (i = 0; i < w1 - old; i++) {
                *trgt.ui8 = *src.ui8 - src.ui8[offset3];
                trgt.ui8 += stride;
              }
            else
              for (i = 0; i < w1 - old; i++) {
                *trgt.ui8 = 0;    // zeros
                trgt.ui8 += stride;
              }
            src.ui8 -= offset1;
          } else
            for (i = 0; i < ww; i++) {
              *trgt.ui8 = *src.ui8; // original values
              trgt.ui8 += stride;
              src.ui8 += stride;
            }
          for (i = ww; i < srcinfo.rdims[0]; i++) { // middle part
            *trgt.ui8 = *src.ui8 - src.ui8[offset1];
            src.ui8 += stride;
            trgt.ui8 += stride;
          }
          for (i = w1; i < ww; i++) { // right edge
            *trgt.ui8 = 0;        // zeros
            trgt.ui8 += stride;
          }
        } while (trgtinfo.advanceLoop(&trgt.ui8),
                 srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
        break;
      case LUX_INT16:
        do {
          // do the left edge
          if (o > 0 || old) {
            if (circular)
              for (i = 0; i < w1 - old; i++) {
                *trgt.i16 = *src.i16 - src.i16[offset3];
                trgt.i16 += stride;
              }
            else
              for (i = 0; i < w1 - old; i++) {
                *trgt.i16 = 0;    // zeros
                trgt.i16 += stride;
              }
            src.i16 -= offset1;
          } else
            for (i = 0; i < ww; i++) {
              *trgt.i16 = *src.i16; // original values
              trgt.i16 += stride;
              src.i16 += stride;
            }
          for (i = ww; i < srcinfo.rdims[0]; i++) { // middle part
            *trgt.i16 = *src.i16 - src.i16[offset1];
            src.i16 += stride;
            trgt.i16 += stride;
          }
          for (i = w1; i < ww; i++) { // right edge
            *trgt.i16 = 0;        // zeros
            trgt.i16 += stride;
          }
        } while (trgtinfo.advanceLoop(&trgt.ui8),
                 srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
        break;
      case LUX_INT32:
        do {
          // do the left edge
          if (o > 0 || old) {
            if (circular)
              for (i = 0; i < w1 - old; i++) {
                *trgt.i32 = *src.i32 - src.i32[offset3];
                trgt.i32 += stride;
              }
            else
              for (i = 0; i < w1 - old; i++) {
                *trgt.i32 = 0;    // zeros
                trgt.i32 += stride;
              }
            src.i32 -= offset1;
          } else
            for (i = 0; i < ww; i++) {
              *trgt.i32 = *src.i32; // original values
              trgt.i32 += stride;
              src.i32 += stride;
            }
          for (i = ww; i < srcinfo.rdims[0]; i++) { // middle part
            *trgt.i32 = *src.i32 - src.i32[offset1];
            src.i32 += stride;
            trgt.i32 += stride;
          }
          for (i = w1; i < ww; i++) { // right edge
            *trgt.i32 = 0;        // zeros
            trgt.i32 += stride;
          }
        } while (trgtinfo.advanceLoop(&trgt.ui8),
                 srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
        break;
      case LUX_INT64:
        do {
          // do the left edge
          if (o > 0 || old) {
            if (circular)
              for (i = 0; i < w1 - old; i++) {
                *trgt.i64 = *src.i64 - src.i64[offset3];
                trgt.i64 += stride;
              }
            else
              for (i = 0; i < w1 - old; i++) {
                *trgt.i64 = 0;    // zeros
                trgt.i64 += stride;
              }
            src.i64 -= offset1;
          } else
            for (i = 0; i < ww; i++) {
              *trgt.i64 = *src.i64; // original values
              trgt.i64 += stride;
              src.i64 += stride;
            }
          for (i = ww; i < srcinfo.rdims[0]; i++) { // middle part
            *trgt.i64 = *src.i64 - src.i64[offset1];
            src.i64 += stride;
            trgt.i64 += stride;
          }
          for (i = w1; i < ww; i++) { // right edge
            *trgt.i64 = 0;        // zeros
            trgt.i64 += stride;
          }
        } while (trgtinfo.advanceLoop(&trgt.ui8),
                 srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
        break;
      case LUX_FLOAT:
        do {
          // do the left edge
          if (o > 0 || old) {
            if (circular)
              for (i = 0; i < w1 - old; i++) {
                *trgt.f = *src.f - src.f[offset3];
                trgt.f += stride;
              }
            else
              for (i = 0; i < w1 - old; i++) {
                *trgt.f = 0;    // zeros
                trgt.f += stride;
              }
            src.f -= offset1;
          } else
            for (i = 0; i < ww; i++) {
              *trgt.f = *src.f; // original values
              trgt.f += stride;
              src.f += stride;
            }
          for (i = ww; i < srcinfo.rdims[0]; i++) { // middle part
            *trgt.f = *src.f - src.f[offset1];
            src.f += stride;
            trgt.f += stride;
          }
          for (i = w1; i < ww; i++) { // right edge
            *trgt.f = 0;        // zeros
            trgt.f += stride;
          }
        } while (trgtinfo.advanceLoop(&trgt.ui8),
                 srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
        break;
      case LUX_DOUBLE:
        do {
          // do the left edge
          if (o > 0 || old) {
            if (circular)
              for (i = 0; i < w1 - old; i++) {
                *trgt.d = *src.d - src.d[offset3];
                trgt.d += stride;
              }
            else
              for (i = 0; i < w1 - old; i++) {
                *trgt.d = 0;    // zeros
                trgt.d += stride;
              }
            src.d -= offset1;
          } else
            for (i = 0; i < ww; i++) {
              *trgt.d = *src.d; // original values
              trgt.d += stride;
              src.d += stride;
            }
          for (i = ww; i < srcinfo.rdims[0]; i++) { // middle part
            *trgt.d = *src.d - src.d[offset1];
            src.d += stride;
            trgt.d += stride;
          }
          for (i = w1; i < ww; i++) { // right edge
            *trgt.d = 0;        // zeros
            trgt.d += stride;
          }
        } while (trgtinfo.advanceLoop(&trgt.ui8),
                 srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
        break;
      case LUX_CFLOAT:
        do {
          // do the left edge
          if (o > 0 || old) {
            if (circular)
              for (i = 0; i < w1 - old; i++) {
                trgt.cf->real = src.cf->real - src.cf[offset3].real;
                trgt.cf->imaginary = src.cf->imaginary - src.cf[offset3].imaginary;
                trgt.cf += stride;
              }
            else
              for (i = 0; i < w1 - old; i++) {
                trgt.cf->real = trgt.cf->imaginary = 0;         // zeros
                trgt.cf += stride;
              }
            src.cf -= offset1;
          } else
            for (i = 0; i < ww; i++) {
              trgt.cf->real = src.cf->real; // original values
              trgt.cf->imaginary = src.cf->imaginary; // original values
              trgt.cf += stride;
              src.cf += stride;
            }
          for (i = ww; i < srcinfo.rdims[0]; i++) { // middle part
            trgt.cf->real = src.cf->real - src.cf[offset1].real;
            trgt.cf->imaginary = src.cf->imaginary - src.cf[offset1].imaginary;
            src.cf += stride;
            trgt.cf += stride;
          }
          for (i = w1; i < ww; i++) { // right edge
            trgt.cf->real = trgt.cf->imaginary = 0;     // zeros
            trgt.cf += stride;
          }
        } while (trgtinfo.advanceLoop(&trgt.ui8),
                 srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
        break;
    case LUX_CDOUBLE:
        do {
          // do the left edge
          if (o > 0 || old) {
            if (circular)
              for (i = 0; i < w1 - old; i++) {
                trgt.cd->real = src.cd->real - src.cd[offset3].real;
                trgt.cd->imaginary = src.cd->imaginary - src.cd[offset3].imaginary;
                trgt.cd += stride;
              }
            else
              for (i = 0; i < w1 - old; i++) {
                trgt.cd->real = trgt.cd->imaginary = 0;         // zeros
                trgt.cd += stride;
              }
            src.cd -= offset1;
          } else
            for (i = 0; i < ww; i++) {
              trgt.cd->real = src.cd->real; // original values
              trgt.cd->imaginary = src.cd->imaginary; // original values
              trgt.cd += stride;
              src.cd += stride;
            }
          for (i = ww; i < srcinfo.rdims[0]; i++) { // middle part
            trgt.cd->real = src.cd->real - src.cd[offset1].real;
            trgt.cd->imaginary = src.cd->imaginary - src.cd[offset1].imaginary;
            src.cd += stride;
            trgt.cd += stride;
          }
          for (i = w1; i < ww; i++) { // right edge
            trgt.cd->real = trgt.cd->imaginary = 0;     // zeros
            trgt.cd += stride;
          }
        } while (trgtinfo.advanceLoop(&trgt.ui8),
                 srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
        break;
    }
    if (loop < srcinfo.naxes - 1)
    { nextLoops(&srcinfo, &trgtinfo);
      if (isFreeTemp(iq))       // can use iq to hold next result
      { n = result;
        result = iq;
        iq = n; }
      else                      // need new temp for next result
      { iq = result;
        result = array_clone(iq, type); }
      src.ui8 = (uint8_t*) array_data(iq);
      trgt.ui8 = (uint8_t*) array_data(result); }
  }
  return result;
}
//-------------------------------------------------------------------------
int32_t varsmooth(ArgumentCount narg, Symbol ps[], int32_t cumul)
// SMOOTH(data [[, axis], widths] [,/FW_EDGE_NEIGHBOR])
{
  int32_t       axisSym, widthSym, nWidth, nData, nDim, result, done, type, outType,
    i1, i2, i, step, widthNDim, *widthDim, axis;
  float         weight;
  Pointer       src, trgt, width, width0;
  Scalar        sum;
  LoopInfo      srcinfo, trgtinfo;

  switch (narg) {
    case 3:                     // data, axis, widths
      axisSym = ps[1];
      widthSym = ps[2];
      break;
    case 2:                     // data, widths
      axisSym = 0;
      widthSym = ps[1];
      break;
    case 1:                     // data
      axisSym = widthSym = 0;
      break; }

  if (symbol_class(ps[0]) != LUX_ARRAY)         // <data> not an array
    return cerror(NEED_ARR, ps[0]);

  if (numerical(ps[0], NULL, &nDim, &nData, NULL) < 0)
    // <data> not numerical
    return LUX_ERROR;

  if (widthSym) {
    if (numerical(widthSym, &widthDim, &widthNDim, &nWidth, NULL) < 0)
      // <width> not numerical
      return LUX_ERROR;
    done = lux_long(1, &widthSym);      // ensure LONG
    width0.i32 = width.i32 = (int32_t*) array_data(done);
  }

  if (standardLoop(ps[0], axisSym, SL_SAMEDIMS | SL_UPGRADE | SL_EACHCOORD,
                   cumul? LUX_INT32: LUX_FLOAT,
                   &srcinfo, &src, &result, &trgtinfo, &trgt) < 0)
    /* generating appropriate output symbol and initializing for looping
     through the data did not succeed */
    return LUX_ERROR;

  // the dimensions of <widths> must be equal to the corresponding
  // dimensions of <data>
  if (widthSym) {
    if (widthNDim > nDim)
      return cerror(INCMP_ARG, widthSym);
    for (i = 0; i < widthNDim; i++)
      if (widthDim[i] != srcinfo.dims[i])
        return cerror(INCMP_DIMS, widthSym); }

  outType = symbol_type(result);
  type = symbol_type(ps[0]);

  axis = srcinfo.axes[0];
  step = srcinfo.step[0];

  // now do the real work
  switch (outType) {
    case LUX_INT32:             // cumul is set for this one
      switch (type) {
        case LUX_INT8:
          do {
            i1 = -*width.i32/2;
            i2 = i1 + *width.i32;
            if (i1 + srcinfo.coords[axis] < 0) {
              i1 = -srcinfo.coords[axis];
              if (internalMode & 1) // /FW_EDGE_NEIGHBOR
                i2 = i1 + *width.i32; }
            if (i2 + srcinfo.coords[axis] > srcinfo.dims[axis])
            { i2 = srcinfo.dims[axis] - srcinfo.coords[axis];
              if (internalMode & 1 // /FW_EDGE_NEIGHBOR
                  && *width.i32 <= srcinfo.dims[axis]) // not wider than data
                i1 = i2 - *width.i32; }
            i1 *= step;
            i2 *= step;
            sum.i32 = 0.0;
            for (i = i1; i < i2; i += step)
              sum.i32 += (int32_t) src.ui8[i];
            *trgt.i32 = sum.i32;
            done = trgtinfo.advanceLoop(&trgt.ui8),
              srcinfo.advanceLoop(&src.ui8);
            width.i32++;
            if (done == widthNDim)      // done with indicated axis
              width.i32 = width0.i32;
          } while (done < srcinfo.rndim);
          break;
        case LUX_INT16:
          do {
            i1 = -*width.i32/2;
            i2 = i1 + *width.i32;
            if (i1 + srcinfo.coords[axis] < 0) {
              i1 = -srcinfo.coords[axis];
              if (internalMode & 1) // /FW_EDGE_NEIGHBOR
                i2 = i1 + *width.i32; }
            if (i2 + srcinfo.coords[axis] > srcinfo.dims[axis])
            { i2 = srcinfo.dims[axis] - srcinfo.coords[axis];
              if (internalMode & 1 // /FW_EDGE_NEIGHBOR
                  && *width.i32 <= srcinfo.dims[axis]) // not wider than data
                i1 = i2 - *width.i32; }
            i1 *= step;
            i2 *= step;
            sum.i32 = 0.0;
            for (i = i1; i < i2; i += step)
              sum.i32 += (int32_t) src.i16[i];
            *trgt.i32 = sum.i32;
            done = trgtinfo.advanceLoop(&trgt.ui8),
              srcinfo.advanceLoop(&src.ui8);
            width.i32++;
            if (done == widthNDim)      // done with indicated axis
              width.i32 = width0.i32;
          } while (done < srcinfo.rndim);
          break;
        case LUX_INT32:
          do {
            i1 = -*width.i32/2;
            i2 = i1 + *width.i32;
            if (i1 + srcinfo.coords[axis] < 0) {
              i1 = -srcinfo.coords[axis];
              if (internalMode & 1) // /FW_EDGE_NEIGHBOR
                i2 = i1 + *width.i32; }
            if (i2 + srcinfo.coords[axis] > srcinfo.dims[axis])
            { i2 = srcinfo.dims[axis] - srcinfo.coords[axis];
              if (internalMode & 1 // /FW_EDGE_NEIGHBOR
                  && *width.i32 <= srcinfo.dims[axis]) // not wider than data
                i1 = i2 - *width.i32; }
            i1 *= step;
            i2 *= step;
            sum.i32 = 0.0;
            for (i = i1; i < i2; i += step)
              sum.i32 += src.i32[i];
            *trgt.i32 = sum.i32;
            done = trgtinfo.advanceLoop(&trgt.ui8),
              srcinfo.advanceLoop(&src.ui8);
            width.i32++;
            if (done == widthNDim)      // done with indicated axis
              width.i32 = width0.i32;
          } while (done < srcinfo.rndim);
          break; }
      break;
    case LUX_INT64:
      switch (type) {
        case LUX_INT8:
          do {
            i1 = -*width.i32/2;
            i2 = i1 + *width.i32;
            if (i1 + srcinfo.coords[axis] < 0) {
              i1 = -srcinfo.coords[axis];
              if (internalMode & 1) // /FW_EDGE_NEIGHBOR
                i2 = i1 + *width.i32; }
            if (i2 + srcinfo.coords[axis] > srcinfo.dims[axis])
            { i2 = srcinfo.dims[axis] - srcinfo.coords[axis];
              if (internalMode & 1 // /FW_EDGE_NEIGHBOR
                  && *width.i32 <= srcinfo.dims[axis]) // not wider than data
                i1 = i2 - *width.i32; }
            i1 *= step;
            i2 *= step;
            sum.i64 = 0.0;
            for (i = i1; i < i2; i += step)
              sum.i64 += src.ui8[i];
            *trgt.i64 = sum.i64;
            done = trgtinfo.advanceLoop(&trgt.ui8),
              srcinfo.advanceLoop(&src.ui8);
            width.i32++;
            if (done == widthNDim)      // done with indicated axis
              width.i32 = width0.i32;
          } while (done < srcinfo.rndim);
          break;
        case LUX_INT16:
          do {
            i1 = -*width.i32/2;
            i2 = i1 + *width.i32;
            if (i1 + srcinfo.coords[axis] < 0) {
              i1 = -srcinfo.coords[axis];
              if (internalMode & 1) // /FW_EDGE_NEIGHBOR
                i2 = i1 + *width.i32; }
            if (i2 + srcinfo.coords[axis] > srcinfo.dims[axis])
            { i2 = srcinfo.dims[axis] - srcinfo.coords[axis];
              if (internalMode & 1 // /FW_EDGE_NEIGHBOR
                  && *width.i32 <= srcinfo.dims[axis]) // not wider than data
                i1 = i2 - *width.i32; }
            i1 *= step;
            i2 *= step;
            sum.i64 = 0.0;
            for (i = i1; i < i2; i += step)
              sum.i64 += src.i16[i];
            *trgt.i64 = sum.i64;
            done = trgtinfo.advanceLoop(&trgt.ui8),
              srcinfo.advanceLoop(&src.ui8);
            width.i32++;
            if (done == widthNDim)      // done with indicated axis
              width.i32 = width0.i32;
          } while (done < srcinfo.rndim);
          break;
        case LUX_INT32:
          do {
            i1 = -*width.i32/2;
            i2 = i1 + *width.i32;
            if (i1 + srcinfo.coords[axis] < 0) {
              i1 = -srcinfo.coords[axis];
              if (internalMode & 1) // /FW_EDGE_NEIGHBOR
                i2 = i1 + *width.i32; }
            if (i2 + srcinfo.coords[axis] > srcinfo.dims[axis])
            { i2 = srcinfo.dims[axis] - srcinfo.coords[axis];
              if (internalMode & 1 // /FW_EDGE_NEIGHBOR
                  && *width.i32 <= srcinfo.dims[axis]) // not wider than data
                i1 = i2 - *width.i32; }
            i1 *= step;
            i2 *= step;
            sum.i64 = 0.0;
            for (i = i1; i < i2; i += step)
              sum.i64 += src.i32[i];
            *trgt.i64 = sum.i64;
            done = trgtinfo.advanceLoop(&trgt.ui8),
              srcinfo.advanceLoop(&src.ui8);
            width.i32++;
            if (done == widthNDim)      // done with indicated axis
              width.i32 = width0.i32;
          } while (done < srcinfo.rndim);
          break;
        case LUX_INT64:
          do {
            i1 = -*width.i32/2;
            i2 = i1 + *width.i32;
            if (i1 + srcinfo.coords[axis] < 0) {
              i1 = -srcinfo.coords[axis];
              if (internalMode & 1) // /FW_EDGE_NEIGHBOR
                i2 = i1 + *width.i32; }
            if (i2 + srcinfo.coords[axis] > srcinfo.dims[axis])
            { i2 = srcinfo.dims[axis] - srcinfo.coords[axis];
              if (internalMode & 1 // /FW_EDGE_NEIGHBOR
                  && *width.i32 <= srcinfo.dims[axis]) // not wider than data
                i1 = i2 - *width.i32; }
            i1 *= step;
            i2 *= step;
            sum.i64 = 0.0;
            for (i = i1; i < i2; i += step)
              sum.i64 += src.i64[i];
            *trgt.i64 = sum.i64;
            done = trgtinfo.advanceLoop(&trgt.ui8),
              srcinfo.advanceLoop(&src.ui8);
            width.i32++;
            if (done == widthNDim)      // done with indicated axis
              width.i32 = width0.i32;
          } while (done < srcinfo.rndim);
          break; }
      break;
    case LUX_FLOAT:
      switch (type) {
        case LUX_INT8:
          do {
            i1 = -*width.i32/2;
            i2 = i1 + *width.i32;
            if (i1 + srcinfo.coords[axis] < 0) {
              i1 = -srcinfo.coords[axis];
              if (internalMode & 1) // /FW_EDGE_NEIGHBOR
                i2 = i1 + *width.i32; }
            if (i2 + srcinfo.coords[axis] > srcinfo.dims[axis])
            { i2 = srcinfo.dims[axis] - srcinfo.coords[axis];
              if (internalMode & 1 // /FW_EDGE_NEIGHBOR
                  && *width.i32 <= srcinfo.dims[axis]) // not wider than data
                i1 = i2 - *width.i32; }
            i1 *= step;
            i2 *= step;
            sum.f = 0.0;
            weight = cumul? 1: (i2 - i1)/step;
            for (i = i1; i < i2; i += step)
              sum.f += (float) src.ui8[i];
            *trgt.f = sum.f/weight;
            done = trgtinfo.advanceLoop(&trgt.ui8),
              srcinfo.advanceLoop(&src.ui8);
            width.i32++;
            if (done == widthNDim)      // done with indicated axis
              width.i32 = width0.i32;
          } while (done < srcinfo.rndim);
          break;
        case LUX_INT16:
          do {
            i1 = -*width.i32/2;
            i2 = i1 + *width.i32;
            if (i1 + srcinfo.coords[axis] < 0) {
              i1 = -srcinfo.coords[axis];
              if (internalMode & 1) // /FW_EDGE_NEIGHBOR
                i2 = i1 + *width.i32; }
            if (i2 + srcinfo.coords[axis] > srcinfo.dims[axis])
            { i2 = srcinfo.dims[axis] - srcinfo.coords[axis];
              if (internalMode & 1 // /FW_EDGE_NEIGHBOR
                  && *width.i32 <= srcinfo.dims[axis]) // not wider than data
                i1 = i2 - *width.i32; }
            i1 *= step;
            i2 *= step;
            sum.f = 0.0;
            weight = cumul? 1: (i2 - i1)/step;
            for (i = i1; i < i2; i += step)
              sum.f += (float) src.i16[i];
            *trgt.f = sum.f/weight;
            done = trgtinfo.advanceLoop(&trgt.ui8),
              srcinfo.advanceLoop(&src.ui8);
            width.i32++;
            if (done == widthNDim)      // done with indicated axis
              width.i32 = width0.i32;
          } while (done < srcinfo.rndim);
          break;
        case LUX_INT32:
          do {
            i1 = -*width.i32/2;
            i2 = i1 + *width.i32;
            if (i1 + srcinfo.coords[axis] < 0) {
              i1 = -srcinfo.coords[axis];
              if (internalMode & 1) // /FW_EDGE_NEIGHBOR
                i2 = i1 + *width.i32; }
            if (i2 + srcinfo.coords[axis] > srcinfo.dims[axis])
            { i2 = srcinfo.dims[axis] - srcinfo.coords[axis];
              if (internalMode & 1 // /FW_EDGE_NEIGHBOR
                  && *width.i32 <= srcinfo.dims[axis]) // not wider than data
                i1 = i2 - *width.i32; }
            i1 *= step;
            i2 *= step;
            sum.f = 0.0;
            weight = cumul? 1: (i2 - i1)/step;
            for (i = i1; i < i2; i += step)
              sum.f += (float) src.i32[i];
            *trgt.f = sum.f/weight;
            done = trgtinfo.advanceLoop(&trgt.ui8),
              srcinfo.advanceLoop(&src.ui8);
            width.i32++;
            if (done == widthNDim)      // done with indicated axis
              width.i32 = width0.i32;
          } while (done < srcinfo.rndim);
          break;
        case LUX_INT64:
          do {
            i1 = -*width.i32/2;
            i2 = i1 + *width.i32;
            if (i1 + srcinfo.coords[axis] < 0) {
              i1 = -srcinfo.coords[axis];
              if (internalMode & 1) // /FW_EDGE_NEIGHBOR
                i2 = i1 + *width.i32; }
            if (i2 + srcinfo.coords[axis] > srcinfo.dims[axis])
            { i2 = srcinfo.dims[axis] - srcinfo.coords[axis];
              if (internalMode & 1 // /FW_EDGE_NEIGHBOR
                  && *width.i32 <= srcinfo.dims[axis]) // not wider than data
                i1 = i2 - *width.i32; }
            i1 *= step;
            i2 *= step;
            sum.f = 0.0;
            weight = cumul? 1: (i2 - i1)/step;
            for (i = i1; i < i2; i += step)
              sum.f += (float) src.i64[i];
            *trgt.f = sum.f/weight;
            done = trgtinfo.advanceLoop(&trgt.ui8),
              srcinfo.advanceLoop(&src.ui8);
            width.i32++;
            if (done == widthNDim)      // done with indicated axis
              width.i32 = width0.i32;
          } while (done < srcinfo.rndim);
          break;
        case LUX_FLOAT:
          do {
            i1 = -*width.i32/2;
            i2 = i1 + *width.i32;
            if (i1 + srcinfo.coords[axis] < 0) {
              i1 = -srcinfo.coords[axis];
              if (internalMode & 1) // /FW_EDGE_NEIGHBOR
                i2 = i1 + *width.i32; }
            if (i2 + srcinfo.coords[axis] > srcinfo.dims[axis])
            { i2 = srcinfo.dims[axis] - srcinfo.coords[axis];
              if (internalMode & 1 // /FW_EDGE_NEIGHBOR
                  && *width.i32 <= srcinfo.dims[axis]) // not wider than data
                i1 = i2 - *width.i32; }
            i1 *= step;
            i2 *= step;
            sum.f = 0.0;
            weight = cumul? 1: (i2 - i1)/step;
            for (i = i1; i < i2; i += step)
              sum.f += src.f[i];
            *trgt.f = sum.f/weight;
            done = trgtinfo.advanceLoop(&trgt.ui8),
              srcinfo.advanceLoop(&src.ui8);
            width.i32++;
            if (done == widthNDim) // done with indicated axes
              width.i32 = width0.i32;
          } while (done < srcinfo.rndim);
          break; }
      break;
    case LUX_DOUBLE:
      switch (type) {
        case LUX_INT8:
          do {
            i1 = -*width.i32/2;
            i2 = i1 + *width.i32;
            if (i1 + srcinfo.coords[axis] < 0) {
              i1 = -srcinfo.coords[axis];
              if (internalMode & 1) // /FW_EDGE_NEIGHBOR
                i2 = i1 + *width.i32; }
            if (i2 + srcinfo.coords[axis] > srcinfo.dims[axis])
            { i2 = srcinfo.dims[axis] - srcinfo.coords[axis];
              if (internalMode & 1 // /FW_EDGE_NEIGHBOR
                  && *width.i32 <= srcinfo.dims[axis]) // not wider than data
                i1 = i2 - *width.i32; }
            i1 *= step;
            i2 *= step;
            sum.d = 0.0;
            weight = cumul? 1: (i2 - i1)/step;
            for (i = i1; i < i2; i += step)
              sum.d += (double) src.ui8[i];
            *trgt.d = sum.d/weight;
            done = trgtinfo.advanceLoop(&trgt.ui8),
              srcinfo.advanceLoop(&src.ui8);
            width.i32++;
            if (done == widthNDim)      // done with indicated axis
              width.i32 = width0.i32;
          } while (done < srcinfo.rndim);
          break;
        case LUX_INT16:
          do {
            i1 = -*width.i32/2;
            i2 = i1 + *width.i32;
            if (i1 + srcinfo.coords[axis] < 0) {
              i1 = -srcinfo.coords[axis];
              if (internalMode & 1) // /FW_EDGE_NEIGHBOR
                i2 = i1 + *width.i32; }
            if (i2 + srcinfo.coords[axis] > srcinfo.dims[axis])
            { i2 = srcinfo.dims[axis] - srcinfo.coords[axis];
              if (internalMode & 1 // /FW_EDGE_NEIGHBOR
                  && *width.i32 <= srcinfo.dims[axis]) // not wider than data
                i1 = i2 - *width.i32; }
            i1 *= step;
            i2 *= step;
            sum.d = 0.0;
            weight = cumul? 1: (i2 - i1)/step;
            for (i = i1; i < i2; i += step)
              sum.d += (double) src.i16[i];
            *trgt.d = sum.d/weight;
            done = trgtinfo.advanceLoop(&trgt.ui8),
              srcinfo.advanceLoop(&src.ui8);
            width.i32++;
            if (done == widthNDim)      // done with indicated axis
              width.i32 = width0.i32;
          } while (done < srcinfo.rndim);
          break;
        case LUX_INT32:
          do {
            i1 = -*width.i32/2;
            i2 = i1 + *width.i32;
            if (i1 + srcinfo.coords[axis] < 0) {
              i1 = -srcinfo.coords[axis];
              if (internalMode & 1) // /FW_EDGE_NEIGHBOR
                i2 = i1 + *width.i32; }
            if (i2 + srcinfo.coords[axis] > srcinfo.dims[axis])
            { i2 = srcinfo.dims[axis] - srcinfo.coords[axis];
              if (internalMode & 1 // /FW_EDGE_NEIGHBOR
                  && *width.i32 <= srcinfo.dims[axis]) // not wider than data
                i1 = i2 - *width.i32; }
            i1 *= step;
            i2 *= step;
            sum.d = 0.0;
            weight = cumul? 1: (i2 - i1)/step;
            for (i = i1; i < i2; i += step)
              sum.d += (double) src.i32[i];
            *trgt.d = sum.d/weight;
            done = trgtinfo.advanceLoop(&trgt.ui8),
              srcinfo.advanceLoop(&src.ui8);
            width.i32++;
            if (done == widthNDim)      // done with indicated axis
              width.i32 = width0.i32;
          } while (done < srcinfo.rndim);
          break;
        case LUX_INT64:
          do {
            i1 = -*width.i32/2;
            i2 = i1 + *width.i32;
            if (i1 + srcinfo.coords[axis] < 0) {
              i1 = -srcinfo.coords[axis];
              if (internalMode & 1) // /FW_EDGE_NEIGHBOR
                i2 = i1 + *width.i32; }
            if (i2 + srcinfo.coords[axis] > srcinfo.dims[axis])
            { i2 = srcinfo.dims[axis] - srcinfo.coords[axis];
              if (internalMode & 1 // /FW_EDGE_NEIGHBOR
                  && *width.i32 <= srcinfo.dims[axis]) // not wider than data
                i1 = i2 - *width.i32; }
            i1 *= step;
            i2 *= step;
            sum.d = 0.0;
            weight = cumul? 1: (i2 - i1)/step;
            for (i = i1; i < i2; i += step)
              sum.d += (double) src.i64[i];
            *trgt.d = sum.d/weight;
            done = trgtinfo.advanceLoop(&trgt.ui8),
              srcinfo.advanceLoop(&src.ui8);
            width.i32++;
            if (done == widthNDim)      // done with indicated axis
              width.i32 = width0.i32;
          } while (done < srcinfo.rndim);
          break;
        case LUX_FLOAT:
          do {
            i1 = -*width.i32/2;
            i2 = i1 + *width.i32;
            if (i1 + srcinfo.coords[axis] < 0) {
              i1 = -srcinfo.coords[axis];
              if (internalMode & 1) // /FW_EDGE_NEIGHBOR
                i2 = i1 + *width.i32; }
            if (i2 + srcinfo.coords[axis] > srcinfo.dims[axis])
            { i2 = srcinfo.dims[axis] - srcinfo.coords[axis];
              if (internalMode & 1 // /FW_EDGE_NEIGHBOR
                  && *width.i32 <= srcinfo.dims[axis]) // not wider than data
                i1 = i2 - *width.i32; }
            i1 *= step;
            i2 *= step;
            sum.d = 0.0;
            weight = cumul? 1: (i2 - i1)/step;
            for (i = i1; i < i2; i += step)
              sum.d += (double) src.f[i];
            *trgt.d = sum.d/weight;
            done = trgtinfo.advanceLoop(&trgt.ui8),
              srcinfo.advanceLoop(&src.ui8);
            width.i32++;
            if (done == widthNDim)      // done with indicated axis
              width.i32 = width0.i32;
          } while (done < srcinfo.rndim);
          break;
        case LUX_DOUBLE:
          do {
            i1 = -*width.i32/2;
            i2 = i1 + *width.i32;
            if (i1 + srcinfo.coords[axis] < 0) {
              i1 = -srcinfo.coords[axis];
              if (internalMode & 1) // /FW_EDGE_NEIGHBOR
                i2 = i1 + *width.i32; }
            if (i2 + srcinfo.coords[axis] > srcinfo.dims[axis])
            { i2 = srcinfo.dims[axis] - srcinfo.coords[axis];
              if (internalMode & 1 // /FW_EDGE_NEIGHBOR
                  && *width.i32 <= srcinfo.dims[axis]) // not wider than data
                i1 = i2 - *width.i32; }
            i1 *= step;
            i2 *= step;
            sum.d = 0.0;
            weight = cumul? 1: (i2 - i1)/step;
            for (i = i1; i < i2; i += step)
              sum.d += src.d[i];
            *trgt.d = sum.d/weight;
            done = trgtinfo.advanceLoop(&trgt.ui8),
              srcinfo.advanceLoop(&src.ui8);
            width.i32++;
            if (done == widthNDim)      // done with indicated axis
              width.i32 = width0.i32;
          } while (done < srcinfo.rndim);
          break; }
      break; }
  return result;
}
//-------------------------------------------------------------------------

int32_t smooth(ArgumentCount narg, Symbol ps[], int32_t cumul)
/* return smoothed version syntax: y = smooth/runcum(x [,axis,width]
  [,/FW_EDGE_NEIGHBOR]) <axis> is the dimension along which summing is
  performed; for each of the remaining coordinates summing is started
  over at zero.  <width> is the summing width (only the integer part
  counts).  If <axis> isn't specified, or is negative, then <x> is
  taken to be a 1D array.  <cumul> determines and whether running
  averages (0) or running totals (1) are returned.  /FW_EDGE_NEIGHBOR
  governs smoothing near the edges: use the value of the nearest
  full-width neighboring average (set; default) or the local symmetric
  lesser-width average (unset).  FW_EDGE_NEIGHBOR & cumul extension by
  LS 26nov92 5mar93 implement LUX_INT32, LUX_FLOAT, and LUX_DOUBLE,
  because "integer" range of floats is too small. */
{
  Symboltype    type;
  int32_t       n, result, i, offset, stride, nWidth, w1, w2, ww, loop, three=3, norm,
    iq, jq;
  Pointer       src, trgt, width;
  Scalar        value;
  LoopInfo      srcinfo, trgtinfo;
  int32_t mode;

  mode = 0;
  if (narg > 2)                         // have <axes>
    jq = ps[1];
  else {                        // no <axes>: check for /ALL
    jq = 0;
    mode = (internalMode & 4)? SL_ALLAXES: 0;
  }

  if (standardLoop(ps[0], jq, mode | SL_SAMEDIMS | SL_UPGRADE | SL_EACHROW,
                   LUX_FLOAT, &srcinfo, &src, &result, &trgtinfo, &trgt) < 0)
    return LUX_ERROR;
  if (narg > 1) {               // have <width>
    // check that it is numerical and ensure that it is LUX_INT32
    if (getNumerical(ps[narg - 1], LUX_INT32, &nWidth, &width,
                     GN_EXACT | GN_UPDATE, NULL, NULL) < 0)
      return LUX_ERROR;
    if (nWidth > 1 && nWidth != srcinfo.naxes)
      return luxerror("Number of widths must be 1 or equal to number of axes",
                   ps[2]);
  } else {
    width.i32 = &three;
    nWidth = 1;
  }
  type = symbol_type(result);
  iq = ps[0];                   // data
  if (!srcinfo.naxes)
    srcinfo.naxes++;
  for (loop = 0; loop < srcinfo.naxes; loop++) {
    ww = (*width.i32 > srcinfo.rdims[0])? srcinfo.rdims[0]: *width.i32;
    if (nWidth > 1)
      width.i32++;
    norm = cumul? 1: ww;
    stride = srcinfo.step[0];
    offset = -stride*ww;
    /*
      n = 5 ∧ ww = 3
      tgt[0] ← src[0] (/PW) ∨ src[0..2] (/NOPW)  0 1
      tgt[1] ← src[0..2]                         1 1
      tgt[2] ← src[1..3]                         2 2
      tgt[3] ← src[2..4]                         3 3
      tgt[4] ← src[4] (/PW) ∨ src[2..4] (/NOPW)  4 3
      w1 = 2, w2 = 4

      n = 5 ∧ ww = 4
      tgt[0] ← src[0..1] (/PW) ∨ src[0..3] (/NOPW)  ½ 1½
      tgt[1] ← src[0..3]                           1½ 1½
      tgt[2] ← src[1..4]                           2½ 2½
      tgt[3] ← src[3..4] (/PW) ∨ src[1..4] (/NOPW) 3½ 2½
      tgt[4] ← src[4] (/PW) ∨ src[1..4] (/NOPW)    4  2½
      w1 = 2, w2 = 3

      n = 6 ∧ ww = 3
      tgt[0] ← src[0] (/PW) ∨ src[0..2] (/NOPW) 0 1
      tgt[1] ← src[0..2]                        1 1
      tgt[2] ← src[1..3]                        2 2
      tgt[3] ← src[2..4]                        3 3
      tgt[4] ← src[3..5]                        4 4
      tgt[5] ← src[5] (/PW) ∨ src[3..5] (/NOPW) 5 4
      w1 = 2, w2 = 5

      n = 6 ∧ ww = 4
      tgt[0] ← src[0..1] (/PW) ∨ src[0..3] (/NOPW)  ½ 1½
      tgt[1] ← src[0..3]                           1½ 1½
      tgt[2] ← src[1..4]                           2½ 2½
      tgt[3] ← src[2..5]                           3½ 3½
      tgt[4] ← src[4..5] (/PW) ∨ src[2..5] (/NOPW) 4½ 3½
      tgt[5] ← src[5] (/PW) ∨ src[2..5] (/NOPW)    5  3½
      w1 = 2, w2 = 4

      n ww w1 w2
      5  3  2  4
      5  4  2  3
      6  3  2  5
      6  4  2  4

      w1 = ⌊(ww + 1)/2⌋
      w2 = ⌊n - ww/2⌋
    */
    w1 = (ww + 1)/2;
    w2 = srcinfo.rdims[0] - ww/2;
    switch (symbol_type(iq)) {
      default:
        return cerror(ILL_TYPE, ps[0]);
      case LUX_INT8:
        do {
          value.i32 = 0;            // initialize
          // left-hand edge
          if (internalMode & 1) { // /PARTIAL_WIDTH
            norm = cumul? ww: 0;
            if (ww%2) {                 // odd width
              value.i32 += *src.ui8;
              src.ui8 += stride;
              if (!cumul)
                ++norm;
              *trgt.ui8 = (uint8_t) (value.i32/norm);
              trgt.ui8 += stride;
              i = 1;
            } else
              i = 0;
            for ( ; i < w1; i++) {
              value.i32 += *src.ui8;
              src.ui8 += stride;
              value.i32 += *src.ui8;
              src.ui8 += stride;
              if (!cumul)
                norm += 2;
              *trgt.ui8 = (uint8_t) (value.i32/norm);
              trgt.ui8 += stride;
            }
          } else {              // full width
            for (i = 0; i < ww; i++) { // do the left edge
              value.i32 += *src.ui8;
              src.ui8 += stride;
            }
            uint8_t v = (uint8_t) (value.i32/norm);
            for (i = 0; i < w1; i++) {
              *trgt.ui8 = v;
              trgt.ui8 += stride;
            }
          }
          // middle part
          for ( ; i < w2; i++) {
            value.i32 += *src.ui8 - src.ui8[offset];
            src.ui8 += stride;
            *trgt.ui8 = (uint8_t) (value.i32/norm);
            trgt.ui8 += stride;
          }
          // right-hand edge
          if (internalMode & 1) { // /PARTIAL_WIDTH
            for ( ; i < srcinfo.rdims[0] - !(ww%2); i++) {
              value.i32 -= src.ui8[offset];
              offset += stride;
              value.i32 -= src.ui8[offset];
              offset += stride;
              if (!cumul)
                norm -= 2;
              *trgt.ui8 = (uint8_t) (value.i32/norm);
              trgt.ui8 += stride;
            }
            if (!(ww%2)) {
              value.i32 -= src.ui8[offset];
              offset += stride;
              if (!cumul)
                --norm;
              *trgt.ui8 = (uint8_t) (value.i32/norm);
              trgt.ui8 += stride;
            }
          } else {
            uint8_t v = (uint8_t) (value.i32/norm);
            for ( ; i < srcinfo.rdims[0]; i++) { // right edge
              *trgt.ui8 = v;
              trgt.ui8 += stride;
            }
          }
        } while (trgtinfo.advanceLoop(&trgt.ui8),
                 srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
        break;
      case LUX_INT16:
        do {
          value.i32 = 0;            // initialize
          // left-hand edge
          if (internalMode & 1) { // /PARTIAL_WIDTH
            norm = cumul? ww: 0;
            if (ww%2) {                 // odd width
              value.i32 += *src.i16;
              src.i16 += stride;
              if (!cumul)
                ++norm;
              *trgt.i16 = (int16_t) (value.i32/norm);
              trgt.i16 += stride;
              i = 1;
            } else
              i = 0;
            for ( ; i < w1; i++) {
              value.i32 += *src.i16;
              src.i16 += stride;
              value.i32 += *src.i16;
              src.i16 += stride;
              if (!cumul)
                norm += 2;
              *trgt.i16 = (int16_t) (value.i32/norm);
              trgt.i16 += stride;
            }
          } else {              // full width
            for (i = 0; i < ww; i++) { // do the left edge
              value.i32 += *src.i16;
              src.i16 += stride;
            }
            int16_t v = (int16_t) (value.i32/norm);
            for (i = 0; i < w1; i++) {
              *trgt.i16 = v;
              trgt.i16 += stride;
            }
          }
          // middle part
          for ( ; i < w2; i++) {
            value.i32 += *src.i16 - src.i16[offset];
            src.i16 += stride;
            *trgt.i16 = (int16_t) (value.i32/norm);
            trgt.i16 += stride;
          }
          // right-hand edge
          if (internalMode & 1) { // /PARTIAL_WIDTH
            for ( ; i < srcinfo.rdims[0] - !(ww%2); i++) {
              value.i32 -= src.i16[offset];
              offset += stride;
              value.i32 -= src.i16[offset];
              offset += stride;
              if (!cumul)
                norm -= 2;
              *trgt.i16 = (int16_t) (value.i32/norm);
              trgt.i16 += stride;
            }
            if (!(ww%2)) {
              value.i32 -= src.i16[offset];
              offset += stride;
              if (!cumul)
                --norm;
              *trgt.i16 = (int16_t) (value.i32/norm);
              trgt.i16 += stride;
            }
          } else {
            int16_t v = (int16_t) (value.i32/norm);
            for ( ; i < srcinfo.rdims[0]; i++) { // right edge
              *trgt.i16 = v;
              trgt.i16 += stride;
            }
          }
        } while (trgtinfo.advanceLoop(&trgt.ui8),
                 srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
        break;
      case LUX_INT32:
        do {
          value.i32 = 0;            // initialize
          // left-hand edge
          if (internalMode & 1) { // /PARTIAL_WIDTH
            norm = cumul? ww: 0;
            if (ww%2) {                 // odd width
              value.i32 += *src.i32;
              src.i32 += stride;
              if (!cumul)
                ++norm;
              *trgt.i32 = value.i32/norm;
              trgt.i32 += stride;
              i = 1;
            } else
              i = 0;
            for ( ; i < w1; i++) {
              value.i32 += *src.i32;
              src.i32 += stride;
              value.i32 += *src.i32;
              src.i32 += stride;
              if (!cumul)
                norm += 2;
              *trgt.i32 = value.i32/norm;
              trgt.i32 += stride;
            }
          } else {              // full width
            for (i = 0; i < ww; i++) { // do the left edge
              value.i32 += *src.i32;
              src.i32 += stride;
            }
            int32_t v = value.i32/norm;
            for (i = 0; i < w1; i++) {
              *trgt.i32 = v;
              trgt.i32 += stride;
            }
          }
          // middle part
          for ( ; i < w2; i++) {
            value.i32 += *src.i32 - src.i32[offset];
            src.i32 += stride;
            *trgt.i32 = value.i32/norm;
            trgt.i32 += stride;
          }
          // right-hand edge
          if (internalMode & 1) { // /PARTIAL_WIDTH
            for ( ; i < srcinfo.rdims[0] - !(ww%2); i++) {
              value.i32 -= src.i32[offset];
              offset += stride;
              value.i32 -= src.i32[offset];
              offset += stride;
              if (!cumul)
                norm -= 2;
              *trgt.i32 = value.i32/norm;
              trgt.i32 += stride;
            }
            if (!(ww%2)) {
              value.i32 -= src.i32[offset];
              offset += stride;
              if (!cumul)
                --norm;
              *trgt.i32 = value.i32/norm;
              trgt.i32 += stride;
            }
          } else {
            int32_t v = value.i32/norm;
            for ( ; i < srcinfo.rdims[0]; i++) { // right edge
              *trgt.i32 = v;
              trgt.i32 += stride;
            }
          }
        } while (trgtinfo.advanceLoop(&trgt.ui8),
                 srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
        break;
      case LUX_INT64:
        do {
          value.i64 = 0;            // initialize
          // left-hand edge
          if (internalMode & 1) { // /PARTIAL_WIDTH
            norm = cumul? ww: 0;
            if (ww%2) {                 // odd width
              value.i64 += *src.i64;
              src.i64 += stride;
              if (!cumul)
                ++norm;
              *trgt.i64 = value.i64/norm;
              trgt.i64 += stride;
              i = 1;
            } else
              i = 0;
            for ( ; i < w1; i++) {
              value.i64 += *src.i64;
              src.i64 += stride;
              value.i64 += *src.i64;
              src.i64 += stride;
              if (!cumul)
                norm += 2;
              *trgt.i64 = value.i64/norm;
              trgt.i64 += stride;
            }
          } else {              // full width
            for (i = 0; i < ww; i++) { // do the left edge
              value.i64 += *src.i64;
              src.i64 += stride;
            }
            int32_t v = value.i64/norm;
            for (i = 0; i < w1; i++) {
              *trgt.i64 = v;
              trgt.i64 += stride;
            }
          }
          // middle part
          for ( ; i < w2; i++) {
            value.i64 += *src.i64 - src.i64[offset];
            src.i64 += stride;
            *trgt.i64 = value.i64/norm;
            trgt.i64 += stride;
          }
          // right-hand edge
          if (internalMode & 1) { // /PARTIAL_WIDTH
            for ( ; i < srcinfo.rdims[0] - !(ww%2); i++) {
              value.i64 -= src.i64[offset];
              offset += stride;
              value.i64 -= src.i64[offset];
              offset += stride;
              if (!cumul)
                norm -= 2;
              *trgt.i64 = value.i64/norm;
              trgt.i64 += stride;
            }
            if (!(ww%2)) {
              value.i64 -= src.i64[offset];
              offset += stride;
              if (!cumul)
                --norm;
              *trgt.i64 = value.i64/norm;
              trgt.i64 += stride;
            }
          } else {
            int32_t v = value.i64/norm;
            for ( ; i < srcinfo.rdims[0]; i++) { // right edge
              *trgt.i64 = v;
              trgt.i64 += stride;
            }
          }
        } while (trgtinfo.advanceLoop(&trgt.ui8),
                 srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
        break;
      case LUX_FLOAT:
        // we use the Kahan algorithm to limit roundoff error
        do {
          Kahan_sum<float> ks;
          // left-hand edge
          if (internalMode & 1) { // /PARTIAL_WIDTH
            norm = cumul? ww: 0;
            if (ww%2) {                 // odd width
              ks += *src.f;
              src.f += stride;
              if (!cumul)
                ++norm;
              *trgt.f = ks()/norm;
              trgt.f += stride;
              i = 1;
            } else
              i = 0;
            for ( ; i < w1; i++) {
              ks += *src.f;
              src.f += stride;
              ks += *src.f;
              src.f += stride;
              if (!cumul)
                norm += 2;
              *trgt.f = ks()/norm;
              trgt.f += stride;
            }
          } else {              // full width
            for (i = 0; i < ww; i++) { // do the left edge
              ks += *src.f;
              src.f += stride;
            }
            double v = ks()/norm;
            for (i = 0; i < w1; i++) {
              *trgt.f = v;
              trgt.f += stride;
            }
          }
          // middle part
          for ( ; i < w2; i++) {
            ks += *src.f - src.f[offset];
            src.f += stride;
            *trgt.f = ks()/norm;
            trgt.f += stride;
          }
          // right-hand edge
          if (internalMode & 1) { // /PARTIAL_WIDTH
            for ( ; i < srcinfo.rdims[0] - !(ww%2); i++) {
              ks += -src.f[offset];
              offset += stride;
              ks += -src.f[offset];
              offset += stride;
              if (!cumul)
                norm -= 2;
              *trgt.f = ks()/norm;
              trgt.f += stride;
            }
            if (!(ww%2)) {
              ks += -src.f[offset];
              offset += stride;
              if (!cumul)
                --norm;
              *trgt.f = ks()/norm;
              trgt.f += stride;
            }
          } else {
            float v = ks()/norm;
            for ( ; i < srcinfo.rdims[0]; i++) { // right edge
              *trgt.f = v;
              trgt.f += stride;
            }
          }
        } while (trgtinfo.advanceLoop(&trgt.ui8),
                 srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
        break;
      case LUX_DOUBLE:
        // we use the Kahan algorithm to limit roundoff error
        do {
          Kahan_sum<double> ks;
          // left-hand edge
          if (internalMode & 1) { // /PARTIAL_WIDTH
            norm = cumul? ww: 0;
            if (ww%2) {                 // odd width
              ks += *src.d;
              src.d += stride;
              if (!cumul)
                ++norm;
              *trgt.d = ks()/norm;
              trgt.d += stride;
              i = 1;
            } else
              i = 0;
            for ( ; i < w1; i++) {
              ks += *src.d;
              src.d += stride;
              ks += *src.d;
              src.d += stride;
              if (!cumul)
                norm += 2;
              *trgt.d = ks()/norm;
              trgt.d += stride;
            }
          } else {              // full width
            for (i = 0; i < ww; i++) { // do the left edge
              ks += *src.d;
              src.d += stride;
            }
            double v = ks()/norm;
            for (i = 0; i < w1; i++) {
              *trgt.d = v;
              trgt.d += stride;
            }
          }
          // middle part
          for ( ; i < w2; i++) {
            ks += *src.d - src.d[offset];
            src.d += stride;
            *trgt.d = ks()/norm;
            trgt.d += stride;
          }
          // right-hand edge
          if (internalMode & 1) { // /PARTIAL_WIDTH
            for ( ; i < srcinfo.rdims[0] - !(ww%2); i++) {
              ks  += -src.d[offset];
              offset += stride;
              ks += -src.d[offset];
              offset += stride;
              if (!cumul)
                norm -= 2;
              *trgt.d = ks()/norm;
              trgt.d += stride;
            }
            if (!(ww%2)) {
              ks += -src.d[offset];
              offset += stride;
              if (!cumul)
                --norm;
              *trgt.d = ks()/norm;
              trgt.d += stride;
            }
          } else {
            double v = ks()/norm;
            for ( ; i < srcinfo.rdims[0]; i++) { // right edge
              *trgt.d = v;
              trgt.d += stride;
            }
          }
        } while (trgtinfo.advanceLoop(&trgt.ui8),
                 srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
        break;
    }
    if (loop < srcinfo.naxes - 1) {
      if (isFreeTemp(iq)
          && lux_type_size[symbol_type(iq)] == lux_type_size[type])
                                // can use iq to hold next result
      { n = result;
        result = iq;
        symbol_type(result) = type;
        iq = n; }
      else                      // need new temp for next result
      { iq = result;
        result = array_clone(iq, type); }
      if (type < LUX_DOUBLE)
        type = LUX_FLOAT;
      srcinfo.type = type;
      srcinfo.stride = lux_type_size[type]; // original may be < LUX_FLOAT
      nextLoops(&srcinfo, &trgtinfo);
      src.ui8 = (uint8_t*) array_data(iq);
      trgt.ui8 = (uint8_t*) array_data(result);
    }
  }
  return result;
}
//-------------------------------------------------------------------------
int32_t lux_smooth(ArgumentCount narg, Symbol ps[])
// smoothing
{
  return smooth(narg, ps, 0);
}
//-------------------------------------------------------------------------
int32_t lux_runcum(ArgumentCount narg, Symbol ps[])
// running cumulative
{ return smooth(narg, ps, 1); }
//----------------------------------------------------------------------
static int32_t  pcmp_type;
static Pointer  pcmp_ptr;
int32_t pcmp(const void *arg1, const void *arg2)
{
  extern Pointer        pcmp_ptr;
  extern int32_t        pcmp_type;
  Scalar        d1, d2;

  switch (pcmp_type) {
    case LUX_INT8:
      d1.ui8 = pcmp_ptr.ui8[*(int32_t *) arg1];
      d2.ui8 = pcmp_ptr.ui8[*(int32_t *) arg2];
      return d1.ui8 < d2.ui8? -1: (d1.ui8 > d2.ui8? 1: 0);
    case LUX_INT16:
      d1.i16 = pcmp_ptr.i16[*(int32_t *) arg1];
      d2.i16 = pcmp_ptr.i16[*(int32_t *) arg2];
      return d1.i16 < d2.i16? -1: (d1.i16 > d2.i16? 1: 0);
    case LUX_INT32:
      d1.i32 = pcmp_ptr.i32[*(int32_t *) arg1];
      d2.i32 = pcmp_ptr.i32[*(int32_t *) arg2];
      return d1.i32 < d2.i32? -1: (d1.i32 > d2.i32? 1: 0);
    case LUX_INT64:
      d1.i64 = pcmp_ptr.i64[*(int32_t *) arg1];
      d2.i64 = pcmp_ptr.i64[*(int32_t *) arg2];
      return d1.i64 < d2.i64? -1: (d1.i64 > d2.i64? 1: 0);
    case LUX_FLOAT:
      d1.f = pcmp_ptr.f[*(int32_t *) arg1];
      d2.f = pcmp_ptr.f[*(int32_t *) arg2];
      return d1.f < d2.f? -1: (d1.f > d2.f? 1: 0);
    case LUX_DOUBLE:
      d1.d = pcmp_ptr.d[*(int32_t *) arg1];
      d2.d = pcmp_ptr.d[*(int32_t *) arg2];
      return d1.d < d2.d? -1: (d1.d > d2.d? 1: 0);
    }
  return 1;                     // or some compilers complain
}
//----------------------------------------------------------------------
int32_t pcmp2(const void *arg1, const void *arg2)
{
  extern Pointer        pcmp_ptr;
  extern int32_t        pcmp_type;
  Scalar        d1, d2;

  switch (pcmp_type) {
    case LUX_INT8:
      d1.ui8 = *(uint8_t *) arg1;
      d2.ui8 = pcmp_ptr.ui8[*(int32_t *) arg2];
      return d1.ui8 < d2.ui8? -1: (d1.ui8 > d2.ui8? 1: 0);
    case LUX_INT16:
      d1.i16 = *(int16_t *) arg1;
      d2.i16 = pcmp_ptr.i16[*(int32_t *) arg2];
      return d1.i16 < d2.i16? -1: (d1.i16 > d2.i16? 1: 0);
    case LUX_INT32:
      d1.i32 = *(int32_t *) arg1;
      d2.i32 = pcmp_ptr.i32[*(int32_t *) arg2];
      return d1.i32 < d2.i32? -1: (d1.i32 > d2.i32? 1: 0);
    case LUX_INT64:
      d1.i64 = *(int64_t *) arg1;
      d2.i64 = pcmp_ptr.i64[*(int32_t *) arg2];
      return d1.i64 < d2.i64? -1: (d1.i64 > d2.i64? 1: 0);
    case LUX_FLOAT:
      d1.f = *(float *) arg1;
      d2.f = pcmp_ptr.f[*(int32_t *) arg2];
      return d1.f < d2.f? -1: (d1.f > d2.f? 1: 0);
    case LUX_DOUBLE:
      d1.d = *(double *) arg1;
      d2.d = pcmp_ptr.d[*(int32_t *) arg2];
      return d1.d < d2.d? -1: (d1.d > d2.d? 1: 0);
    }
  return 1;                     // or some compilers complain
}
//----------------------------------------------------------------------
int32_t lux_match(ArgumentCount narg, Symbol ps[])
/* y = match(target, set) returns the index of each element of <target>
   in <set>, or -1 for elements that are not in <set>.
   LS 14apr97 */
{
 int32_t        nTarget, nSet, iq, step, *ptr, i, *p;
 Pointer        target, set, result;
 Symboltype     maxType;

 if (getSimpleNumerical(ps[0], &target, &nTarget) < 0)
   return -1;                   // some error
 if (getSimpleNumerical(ps[1], &set, &nSet) < 0)
   return -1;

 // make arguments have same data type
 maxType = symbol_type(ps[0]);
 if (symbol_type(ps[1]) > maxType) { // need to upgrade ps[0]
   maxType = symbol_type(ps[1]);
   iq = lux_convert(1, ps, maxType, 1);
   getSimpleNumerical(iq, &target, &nTarget);
 } else if (symbol_type(ps[1]) < maxType) { // upgrade ps[1]
   maxType = symbol_type(ps[0]);
   iq = lux_convert(1, ps + 1, maxType, 1);
   getSimpleNumerical(iq, &set, &nSet);
 }

 // create result array
 if (symbol_class(ps[0]) == LUX_ARRAY) {
   iq = array_scratch(LUX_INT32, array_num_dims(ps[0]), array_dims(ps[0]));
   result.i32 = (int32_t*) array_data(iq);
 }
 else {
   iq = scalar_scratch(LUX_INT32);
   result.i32 = &scalar_value(iq).i32;
 }

 ptr = (int32_t*) malloc(nSet*sizeof(int32_t));
 if (!ptr)
   return cerror(ALLOC_ERR, 0);
 for (i = 0; i < nSet; i++)
   ptr[i] = i;
 pcmp_ptr = set;
 pcmp_type = maxType;
 qsort(ptr, nSet, sizeof(int32_t), pcmp);

 step = lux_type_size[maxType];
 while (nTarget--) {
   p = (int32_t*) bsearch(target.ui8, ptr, nSet, sizeof(int32_t), pcmp2);
   *result.i32++ = p? *p: -1;
   target.ui8 += step;
 }
 free(ptr);
 return iq;
}
//----------------------------------------------------------------------
int32_t lux_not(ArgumentCount narg, Symbol ps[])
/* returns a uint8_t 1 for every 0, and 0 for every non-zero element.
   LS 25feb93 */
{
 int32_t        iq, i, type;
 Array *h;
 Pointer       arg, result;

 iq = ps[0];
 GET_NUMERICAL(arg, narg);
 type = sym[iq].type;
 switch (symbol_class(iq))
 { case LUX_ARRAY:  iq = array_clone(iq, LUX_INT8);  h = HEAD(iq);
                result.i32 = LPTR(h);  break;
   case LUX_SCALAR: iq = scalar_scratch(LUX_INT8);
                result.i32 = &sym[iq].spec.scalar.i32;  break; }
 switch (type)
 { case LUX_INT8:    while (narg--) *result.ui8++ = (*arg.ui8++)? 0: 1;  break;
   case LUX_INT16:    while (narg--) *result.ui8++ = (*arg.i16++)? 0: 1;  break;
   case LUX_INT32:    while (narg--) *result.ui8++ = (*arg.i32++)? 0: 1;  break;
   case LUX_INT64:    while (narg--) *result.ui8++ = (*arg.i64++)? 0: 1;  break;
   case LUX_FLOAT:   while (narg--) *result.ui8++ = (*arg.f++)? 0: 1;  break;
   case LUX_DOUBLE:  while (narg--) *result.ui8++ = (*arg.d++)? 0: 1;  break; }
 return iq;
}
//----------------------------------------------------------------------
int32_t lux_table(ArgumentCount narg, Symbol ps[])
/* General linear interpolation routine.
   Syntax:  ynew = table(x, y, [index,] xnew)
   Use:  The first dimensions of <x> and <y> must be equal and
     specify an interpolation table.  Second and higher dimensions
     of <x>, <y>, and <xnew> specify all interpolation tables.
     All dimensions shared between <x>, <y>, and <xnew> (except
     for the first dimension of <xnew>) must be equal.
     Variables are repeated to accomodate omitted dimensions.
     <xnew> contains all points at which is to be interpolated.
     The result gets the first dimension of <xnew>, and all
     shared higher dimensions of <x>, <y>, and <xnew>.
     E.g. if <x> has dimensions [3,5,2], <y> [3], and <xnew> [7],
     then the result gets dimensions [7,5,2]
*/
{
 int32_t        symx, symy, symf, nTable, nOut, nRepeat, ix, n1,
        symr, i, nsymx, nsymy, nsymf, nLoop, n2;
 Array *hx, *hy, *hf, *hMax, *hr;
 Pointer        x, y, xf, r, ox, oy, of, nx, ny;
 Symboltype topType;
 Scalar         grad;
 int32_t        lux_table2d(int32_t, int32_t []);

 if (narg == 4)
   return lux_table2d(narg, ps);
 topType = LUX_FLOAT;
 symx = *ps++;                                  // x
 CK_ARR(symx, 1);
 symy = *ps++;                                  // y
 CK_ARR(symy, 2);
 symf = *ps;                                    // xnew
 CK_ARR(symf, narg);
 if (sym[symx].type == LUX_DOUBLE || sym[symy].type == LUX_DOUBLE
     || sym[symf].type == LUX_DOUBLE)
   topType = LUX_DOUBLE;
 if (topType == LUX_DOUBLE)
 { nsymx = lux_double(1, &symx);
   nsymy = lux_double(1, &symy);
   nsymf = lux_double(1, &symf); }
 else
 { nsymx = lux_float(1, &symx);
   nsymy = lux_float(1, &symy);
   nsymf = lux_float(1, &symf); }
 hx = HEAD(nsymx);
 hy = HEAD(nsymy);
 hf = HEAD(nsymf);
 if (hx->dims[0] != hy->dims[0])        // first dim of x,y must be equal
   return cerror(INCMP_DIMS, symy);
        // now find the argument (x or y) that is largest
 hMax = hx;
 if (hy->ndim > hMax->ndim)
   hMax = hy;
        // all common dims between x and y must be equal
 for (i = 1; i < (int32_t) ((hMax == hx)? hy->ndim: hx->ndim); i++)
   if (hy->dims[i] != hx->dims[i])
     return cerror(INCMP_DIMS, symy);
 nTable = hx->dims[0];
 if (internalMode & 1)          // each xnew applied to each table
 { if (hMax->ndim > 1)
   { GET_SIZE(nRepeat, (&hMax->dims[1]), hMax->ndim - 1); }
   else nRepeat = 1;
   GET_SIZE(nOut, hf->dims, hf->ndim);
   nLoop = 1; }
 else
 { for (i = 1; i < (int32_t) ((hMax->ndim > hf->ndim)? hf->ndim: hMax->ndim); i++)
     if (hMax->dims[i] != hf->dims[i])
       return cerror(INCMP_DIMS, symf);
   if (hMax->ndim > hf->ndim)
   { GET_SIZE(nRepeat, (&hMax->dims[hf->ndim]),
              ((int32_t) hMax->ndim - hf->ndim)); }
   else nRepeat = 1;
   nOut = hf->dims[0];
   GET_SIZE(nLoop, (&hf->dims[1]), hf->ndim - 1);
 }
 i = nOut*nRepeat*nLoop;
        // create result array
 if ((symr = array_scratch(topType, 1, &i)) < 0)
   return -1;
 hr = HEAD(symr);
 // fix dimensional structure of result
 if (internalMode & 1)          // first dim from xnew, higher from x,y,xnew
 { if (hMax->ndim > hf->ndim)
   { hr->dims[0] = hf->dims[0];
     memcpy(hr->dims + 1, hMax->dims + 1, (hMax->ndim - 1)*sizeof(int32_t));
     hr->ndim = hMax->ndim; }
   else
   { memcpy(hr->dims, hf->dims, hf->ndim*sizeof(int32_t));
     hr->ndim = hf->ndim; }
 }
 else                           // all dims from xnew, all higher from x,y
 { memcpy(hr->dims, hf->dims, hf->ndim*sizeof(int32_t));
   if (hMax->ndim > 1)
     memcpy(hr->dims + hf->ndim, hMax->dims + 1, (hMax->ndim - 1)*sizeof(int32_t));
   hr->ndim = hf->ndim + hMax->ndim - 1; }
 ox.i32 = x.i32 = LPTR(hx);
 oy.i32 = y.i32 = LPTR(hy);
 of.i32 = LPTR(hf);
 nx.ui8 = ox.ui8 + sym[symx].spec.array.bstore - sizeof(Array);
 ny.ui8 = oy.ui8 + sym[symy].spec.array.bstore - sizeof(Array);
 r.i32 = LPTR(hr);
        // now the real work
        /* Search strategy:  hope that the next xnew value is near
           the previous, start searching at previous position */
 switch (topType) {
 case LUX_FLOAT:
   ix = (internalMode & 2)? nTable/2: 0; // start looking in the middle or left
   while (nRepeat--) {
     xf.f = of.f;
     for (n2 = nLoop; n2; n2--) {
       for (n1 = nOut; n1; n1--) {
         if (x.f[nTable - 1] > x.f[0]) { // assume monotonic rise
           if (*xf.f > x.f[ix]) {
             while (ix < nTable - 1 && *xf.f > x.f[ix]) ix++;
             ix--;
           } else
             while (ix > 0 && *xf.f < x.f[ix]) ix--;
         } else {               // assume monotonic fall
           if (*xf.f < x.f[ix]) {
             while (ix < nTable - 1 && *xf.f < x.f[ix]) ix++;
             ix--;
           } else
             while (ix > 0 && *xf.f > x.f[ix]) ix--;
         }
         // now ix points just left of xf, or at last element but one of x,
         // or at first element of x
         grad.f = x.f[ix + 1] - x.f[ix];
         if (grad.f != 0.0) grad.f = (y.f[ix + 1] - y.f[ix])/grad.f;
         *r.f++ = y.f[ix] + grad.f*(*xf.f++ - x.f[ix]);
       }
       x.f += nTable;  if (x.f == nx.f) x.f = ox.f;
       y.f += nTable;  if (y.f == ny.f) y.f = oy.f;
     }
   }
   break;
 case LUX_DOUBLE:
   ix = (internalMode & 2)? nTable/2: 0; // start looking in the middle or left
   while (nRepeat--) {
     xf.d = of.d;
     for (n2 = nLoop; n2; n2--) {
       for (n1 = nOut; n1; n1--) {
         if (x.d[nTable - 1] > x.d[0]) { // assume monotonic rise
           if (*xf.d > x.d[ix]) {
             while (ix < nTable - 1 && *xf.d > x.d[ix]) ix++;
             ix--;
           } else
             while (ix > 0 && *xf.d < x.d[ix]) ix--;
         } else {               // assume monotonic fall
           if (*xf.d < x.d[ix]) {
             while (ix < nTable - 1 && *xf.d < x.d[ix]) ix++;
             ix--;
           } else
             while (ix > 0 && *xf.d > x.d[ix]) ix--;
         }
         // now ix points just left of xf, or at last element but one of x,
         // or at first element of x
         grad.d = x.d[ix + 1] - x.d[ix];
         if (grad.d != 0.0) grad.d = (y.d[ix + 1] - y.d[ix])/grad.d;
         *r.d++ = y.d[ix] + grad.d*(*xf.d++ - x.d[ix]);
       }
       x.d += nTable;  if (x.d == nx.d) x.d = ox.d;
       y.d += nTable;  if (y.d == ny.d) y.d = oy.d;
     }
   }
   break;
 }
 return symr;
}
//----------------------------------------------------------------------
int32_t lux_table2d(ArgumentCount narg, Symbol ps[])
/* General linear interpolation routine.
   Syntax:  ynew = table2d(x, y, index, xnew)
   Standard use:  <x> and <y> have the same dimensions; <xnew> and <index>
     have the same dimensions.  The 2nd and further dimensions of <x> and
     <y> specify all interpolation tables.  Each element of <index>
     specifies which interpolation table to use for the corresponding
     element of <xnew>.  For non-integer elements of <index>, the
     interpolations in the tables for the adjoinging integer indices
     are determined, and a linearly weighted average of those is
     returned.
   Short-hands:  <x> may have more or less dimensions than <y>.
     Likewise, <xnew> and <index> may have an unequal number of dimensions.
     Variables are repeated to accomodate omitted dimensions.
  LS 30aug93
*/
{
 int32_t        symx, symy, symf, symi, nTable, nIndex, nRepeat, ix,
        symr, i, nsymx, nsymy, nsymf, nsymi, nx, ny, iTable;
 Array *hx, *hy, *hf, *hi;
 Pointer        x, y, xf, xi, r, ox, oy, of, nf, oi, ni;
 Symboltype topType;
 Scalar         grad, table;

 topType = LUX_FLOAT;
                /* <x> and <y> must be arrays;
                   <xnew> and <index> may be scalars or arrays */
 symx = *ps++;                                  // x
 CK_ARR(symx, 1);
 symy = *ps++;                                  // y
 CK_ARR(symy, 2);
 symi = *ps++;                                  // index
 symf = *ps;                                    // xnew
                // any LUX_DOUBLE argument -> all calculations LUX_DOUBLE
 if (sym[symx].type == LUX_DOUBLE || sym[symy].type == LUX_DOUBLE
     || sym[symf].type == LUX_DOUBLE)  topType = LUX_DOUBLE;
 if (topType == LUX_DOUBLE)
 { nsymx = lux_double(1, &symx);
   nsymy = lux_double(1, &symy);
   nsymf = lux_double(1, &symf);
   nsymi = lux_double(1, &symi); }
 else
 { nsymx = lux_float(1, &symx);
   nsymy = lux_float(1, &symy);
   nsymf = lux_float(1, &symf);
   nsymi = lux_float(1, &symi); }
                // check for valid dimensions
 hx = HEAD(nsymx);
 hy = HEAD(nsymy);
 if (hx->dims[0] != hy->dims[0])        // first dim of x,y must be equal
   return cerror(INCMP_DIMS, symy);
 nRepeat = 1;                   // the number of tables
                // shared dimensions must be equal
 if (hy->ndim > hx->ndim)
 { for (i = 1; i < (int32_t) hx->ndim; i++)  if (hx->dims[i] != hy->dims[i])
     return cerror(INCMP_DIMS, symy);
   if (hy->ndim > 1)
   { GET_SIZE(nRepeat, (hy->dims + 1), hy->ndim - 1); }
 }
 else
 { for (i = 1; i < (int32_t) hy->ndim; i++)  if (hx->dims[i] != hy->dims[i])
     return cerror(INCMP_DIMS, symy);
   if (hx->ndim > 1)
   { GET_SIZE(nRepeat, (hx->dims + 1), hx->ndim - 1); }
 }
 if (symbol_class(nsymf) == LUX_ARRAY && symbol_class(nsymi) == LUX_ARRAY)
 { hf = HEAD(nsymf);
   hi = HEAD(nsymi);
        /* shared dimensions must be equal; single numbers are taken
           as arrays with zero dimensions */
   if (hf->ndim > hi->ndim)
   { if ((int32_t) hi->ndim > 1 || (hi->dims[0] > 1 && hf->dims[0] > 1))
                                                // not zero dimensions
       for (i = 0; i < (int32_t) hi->ndim; i++)  if (hf->dims[i] != hi->dims[i])
         return cerror(INCMP_DIMS, symi);
     GET_SIZE(nIndex, hf->dims, hf->ndim);
     ix = symf; }
   else
   { if ((int32_t) hf->ndim > 1 || (hf->dims[0] > 1 && hi->dims[0] > 1))
       for (i = 0; i < (int32_t) hf->ndim; i++)  if (hf->dims[i] != hi->dims[i])
         return cerror(INCMP_DIMS, symi);
     if (hi->ndim > 1 || hi->dims[0] > hf->dims[0])
     { GET_SIZE(nIndex, hi->dims, hi->ndim);
       ix = symi; }
     else
     { GET_SIZE(nIndex, hf->dims, hf->ndim);
       ix = symf; }
   }
 }
 else
 { ix = 0;
   if (symbol_class(nsymf) == LUX_ARRAY)
   { hf = HEAD(nsymf);
     ix = symf;
     GET_SIZE(nIndex, hf->dims, hf->ndim); }
   else if (symbol_class(nsymi) == LUX_ARRAY)
   { hi = HEAD(nsymi);
     ix = symi;
     GET_SIZE(nIndex, hi->dims, hi->ndim); }
 }
        // create result symbol
 if (ix)
 { if ((symr = array_clone(ix, topType)) < 0)
     return -1;
   r.i32 = LPTR(HEAD(symr)); }
 else
 { if ((symr = scalar_scratch(topType)) < 0)
     return -1;
   nIndex = 1;
   r.i32 = &sym[symr].spec.scalar.i32; }
 nTable = hx->dims[0];          // number of elements per table
 ox.i32 = LPTR(hx);
 oy.i32 = LPTR(hy);
 if (symbol_class(nsymf) == LUX_ARRAY)
 { of.i32 = xf.i32 = LPTR(hf);
   nf.ui8 = of.ui8 + sym[symf].spec.array.bstore - sizeof(Array); }
 else
 { of.i32 = xf.i32 = &sym[nsymf].spec.scalar.i32;
   nf.ui8 = of.ui8 + lux_type_size[topType]; }
 if (symbol_class(nsymi) == LUX_ARRAY)
 { oi.i32 = xi.i32 = LPTR(hi);
   ni.ui8 = oi.ui8 + sym[symi].spec.array.bstore - sizeof(Array); }
 else
 { oi.i32 = xi.i32 = &sym[nsymi].spec.scalar.i32;
   ni.ui8 = oi.ui8 + lux_type_size[topType]; }
 nx = (sym[symx].spec.array.bstore - sizeof(Array))
   / lux_type_size[topType];
 ny = (sym[symy].spec.array.bstore - sizeof(Array))
   / lux_type_size[topType];
        // now the real work
        /* Search strategy:  hope that the next xnew value is near
           the previous, start searching at previous position */
 switch (topType)
 { case LUX_FLOAT:
     ix = nTable/2;             // start looking in the middle
     while (nIndex--)
     { table.f = *xi.f;
       iTable = (int32_t) table.f;
       if (iTable < 0) iTable = 0;
       if (iTable > nRepeat - ((float) iTable == table.f? 1: 2))
         iTable = nRepeat - 2;
       x.f = ox.f + iTable*nTable % nx;         // start of relevant table
       y.f = oy.f + iTable*nTable % ny;
       if (*xf.f > x.f[ix])
       { while (ix < nTable - 1 && *xf.f > x.f[ix]) ix++;
         ix--; }
       else
         while (ix > 0 && *xf.f < x.f[ix]) ix--;
        // now ix points just left of xf, or at last element but one of x,
        // or at first element of x
       grad.f = x.f[ix + 1] - x.f[ix];
       if (grad.f != 0.0) grad.f = (y.f[ix + 1] - y.f[ix])/grad.f;
       *r.f = y.f[ix] + grad.f*(*xf.f - x.f[ix]);
       if (table.f != (float) iTable)
       { x.f += nTable % nx;  y.f += nTable % ny;
         if (*xf.f > x.f[ix])
         { while (ix < nTable - 1 && *xf.f > x.f[ix]) ix++;
           ix--; }
         else
           while (ix > 0 && *xf.f < x.f[ix]) ix--;
         grad.f = x.f[ix + 1] - x.f[ix];
         if (grad.f != 0.0) grad.f = (y.f[ix + 1] - y.f[ix])/grad.f;
         *r.f += (y.f[ix] + grad.f*(*xf.f - x.f[ix]) - *r.f)
           *(table.f - iTable); }
       r.f++;
       if (++xf.f == nf.f) xf.f = of.f;
       if (++xi.f == ni.f) xi.f = oi.f; }
     break;
   case LUX_DOUBLE:
     ix = nTable/2;             // start looking in the middle
     while (nIndex--)
     { table.d = *xi.d;
       iTable = (int32_t) table.d;
       if (iTable < 0) iTable = 0;
       if (iTable > nRepeat - ((double) iTable == table.d? 1: 2))
         iTable = nRepeat - 2;
       x.d = ox.d + iTable*nTable % nx;
       y.d = oy.d + iTable*nTable % ny;
       if (*xf.d > x.d[ix])
       { while (ix < nTable - 1 && *xf.d > x.d[ix]) ix++;
         ix--; }
       else
         while (ix > 0 && *xf.d < x.d[ix]) ix--;
        // now ix points just left of xf, or at last element but one of x,
        // or at first element of x
       grad.d = x.d[ix + 1] - x.d[ix];
       if (grad.d != 0.0) grad.d = (y.d[ix + 1] - y.d[ix])/grad.d;
       *r.d = y.d[ix] + grad.d*(*xf.d - x.d[ix]);
       if (table.d != (double) iTable)
       { x.d += nTable % nx;  y.d += nTable % ny;
         if (*xf.d > x.d[ix])
         { while (ix < nTable - 1 && *xf.d > x.d[ix]) ix++;
           ix--; }
         else
           while (ix > 0 && *xf.d < x.d[ix]) ix--;
         grad.d = x.d[ix + 1] - x.d[ix];
         if (grad.d != 0.0) grad.d = (y.d[ix + 1] - y.d[ix])/grad.d;
         *r.d += (y.d[ix] + grad.d*(*xf.d - x.d[ix]) - *r.d)
           *(table.d - iTable); }
       r.d++;
       if (++xf.d == nf.d) xf.d = of.d;
       if (++xi.d == ni.d) xi.d = oi.d; }
     break;
   }
 return symr;
}
//----------------------------------------------------------------------
int32_t lux_push(ArgumentCount narg, Symbol ps[])
/* Push a number of variables onto the user stack
   LS 27may94 3aug99 */
{
  int32_t       iq;

  while (narg--) {
    if (stackPointer == stack)
      return luxerror("Stack full", *ps);
    getFreeExecutable(iq);
    copyToSym(iq, *ps);
    sym[iq].xx = (symbolIsNamed(*ps)? *ps: 0); // number of original symbol
    ps++;
    *--stackPointer = iq;
    clist_symbols(stackSym) = stackPointer;
    symbol_memory(stackSym) += sizeof(int16_t);
  }
  return 1;
}
//----------------------------------------------------------------------
int32_t lux_pop(ArgumentCount narg, Symbol ps[])
/* pop a number of variables from the user stack, in reverse order from PUSH;
   i.e. after  push,x,y  pop,x,y  restores x and y.
   pop,NUM=<number> pops the top <number> symbols into their original sources.
   If pop has arguments that are not named variables, then
   these are taken to represent a number and that number of symbols is popped.
   LS 27may94 3aug99 */
{
  char  isError = 0;
  int32_t       number, iq, i1, i2, i;

  if (ps[0]) {                  // have NUM
    number = int_arg(ps[0]);
    while (number--) {
      if (stackPointer == stack + STACKSIZE)
        return luxerror("Stack is empty", *ps);
      iq = *stackPointer++;
      if (!sym[iq].xx) {
        luxerror("No named variable to restore to", 0);
        isError = 1;
      } else if (lux_replace(sym[iq].xx, iq) == LUX_ERROR)
        isError = 1;
      zap(iq);
      symbol_memory(stackSym) -= sizeof(int16_t);
      clist_symbols(stackSym) = stackPointer;
    }
  }

  // any remaining arguments
  i1 = i2 = 1;
  while (i2 < narg) {
    while (i2 < narg && symbolIsNamed(ps[i2]))
      i2++;                     // explicit symbols to pop
    for (i = i2 - 1; i >= i1; i--) { // pop in reverse order
      if (stackPointer == stack + STACKSIZE)
        return luxerror("Stack is empty", ps[i]);
      if (lux_replace(ps[i], *stackPointer) == LUX_ERROR)
        isError = 1;
      zap(*stackPointer++);
      symbol_memory(stackSym) -= sizeof(int16_t);
      clist_symbols(stackSym) = stackPointer;
    }
    if (i2 < narg) {            // number of symbols to pop
      number = int_arg(ps[i2]);
      while (number--) {
        if (stackPointer == stack + STACKSIZE)
          return luxerror("Stack is empty", *ps);
        iq = *stackPointer++;
        if (!sym[iq].xx) {
          luxerror("No variable to restore to", 0);
          isError = 1;
        } else if (lux_replace(sym[iq].xx, iq) == LUX_ERROR)
          isError = 1;
        zap(iq);
        symbol_memory(stackSym) -= sizeof(int16_t);
        clist_symbols(stackSym) = stackPointer;
      }
    }
    i1 = ++i2;
  }
  return isError? LUX_ERROR: LUX_OK;
}
//----------------------------------------------------------------------
int32_t lux_dump_stack(ArgumentCount narg, Symbol ps[])
/* Displays the contents of the user stack
   LS 27may94 */
{
  int32_t       iq, count = 0;
  int16_t       *p;

  if (stackPointer == stack + STACKSIZE) {
    puts("The stack is empty.");
    return 1;
  }
  for (p = stackPointer; p < stack + STACKSIZE; p++) {
    iq = *p;
    printf("%3d %10s ", count++, varName(sym[iq].xx));
    symdumpswitch(iq, I_VALUE | I_TRUNCATE | I_LENGTH);
  }
  return 1;
}
//----------------------------------------------------------------------
int32_t peek(ArgumentCount narg, Symbol ps[])
// display memory contents
{
  int32_t       start, nr;
  uint8_t       *adr, i;
  char  s;

// we're in trouble on OSF machines, where pointers are bigger than ints.
// how to specify a large address then?
  start = int_arg(ps[0]);
  if (narg > 1) nr = int_arg(ps[1]); else nr = 1;
  if (nr < 1) nr = 1;
  adr = (uint8_t *) NULL + start;
  while (nr--)
  { i = *adr++;
    if (isprint(i)) s = (char) i; else s = '*';
    printf("%3d %02x %c\n", i, i, s); }
  return 1;
}
//----------------------------------------------------------------------
int32_t lux_atomize(ArgumentCount narg, Symbol ps[])
/* atomizes and displays symbol *ps - for debugging purposes
   second argument determines whether all individual atomic arguments
   are also lux_dump-ed
   LS 18may93 */
{
 char   *symbol_ident(char *, int32_t, int32_t, char);
 int32_t        iq, dump;
 void   dumpLine(int32_t), dumpTree(int32_t);

 iq = int_arg(*ps++);
 if (internalMode & 2)
   dumpLine(iq);
 else if (internalMode & 1)
   dumpTree(iq);
 else
 { if (narg > 1) dump = int_arg(*ps); else dump = 0;
   puts(symbolIdent(iq, dump));
 }
 return 1;
}
//----------------------------------------------------------------------
int32_t lux_redirect_diagnostic(ArgumentCount narg, Symbol ps[])
// specifies file to which diagnostic information is redirected
// LS 29may93
{
 extern char    *defaultRedirect;
 char   *p;

 if (!narg)
   p = defaultRedirect;
 else
   p = string_arg(*ps);
 if (stderr != freopen(p, "w", stderr))
   printf("Cannot open file %s for diagnostic output\n", p);
 return 1;
}
//----------------------------------------------------------------------
int32_t lux_default(ArgumentCount narg, Symbol ps[])
/* syntax:  default,var1,val1,var2,val2,...
   assigns <val1>, <val2>,... to <var1>, <var2>,... if <var1> etc. are
   undefined, i.e. specifies default values.   LS 4jun93 */
// the arguments are not evaluated before entering this routine, so we
// can use previous arguments in later arguments, e.g.,
// DEFAULT,A,3,B,2*A  works fine even when A and B are both undefined
// before this statement was executed.  LS 27mar97
{
 int32_t        i, n = 1, iq;

 for (i = 0; i < narg/2; i++)
 { iq = *ps;
   iq = transfer(iq);           // resolve pointers
   if (!iq)                     // pointer to nothing
   { iq = *ps;                  // get last one in chain
     while (symbol_class(iq) == LUX_TRANSFER && transfer_target(iq))
       iq = transfer_target(iq);
     n = lux_replace(*ps, ps[1]); }
   else
     if (symbol_class(iq) == LUX_UNDEFINED
         && lux_replace(*ps, ps[1]) != 1)
       n = -1;
   ps += 2; }
 return n;
}
//----------------------------------------------------------------------
int32_t local_maxormin(ArgumentCount narg, Symbol ps[], int32_t code)
/* arguments:  (array, position, code)
   searches for local extreme in <array>, starting at <position> (which
   is either a scalar or an array of indices)
   and walking along the steepest gradient.
    code bits    unset        set
        0       seek value   seek position
        1       seek min     seek max
        2       grid pos     sub-grid pos  (in development)
        3       quadratic    quadratic, but force closeness
   If a subgrid position or value is sought, then the local field is
   approximated by a quadratic hypersurface and the zero-gradient point
   on that is determined.  If bit 3 is set, then the solution is checked
   for closeness to the integer position of the extreme.  If the subgrid
   position is more than 0.75 px away from the integer position in either
   coordinate, then the off-diagonal elements of the hessian matrix are
   set to zero (i.e. the quadratic is assumed aligned with the coordinate
   axes), thereby forcing a position within one px of the integer
   position.  LS 28jan95
   This closeness-scheme does not work if the integer position of the
   maximum is on one of the edges of the data: closeness is not enforced
   on the edges.
   The found position is stored as index to <array>
   in !LASTMAXLOC or !LASTMINLOC (always integer), and the value at that
   position in !LASTMAX or !LASTMIN.   LS 9jun93 */
// added ghost-array support  LS 23jun93
{
 int32_t                iq, *dims, ndim, n, i, currentIndex,
                comparisonIndex, nextIndex,
                size[MAX_DIMS], typesize, max, indx[MAX_DIMS], j,
                defaultOffset, *indexPtr, n2, ntarget, nelem;
 Symboltype type, trgttype;
 extern int32_t         lastmaxloc, lastminloc, lastmin_sym, lastmax_sym;
 extern Scalar  lastmax, lastmin;
 char           count[MAX_DIMS], *fname, filemap = 0, ready, edge;
 float          *grad, *grad2, *hessian, *hessian2, x, v;
 Scalar                 value, nextValue;
 Pointer        data, trgt, extr;
 FILE           *fp;

                // treat arguments
 iq = ps[0];                            // data array
 if ((n = symbol_class(iq)) == LUX_FILEMAP)
   filemap = 1;
 else if (n != LUX_ARRAY)
   return cerror(NEED_ARR, iq);
 if (filemap) {
   fname = file_map_file_name(iq);
   if ((fp = fopen(fname, "r")) == NULL) {
     printf("File %s; ", fname);
     return cerror(ERR_OPEN, iq);
   }
 }
 data.i32 = (int32_t*) array_data(iq);
 dims = array_dims(iq);
 ndim = array_num_dims(iq);
 type = symbol_type(iq);
 if (!isNumericalType(type))
   return cerror(ILL_TYPE, iq);
 n = nelem = array_size(iq);
 typesize = lux_type_size[type];

 iq = lux_long(1, &ps[1]);      // position
 switch (symbol_class(iq)) {
   case LUX_SCALAR:
     indexPtr = &scalar_value(iq).i32;    // index
     ntarget = 1;
     break;
   case LUX_ARRAY:
     ntarget = array_size(iq);
     indexPtr = (int32_t*) array_data(iq);
     break;
   default:
     return cerror(ILL_CLASS, iq);
 }
 code |= internalMode*4;
 max = (code & 2)? 1: 0;        // searching max?
 /* code       sought              return type
    0,2       grid value            source type
    1,3       grid position         LONG
    4,6       subgrid value         LUX_FLOAT
    5,7       subgrid position      LUX_FLOAT  */
 trgttype = (code & 4)? LUX_FLOAT: (code & 1)? LUX_INT32: type;
 if ((code & 5) == 5) {                 // seek sub-grid position
                                // may need extra dimensions in result
   if (symbol_class(iq) == LUX_SCALAR) {
     size[0] = ndim;
     j = 1;
   } else {
     if (array_num_dims(iq) >= MAX_DIMS) {
       puts("Sub-grid position array needs too many dimensions.");
       return cerror(N_DIMS_OVR, 0);
     }
     memcpy(size, array_dims(iq), array_num_dims(iq)*sizeof(float));
     j = array_num_dims(iq);
     size[j++] = ndim;
   }
   iq = array_scratch(LUX_FLOAT, j, size);
   trgt.i32 = (int32_t*) array_data(iq);
 } else {                       // not seeking subgrid position
   if (symbol_class(iq) == LUX_SCALAR) {
     iq = scalar_scratch(trgttype);
     trgt.i32 = &scalar_value(iq).i32;
   } else {                     // array
     iq = array_clone(iq, trgttype);
     trgt.i32 = (int32_t*) array_data(iq);
   }
 }
 defaultOffset = -1;
 size[0] = 1;
 for (i = 1; i < ndim; i++) {
   size[i] = size[i - 1]*dims[i - 1];
   defaultOffset -= size[i];
 }
 if (code & 4) {                // subgrid stuff
   ALLOCATE(grad, ndim, float);
   ALLOCATE(grad2, ndim, float);
   ALLOCATE(hessian, ndim*ndim, float);
   ALLOCATE(hessian2, ndim, float);
 }
                // setup for search
 n = ntarget;
 while (n--) {
   currentIndex = *indexPtr++;
   if (currentIndex < 0)        // outside of data array
     currentIndex = 0;
   else if (currentIndex >= nelem) // outside of data array
     currentIndex = nelem - 1;
   if (filemap) {
     if (fseek(fp, typesize*currentIndex, SEEK_SET))
       return cerror(POS_ERR, iq);
     if (fread(&nextValue.ui8, typesize, 1, fp) != 1)
       return cerror(READ_ERR, iq);
   } else
     memcpy(&nextValue.ui8, data.ui8 + typesize*currentIndex, typesize);
   nextIndex = comparisonIndex = currentIndex;
   for (i = 0; i < ndim; i++) {
     indx[i] = nextIndex % dims[i] - 1;
     if (indx[i] < 0) {
       indx[i] = 0;
       count[i] = 1;
     } else {
       comparisonIndex -= size[i];
       count[i] = 0;
     }
     nextIndex /= dims[i];
   }
   nextIndex = currentIndex;

        /* now comparisonIndex points at first value to be compared
           and step[] contains step sizes to move around currentIndex */
                // now do the search
   do {
     do {
       if (filemap)
         readGhost(fp, value.ui8, comparisonIndex, typesize);
       switch (type) {
         case LUX_INT8:
           if (!filemap)
             value.ui8 = data.ui8[comparisonIndex];
           if ((max)? value.ui8 > nextValue.ui8: value.ui8 < nextValue.ui8) {
             nextIndex = comparisonIndex;
             nextValue.ui8 = value.ui8;
           }
           break;
         case LUX_INT16:
           if (!filemap)
             value.i16 = data.i16[comparisonIndex];
           if ((max)? value.i16 > nextValue.i16: value.i16 < nextValue.i16) {
             nextIndex = comparisonIndex;
             nextValue.i16 = value.i16;
           }
           break;
         case LUX_INT32:
           if (!filemap)
             value.i32 = data.i32[comparisonIndex];
           if ((max)? value.i32 > nextValue.i32: value.i32 < nextValue.i32) {
             nextIndex = comparisonIndex;
             nextValue.i32 = value.i32;
           }
           break;
         case LUX_INT64:
           if (!filemap)
             value.i64 = data.i64[comparisonIndex];
           if ((max)? value.i64 > nextValue.i64: value.i64 < nextValue.i64) {
             nextIndex = comparisonIndex;
             nextValue.i64 = value.i64;
           }
           break;
         case LUX_FLOAT:
           if (!filemap)
             value.f = data.f[comparisonIndex];
           if ((max)? value.f > nextValue.f: value.f < nextValue.f) {
             nextIndex = comparisonIndex;
             nextValue.f = value.f;
           }
           break;
         case LUX_DOUBLE:
           if (!filemap)
             value.d = data.d[comparisonIndex];
           if ((max)? value.d > nextValue.d: value.d < nextValue.d) {
             nextIndex = comparisonIndex;
             nextValue.d = value.d;
           }
           break;
       }
       do {
         for (i = 0; i < ndim; i++) { // step around currentIndex
           comparisonIndex += size[i];
           indx[i]++;
           count[i]++;
           if (count[i] == 3) {
             indx[i] -= 3;
             comparisonIndex -= 3*size[i];
             count[i] = 0;
           } else
             break;
         }
         if (i == ndim)
           break;
         j = 0;
         for (n2 = 0; n2 < ndim; n2++)
           if (indx[n2] < 0 || indx[n2] == dims[n2]) {
             j = 1;
             break;
           }
       } while (j);
     } while (i != ndim);
     if (nextIndex == currentIndex)
       break;                   // found local extreme
        // prepare for next round
     currentIndex = comparisonIndex = nextIndex;
     for (i = 0; i < ndim; i++) {
       indx[i] = nextIndex % dims[i] - 1;
       if (indx[i] < 0) {
         indx[i] = 0;
         count[i] = 1;
       } else {
         comparisonIndex -= size[i];
         count[i] = 0;
       }
       nextIndex /= dims[i];
     }
     nextIndex = currentIndex;
   } while (1);
   if (code & 4) {              // subgrid
     n2 = currentIndex;
     edge = 0;
     for (i = 0; i < ndim; i++)         { // calculate coordinates
       indx[i] = n2 % dims[i];
       n2 /= dims[i];
       if (indx[i] < 1 || indx[i] > dims[i] - 2)
         edge = 1;
     }
     if (edge)                  // data point on edge: no reliable fit
       for (i = 0; i < ndim; i++)
         grad2[i] = 0.0;
     else {
       switch (type) {
         case LUX_INT8:
           // calculate gradient and hessian matrix
           if (!filemap) {
             extr.ui8 = &data.ui8[currentIndex];
             v = (float) *extr.ui8;
             for (i = 0; i < ndim; i++)
               grad[i] = grad2[i] = ((float) extr.ui8[size[i]]
                                     - (float) extr.ui8[-size[i]])/2;
             ready = 1;                 // zero gradient?
             for (i = 0; i < ndim; i++) {
               if (grad[i] != 0) {
                 ready = 0;
                 break;
               }
             }
             if (ready)
               break;
             for (i = 0; i < ndim; i++)
               for (j = 0; j <= i; j++) {
                 if (i == j)
                   hessian[i + i*ndim] = hessian2[i] =
                     (float) extr.ui8[size[i]] + (float) extr.ui8[-size[i]] - 2*v;
                 else {
                   x = ((float) extr.ui8[size[i] + size[j]]
                        + (float) extr.ui8[-size[i] - size[j]]
                        - (float) extr.ui8[size[i] - size[j]]
                        - (float) extr.ui8[size[j] - size[i]])/4;
                   hessian[i + j*ndim] = hessian[j + i*ndim] = x;
                 }
               }
           } else {             // filemap array
             readGhost(fp, value.ui8, currentIndex, typesize);
             v = (float) value.ui8;
             for (i = 0; i < ndim; i++) {
               readGhost(fp, value.ui8, currentIndex + size[i], typesize);
               grad[i] = (float) value.ui8;
               readGhost(fp, value.ui8, currentIndex - size[i], typesize);
               grad[i] = (grad[i] - (float) value.ui8)/2;
             }
             ready = 1;                 // zero gradient?
             for (i = 0; i < ndim; i++) {
               if (grad[i] != 0) {
                 ready = 0;
                 break;
               }
             }
             if (ready)
               break;
             for (i = 0; i < ndim; i++)
               for (j = 0; j <= i; j++) {
                 if (i == j) {
                   readGhost(fp, value.ui8, currentIndex + size[i], typesize);
                   x = (float) value.ui8;
                   readGhost(fp, value.ui8, currentIndex - size[i], typesize);
                   hessian[i + i*ndim] = hessian2[i] =
                     x + (float) value.ui8 - 2*v;
                 } else {
                   readGhost(fp, value.ui8, currentIndex + size[i] + size[j],
                             typesize);
                   x = (float) value.ui8;
                   readGhost(fp, value.ui8, currentIndex - size[i] - size[j],
                             typesize);
                   x += (float) value.ui8;
                   readGhost(fp, value.ui8, currentIndex + size[i] - size[j],
                             typesize);
                   x -= (float) value.ui8;
                   readGhost(fp, value.ui8, currentIndex - size[i] + size[j],
                             typesize);
                   x -= (float) value.ui8;
                   hessian[i + j*ndim] = hessian[j + i*ndim] = x/4;
                 }
               }
           }
           break;
         case LUX_INT16:
           // calculate gradient and hessian matrix
           if (!filemap) {
             extr.i16 = &data.i16[currentIndex];
             v = (float) *extr.i16;
             for (i = 0; i < ndim; i++)
               grad[i] = grad2[i] = ((float) extr.i16[size[i]]
                                     - (float) extr.i16[-size[i]])/2;
             ready = 1;                 // zero gradient?
             for (i = 0; i < ndim; i++) {
               if (grad[i] != 0) {
                 ready = 0;
                 break;
               }
             }
             if (ready)
               break;
             for (i = 0; i < ndim; i++)
               for (j = 0; j <= i; j++) {
                 if (i == j)
                   hessian[i + i*ndim] = hessian2[i] =
                     (float) extr.i16[size[i]] + (float) extr.i16[-size[i]] - 2*v;
                 else {
                   x = ((float) extr.i16[size[i] + size[j]]
                        + (float) extr.i16[-size[i] - size[j]]
                        - (float) extr.i16[size[i] - size[j]]
                        - (float) extr.i16[size[j] - size[i]])/4;
                   hessian[i + j*ndim] = hessian[j + i*ndim] = x;
                 }
               }
           } else {             // filemap array
             readGhost(fp, value.i16, currentIndex, typesize);
             v = (float) value.i16;
             for (i = 0; i < ndim; i++) {
               readGhost(fp, value.i16, currentIndex + size[i], typesize);
               grad[i] = (float) value.i16;
               readGhost(fp, value.i16, currentIndex - size[i], typesize);
               grad[i] = (grad[i] - (float) value.i16)/2;
             }
             ready = 1;                 // zero gradient?
             for (i = 0; i < ndim; i++) {
               if (grad[i] != 0) {
                 ready = 0;
                 break;
               }
             }
             if (ready)
               break;
             for (i = 0; i < ndim; i++)
               for (j = 0; j <= i; j++) {
                 if (i == j) {
                   readGhost(fp, value.i16, currentIndex + size[i], typesize);
                   x = (float) value.i16;
                   readGhost(fp, value.i16, currentIndex - size[i], typesize);
                   hessian[i + i*ndim] = hessian2[i] =
                     x + (float) value.i16 - 2*v;
                 } else {
                   readGhost(fp, value.i16, currentIndex + size[i] + size[j],
                             typesize);
                   x = (float) value.i16;
                   readGhost(fp, value.i16, currentIndex - size[i] - size[j],
                             typesize);
                   x += (float) value.i16;
                   readGhost(fp, value.i16, currentIndex + size[i] - size[j],
                             typesize);
                   x -= (float) value.i16;
                   readGhost(fp, value.i16, currentIndex - size[i] + size[j],
                             typesize);
                   x -= (float) value.i16;
                   hessian[i + j*ndim] = hessian[j + i*ndim] = x/4;
                 }
               }
           }
           break;
         case LUX_INT32:
           // calculate gradient and hessian matrix
           if (!filemap) {
             extr.i32 = &data.i32[currentIndex];
             v = (float) *extr.i32;
             for (i = 0; i < ndim; i++)
               grad[i] = grad2[i] = ((float) extr.i32[size[i]]
                                     - (float) extr.i32[-size[i]])/2;
             ready = 1;                 // zero gradient?
             for (i = 0; i < ndim; i++) {
               if (grad[i] != 0) {
                 ready = 0;
                 break;
               }
             }
             if (ready)
               break;
             for (i = 0; i < ndim; i++)
               for (j = 0; j <= i; j++) {
                 if (i == j)
                   hessian[i + i*ndim] = hessian2[i] =
                     (float) extr.i32[size[i]] + (float) extr.i32[-size[i]] - 2*v;
                 else {
                   x = ((float) extr.i32[size[i] + size[j]]
                        + (float) extr.i32[-size[i] - size[j]]
                        - (float) extr.i32[size[i] - size[j]]
                        - (float) extr.i32[size[j] - size[i]])/4;
                   hessian[i + j*ndim] = hessian[j + i*ndim] = x;
                 }
               }
           } else {             // filemap array
             readGhost(fp, value.i32, currentIndex, typesize);
             v = (float) value.i32;
             for (i = 0; i < ndim; i++) {
               readGhost(fp, value.i32, currentIndex + size[i], typesize);
               grad[i] = (float) value.i32;
               readGhost(fp, value.i32, currentIndex - size[i], typesize);
               grad[i] = (grad[i] - (float) value.i32)/2;
             }
             ready = 1;                 // zero gradient?
             for (i = 0; i < ndim; i++) {
               if (grad[i] != 0) {
                 ready = 0;
                 break;
               }
             }
             if (ready)
               break;
             for (i = 0; i < ndim; i++)
               for (j = 0; j <= i; j++) {
                 if (i == j) {
                   readGhost(fp, value.i32, currentIndex + size[i], typesize);
                   x = (float) value.i32;
                   readGhost(fp, value.i32, currentIndex - size[i], typesize);
                   hessian[i + i*ndim] = hessian2[i] =
                     x + (float) value.i32 - 2*v;
                 } else {
                   readGhost(fp, value.i32, currentIndex + size[i] + size[j],
                             typesize);
                   x = (float) value.i32;
                   readGhost(fp, value.i32, currentIndex - size[i] - size[j],
                             typesize);
                   x += (float) value.i32;
                   readGhost(fp, value.i32, currentIndex + size[i] - size[j],
                             typesize);
                   x -= (float) value.i32;
                   readGhost(fp, value.i32, currentIndex - size[i] + size[j],
                             typesize);
                   x -= (float) value.i32;
                   hessian[i + j*ndim] = hessian[j + i*ndim] = x/4;
                 }
               }
           }
           break;
         case LUX_INT64:
           // calculate gradient and hessian matrix
           if (!filemap) {
             extr.i64 = &data.i64[currentIndex];
             v = (float) *extr.i64;
             for (i = 0; i < ndim; i++)
               grad[i] = grad2[i] = ((float) extr.i64[size[i]]
                                     - (float) extr.i64[-size[i]])/2;
             ready = 1;                 // zero gradient?
             for (i = 0; i < ndim; i++) {
               if (grad[i] != 0) {
                 ready = 0;
                 break;
               }
             }
             if (ready)
               break;
             for (i = 0; i < ndim; i++)
               for (j = 0; j <= i; j++) {
                 if (i == j)
                   hessian[i + i*ndim] = hessian2[i] =
                     (float) extr.i64[size[i]] + (float) extr.i64[-size[i]] - 2*v;
                 else {
                   x = ((float) extr.i64[size[i] + size[j]]
                        + (float) extr.i64[-size[i] - size[j]]
                        - (float) extr.i64[size[i] - size[j]]
                        - (float) extr.i64[size[j] - size[i]])/4;
                   hessian[i + j*ndim] = hessian[j + i*ndim] = x;
                 }
               }
           } else {             // filemap array
             readGhost(fp, value.i64, currentIndex, typesize);
             v = (float) value.i64;
             for (i = 0; i < ndim; i++) {
               readGhost(fp, value.i64, currentIndex + size[i], typesize);
               grad[i] = (float) value.i64;
               readGhost(fp, value.i64, currentIndex - size[i], typesize);
               grad[i] = (grad[i] - (float) value.i64)/2;
             }
             ready = 1;                 // zero gradient?
             for (i = 0; i < ndim; i++) {
               if (grad[i] != 0) {
                 ready = 0;
                 break;
               }
             }
             if (ready)
               break;
             for (i = 0; i < ndim; i++)
               for (j = 0; j <= i; j++) {
                 if (i == j) {
                   readGhost(fp, value.i64, currentIndex + size[i], typesize);
                   x = (float) value.i64;
                   readGhost(fp, value.i64, currentIndex - size[i], typesize);
                   hessian[i + i*ndim] = hessian2[i] =
                     x + (float) value.i64 - 2*v;
                 } else {
                   readGhost(fp, value.i64, currentIndex + size[i] + size[j],
                             typesize);
                   x = (float) value.i64;
                   readGhost(fp, value.i64, currentIndex - size[i] - size[j],
                             typesize);
                   x += (float) value.i64;
                   readGhost(fp, value.i64, currentIndex + size[i] - size[j],
                             typesize);
                   x -= (float) value.i64;
                   readGhost(fp, value.i64, currentIndex - size[i] + size[j],
                             typesize);
                   x -= (float) value.i64;
                   hessian[i + j*ndim] = hessian[j + i*ndim] = x/4;
                 }
               }
           }
           break;
         case LUX_FLOAT:
           // calculate gradient and hessian matrix
           if (!filemap) {
             extr.f = &data.f[currentIndex];
             v = (float) *extr.f;
             for (i = 0; i < ndim; i++)
               grad[i] = grad2[i] = ((float) extr.f[size[i]]
                                     - (float) extr.f[-size[i]])/2;
             ready = 1;                 // zero gradient?
             for (i = 0; i < ndim; i++) {
               if (grad[i] != 0) {
                 ready = 0;
                 break;
               }
             }
             if (ready)
               break;
             for (i = 0; i < ndim; i++)
               for (j = 0; j <= i; j++) {
                 if (i == j)
                   hessian[i + i*ndim] = hessian2[i] =
                     (float) extr.f[size[i]] + (float) extr.f[-size[i]] - 2*v;
                 else {
                   x = ((float) extr.f[size[i] + size[j]]
                        + (float) extr.f[-size[i] - size[j]]
                        - (float) extr.f[size[i] - size[j]]
                        - (float) extr.f[size[j] - size[i]])/4;
                   hessian[i + j*ndim] = hessian[j + i*ndim] = x;
                 }
               }
           } else {             // filemap array
             readGhost(fp, value.f, currentIndex, typesize);
             v = (float) value.f;
             for (i = 0; i < ndim; i++) {
               readGhost(fp, value.f, currentIndex + size[i], typesize);
               grad[i] = (float) value.f;
               readGhost(fp, value.f, currentIndex - size[i], typesize);
               grad[i] = (grad[i] - (float) value.f)/2;
             }
             ready = 1;                 // zero gradient?
             for (i = 0; i < ndim; i++) {
               if (grad[i] != 0) {
                 ready = 0;
                 break;
               }
             }
             if (ready)
               break;
             for (i = 0; i < ndim; i++)
               for (j = 0; j <= i; j++) {
                 if (i == j) {
                   readGhost(fp, value.f, currentIndex + size[i], typesize);
                   x = (float) value.f;
                   readGhost(fp, value.f, currentIndex - size[i], typesize);
                   hessian[i + i*ndim] = hessian2[i] =
                     x + (float) value.f - 2*v;
                 } else {
                   readGhost(fp, value.f, currentIndex + size[i] + size[j],
                             typesize);
                   x = (float) value.f;
                   readGhost(fp, value.f, currentIndex - size[i] - size[j],
                             typesize);
                   x += (float) value.f;
                   readGhost(fp, value.f, currentIndex + size[i] - size[j],
                             typesize);
                   x -= (float) value.f;
                   readGhost(fp, value.f, currentIndex - size[i] + size[j],
                             typesize);
                   x -= (float) value.f;
                   hessian[i + j*ndim] = hessian[j + i*ndim] = x/4;
                 }
               }
           }
           break;
         case LUX_DOUBLE:
           // calculate gradient and hessian matrix
           if (!filemap) {
             extr.d = &data.d[currentIndex];
             v = (float) *extr.d;
             for (i = 0; i < ndim; i++)
               grad[i] = grad2[i] = ((float) extr.d[size[i]]
                                     - (float) extr.d[-size[i]])/2;
             ready = 1;                 // zero gradient?
             for (i = 0; i < ndim; i++) {
               if (grad[i] != 0) {
                 ready = 0;
                 break;
               }
             }
             if (ready)
               break;
             for (i = 0; i < ndim; i++)
               for (j = 0; j <= i; j++) {
                 if (i == j)
                   hessian[i + i*ndim] = hessian2[i] =
                     (float) extr.d[size[i]] + (float) extr.d[-size[i]] - 2*v;
                 else {
                   x = ((float) extr.d[size[i] + size[j]]
                        + (float) extr.d[-size[i] - size[j]]
                        - (float) extr.d[size[i] - size[j]]
                        - (float) extr.d[size[j] - size[i]])/4;
                   hessian[i + j*ndim] = hessian[j + i*ndim] = x;
                 }
               }
           } else {             // filemap array
             readGhost(fp, value.d, currentIndex, typesize);
             v = (float) value.d;
             for (i = 0; i < ndim; i++) {
               readGhost(fp, value.d, currentIndex + size[i], typesize);
               grad[i] = (float) value.d;
               readGhost(fp, value.d, currentIndex - size[i], typesize);
               grad[i] = (grad[i] - (float) value.d)/2;
             }
             ready = 1;                 // zero gradient?
             for (i = 0; i < ndim; i++) {
               if (grad[i] != 0) {
                 ready = 0;
                 break;
               }
             }
             if (ready)
               break;
             for (i = 0; i < ndim; i++)
               for (j = 0; j <= i; j++) {
                 if (i == j) {
                   readGhost(fp, value.d, currentIndex + size[i], typesize);
                   x = (float) value.d;
                   readGhost(fp, value.d, currentIndex - size[i], typesize);
                   hessian[i + i*ndim] = hessian2[i] =
                     x + (float) value.d - 2*v;
                 } else {
                   readGhost(fp, value.d, currentIndex + size[i] + size[j],
                             typesize);
                   x = (float) value.d;
                   readGhost(fp, value.d, currentIndex - size[i] - size[j],
                             typesize);
                   x += (float) value.d;
                   readGhost(fp, value.d, currentIndex + size[i] - size[j],
                             typesize);
                   x -= (float) value.d;
                   readGhost(fp, value.d, currentIndex - size[i] + size[j],
                             typesize);
                   x -= (float) value.d;
                   hessian[i + j*ndim] = hessian[j + i*ndim] = x/4;
                 }
               }
           }
           break;
       }
       if (ready) {
         for (i = 0; i < ndim; i++)
           grad2[i] = 0;
       } else {                         // solve  hessian.x = gradient
         f_decomp(hessian, ndim, ndim);
         ready = 1;
         for (i = ndim*ndim - 1; i >= 0; i -= ndim + 1)
           if (!hessian[i]) {   // zero determinant, indeterminate fit
             ready = 0;
             for (i = 0; i < ndim; i++)
               grad2[i] = 0.0;
             break;
           }
         if (ready) {
           f_solve(hessian, grad2, ndim, ndim);
           if (code & 4) {              // force closeness
             j = 0;
             for (i = 0; !j && i < ndim; i++)
               if (fabs(grad2[i]) > 0.75) {
                 j = 1;
                 break;
               }
             if (j)             // point too far away from integer extreme?
               // then assume quadratic is aligned with axes
               for (i = 0; i < ndim; i++)
                 grad2[i] = hessian2[i]? grad[i]/hessian2[i]: 0.0;
           }
         }
         nextValue.f = 0;
         for (i = 0; i < ndim; i++)
           nextValue.f += grad2[i] * grad[i];
         nextValue.f = v - 0.5*nextValue.f; }
     }
   }
   if (code & 1) {              // store location
     if (code & 4) {            // subgrid location
       if (internalMode & 4) {  // store relative position
         for (i = 0; i < ndim; i++)
           trgt.f[i*ntarget] = - grad2[i];
       } else {                         // store absolute position
         for (i = 0; i < ndim; i++)
           trgt.f[i*ntarget] = indx[i] - grad2[i];
       }
       trgt.f++;
     } else
       *trgt.i32++ = currentIndex;
   } else {                     // store value
     if (code & 4) {            // subgrid value
       switch (type) {
       case LUX_INT8:
         *trgt.f++ = (float) nextValue.ui8;
         break;
       case LUX_INT16:
         *trgt.f++ = (float) nextValue.i16;
         break;
       case LUX_INT32:
         *trgt.f++ = (float) nextValue.i32;
         break;
       case LUX_INT64:
         *trgt.f++ = (float) nextValue.i64;
         break;
       case LUX_FLOAT:
         *trgt.f++ = (float) nextValue.f;
         break;
       case LUX_DOUBLE:
         *trgt.f++ = (float) nextValue.d;
         break;
       }
     } else {                   // grid value
       switch (trgttype) {
       case LUX_INT8:
         *trgt.ui8++ = nextValue.ui8;
         break;
       case LUX_INT16:
         *trgt.i16++ = nextValue.i16;
         break;
       case LUX_INT32:
         *trgt.i32++ = nextValue.i32;
         break;
       case LUX_INT64:
         *trgt.i64++ = nextValue.i64;
         break;
       case LUX_FLOAT:
         *trgt.f++ = nextValue.f;
         break;
       case LUX_DOUBLE:
         *trgt.d++ = nextValue.d;
         break;
       }
     }
   }
 }
 if (code & 4) {                // sought subgrid extreme
   free(grad);
   free(grad2);
   free(hessian);
   free(hessian2);
 }
 if (filemap)
   fclose(fp);
                // prepare result
 if (max) {                     // sought maximum
   lastmaxloc = currentIndex;
   memcpy(&lastmax.ui8, &nextValue.ui8, (code & 4)? sizeof(float): typesize);
   symbol_type(lastmax_sym) = (code & 4)? LUX_FLOAT: type;
 } else {
   lastminloc = currentIndex;
   memcpy(&lastmin.ui8, &nextValue.ui8, (code & 4)? sizeof(float): typesize);
   symbol_type(lastmin_sym) = (code & 4)? LUX_FLOAT: type;
 }
 return iq;
}
//----------------------------------------------------------------------
int32_t lux_local_maxf(ArgumentCount narg, Symbol ps[])
/* find value of local maximum in <array>, starting at <position> and
   walking along the steepest gradient.
   Adjusts !LASTMAX, !LASTMAXLOC.
   syntax:  local_max(array, position)
   LS 9jun93 */
{ return local_maxormin(narg, ps, 2); }
//----------------------------------------------------------------------
int32_t lux_local_maxloc(ArgumentCount narg, Symbol ps[])
/* find position of local maximum in <array>, starting at <position> and
   walking along the steepest gradient.
   Adjusts !LASTMAX, !LASTMAXLOC.
   syntax:  local_maxloc(array, position)
   LS 9jun93 */
{ return local_maxormin(narg, ps, 3); }
//----------------------------------------------------------------------
int32_t lux_local_minf(ArgumentCount narg, Symbol ps[])
/* find value of local minimum in <array>, starting at <position> and
   walking along the steepest gradient.
   Adjusts !LASTMIN, !LASTMINLOC.
   syntax:  local_min(array, position)
   LS 9jun93 */
{ return local_maxormin(narg, ps, 0); }
//----------------------------------------------------------------------
int32_t lux_local_minloc(ArgumentCount narg, Symbol ps[])
/* find position of local minimum in <array>, starting at <position> and
   walking along the steepest gradient.
   Adjusts !LASTMIN, !LASTMINLOC.
   syntax:  local_minloc(array, position)
   LS 9jun93 */
{ return local_maxormin(narg, ps, 1); }
//----------------------------------------------------------------------
int32_t lux_zinv(ArgumentCount narg, Symbol ps[])
/* y = zinv(x)
   y = 1.0/x if x not equal to 0, y = 0 otherwise.
   LS 30aug93 */
{
 int32_t        result, iq, n;
 Symboltype topType;
 Pointer        data, target;
 double         value;

 iq = *ps;
 if (!symbolIsNumerical(iq))
   return cerror(ILL_CLASS, iq);
 topType = symbol_type(iq);
 if (topType < LUX_FLOAT)
   topType = LUX_FLOAT;
 switch (symbol_class(iq)) {
   case LUX_ARRAY: case LUX_CARRAY:
     data.i32 = (int32_t*) array_data(iq);
     if (isFreeTemp(iq)
         && lux_type_size[symbol_type(iq)] == lux_type_size[topType])
       result = iq;
     else
       result = array_clone(iq, topType);
     target.i32 = (int32_t*) array_data(result);
     n = array_size(iq);
     break;
   case LUX_SCALAR:
     data.i32 = &scalar_value(iq).i32;
     n = 1;
     result = scalar_scratch(topType);
     target.i32 = &scalar_value(result).i32;
     break;
   case LUX_CSCALAR:
     data.cf = complex_scalar_data(iq).cf;
     n = 1;
     result = scalar_scratch(topType);
     target.cf = complex_scalar_data(result).cf;
     break;
   default:
     return cerror(ILL_CLASS, *ps);
 }

 switch (symbol_type(iq)) {
   case LUX_INT8:
     while (n--) {
       *target.f++ = *data.ui8? 1.0/ *data.ui8: 0.0;
       data.ui8++;
     }
     break;
   case LUX_INT16:
     while (n--) {
       *target.f++ = *data.i16? 1.0/ *data.i16: 0.0;
       data.i16++;
     }
     break;
   case LUX_INT32:
     while (n--) {
       *target.f++ = *data.i32? 1.0/ *data.i32: 0.0;
       data.i32++;
     }
     break;
   case LUX_INT64:
     while (n--) {
       *target.f++ = *data.i64? 1.0/ *data.i64: 0.0;
       data.i64++;
     }
     break;
   case LUX_FLOAT:
     while (n--) {
       *target.f++ = *data.f? 1.0/ *data.f: 0.0;
       data.f++;
     }
     break;
   case LUX_DOUBLE:
     while (n--) {
       *target.d++ = *data.d? 1.0/ *data.d: 0.0;
       data.d++;
     }
     break;
   case LUX_CFLOAT:
     while (n--) {
       value = data.cf->real*data.cf->real
         + data.cf->imaginary*data.cf->imaginary;
       value = value? 1.0/value: 0.0;
       target.cf->real = value*data.cf->real;
       target.cf->imaginary = value*data.cf->imaginary;
       data.cf++;
       target.cf++;
     }
     break;
   case LUX_CDOUBLE:
     while (n--) {
       value = data.cd->real*data.cd->real
         + data.cd->imaginary*data.cd->imaginary;
       value = value? 1.0/value: 0.0;
       target.cd->real = value*data.cd->real;
       target.cd->imaginary = value*data.cd->imaginary;
       data.cd++;
       target.cd++;
     }
     break;
 }
 if (result == iq)
   symbol_type(result) = topType;
 return result;
}
//----------------------------------------------------------------------
int32_t lux_bsmooth(ArgumentCount narg, Symbol ps[])
// binary smooth.  Smooths an array by repeatedly applying two-element
// (binary) averaging along dimension AXIS.  The number of repeats is
// WIDTH*WIDTH/2, so the FWHM of this filter is about WIDTH.
// Syntax:  Y = BSMOOTH(X [[, AXIS], WIDTH])
// LS 10dec95
// A test on a SUN Sparc 5 reveals that BSMOOTH on a large LUX_FLOAT array
// is faster than an equivalent GSMOOTH when WIDTH < 7.  Replacing
// division by 2 with a shift operation may speed things up for integer
// data.
{
  int32_t       iq, axis, result_sym, *dims, ndim, xdims[MAX_DIMS],
        width, i, n, tally[MAX_DIMS], step[MAX_DIMS], m, done, j, k, stride;
  Symboltype outtype, type;
  Pointer       src, trgt, src0, trgt0;
  float         fwidth;

  if (narg > 2)
  { iq = ps[2];                         // WIDTH
    axis = int_arg(ps[1]); }    // AXIS
  else
  { iq = ps[1];                         // WIDTH
    axis = -1; }                // no AXIS
  if (narg > 1)
  { fwidth = float_arg(iq);
    if (fwidth <= 0)
      return cerror(NEED_POS_ARG, iq);
    width = (int32_t) (fwidth*fwidth/2);
    if (!width)
      width = 1; }
  else
    width = 4;
  iq = ps[0];                   // X
  if (iq < 0)
    return iq;                  // error pass-thru
  switch (symbol_class(iq))
  { case LUX_SCALAR:
      //trivial, just return value
      return iq;
    case LUX_ARRAY:
      outtype = type = array_type(iq);
      if (type < LUX_FLOAT)
      { outtype = LUX_FLOAT;
        iq = lux_float(1, &iq); } //float the input
      src0.i32 = (int32_t *) array_data(iq);
      m = array_size(iq);
      if (axis >= 0)            // axis specified
      { dims = array_dims(iq);
        ndim = array_num_dims(iq);
        if (axis >= ndim)
          return cerror(ILL_DIM, ps[1]); }
      else                      // mimic 1D array
      { dims = &m;
        ndim = 1;
        axis = 0; }
      break;
    default:
      return cerror(ILL_CLASS, iq); }
  memcpy(xdims, dims, sizeof(int32_t)*ndim);            // copy dims
  if (width >= (axis >= 0? xdims[axis]: m)) // just return original
    return iq;
  result_sym = array_clone(iq, outtype);
  trgt0.i32 = (int32_t *) array_data(result_sym);
  // set up for walk through array
  n = *step = lux_type_size[outtype];
  for (i = 1; i < ndim; i++)
    step[i] = (n *= dims[i - 1]);
  if (axis)                              // put requested axis first
  { n = *step;
    *step = step[axis];
    step[axis] = n;
    n = *xdims;
    *xdims = xdims[axis];
    xdims[axis] = n; }
  for (i = ndim - 1; i > 0; i--)
    step[i] -= step[i - 1]*xdims[i - 1];
  step[ndim] = 0;
  xdims[ndim] = 1;
  stride = step[0]/lux_type_size[outtype];
  for (k = 0; k < width; k++)
  { for (i = 1; i <= ndim; i++)
      tally[i] = 1;
    trgt = trgt0;
    src = src0;
    src0 = trgt0;
    done = 1;
    do
    { if (k % 2)
      { j = xdims[0] - 1;
        trgt.f += j*stride;
        src.f += j*stride;
        switch (outtype)
        { case LUX_FLOAT:
            while (j--)
            { *trgt.f = *src.f;
              src.f -= stride;
              *trgt.f += *src.f;
              *trgt.f /= 2;
              trgt.f -= stride; }
            *trgt.f = trgt.f[stride];
            trgt.f += xdims[0]*stride;
            src.f += xdims[0]*stride;
            break;
          case LUX_DOUBLE:
            while (j--)
            { *trgt.d = *src.d;
              src.d -= stride;
              *trgt.d += *src.d;
              *trgt.d /= 2;
              trgt.d -= stride; }
            *trgt.d = trgt.d[stride];
            trgt.d += xdims[0]*stride;
            src.d += xdims[0]*stride;
            break; }
      }
      else
        switch (outtype)
        { case LUX_FLOAT:
            j = xdims[0] - 1;
            while (j--)
            { *trgt.f = *src.f;
              src.f += stride;
              *trgt.f += *src.f;
              *trgt.f /= 2;
              trgt.f += stride; }
            *trgt.f = trgt.f[-stride];
            trgt.f += stride;
            src.f += stride;
            break;
          case LUX_DOUBLE:
            j = xdims[0] - 1;
            while (j--)
            { *trgt.d = *src.d;
              src.d += stride;
              *trgt.d += *src.d;
              *trgt.d /= 2;
              trgt.d += stride; }
            *trgt.d = trgt.d[-stride];
            trgt.d += stride;
            src.d += stride;
            break; }
      src.ui8 += step[1];
      trgt.ui8 += step[1];
      for (i = 1; i < ndim; i++)
      { if (tally[i]++ != xdims[i])
        { done = 0;
          break; }
        tally[i] = 1;
        done = 1;
        src.ui8 += step[i + 1]; }
    } while (!done);
  }
  return result_sym;
}
//----------------------------------------------------------------------
