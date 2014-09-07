/* This is file sincos.cc.

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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE             /* for sincos */
#endif

#include <math.h>               /* for sincos */
#include "action.hh"             /* for BIND */

#if !HAVE_SINCOS
void sincos(double x, double *sinx, double *cosx)
{
  if (sinx)
    *sinx = sin(x);
  if (cosx)
    *cosx = cos(x);
}
#endif
BIND(sincos, v_ddpdp_iDaoDqDq_012, s, SINCOS, 3, 3, NULL);
