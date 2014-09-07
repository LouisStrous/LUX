/* This is file astron.hh.

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
#define CAL_CALENDAR_BASE	(Calendar)(0)
#define CAL_CALENDAR_BITS	(Calendar)(4)
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
  CAL_LONG,                     /* 1 */
  CAL_DOUBLE,                   /* 2 */
  CAL_TEXT,                     /* 3 */
};

#define CAL_TIME_BASE	10
#define CAL_TIME_BITS	2
enum Calendar_timescale {
  CAL_UTC,                      /* 0 */
  CAL_TAI,                      /* 1 */
  CAL_TT,                       /* 2 */
  CAL_LT,                       /* 3 */
};

#define CAL_ORDER_BASE	14
#define CAL_ORDER_BITS	1
enum Calendar_order {
  CAL_YMD,                      /* 0 */
  CAL_DMY                       /* 1 */
};
