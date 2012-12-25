#include <stdio.h>
#include <string.h>

	/* buffer limits */
#define BUFSIZE		512	/* line input buffer */
#define HISTORYSIZE	40	/* # lines in historybuffer */

#define HISTORYBUFFER	1	/* in calls to getNewLine */

	/* line editor definitions */
/* a "line" is only complete when the RETURN key is pressed.  [L]
   a "sentence" fits in a single line on the screen.  [S]
   an "exp" is a single unambiguous expression.  [E]
   a "Word" is a single Word.  [W]
   a "char" is a single character. [C] */
/* the codes are all the same as in standard Emacs except where noted */

/* general: */
#define ESC	'\x1b'	/* ESC */
#define DEL	'\x7f'	/* DEL */
#define LAR	0x144 	/* ESC [ D			(leftarrow) */
#define RAR 	0x143	/* ESC [ C			(rightarrow) */
#define DAR 	0x142	/* ESC [ B			(downarrow) */
#define UAR 	0x141	/* ESC [ A			(uparrow) */
#define INS	0x237e	/* ESC [ 2 ~			(insert) */
#define BKS	'\x08'	/* BACKSPACE */
#define TAB	'\x09'	/* TAB */
#define RET	'\x0a'	/* RET */
#define QUIT	0x40a	/* C-x RET */

/* line: */
#define BGL	'\x01'	/* start of line		(C-a) */
#define ENL	'\x05'	/* end of line			(C-e) */
#define FWL	'\x0e'	/* next line from history	(C-n) */
#define BKL	'\x10'	/* previous line from history	(C-p) */
#define DLL	'\x0b'	/* kill line forward		(C-k) */
#define BDL	0x44b	/* kill line back		(C-x k) */

/* sentence: */
#define BGS	0x261	/* start of sentence		(ESC-a) */
#define ENS	0x265	/* end of sentence		(ESC-e) */
#define DLS	0x26b	/* kill sentence forward	(ESC-k) */
#define BDS	0x47f	/* kill sentence back		(C-x del) */
#define BGSM	0xffffffe1	/* start of sentence	(M-a) */
#define ENSM	0xffffffe5	/* end of sentence	(M-e) */
#define DLSM	0xffffffeb	/* kill s forward	(M-k) */

/* expr: */
#define FWE	0x206	/* forward expr			(ESC C-f) */
#define BKE	0x202	/* back expr			(ESC C-b) */
#define DLE	0x20b	/* kill expr forward		(ESC C-k) */
/* #define BDE	0xffffffff /* kill expr back		(C-M-del) */
#define TRE	0x214	/* transpose exprs		(ESC C-t) */
#define FWEM	0xffffff86	/* forward expr		(C-M-f) */
#define BKEM	0xffffff82	/* back expr		(C-M-b) */
#define DLEM	0xffffff8b	/* kill expr forw	(C-M-k) */

/* Word: */
#define FWW	0x266	/* forward Word			(ESC-f) */
#define BKW	0x262	/* back Word			(ESC-b) */
#define DLW	0x264	/* delete Word forward		(ESC-d) */
#define BDW	0x27f	/* delete Word back		(ESC-del) */
#define TRW	0x274	/* transpose words		(ESC-t) */
#define FWWM	0xffffffe6	/* forward Word		(M-f) */
#define BKWM	0xffffffe2	/* backward Word	(M-b) */
#define DLWM	0xffffffe4	/* delete w forw	(M-d) */
#define BDWM	0xffffffff	/* delete w back	(M-del) */
#define TRWM	0xfffffff4	/* transpose words	(M-t) */

/* char: */
#define FWC	'\x06'	/* forward char			(C-f) */
#define BKC	'\x02'	/* back char			(C-b) */
#define DLC	'\x04'	/* delete char forward		(C-d) */
#define BDC	BKS	/* delete char back		(del) */
#define TRC	'\x14'	/* transpose chars		(C-t) */

/* extra: */
#define FIND	'\x73'	/* find				(C-s) */
#define JOIN	'\x67'	/* join				(C-g) */
#define SUBST	'\x72'	/* replace			(C-r) */
#define COMPLT	TAB	/* complete			(tab) */

#define PRINTHISTORYLINE	1
#define PRINTNEWLINE		2

	/* find constants & search types */
#define RIGHTMARGIN	1
#define LEFTMARGIN	2

#define NEWFIND		0
#define CONTINUEFIND	1	/* must be one higher than NEWFIND */
#define JUSTFOUND	2	/* must be one higher than CONTINUEFIND */

	/* join types */
#define FIRSTJOIN	1
#define MOREJOIN	2
