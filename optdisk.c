/* ANA Commands for Recording from pore2 on to Optical Disk */
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
 
#include "ana_structures.h"
#include <ctype.h>
#include <stdlib.h>
#if	NeXT
#include <sys/ioctl.h>
#else
#include <termio.h>    /* added termio and unistd to enable unbuffered */
#include <unistd.h>    /* input -> history buffer etc.  LS 8may92 */
#endif
 static	int	fd;	/* file number */
 static	int	open_flag=0, kilroy=0;
 extern int	*pscrat;
 extern char	*expand_name(char *, char *);
 extern struct sym_desc	sym[];
#if	NeXT
 static struct sgttyb new_params, old_params;
#else
 static struct termio new_params, old_params;
#endif
 /*------------------------------------------------------------------------- */
int ana_op1cmd_send(int narg, int ps[])
 /* send commands to optical disk # 1.  Commands are changed to upper case.
   If a command ends with a digit, a colon : is added before send-off. 
   LS 17sep92 */
 /* modified 11/14/92 ras */
 {
 int	n, n2;
 char	*buf, *p;
 
 buf = (char *) pscrat;
 if (sym[*ps].class != 2) return execute_error(70);
 if (open_flag == 0) {
 fd = open("/dev/ttyd1", O_RDWR);
 if (fd < 0 ) { perror("could not open /dev/ttyd1");   return -1; }
#if	NeXT
 /* not implemented yet on NeXT */
 if (ioctl(fd, TIOCGETP, &old_params) < 0)
   {printf("IOCTL get failed\n"); }
 else {
 new_params = old_params;
 new_params.sg_flags = CBREAK | ANYP | CRMOD; }
#else
 if (ioctl(fd, TCGETA, &old_params) < 0)
   {printf("IOCTL get failed\n"); }
 else {
 new_params = old_params;
 /* set the line to 9600 baud and non-canonical */
 new_params.c_cflag = B9600 | CS8 | CREAD | HUPCL | CLOCAL;
 new_params.c_cc[VMIN] = 0;
 new_params.c_cc[VTIME] = 20;	/* timeout in ticks, 10 ticks per second */
 new_params.c_lflag &= ICANON;
 new_params.c_lflag &= ~ECHO;
 if (ioctl(fd, TCSETA, &new_params) < 0) perror("IOCTL set failed");
 }
#endif
 open_flag = 1; 
 }
  p = (char *) sym[*ps].spec.array.ptr;
   strcpy(buf + 1, p);
   for (p = buf + 1; *p; p++) *p = toupper(*p);
   if (isdigit(*(p - 1))) { *p++ = ':'; *p = 0; }
   *buf = 2;
   buf[strlen(buf)] = 4;
   if (ioctl(fd, TCFLSH, 0)) perror("flush failed");
   if (write(fd, buf, strlen(buf)) < 0)
   { perror("could not write to optical disk");
     return -1; }
 return 1;
 /* note no checking of response done, this is done in ana_op1cmd_check
 allows user to do something else in the meantime */
 }
 /*------------------------------------------------------------------------- */
int ana_op1cmd_check(int narg, int ps[])
 {
 /* this is a function, returns either 1 or 4 which are symbols for 1 or 0 */
 int	n, n2;
 char	*buf;
 buf = (char *) pscrat;
 if (open_flag == 0) return 4;
 new_params.c_lflag &= ~ICANON;
 if (ioctl(fd, TCSETA, &new_params) < 0) perror("IOCTL set failed"); 
 n = read(fd, buf, 1);	/* first character should be an ACK */
 if (n <= 0) { perror("read back from video disk");	return 4; }
 if ( *buf == 6 ) {
 printf("got an ACK\n");
 new_params.c_lflag &= ICANON;
 if (ioctl(fd, TCSETA, &new_params) < 0) perror("IOCTL set failed"); 
 return 1; }
 printf("got something else, value = %d\n", *buf);
 /* no ack, a problem ? nak is a 21*/
 if ( *buf == 21 ) {	/* read error number if nak */
 n=read(fd, buf, 2);
 if ( n < 2 || *buf != '2')
 	{ perror("can't read error code");	return 4; }
 switch (buf[1]) {
 case '1': printf("video disk transmission error\n"); break;
 case '2': printf("video disk buffer overflow\n"); break;
 }
 }
 /* after one of these, just flush the buffer */
 new_params.c_lflag &= ICANON;
 if (ioctl(fd, TCSETA, &new_params) < 0) perror("IOCTL set failed"); 
 if (ioctl(fd, TCFLSH, 0)) perror("flush failed");
 return 4;
 }
 /*------------------------------------------------------------------------- */
int ana_op1read(int narg, int ps[])
 {
 int	n, result_sym, i, j;
 char	*buf, *p;
 
 if (open_flag == 0) return -1;
 buf = (char *) pscrat;
 n = read(fd, buf, 40);
 if (n <= 0) return -1;
 /* 9/21/93, leave out the control characters to make it easier to use
 for checking, also to make compatible with older vms code */
 /* if nothing but control characters, keep them and pass to user */
 p = buf; i = n; j = 0;
 while ( *p < 32 ) { i--; p++; if (i <= 0) break; }
 if (i>0) { while ( *(p+j) >= 32 ) { j++; i--; if (i <= 0) break; }}
 if (j>0) { n=j; buf = p; }
 result_sym = string_scratch(n);
 bcopy(buf, sym[result_sym].spec.array.ptr, n);
 p = (char *) sym[result_sym].spec.array.ptr;
 *(p+n) = 0;
 return result_sym;
 }
/*------------------------------------------------------------------------- */
int ana_op1cmd(int narg, int ps[])
 /* communication with optical disk drive # 1  */
 /*   Call:  OP1CMD,string */
 {
 if (ana_op1cmd_send( narg, ps) != 1 ) return -1;
 if (ana_op1cmd_check( narg, ps) != 1) return -1;
 return 1;
 }
/*------------------------------------------------------------------------- */ 
int ana_op1cmd_f(int narg, int ps[])
/* function form of op1cmd; returns 0 or 1 to easily test success and
to be compatible with older vms version */
 {
 if (ana_op1cmd_send( narg, ps) != 1 ) return 4;
 if (ana_op1cmd_check( narg, ps) != 1) return 4;
 return 1;
 }
/*------------------------------------------------------------------------- */
