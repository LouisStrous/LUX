/* This is file tape.cc.

Copyright 2013 Louis Strous, Richard Shine
Copyright 2014 Louis Strous

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
// File tape.c
// LUX routines for tape I/O.
#if HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mtio.h>
#include <unistd.h>
#include <fcntl.h>
#include "action.hh"
// ioctl() is generally defined in sys/ioctl.h or in unistd.h
#include <sys/ioctl.h>
#include <unistd.h>
                                                // tape things
struct  mtop    rew = {MTREW, 1 };
struct  mtop    unl = {MTOFFL, 1 };
struct  mtop    mtweof = {MTWEOF, 1 };
struct  mtop    tape_op = {MTFSR, 1 };
struct  mtget   ti;
int32_t        tape_lun, io_status;
extern int32_t        byte_count;        // defined in files.c
int32_t        tape_messages=1;
static        int32_t        tape_fd[MAXTAPE], neof[MAXTAPE], neot[MAXTAPE];
//-------------------------------------------------------------------------
int32_t lux_tape_status(ArgumentCount narg, Symbol ps[])// print tape status
{
  int32_t   fd, j;

  printf("Tape status:\n");
  for (j = 0; j < MAXTAPE; j++) {
    if ((fd = tape_fd[j]) > 0) {
      printf("\ndrive # %d  ",j);
      if (ioctl(fd, MTIOCGET, &ti))
        perror("tape_status");
    }
  }
  printf("\n");
  return 1;
}
//-------------------------------------------------------------------------
int32_t tape_setup(ArgumentCount narg, Symbol ps[])                // for internal use
{
  char        *name;

  name = strsave("$ANADRIVE0");
  errno = 0;
  tape_lun = int_arg( ps[0] );                // get tape logical unit number
  if (tape_lun < 0 || tape_lun > MAXTAPE) {
    printf("illegal tape unit specified\n");
    return LUX_ERROR;
  }
  if (tape_fd[tape_lun] == 0) {        // new ?
    neof[tape_lun] = neot[tape_lun] = 0;
    // determine tape drive name.  Modified 10/5/92 LS
    sprintf(&name[9], "%1i", tape_lun);
    if (*expand_name(name, NULL) == '$')  // no translation
      printf("Environment variable %s for tape drive %1d is not defined -- illegal tape drive?",
             name, tape_lun);
    else {
      printf("tape drive: %s\n", expname);
      if ((tape_fd[tape_lun] = open(expname, O_RDWR, 0)) < 0) {
        errno = 0;
        if ((tape_fd[tape_lun] = open(expname, O_RDONLY, 0)) >= 0) {
          printf("Could open tape unit %d (%s) for reading only\n", tape_lun,
                 expname);
          puts("The tape may be write-protected.");
        } else {
          perror("tape_setup:");
          printf("Could not open tape unit %d (%s) for reading\n", tape_lun,
               expname);
        }
      }
    }
  }
  if (tape_fd[tape_lun] < 0) {
    perror("tape_setup error");
    return LUX_ERROR;
  }

  return tape_fd[tape_lun];
}
//-------------------------------------------------------------------------
int32_t check_tape_io(int32_t iq)
{
  io_status = errno;
  if (iq)
    printf("return status of ioctl = %d\n", iq);
  if (errno)
    perror("check_tape_io error");
  if (errno == 0)
    io_status = 1;
  else
    io_status = errno;
  return LUX_OK;
}
//-------------------------------------------------------------------------
int32_t lux_rewind(ArgumentCount narg, Symbol ps[])// rewind a tape drive
{
  int32_t        fd;

  if ((fd = tape_setup(narg,ps)) < 0)
    return LUX_ERROR;
  return check_tape_io(ioctl(fd, MTIOCTOP, &rew));
}
//-------------------------------------------------------------------------
int32_t lux_weof(ArgumentCount narg, Symbol ps[]) // write an eof on tape drive
{
int32_t        fd;
if ( (fd = tape_setup(narg,ps)) < 0) return -1;;
return check_tape_io( ioctl(fd, MTIOCTOP, &mtweof) );
}
//-------------------------------------------------------------------------
int32_t lux_unload(ArgumentCount narg, Symbol ps[])// unload a tape drive
{
  int32_t        fd;

  if ((fd = tape_setup(narg,ps)) < 0)
    return LUX_ERROR;
  if (ioctl(fd, MTIOCTOP, &unl))
    perror("tape unload");
  if (close(fd))
    perror("tape close");
  tape_fd[tape_lun] = 0;
  return 1;
}
//-------------------------------------------------------------------------
int32_t lux_skipr(ArgumentCount narg, Symbol ps[]) // skip records
{
int32_t        fd, nr;
if ( (fd = tape_setup(narg,ps)) < 0) return -1;;
if (narg > 1 ) nr = int_arg( ps[1] ); else nr = 1;
if (nr == 0)  return 1;
if (nr > 0) {
tape_op.mt_op = MTFSR;        tape_op.mt_count = nr;
} else {
tape_op.mt_op = MTBSR;        tape_op.mt_count = -nr;
}
return check_tape_io( ioctl(fd, MTIOCTOP, &tape_op) );
}
//-------------------------------------------------------------------------
int32_t lux_skipf(ArgumentCount narg, Symbol ps[]) // skip records
{
int32_t        fd, nf;
if ( (fd = tape_setup(narg,ps)) < 0) return -1;;
if (narg > 1 ) nf = int_arg( ps[1] ); else nf = 1;
if (nf == 0)  return 1;
if (nf > 0) {
tape_op.mt_op = MTFSF;        tape_op.mt_count = nf;
} else {
tape_op.mt_op = MTBSF;        tape_op.mt_count = -nf;
}
return check_tape_io( ioctl(fd, MTIOCTOP, &tape_op) );
}
//-------------------------------------------------------------------------
int32_t lux_taprd(ArgumentCount narg, Symbol ps[])
// read tape record.  Modified LS 27mar98 to read records until the
// requested number of bytes is read (or an error occurs)
{
  int32_t        fd, nbr, iq, n, type, nread, cur;
  Pointer q1;

  if ( (fd = tape_setup(narg,ps)) < 0)
    return LUX_ERROR;
                                // get the size of the input array to load
  iq = ps[1];
  if (symbol_class(iq) != LUX_ARRAY)
    return cerror(NEED_ARR, iq);

  q1.i32 = (int32_t *) array_data(iq);
  type = array_type(iq);
  n = cur = lux_type_size[type]*array_size(iq);
  nread = 0;
  do {
    nbr = read(fd, q1.ui8, cur);
    if (nbr < 0)                // some error
      break;
    q1.ui8 += nbr;
    nread += nbr;
    cur -= nbr;
  } while (cur > 0);
  byte_count = nread;
  if (nread != n) {
    perror("TAPRD:");
    puts("TAPRD - Could not satisfy request\n");
    printf("Requested bytes: %1d; Read: %1d\n", n, nread);
  }
#if !WORDS_BIGENDIAN
  endian(q1.ui8, n, type);
#endif
  io_status = errno;
  if (errno > 0)
    perror("taprd");
  return 1;
}
 //-------------------------------------------------------------------------
int32_t lux_tapwrt(ArgumentCount narg, Symbol ps[]) // read tape record
 {
 int32_t        fd, nbr, iq, j, nd, n, type;
 Array *h;
 Pointer q1;
 if ( (fd = tape_setup(narg,ps)) < 0) return -1;
                                 // get the size of the input array to load
 iq = ps[1];
 CK_ARR(iq, 1);
 h = (Array *) sym[iq].spec.array.ptr;
 q1.i32 = (int32_t *) ((char *)h + sizeof(Array));
 nd = h->ndim;
 type = sym[iq].type;
 n = lux_type_size[type]; errno = 0;
 for(j=0;j<nd;j++) n *= h->dims[j];
#if !WORDS_BIGENDIAN
 endian(q1.ui8, n, type);
#endif
 nbr = write(fd, q1.ui8, n);
#if !WORDS_BIGENDIAN
 endian(q1.ui8, n, type);                                // restore original
#endif
 byte_count = nbr;                                // want this for write
 io_status = errno;
 if (errno) perror("tapwrt");
 if (nbr != n) { printf("tape write problem\n");  return -1; }
 return 1;
 }
 //-------------------------------------------------------------------------
int32_t lux_tapebufout(ArgumentCount narg, Symbol ps[])                        // write tape record
 /*the call is TAPEBUFOUT,tape#,array,[recsize,offset,len]
 the defaults for offset and len are just the whole array (or what is left of
 it if just offset is specified) and the default for recsize is 16384 bytes
 intended for dumping large amounts of data */
{
 int32_t        fd, nbr, iq, j, nd, n, recsize, nb, ic, offset, len, type;
 Array *h;
 Pointer q1;
#if !WORDS_BIGENDIAN
 char        *p;
#endif

 if ( (fd = tape_setup(narg,ps)) < 0) return -1;;
                                 // get the size of the output array
 iq = ps[1];
 CK_ARR(iq, 1);
 h = (Array *) sym[iq].spec.array.ptr;
 q1.i32 = (int32_t *) ((char *)h + sizeof(Array));
 nd = h->ndim;
 type = sym[iq].type;
 nb = n = lux_type_size[sym[iq].type];
 errno = 0;
 for(j=0;j<nd;j++) n *= h->dims[j];
 recsize = 16384;  offset = 0;
 if (narg >2) recsize = int_arg(ps[2]);
 // may want a different offset and/or length
 if (narg >3) offset = int_arg(ps[3]);
 offset *= nb;                // offset in bytes
 if (offset > n) {printf("offset exceeds array size, %d %d\n",offset, n);
         return -1; }
 if (narg >4) { len = int_arg(ps[4]);  len = len * nb;
         n = MIN(len, n - offset); }
 if (n < 14) {
   printf("data size too small for a tape read/write, use >= 14, %d\n", n);
   return -1; }
#if !WORDS_BIGENDIAN
 p = (char *) q1.ui8;
 endian((uint8_t *) p, n, type);
#endif
 q1.ui8 += offset;        // starting address for writing
 ic = n;
 //printf("in tapebufout, fd = %d, n = %d, recordsize = %d\n",fd,n,recsize);
 while (ic > 0) {
 recsize = MIN(recsize, ic);
 nbr = write (fd, q1.ui8, recsize);
 byte_count = nbr;                                // want this for write
 io_status = errno;
 //printf("number written = %d\n",nbr);
 if (errno) perror("tapbufout");
 if (nbr <= 0 ) {                                        //problem
         if (nbr == 0) { printf("returned 0 while writing array\n");  }
         else { perror("tapebufout"); } return 0; }
 ic -= nbr;        q1.ui8 += nbr;
 }
#if !WORDS_BIGENDIAN
 endian((uint8_t *) p, n, type);                                // restore original
#endif
 return 1;
 }
 //-------------------------------------------------------------------------
