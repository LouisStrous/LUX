#define CAL_CALENDAR_BASE	(0)
#define CAL_CALENDAR_BITS	(4)
enum Calendar { CAL_DEFAULT,             /* 0 */
                CAL_COMMON,              /* 1 */
                CAL_GREGORIAN,           /* 2 */
                CAL_ISLAMIC,             /* 3 */
                CAL_JULIAN,              /* 4 */
                CAL_HEBREW,              /* 5 */
                CAL_EGYPTIAN,            /* 6 */
                CAL_JD,                  /* 7 */
                CAL_CJD,                 /* 8 */
                CAL_LUNAR,               /* 9 */
                CAL_MAYAN,               /* 10 */
                CAL_LONGCOUNT,           /* 11 */
                CAL_LATIN };             /* 12 */

#define CAL_OUTPUT_BASE	8
#define CAL_OUTPUT_BITS	2
enum Calendar_outputtype {
  CAL_NUMERIC,                  /* 0 */
  CAL_TEXT,                     /* 1 */
  CAL_ISOTEXT                   /* 2 */
};

#define CAL_TIME_BASE	10
#define CAL_TIME_BITS	2
enum Calendar_timescale {
  CAL_UTC,                      /* 0 */
  CAL_TAI,                      /* 1 */
  CAL_TT                        /* 2 */
};

#define CAL_ORDER_BASE	14
#define CAL_ORDER_BITS	1
enum Calendar_order {
  CAL_YMD,                      /* 0 */
  CAL_DMY                       /* 1 */
};
