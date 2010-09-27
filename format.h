/* header file for ANA string formatting */

/* format_check store choices */
#define STORE	1
#define NOSTORE	0

/* format_check return value macros */
#define	isIntegerFormat(A)	((A >= 0 && A < 6)? 1: 0)
#define isFloatFormat(A)	((A > 5 && A < 14)? 1: 0)
#define isStringFormat(A)	((A == 14)? 1: 0)

