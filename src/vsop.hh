/* This is file vsop.hh.

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
#include "action.hh"

struct planetIndex {
  uint16_t index;
  uint16_t nTerms;
};
struct VSOPdata {
  struct planetIndex indices[6*3*8];
  uint16_t nTerms;
  double *terms;
};
VSOPdata* planetIndicesForTolerance(VSOPdata *data, double tolerance);
extern VSOPdata VSOP87Adata;
extern VSOPdata VSOP87Cdata;

void XYZJ2000fromVSOPA(double T, int32_t object, double *pos, double tolerance);
void XYZdatefromVSOPC(double T, int32_t object, double *pos, double tolerance);