int32_t lux_tapebufin(ArgumentCount narg, Symbol ps[])// read tape record
 /*the call is TAPEBUFIN,tape#,array,[recsize,offset,len]
 the defaults for offset and len are just the whole array (or what is left of
 it if just offset is specified) and the default for recsize is 32768 bytes
 intended for reading large amounts of data, I/O is asynchronous to allow other
 activities in the meantime, such as reading from another tape or otherwise
 acquiring or processing data
 */
{
 int32_t        fd, nbr, iq, n, recsize, nb, ic, offset, len, type;
 Pointer q1;
#if !WORDS_BIGENDIAN
 char        *p;
#endif

 if ((fd = tape_setup(narg,ps)) < 0)
   return LUX_ERROR;
                                 // get the size of the input array to load
 iq = ps[1];
 if (symbol_class(iq) != LUX_ARRAY)
   return cerror(NEED_ARR, iq);
 q1.i32 = (int32_t *) array_data(iq);
 type = array_type(iq);
 nb = n = lux_type_size[type];
 errno = 0;
 n *= array_size(iq);
 recsize = 32768;
 offset = 0;
 if (narg > 2)
   recsize = int_arg(ps[2]);
 // may want a different offset and/or length
 if (narg > 3)
   offset = int_arg(ps[3]);
 offset *= nb;                // offset in bytes
 if (offset > n) {
   printf("offset exceeds array size, %d %d\n", offset, n);
   return LUX_ERROR;
 }
 if (narg > 4) {
   len = int_arg(ps[4]);
   len = len * nb;
   n = MIN(len, n - offset);
 }
 if (n < 14) {
   printf("data size too small for a tape read/write, use >= 14, %d\n", n);
   return LUX_ERROR;
 }
#if !WORDS_BIGENDIAN
 p = (char *) q1.ui8;
 if (offset != 0)                // need proper imbedding
   endian((uint8_t *) p, n, type);
#endif
 q1.ui8 += offset;
 ic = n;
 //printf("in tapebuferin, fd = %d, n = %d, recordsize = %d\n",fd,n,recsize);
 while (ic > 0) {
   recsize = MIN(recsize, ic);
   nbr = read (fd, q1.ui8, recsize);
 //printf("number read in = %d\n",nbr);
 /* the number read will be -1 for errors but a record larger than recsize
 is also tagged as an error, this often happens on the last record */
 //printf("errno = %d\n",errno);
   if (errno) {
     printf("(error %d) ",errno);
     perror("tapebufin");
   }
   if (nbr == 0) {
     printf("EOF while reading array\n");
     perror("tapebufin");
   }
   if (nbr < 0 ) {
     if (errno == 12) {  // record larger than requested recsize,
                         // OK if last one
       if (ic == recsize)
         nbr = recsize;
       else {
         printf("record sizes seem to be larger than the specified max size\n");
         printf("try again with a larger record size parameter\n");
         return 0;
       }
     } else {
       printf("(%d bytes read) ", nbr);
       perror("tapebufin");
       return 0;
     }
   }
   ic -= nbr;
   q1.ui8 += nbr;
 }
#if !WORDS_BIGENDIAN
 endian((uint8_t *) p, n, type);
#endif
 return 1;
 }
 //-------------------------------------------------------------------------
int32_t lux_wait_for_tape(ArgumentCount narg, Symbol ps[]) // read tape record
{
// a dummy version, we'll need this when we do asynchronous tape i/o
return 1;
}
//-------------------------------------------------------------------------
