/* File tape.c */
/* ANA routines for tape I/O. */
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
#include "action.h"
/* ioctl() is generally defined in sys/ioctl.h or in unistd.h */
#include <sys/ioctl.h>
#include <unistd.h>
static char rcsid[] __attribute__ ((unused)) =
 "$Id: tape.c,v 4.0 2001/02/07 20:37:05 strous Exp $";
						/* tape things */
struct  mtop    rew = {MTREW, 1 };
struct  mtop    unl = {MTOFFL, 1 };
struct  mtop    mtweof = {MTWEOF, 1 };
struct  mtop    tape_op = {MTFSR, 1 };
struct  mtget   ti;
int	tape_lun, io_status;
extern int	byte_count;	/* defined in files.c */
int	tape_messages=1;
static	int	tape_fd[MAXTAPE], neof[MAXTAPE], neot[MAXTAPE];
/*------------------------------------------------------------------------- */
int ana_tape_status(int narg, int ps[])/* print tape status */
{
  int   fd, j;

  printf("Tape status:\n");
  for (j = 0; j <= MAXTAPE; j++) {
    if ((fd = tape_fd[j]) > 0) {
      printf("\ndrive # %d  ",j);
      if (ioctl(fd, MTIOCGET, &ti))
	perror("tape_status");
    }
  }
  printf("\n");
  return 1;
}
/*------------------------------------------------------------------------- */
int tape_setup(int narg, int ps[])		/* for internal use */
{
  char	*name;

  name = strsave("$ANADRIVE0");
  errno = 0;
  tape_lun = int_arg( ps[0] );		/* get tape logical unit number */
  if (tape_lun < 0 || tape_lun > MAXTAPE) {
    printf("illegal tape unit specified\n");
    return ANA_ERROR;
  }
  if (tape_fd[tape_lun] == 0) {	/* new ? */
    neof[tape_lun] = neot[tape_lun] = 0;
    /* determine tape drive name.  Modified 10/5/92 LS */
    sprintf(&name[9], "%1i", tape_lun);
    if (*expand_name(name, NULL) == '$')  /* no translation */
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
    return ANA_ERROR;
  }

  return tape_fd[tape_lun];
}
/*------------------------------------------------------------------------- */
int check_tape_io(int iq)
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
  return ANA_OK;
}
/*------------------------------------------------------------------------- */
int ana_rewind(int narg, int ps[])/* rewind a tape drive */
{
  int	fd;

  if ((fd = tape_setup(narg,ps)) < 0)
    return ANA_ERROR;
  return check_tape_io(ioctl(fd, MTIOCTOP, &rew));
}
/*------------------------------------------------------------------------- */
int ana_weof(narg,ps)				/* write an eof on tape drive */
int	narg, ps[];
{
int	fd;
if ( (fd = tape_setup(narg,ps)) < 0) return -1;;
return check_tape_io( ioctl(fd, MTIOCTOP, &mtweof) );
}
/*------------------------------------------------------------------------- */
int ana_unload(int narg, int ps[])/* unload a tape drive */
{
  int	fd;

  if ((fd = tape_setup(narg,ps)) < 0)
    return ANA_ERROR;
  if (ioctl(fd, MTIOCTOP, &unl))
    perror("tape unload");
  if (close(fd))
    perror("tape close");
  tape_fd[tape_lun] = 0;
  return 1;
}
/*------------------------------------------------------------------------- */
int ana_skipr(narg,ps)				/* skip records */
int	narg, ps[];
{
int	fd, nr;
if ( (fd = tape_setup(narg,ps)) < 0) return -1;;
if (narg > 1 ) nr = int_arg( ps[1] ); else nr = 1;
if (nr == 0)  return 1;
if (nr > 0) {
tape_op.mt_op = MTFSR;	tape_op.mt_count = nr;
} else {
tape_op.mt_op = MTBSR;	tape_op.mt_count = -nr;
}
return check_tape_io( ioctl(fd, MTIOCTOP, &tape_op) );
}
/*------------------------------------------------------------------------- */
int ana_skipf(narg,ps)				/* skip records */
int	narg, ps[];
{
int	fd, nf;
if ( (fd = tape_setup(narg,ps)) < 0) return -1;;
if (narg > 1 ) nf = int_arg( ps[1] ); else nf = 1;
if (nf == 0)  return 1;
if (nf > 0) {
tape_op.mt_op = MTFSF;	tape_op.mt_count = nf;
} else {
tape_op.mt_op = MTBSF;	tape_op.mt_count = -nf;
}
return check_tape_io( ioctl(fd, MTIOCTOP, &tape_op) );
}
/*------------------------------------------------------------------------- */
int ana_taprd(int narg, int ps[])
/* read tape record.  Modified LS 27mar98 to read records until the */
/* requested number of bytes is read (or an error occurs) */
{
  int	fd, nbr, iq, nd, n, type, nread, cur;
  pointer q1;
  
  if ( (fd = tape_setup(narg,ps)) < 0)
    return ANA_ERROR;
				/* get the size of the input array to load */
  iq = ps[1];
  if (symbol_class(iq) != ANA_ARRAY)
    return cerror(NEED_ARR, iq);

  q1.l = (int *) array_data(iq);
  nd = array_num_dims(iq);
  type = array_type(iq);
  n = cur = ana_type_size[type]*array_size(iq);
  nread = 0;
  do {
    nbr = read(fd, q1.b, cur);
    if (nbr < 0)		/* some error */
      break;
    q1.b += nbr;
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
  endian(q1.b, n, type);
#endif
  io_status = errno;
  if (errno > 0)
    perror("taprd");
  return 1;
}
 /*------------------------------------------------------------------------- */
int ana_tapwrt(narg,ps)				/* read tape record */
 int	narg, ps[];
 {
 int	fd, nbr, iq, j, nd, n, type;
 array	*h;
 pointer q1;
 if ( (fd = tape_setup(narg,ps)) < 0) return -1;
				 /* get the size of the input array to load */
 iq = ps[1];
 CK_ARR(iq, 1);
 h = (array *) sym[iq].spec.array.ptr;
 q1.l = (int *) ((char *)h + sizeof(array));
 nd = h->ndim;
 type = sym[iq].type;
 n = ana_type_size[type]; errno = 0;
 for(j=0;j<nd;j++) n *= h->dims[j];
#if !WORDS_BIGENDIAN
 endian(q1.b, n, type);
#endif
 nbr = write(fd, q1.b, n);
#if !WORDS_BIGENDIAN
 endian(q1.b, n, type);				/* restore original */
#endif
 byte_count = nbr;				/* want this for write */
 io_status = errno;
 if (errno) perror("tapwrt");
 if (nbr != n) { printf("tape write problem\n");  return -1; }
 return 1;
 }
 /*------------------------------------------------------------------------- */
int ana_tapebufout(int narg, int ps[])			/* write tape record */
 /*the call is TAPEBUFOUT,tape#,array,[recsize,offset,len]
 the defaults for offset and len are just the whole array (or what is left of
 it if just offset is specified) and the default for recsize is 16384 bytes
 intended for dumping large amounts of data */
{
 int	fd, nbr, iq, j, nd, n, recsize, nb, ic, offset, len, type;
 array	*h;
 pointer q1;
#if !WORDS_BIGENDIAN
 char	*p;
#endif

 if ( (fd = tape_setup(narg,ps)) < 0) return -1;;
				 /* get the size of the output array */
 iq = ps[1];
 CK_ARR(iq, 1);
 h = (array *) sym[iq].spec.array.ptr;
 q1.l = (int *) ((char *)h + sizeof(array));
 nd = h->ndim;
 type = sym[iq].type;
 nb = n = ana_type_size[sym[iq].type];
 errno = 0;
 for(j=0;j<nd;j++) n *= h->dims[j];
 recsize = 16384;  offset = 0;
 if (narg >2) recsize = int_arg(ps[2]);
 /* may want a different offset and/or length */
 if (narg >3) offset = int_arg(ps[3]);
 offset *= nb;		/* offset in bytes */
 if (offset > n) {printf("offset exceeds array size, %d %d\n",offset, n);
	 return -1; }
 if (narg >4) { len = int_arg(ps[4]);  len = len * nb;
	 n = MIN(len, n - offset); }
 if (n < 14) {
   printf("data size too small for a tape read/write, use >= 14, %d\n", n);
   return -1; }
#if !WORDS_BIGENDIAN
 p = (char *) q1.b;
 endian((byte *) p, n, type);
#endif
 q1.b += offset;	/* starting address for writing */
 ic = n;
 /*printf("in tapebufout, fd = %d, n = %d, recordsize = %d\n",fd,n,recsize);*/
 while (ic > 0) {
 recsize = MIN(recsize, ic);
 nbr = write (fd, q1.b, recsize);
 byte_count = nbr;				/* want this for write */
 io_status = errno;
 /*printf("number written = %d\n",nbr);*/
 if (errno) perror("tapbufout");
 if (nbr <= 0 ) {					/*problem */
	 if (nbr == 0) { printf("returned 0 while writing array\n");  }
	 else { perror("tapebufout"); } return 0; }
 ic -= nbr;	q1.b += nbr;
 }
#if !WORDS_BIGENDIAN
 endian((byte *) p, n, type);				/* restore original */
#endif
 return 1;
 }
 /*------------------------------------------------------------------------- */
int ana_tapebufin(int narg, int ps[])/* read tape record */
 /*the call is TAPEBUFIN,tape#,array,[recsize,offset,len]
 the defaults for offset and len are just the whole array (or what is left of
 it if just offset is specified) and the default for recsize is 32768 bytes
 intended for reading large amounts of data, I/O is asynchronous to allow other
 activities in the meantime, such as reading from another tape or otherwise
 acquiring or processing data
 */
{
 int	fd, nbr, iq, nd, n, recsize, nb, ic, offset, len, type;
 pointer q1;
#if !WORDS_BIGENDIAN
 char	*p;
#endif

 if ((fd = tape_setup(narg,ps)) < 0)
   return ANA_ERROR;
				 /* get the size of the input array to load */
 iq = ps[1];
 if (symbol_class(iq) != ANA_ARRAY)
   return cerror(NEED_ARR, iq);
 q1.l = (int *) array_data(iq);
 nd = array_num_dims(iq);
 type = array_type(iq);
 nb = n = ana_type_size[type];
 errno = 0;
 n *= array_size(iq);
 recsize = 32768;
 offset = 0;
 if (narg > 2)
   recsize = int_arg(ps[2]);
 /* may want a different offset and/or length */
 if (narg > 3)
   offset = int_arg(ps[3]);
 offset *= nb;		/* offset in bytes */
 if (offset > n) {
   printf("offset exceeds array size, %d %d\n", offset, n);
   return ANA_ERROR;
 }
 if (narg > 4) {
   len = int_arg(ps[4]);
   len = len * nb;
   n = MIN(len, n - offset);
 }
 if (n < 14) {
   printf("data size too small for a tape read/write, use >= 14, %d\n", n);
   return ANA_ERROR;
 }
#if !WORDS_BIGENDIAN
 p = (char *) q1.b;
 if (offset != 0)		/* need proper imbedding */
   endian((byte *) p, n, type);
#endif
 q1.b += offset;
 ic = n;
 /*printf("in tapebuferin, fd = %d, n = %d, recordsize = %d\n",fd,n,recsize);*/
 while (ic > 0) {
   recsize = MIN(recsize, ic);
   nbr = read (fd, q1.b, recsize);
 /*printf("number read in = %d\n",nbr);*/
 /* the number read will be -1 for errors but a record larger than recsize
 is also tagged as an error, this often happens on the last record */
 /*printf("errno = %d\n",errno);*/
   if (errno) {
     printf("(error %d) ",errno);
     perror("tapebufin");
   }
   if (nbr == 0) {
     printf("EOF while reading array\n");
     perror("tapebufin");
   }
   if (nbr < 0 ) {
     if (errno == 12) {  /* record larger than requested recsize, */
			 /* OK if last one */
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
   q1.b += nbr;
 }
#if !WORDS_BIGENDIAN
 endian((byte *) p, n, type);
#endif
 return 1;
 }
 /*------------------------------------------------------------------------- */
int ana_wait_for_tape(narg,ps)				/* read tape record */
int	narg, ps[];
{
/* a dummy version, we'll need this when we do asynchronous tape i/o */
return 1;
}
/*------------------------------------------------------------------------- */
