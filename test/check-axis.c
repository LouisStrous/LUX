/* This is file check-axis.c.

Copyright 2013 Louis Strous

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
#include <check.h>
#include <stdlib.h>		/* for EXIT_SUCCESS, EXIT_FAILURE */

#include "config.h"
#include "luxparser.h"
#include "action.h"
#include "axis.h"

/*
  The first series of tests involve an array of 3 by 4 by 2 elements,
  with the value of each element equal to its distance from the
  beginning of the (linearly stored) array.

  0  1  2   12 13 14
  3  4  5   15 16 17
  6  7  8   18 19 20
  9 10 11   21 22 23
*/

START_TEST(axis_012)
{
  int dims[] = { 3, 4, 2 };
  Int iq = array_scratch(LUX_LONG, 3, dims); /* 3 by 4 by 2 array */
  iq = lux_indgen(1, &iq);      /* each element equal to its index */
  ck_assert_msg(iq > 0, "Cannot create array");
  loopInfo srci;
  pointer srcp;
  ck_assert_int_eq(standardLoop(iq, 0, SL_ALLAXES, LUX_LONG, &srci, &srcp, NULL, NULL, NULL), LUX_OK);
  ck_assert_int_eq(srci.ndim, sizeof(dims)/sizeof(*dims)); /* # dimensions */
  ck_assert_int_eq(srci.nelem, dims[0]*dims[1]*dims[2]); /* # elements */
  int i;
  for (i = 0; i < srci.ndim; i++)
    ck_assert_int_eq(srci.coords[i],    0); /* begin coordinates are all 0 */
  /* specified dimensions */
  ck_assert_int_eq(srci.dims[0],        dims[0]);
  ck_assert_int_eq(srci.dims[1],        dims[1]);
  ck_assert_int_eq(srci.dims[2],        dims[2]);
  ck_assert_int_eq(srci.singlestep[0],  1); /* one step along dimension 0 */
  ck_assert_int_eq(srci.singlestep[1],  dims[0]); /* along dimension 1 */
  ck_assert_int_eq(srci.singlestep[2],  dims[0]*dims[1]); /* along dim 2 */
  /* specified axes */
  ck_assert_int_eq(srci.naxes,          3); /* SL_ALLAXES → 3 axes */
  ck_assert_int_eq(srci.axes[0],        0); /* first loop along axis 0 */
  ck_assert_int_eq(srci.axes[1],        1); /* then along axis 1 */
  ck_assert_int_eq(srci.axes[2],        2); /* then along axis 2 */
  /* compressed/rearranged dimensions; all axes were selected in their
     natural order, so in this case the compressed/rearranged
     dimensions are equal to the original dimensions */
  ck_assert_int_eq(srci.rndim,          3);
  ck_assert_int_eq(srci.rdims[0],       dims[0]);
  ck_assert_int_eq(srci.rdims[1],       dims[1]);
  ck_assert_int_eq(srci.rdims[2],       dims[2]);
  ck_assert_int_eq(srci.rsinglestep[0], 1);
  ck_assert_int_eq(srci.rsinglestep[1], dims[0]);
  ck_assert_int_eq(srci.rsinglestep[2], dims[0]*dims[1]);
  /* how many elements to advance the pointer to go to the next item
     in the desired order? */
  ck_assert_int_eq(srci.step[0],        1);
  /* how many elements to advance the pointer additionally to go to
     the next item in the desired order when reaching the end of the
     first selected axis? */
  ck_assert_int_eq(srci.step[1],        0);
  /* how many elements to advance the pointer additionally to go to
     the next item in the desired order when reaching the end of the
     second selected axis? */
  ck_assert_int_eq(srci.step[2],        0);

  ck_assert_int_eq(srci.raxes[0],       0);
  ck_assert_int_eq(srci.raxes[1],       1);
  ck_assert_int_eq(srci.raxes[2],       2);

  ck_assert_int_eq(srci.iraxes[0],      0);
  ck_assert_int_eq(srci.iraxes[1],      1);
  ck_assert_int_eq(srci.iraxes[2],      2);

  ck_assert_int_eq(srci.axisindex,      0);
  ck_assert_int_eq(srci.advanceaxis,    0);
  int count = 0;
  /* advanceLoop returns 0 while inside the first specified axis,
     1 when at the end of the first specified axis (= 0),
     2 when at the end of the second specified axis (= 1),
     3 when at the end of the third specified axis (= 2). */
  /* axes 0,1,2 → dimension sizes 3,4,2 */
  int expect_a[] = {0,0,1,0,0,1,0,0,1,0,0,2,
                    0,0,1,0,0,1,0,0,1,0,0,3};
  int expect_i[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,
                    12,13,14,15,16,17,18,19,20,21,22,23};
  int size = array_size(iq);
  int ok;
  do {
    fail_unless(count < size, "Out of bounds"); /* stay within bounds */
    fail_unless(*srcp.l == expect_i[count],
                "Element bad; expect %d, got %d, count = %d",
                expect_i[count], *srcp.l, count);
    int c0 = count % srci.rdims[0];
    int c1 = (count/srci.rdims[0]) % srci.rdims[1];
    int c2 = count/(srci.rdims[0]*srci.rdims[1]); 
    fail_unless(srci.coords[0] == c0,
                "Coordinate 0 bad; expect %d, got %d, count = %d",
                c0, srci.coords[0], count);
    fail_unless(srci.coords[1] == c1,
                "Coordinate 1 bad; expect %d, got %d, count = %d",
                c1, srci.coords[1], count);
    fail_unless(srci.coords[2] == c2,
                "Coordinate 2 bad; expect %d, got %d, count = %d",
                c2, srci.coords[2], count);
    ok = advanceLoop(&srci, &srcp);
    fail_unless(ok == expect_a[count],
                "advanceLoop return bad; expect %d, got %d, count = %d",
                expect_a[count], ok, count);
    ++count;
  } while (ok < srci.rndim);
  ck_assert_int_eq(count, size); /* we did all elements */
}
END_TEST

START_TEST(axis_102)
{
  int dims[] = { 3, 4, 2 };
  Int iq = array_scratch(LUX_LONG, 3, dims);
  iq = lux_indgen(1, &iq);
  ck_assert_msg(iq > 0, "Cannot create array");
  int one =  1;
  Int axes = array_scratch(LUX_LONG, 1, &one); /* axis index 1 */
  pointer axesp;
  axesp.v = array_data(axes);
  *axesp.l = 1;
  loopInfo srci;
  pointer srcp;
  ck_assert_int_eq(standardLoop(iq, axes, 0, LUX_LONG, &srci, &srcp, NULL, NULL, NULL), LUX_OK);
  ck_assert_int_eq(srci.ndim, sizeof(dims)/sizeof(*dims));
  ck_assert_int_eq(srci.nelem, dims[0]*dims[1]*dims[2]);
  int i;
  for (i = 0; i < srci.ndim; i++)
    ck_assert_int_eq(srci.coords[i],    0);
  /* specified dimensions */
  ck_assert_int_eq(srci.dims[0],        dims[0]);
  ck_assert_int_eq(srci.dims[1],        dims[1]);
  ck_assert_int_eq(srci.dims[2],        dims[2]);
  ck_assert_int_eq(srci.singlestep[0],  1); /* one step along dimension 0 */
  ck_assert_int_eq(srci.singlestep[1],  dims[0]); /* along dimension 1 */
  ck_assert_int_eq(srci.singlestep[2],  dims[0]*dims[1]); /* along dim 2 */
  /* specified axes */
  ck_assert_int_eq(srci.naxes,          1);
  ck_assert_int_eq(srci.axes[0],        1); /* loop along axis 1 */
  /* compressed/rearranged dimensions; we loop along dimension 1
     first, and then along dimensions 0 and 2. */
  ck_assert_int_eq(srci.rndim,          3);
  ck_assert_int_eq(srci.rdims[0],       dims[1]); /* 4 */
  ck_assert_int_eq(srci.rdims[1],       dims[0]); /* 3 */
  ck_assert_int_eq(srci.rdims[2],       dims[2]); /* 2 */
  ck_assert_int_eq(srci.rsinglestep[0], dims[0]); /* 3 */
  ck_assert_int_eq(srci.rsinglestep[1], 1);
  ck_assert_int_eq(srci.rsinglestep[2], dims[0]*dims[1]); /* 12 */
  /* how many elements to advance the pointer to go to the next item
     in the desired order? */
  ck_assert_int_eq(srci.step[0],        srci.rsinglestep[0]); /* 3 */
  /* how many elements to advance the pointer additionally to go to
     the next item in the desired order when reaching the end of the
     first selected axis?  Then we've advanced step[0] elements
     rdims[0] = dims[1] times, but want to end up having advanced
     rsinglestep[1] = 1 element. */
  ck_assert_int_eq(srci.step[1],        srci.rsinglestep[1] - srci.rdims[0]*srci.step[0]); /* 1 - 4*3 = -11 */
  /* how many elements to advance the pointer additionally to go to
     the next item in the desired order when reaching the end of the
     second selected axis?  Then we've advanced step[0] elements
     rdims[0]*rdims[1] times, and step[1] elements rdims[1] times, but
     want to end up having advanced rsinglestep[2] elements. */
  ck_assert_int_eq(srci.step[2],        srci.rsinglestep[2] - srci.step[0]*srci.rdims[0]*srci.rdims[1] - srci.step[1]*srci.rdims[1]);

  ck_assert_int_eq(srci.raxes[0],       1);
  ck_assert_int_eq(srci.raxes[1],       0);
  ck_assert_int_eq(srci.raxes[2],       2);

  ck_assert_int_eq(srci.iraxes[0],      1);
  ck_assert_int_eq(srci.iraxes[1],      0);
  ck_assert_int_eq(srci.iraxes[2],      2);

  ck_assert_int_eq(srci.axisindex,      0);
  ck_assert_int_eq(srci.advanceaxis,    0);
  int count = 0;
  /* advanceLoop returns 0 while inside the first specified axis,
     1 when at the end of the first specified axis,
     2 when at the end of the second specified axis,
     3 when at the end of the third specified axis. */
  /* axes 1,0,2 → dimension sizes 4,3,2 */
  int expect_a[] = {0,0,0,1,0,0,0,1,0,0,0,2,
                    0,0,0,1,0,0,0,1,0,0,0,3};
  int expect_i[] = { 0, 3, 6, 9, 1, 4, 7,10, 2, 5, 8,11, 
                    12,15,18,21,13,16,19,22,14,17,20,23};
  int ok;
  int size = array_size(iq);
  do {
    fail_unless(count < size, "Out of bounds"); /* stay within bounds */
    fail_unless(*srcp.l == expect_i[count],
                "Element bad; expect %d, got %d, count = %d",
                expect_i[count], *srcp.l, count);
    int c0 = count % srci.rdims[0];
    int c1 = (count/srci.rdims[0]) % srci.rdims[1];
    int c2 = count/(srci.rdims[0]*srci.rdims[1]);
    fail_unless(srci.coords[0] == c0,
                "Coordinate 0 bad; expect %d, got %d, count = %d",
                c0, srci.coords[0], count);
    fail_unless(srci.coords[1] == c1,
                "Coordinate 1 bad; expect %d, got %d, count = %d",
                c1, srci.coords[1], count);
    fail_unless(srci.coords[2] == c2,
                "Coordinate 2 bad; expect %d, got %d, count = %d",
                c2, srci.coords[2], count);
    ok = advanceLoop(&srci, &srcp);
    fail_unless(ok == expect_a[count],
                "advanceLoop return bad; expect %d, got %d, count = %d",
                expect_a[count], ok, count);
    ++count;
  } while (ok < srci.rndim);
  ck_assert_int_eq(count, size); /* we did all elements */
}
END_TEST

START_TEST(axis_201)
{
  int dims[] = { 3, 4, 2 };
  Int iq = array_scratch(LUX_LONG, 3, dims);
  iq = lux_indgen(1, &iq);
  ck_assert_msg(iq > 0, "Cannot create array");
  int three =  3;
  Int axes = array_scratch(LUX_LONG, 1, &three);
  pointer axesp;
  axesp.v = array_data(axes);
  axesp.l[0] = 2;
  axesp.l[1] = 0;
  axesp.l[2] = 1;
  loopInfo srci;
  pointer srcp;
  ck_assert_int_eq(standardLoop(iq, axes, 0, LUX_LONG, &srci, &srcp, NULL, NULL, NULL), LUX_OK);
  ck_assert_int_eq(srci.ndim, sizeof(dims)/sizeof(*dims));
  ck_assert_int_eq(srci.nelem, dims[0]*dims[1]*dims[2]);
  int i;
  for (i = 0; i < srci.ndim; i++)
    ck_assert_int_eq(srci.coords[i],    0);
  /* specified dimensions */
  ck_assert_int_eq(srci.dims[0],        dims[0]);
  ck_assert_int_eq(srci.dims[1],        dims[1]);
  ck_assert_int_eq(srci.dims[2],        dims[2]);
  ck_assert_int_eq(srci.singlestep[0],  1); /* one step along dimension 0 */
  ck_assert_int_eq(srci.singlestep[1],  dims[0]); /* along dimension 1 */
  ck_assert_int_eq(srci.singlestep[2],  dims[0]*dims[1]); /* along dim 2 */
  /* specified axes */
  ck_assert_int_eq(srci.naxes,          3);
  ck_assert_int_eq(srci.axes[0],        2);
  ck_assert_int_eq(srci.axes[1],        0);
  ck_assert_int_eq(srci.axes[2],        1);
  /* compressed/rearranged dimensions; we loop along dimension 2
     first, and then along dimensions 0 and 1. */
  ck_assert_int_eq(srci.rndim,          3);
  ck_assert_int_eq(srci.rdims[0],       dims[2]); /* 2 */
  ck_assert_int_eq(srci.rdims[1],       dims[0]); /* 3 */
  ck_assert_int_eq(srci.rdims[2],       dims[1]); /* 4 */
  ck_assert_int_eq(srci.rsinglestep[0], dims[0]*dims[1]); /* 12 */
  ck_assert_int_eq(srci.rsinglestep[1], 1);
  ck_assert_int_eq(srci.rsinglestep[2], dims[0]); /* 3 */
  /* how many elements to advance the pointer to go to the next item
     in the desired order? */
  ck_assert_int_eq(srci.step[0],        srci.rsinglestep[0]); /* 12 */
  /* how many elements to advance the pointer additionally to go to
     the next item in the desired order when reaching the end of the
     first selected axis?  Then we've advanced step[0] elements
     rdims[0] = dims[1] times, but want to end up having advanced
     rsinglestep[1] = 1 element. */
  ck_assert_int_eq(srci.step[1],        srci.rsinglestep[1] - srci.rdims[0]*srci.step[0]); /* 1 - 2*12 = -23 */
  /* how many elements to advance the pointer additionally to go to
     the next item in the desired order when reaching the end of the
     second selected axis?  Then we've advanced step[0] elements
     rdims[0]*rdims[1] times, and step[1] elements rdims[1] times, but
     want to end up having advanced rsinglestep[2] elements. */
  ck_assert_int_eq(srci.step[2],        srci.rsinglestep[2] - srci.step[0]*srci.rdims[0]*srci.rdims[1] - srci.step[1]*srci.rdims[1]);

  ck_assert_int_eq(srci.raxes[0],       2);
  ck_assert_int_eq(srci.raxes[1],       0);
  ck_assert_int_eq(srci.raxes[2],       1);

  ck_assert_int_eq(srci.iraxes[0],      1);
  ck_assert_int_eq(srci.iraxes[1],      2);
  ck_assert_int_eq(srci.iraxes[2],      0);

  ck_assert_int_eq(srci.axisindex,      0);
  ck_assert_int_eq(srci.advanceaxis,    0);
  int count = 0;
  /* advanceLoop returns 0 while inside the first specified axis,
     1 when at the end of the first specified axis,
     2 when at the end of the second specified axis,
     3 when at the end of the third specified axis. */
  /* axes 2,0,1 → dimension sizes 2,3,4 */
  int expect_a[] = {0,1,0,1,0,2,0,1,0,1,0,2,
                    0,1,0,1,0,2,0,1,0,1,0,3};
  int expect_i[] = {0,12,1,13,2,14,3,15, 4,16, 5,17,
                    6,18,7,19,8,20,9,21,10,22,11,23};
  int size = array_size(iq);
  int ok;
  do {
    fail_unless(count < size, "Out of bounds"); /* stay within bounds */
    fail_unless(*srcp.l == expect_i[count],
                "Element bad; expect %d, got %d, count = %d",
                expect_i[count], *srcp.l, count);
    int c0 = count % srci.rdims[0];
    int c1 = (count/srci.rdims[0]) % srci.rdims[1];
    int c2 = count/(srci.rdims[0]*srci.rdims[1]);
    fail_unless(srci.coords[0] == c0,
                "Coordinate 0 bad; expect %d, got %d, count = %d",
                c0, srci.coords[0], count);
    fail_unless(srci.coords[1] == c1,
                "Coordinate 1 bad; expect %d, got %d, count = %d",
                c1, srci.coords[1], count);
    fail_unless(srci.coords[2] == c2,
                "Coordinate 2 bad; expect %d, got %d, count = %d",
                c2, srci.coords[2], count);
    ok = advanceLoop(&srci, &srcp);
    fail_unless(ok == expect_a[count],
                "advanceLoop return bad; expect %d, got %d, count = %d",
                expect_a[count], ok, count);
    ++count;
  } while (ok < srci.rndim);
  ck_assert_int_eq(count, size); /* we did all elements */
}
END_TEST

START_TEST(axis_012_eachrow)
{
  int dims[] = { 3, 4, 2 };
  Int iq = array_scratch(LUX_LONG, 3, dims);
  iq = lux_indgen(1, &iq);
  ck_assert_msg(iq > 0, "Cannot create array");
  loopInfo srci;
  pointer srcp;
  ck_assert_int_eq(standardLoop(iq, 0, SL_ALLAXES | SL_EACHROW, LUX_LONG, &srci, &srcp, NULL, NULL, NULL), LUX_OK);
  ck_assert_int_eq(srci.ndim, sizeof(dims)/sizeof(*dims));
  ck_assert_int_eq(srci.nelem, dims[0]*dims[1]*dims[2]);
  int i;
  for (i = 0; i < srci.ndim; i++)
    ck_assert_int_eq(srci.coords[i],    0);
  /* specified dimensions */
  ck_assert_int_eq(srci.dims[0],        dims[0]);
  ck_assert_int_eq(srci.dims[1],        dims[1]);
  ck_assert_int_eq(srci.dims[2],        dims[2]);
  ck_assert_int_eq(srci.singlestep[0],  1); /* one step along dimension 0 */
  ck_assert_int_eq(srci.singlestep[1],  dims[0]); /* along dimension 1 */
  ck_assert_int_eq(srci.singlestep[2],  dims[0]*dims[1]); /* along dim 2 */
  /* specified axes */
  ck_assert_int_eq(srci.naxes,          3);
  ck_assert_int_eq(srci.axes[0],        0);
  ck_assert_int_eq(srci.axes[1],        1);
  ck_assert_int_eq(srci.axes[2],        2);
  /* compressed/rearranged dimensions; here the same as the originals */
  ck_assert_int_eq(srci.rndim,          3);
  ck_assert_int_eq(srci.rdims[0],       dims[0]);
  ck_assert_int_eq(srci.rdims[1],       dims[1]);
  ck_assert_int_eq(srci.rdims[2],       dims[2]);
  ck_assert_int_eq(srci.rsinglestep[0], 1);
  ck_assert_int_eq(srci.rsinglestep[1], dims[0]);
  ck_assert_int_eq(srci.rsinglestep[2], dims[0]*dims[1]);
  /* how many elements to advance the pointer to go to the next item
     in the desired order? */
  ck_assert_int_eq(srci.step[0],        1);
  /* how many elements to advance the pointer additionally to go to
     the next item in the desired order when reaching the end of the
     first selected axis? */
  ck_assert_int_eq(srci.step[1],        0);
  /* how many elements to advance the pointer additionally to go to
     the next item in the desired order when reaching the end of the
     second selected axis? */
  ck_assert_int_eq(srci.step[2],        0);

  ck_assert_int_eq(srci.raxes[0],       0);
  ck_assert_int_eq(srci.raxes[1],       1);
  ck_assert_int_eq(srci.raxes[2],       2);

  ck_assert_int_eq(srci.iraxes[0],      0);
  ck_assert_int_eq(srci.iraxes[1],      1);
  ck_assert_int_eq(srci.iraxes[2],      2);

  ck_assert_int_eq(srci.axisindex, 0);
  ck_assert_int_eq(srci.advanceaxis, 1); /* advancing along axis 1
                                            (because of SL_EACHROW) */
  /* axes 0,1,2 → dimension sizes 3,4,2 */
  int count = 0;
  int expect_a[] = {1,1,1,2, 1, 1, 1, 3 };
  int expect_i[] = {0,3,6,9,12,15,18,21 };
  int ok;
  int size = dims[1]*dims[2];
  do {
    fail_unless(count < size, "Out of bounds"); /* stay within bounds */
    fail_unless(*srcp.l == expect_i[count],
                "Element bad; expect %d, got %d, count = %d",
                expect_i[count], *srcp.l, count);
    int c0 = 0;                 /* always 0 because of SL_EACHROW */
    int c1 = count % srci.rdims[1];
    int c2 = count/srci.rdims[1];
    fail_unless(srci.coords[0] == c0,
                "Coordinate 0 bad; expect %d, got %d, count = %d",
                c0, srci.coords[0], count);
    fail_unless(srci.coords[1] == c1,
                "Coordinate 1 bad; expect %d, got %d, count = %d",
                c1, srci.coords[1], count);
    fail_unless(srci.coords[2] == c2,
                "Coordinate 2 bad; expect %d, got %d, count = %d",
                c2, srci.coords[2], count);
    /* we have to take care of advancing along the first axis
       ourselves, because of SL_EACHROW */
    srcp.l += srci.rsinglestep[0]*srci.rdims[0];
    ok = advanceLoop(&srci, &srcp);
    fail_unless(ok == expect_a[count],
                "advanceLoop return bad; expect %d, got %d, count = %d",
                expect_a[count], ok, count);
    ++count;
  } while (ok < srci.rndim);
  ck_assert_int_eq(count, size); /* we did all rows */
}
END_TEST

START_TEST(axis_102_eachrow)
{
  int dims[] = { 3, 4, 2 };
  Int iq = array_scratch(LUX_LONG, 3, dims);
  iq = lux_indgen(1, &iq);
  ck_assert_msg(iq > 0, "Cannot create array");
  int one =  1;
  Int axes = array_scratch(LUX_LONG, 1, &one); /* axis index 1 */
  pointer axesp;
  axesp.v = array_data(axes);
  *axesp.l = 1;
  loopInfo srci;
  pointer srcp;
  ck_assert_int_eq(standardLoop(iq, axes, SL_EACHROW, LUX_LONG, &srci, &srcp, NULL, NULL, NULL), LUX_OK);
  ck_assert_int_eq(srci.ndim, sizeof(dims)/sizeof(*dims));
  ck_assert_int_eq(srci.nelem, dims[0]*dims[1]*dims[2]);
  int i;
  for (i = 0; i < srci.ndim; i++)
    ck_assert_int_eq(srci.coords[i],    0);
  /* specified dimensions */
  ck_assert_int_eq(srci.dims[0],        dims[0]);
  ck_assert_int_eq(srci.dims[1],        dims[1]);
  ck_assert_int_eq(srci.dims[2],        dims[2]);
  ck_assert_int_eq(srci.singlestep[0],  1); /* one step along dimension 0 */
  ck_assert_int_eq(srci.singlestep[1],  dims[0]); /* along dimension 1 */
  ck_assert_int_eq(srci.singlestep[2],  dims[0]*dims[1]); /* along dim 2 */
  /* specified axes */
  ck_assert_int_eq(srci.naxes,          1);
  ck_assert_int_eq(srci.axes[0],        1); /* loop along axis 1 */
  /* compressed/rearranged dimensions; we loop along dimension 1
     first, and then along dimensions 0 and 2. */
  ck_assert_int_eq(srci.rndim,          3);
  ck_assert_int_eq(srci.rdims[0],       dims[1]); /* 4 */
  ck_assert_int_eq(srci.rdims[1],       dims[0]); /* 3 */
  ck_assert_int_eq(srci.rdims[2],       dims[2]); /* 2 */
  ck_assert_int_eq(srci.rsinglestep[0], dims[0]); /* 3 */
  ck_assert_int_eq(srci.rsinglestep[1], 1);
  ck_assert_int_eq(srci.rsinglestep[2], dims[0]*dims[1]); /* 12 */
  /* how many elements to advance the pointer to go to the next item
     in the desired order? */
  ck_assert_int_eq(srci.step[0],        srci.rsinglestep[0]); /* 3 */
  /* how many elements to advance the pointer additionally to go to
     the next item in the desired order when reaching the end of the
     first selected axis?  Then we've advanced step[0] elements
     rdims[0] = dims[1] times, but want to end up having advanced
     rsinglestep[1] = 1 element. */
  ck_assert_int_eq(srci.step[1],        srci.rsinglestep[1] - srci.rdims[0]*srci.step[0]); /* 1 - 4*3 = -11 */
  /* how many elements to advance the pointer additionally to go to
     the next item in the desired order when reaching the end of the
     second selected axis?  Then we've advanced step[0] elements
     rdims[0]*rdims[1] times, and step[1] elements rdims[1] times, but
     want to end up having advanced rsinglestep[2] elements. */
  ck_assert_int_eq(srci.step[2],        srci.rsinglestep[2] - srci.step[0]*srci.rdims[0]*srci.rdims[1] - srci.step[1]*srci.rdims[1]);

  ck_assert_int_eq(srci.raxes[0],       1);
  ck_assert_int_eq(srci.raxes[1],       0);
  ck_assert_int_eq(srci.raxes[2],       2);

  ck_assert_int_eq(srci.iraxes[0],      1);
  ck_assert_int_eq(srci.iraxes[1],      0);
  ck_assert_int_eq(srci.iraxes[2],      2);

  ck_assert_int_eq(srci.axisindex,      0);
  ck_assert_int_eq(srci.advanceaxis,    1);
  int count = 0;
  /* advanceLoop returns 0 while inside the first specified axis,
     1 when at the end of the first specified axis,
     2 when at the end of the second specified axis,
     3 when at the end of the third specified axis. */
  /* axes 1,0,2 → dimension sizes 4,3,2 */
  int expect_a[] = {1,1,2, 1, 1, 3};
  int expect_i[] = {0,1,2,12,13,14};
  int ok;
  int size = dims[0]*dims[2];
  do {
    fail_unless(count < size, "Out of bounds"); /* stay within bounds */
    fail_unless(*srcp.l == expect_i[count],
                "Element bad; expect %d, got %d, count = %d",
                expect_i[count], *srcp.l, count);
    int c0 = 0;                 /* always 0 because of SL_EACHROW */
    int c1 = count % srci.rdims[1];
    int c2 = count/srci.rdims[1];
    fail_unless(srci.coords[0] == c0,
                "Coordinate 0 bad; expect %d, got %d, count = %d",
                c0, srci.coords[0], count);
    fail_unless(srci.coords[1] == c1,
                "Coordinate 1 bad; expect %d, got %d, count = %d",
                c1, srci.coords[1], count);
    fail_unless(srci.coords[2] == c2,
                "Coordinate 2 bad; expect %d, got %d, count = %d",
                c2, srci.coords[2], count);
    /* we have to take care of advancing along the first axis
       ourselves, because of SL_EACHROW */
    srcp.l += srci.rsinglestep[0]*srci.rdims[0];
    ok = advanceLoop(&srci, &srcp);
    fail_unless(ok == expect_a[count],
                "advanceLoop return bad; expect %d, got %d, count = %d",
                expect_a[count], ok, count);
    ++count;
  } while (ok < srci.rndim);
  ck_assert_int_eq(count, size);
}
END_TEST

START_TEST(axis_201_eachrow)
{
  int dims[] = { 3, 4, 2 };
  Int iq = array_scratch(LUX_LONG, 3, dims);
  iq = lux_indgen(1, &iq);
  ck_assert_msg(iq > 0, "Cannot create array");
  int three =  3;
  Int axes = array_scratch(LUX_LONG, 1, &three);
  pointer axesp;
  axesp.v = array_data(axes);
  axesp.l[0] = 2;
  axesp.l[1] = 0;
  axesp.l[2] = 1;
  loopInfo srci;
  pointer srcp;
  ck_assert_int_eq(standardLoop(iq, axes, SL_EACHROW, LUX_LONG, &srci, &srcp, NULL, NULL, NULL), LUX_OK);
  ck_assert_int_eq(srci.ndim, sizeof(dims)/sizeof(*dims));
  ck_assert_int_eq(srci.nelem, dims[0]*dims[1]*dims[2]);
  int i;
  for (i = 0; i < srci.ndim; i++)
    ck_assert_int_eq(srci.coords[i],    0);
  /* specified dimensions */
  ck_assert_int_eq(srci.dims[0],        dims[0]);
  ck_assert_int_eq(srci.dims[1],        dims[1]);
  ck_assert_int_eq(srci.dims[2],        dims[2]);
  ck_assert_int_eq(srci.singlestep[0],  1); /* one step along dimension 0 */
  ck_assert_int_eq(srci.singlestep[1],  dims[0]); /* along dimension 1 */
  ck_assert_int_eq(srci.singlestep[2],  dims[0]*dims[1]); /* along dim 2 */
  /* specified axes */
  ck_assert_int_eq(srci.naxes,          3);
  ck_assert_int_eq(srci.axes[0],        2);
  ck_assert_int_eq(srci.axes[1],        0);
  ck_assert_int_eq(srci.axes[2],        1);
  /* compressed/rearranged dimensions; we loop along dimension 2
     first, and then along dimensions 0 and 1. */
  ck_assert_int_eq(srci.rndim,          3);
  ck_assert_int_eq(srci.rdims[0],       dims[2]); /* 2 */
  ck_assert_int_eq(srci.rdims[1],       dims[0]); /* 3 */
  ck_assert_int_eq(srci.rdims[2],       dims[1]); /* 4 */
  ck_assert_int_eq(srci.rsinglestep[0], dims[0]*dims[1]); /* 12 */
  ck_assert_int_eq(srci.rsinglestep[1], 1);
  ck_assert_int_eq(srci.rsinglestep[2], dims[0]); /* 3 */
  /* how many elements to advance the pointer to go to the next item
     in the desired order? */
  ck_assert_int_eq(srci.step[0],        srci.rsinglestep[0]); /* 12 */
  /* how many elements to advance the pointer additionally to go to
     the next item in the desired order when reaching the end of the
     first selected axis?  Then we've advanced step[0] elements
     rdims[0] = dims[1] times, but want to end up having advanced
     rsinglestep[1] = 1 element. */
  ck_assert_int_eq(srci.step[1],        srci.rsinglestep[1] - srci.rdims[0]*srci.step[0]); /* 1 - 2*12 = -23 */
  /* how many elements to advance the pointer additionally to go to
     the next item in the desired order when reaching the end of the
     second selected axis?  Then we've advanced step[0] elements
     rdims[0]*rdims[1] times, and step[1] elements rdims[1] times, but
     want to end up having advanced rsinglestep[2] elements. */
  ck_assert_int_eq(srci.step[2],        srci.rsinglestep[2] - srci.step[0]*srci.rdims[0]*srci.rdims[1] - srci.step[1]*srci.rdims[1]);

  ck_assert_int_eq(srci.raxes[0],       2);
  ck_assert_int_eq(srci.raxes[1],       0);
  ck_assert_int_eq(srci.raxes[2],       1);

  ck_assert_int_eq(srci.iraxes[0],      1);
  ck_assert_int_eq(srci.iraxes[1],      2);
  ck_assert_int_eq(srci.iraxes[2],      0);

  ck_assert_int_eq(srci.axisindex,      0);
  ck_assert_int_eq(srci.advanceaxis,    1);
  int count = 0;
  /* advanceLoop returns 0 while inside the first specified axis,
     1 when at the end of the first specified axis,
     2 when at the end of the second specified axis,
     3 when at the end of the third specified axis. */
  /* axes 2,0,1 → dimension sizes 2,3,4 */
  int expect_a[] = {1,1,2,1,1,2,1,1,2,1, 1, 3};
  int expect_i[] = {0,1,2,3,4,5,6,7,8,9,10,11};
  int ok;
  int size = dims[0]*dims[1];
  do {
    fail_unless(count < size, "Out of bounds"); /* stay within bounds */
    fail_unless(*srcp.l == expect_i[count],
                "Element bad; expect %d, got %d, count = %d",
                expect_i[count], *srcp.l, count);
    int c0 = 0;                 /* always 0 because of SL_EACHROW */
    int c1 = count % srci.rdims[1];
    int c2 = count/srci.rdims[1];
    fail_unless(srci.coords[0] == c0,
                "Coordinate 0 bad; expect %d, got %d, count = %d",
                c0, srci.coords[0], count);
    fail_unless(srci.coords[1] == c1,
                "Coordinate 1 bad; expect %d, got %d, count = %d",
                c1, srci.coords[1], count);
    fail_unless(srci.coords[2] == c2,
                "Coordinate 2 bad; expect %d, got %d, count = %d",
                c2, srci.coords[2], count);
    /* we have to take care of advancing along the first axis
       ourselves, because of SL_EACHROW */
    srcp.l += srci.rsinglestep[0]*srci.rdims[0];
    ok = advanceLoop(&srci, &srcp);
    fail_unless(ok == expect_a[count],
                "advanceLoop return bad; expect %d, got %d, count = %d",
                expect_a[count], ok, count);
    ++count;
  } while (ok < srci.rndim);
  ck_assert_int_eq(count, size);
}
END_TEST

START_TEST(edge)
{
  int dims[] = { 3, 4, 2 };
  Int iq = array_scratch(LUX_LONG, 3, dims);
  iq = lux_indgen(1, &iq);
  ck_assert_msg(iq > 0, "Cannot create array");
  loopInfo srci;
  pointer srcp;
  ck_assert_int_eq(standardLoop(iq, -1, SL_ALLAXES, LUX_LONG, &srci, &srcp, NULL, NULL, NULL), LUX_OK);
  loopInfo esrci = srci;
  /* edge 0 */
  int edge = 0;
  rearrangeEdgeLoop(&esrci, NULL, edge);
  int count = 0;
  int expect_i0[] = { 0,3,6,9,12,15,18,21 };
  do {
    fail_unless(count < sizeof(expect_i0)/sizeof(*expect_i0), "Out of bounds");
    fail_unless(*srcp.l == expect_i0[count],
                "Element bad; expect %d, got %d; edge %d, count %d",
                expect_i0[count], *srcp.l, edge, count);
    ++count;
  } while (advanceLoop(&esrci, &srcp) < esrci.ndim - 1);
  fail_unless(count == sizeof(expect_i0)/sizeof(*expect_i0),
              "Edge element count bad; expect %d, got %d, edge %d",
              sizeof(expect_i0)/sizeof(*expect_i0), count, edge);

  edge = 1;
  rearrangeEdgeLoop(&esrci, NULL, edge);
  count = 0;
  int expect_i1[] = { 2,5,8,11,14,17,20,23 };
  do {
    fail_unless(count < sizeof(expect_i1)/sizeof(*expect_i1), "Out of bounds");
    fail_unless(*srcp.l == expect_i1[count],
                "Element bad; expect %d, got %d; edge %d, count %d",
                expect_i1[count], *srcp.l, edge, count);
    ++count;
  } while (advanceLoop(&esrci, &srcp) < esrci.ndim - 1);
  fail_unless(count == sizeof(expect_i1)/sizeof(*expect_i1),
              "Edge element count bad; expect %d, got %d, edge %d",
              sizeof(expect_i1)/sizeof(*expect_i1), count, edge);

  edge = 2;
  rearrangeEdgeLoop(&esrci, NULL, edge);
  count = 0;
  int expect_i2[] = { 0,1,2,12,13,14 };
  do {
    fail_unless(count < sizeof(expect_i2)/sizeof(*expect_i2), "Out of bounds");
    fail_unless(*srcp.l == expect_i2[count],
                "Element bad; expect %d, got %d; edge %d, count %d",
                expect_i2[count], *srcp.l, edge, count);
    ++count;
  } while (advanceLoop(&esrci, &srcp) < esrci.ndim - 1);
  fail_unless(count == sizeof(expect_i2)/sizeof(*expect_i2),
              "Edge element count bad; expect %d, got %d, edge %d",
              sizeof(expect_i2)/sizeof(*expect_i2), count, edge);

  edge = 3;
  rearrangeEdgeLoop(&esrci, NULL, edge);
  count = 0;
  int expect_i3[] = { 9,10,11,21,22,23 };
  do {
    fail_unless(count < sizeof(expect_i3)/sizeof(*expect_i3), "Out of bounds");
    fail_unless(*srcp.l == expect_i3[count],
                "Element bad; expect %d, got %d; edge %d, count %d",
                expect_i3[count], *srcp.l, edge, count);
    ++count;
  } while (advanceLoop(&esrci, &srcp) < esrci.ndim - 1);
  fail_unless(count == sizeof(expect_i3)/sizeof(*expect_i3),
              "Edge element count bad; expect %d, got %d, edge %d",
              sizeof(expect_i3)/sizeof(*expect_i3), count, edge);

  edge = 4;
  rearrangeEdgeLoop(&esrci, NULL, edge);
  count = 0;
  int expect_i4[] = { 0,1,2,3,4,5,6,7,8,9,10,11 };
  do {
    fail_unless(count < sizeof(expect_i4)/sizeof(*expect_i4), "Out of bounds");
    fail_unless(*srcp.l == expect_i4[count],
                "Element bad; expect %d, got %d; edge %d, count %d",
                expect_i4[count], *srcp.l, edge, count);
    ++count;
  } while (advanceLoop(&esrci, &srcp) < esrci.ndim - 1);
  fail_unless(count == sizeof(expect_i4)/sizeof(*expect_i4),
              "Edge element count bad; expect %d, got %d, edge %d",
              sizeof(expect_i4)/sizeof(*expect_i4), count, edge);

  edge = 5;
  rearrangeEdgeLoop(&esrci, NULL, edge);
  count = 0;
  int expect_i5[] = { 12,13,14,15,16,17,18,19,20,21,22,23 };
  do {
    fail_unless(count < sizeof(expect_i5)/sizeof(*expect_i5), "Out of bounds");
    fail_unless(*srcp.l == expect_i5[count],
                "Element bad; expect %d, got %d; edge %d, count %d",
                expect_i5[count], *srcp.l, edge, count);
    ++count;
  } while (advanceLoop(&esrci, &srcp) < esrci.ndim - 1);
  fail_unless(count == sizeof(expect_i5)/sizeof(*expect_i5),
              "Edge element count bad; expect %d, got %d, edge %d",
              sizeof(expect_i5)/sizeof(*expect_i5), count, edge);
}
END_TEST

START_TEST(simple_args)
{
  struct param_spec_list *parse_standard_arg_fmt(const char *);
  struct param_spec_list *psl;
  struct param_spec ps;
  struct dims_spec ds;

  psl = parse_standard_arg_fmt(NULL);
  ck_assert(psl == NULL);

  psl = parse_standard_arg_fmt("iL");
  ck_assert(       psl                   != NULL     );
  ck_assert_int_eq(psl->num_param_specs,    1        );
  ck_assert_int_eq(psl->return_param_index, -1       );
  ps = psl->param_specs[0];
  ck_assert_int_eq(ps.ref_par,              0        );
  ck_assert_int_eq(ps.num_dims_spec,        0        );
  ck_assert(       ps.dims_spec          == NULL     );
  ck_assert_int_eq(ps.is_optional,          0        );
  ck_assert_int_eq(ps.remaining_dims,       PS_ABSENT);
  ck_assert_int_eq(ps.logical_type,         PS_INPUT );
  ck_assert_int_eq(ps.data_type_limit,      PS_EXACT );
  ck_assert_int_eq(ps.data_type,            LUX_LONG );
  free_param_spec_list(psl);

  psl = parse_standard_arg_fmt("o>D");
  ck_assert(       psl                   != NULL          );
  ck_assert_int_eq(psl->num_param_specs,    1             );
  ck_assert_int_eq(psl->return_param_index, -1            );
  ps = psl->param_specs[0];
  ck_assert_int_eq(ps.ref_par,              0             );
  ck_assert_int_eq(ps.num_dims_spec,        0             );
  ck_assert(       ps.dims_spec          == NULL          );
  ck_assert_int_eq(ps.is_optional,          0             );
  ck_assert_int_eq(ps.remaining_dims,       PS_ABSENT     );
  ck_assert_int_eq(ps.logical_type,         PS_OUTPUT     );
  ck_assert_int_eq(ps.data_type_limit,      PS_LOWER_LIMIT);
  ck_assert_int_eq(ps.data_type,            LUX_DOUBLE    );
  free_param_spec_list(psl);

  psl = parse_standard_arg_fmt("rS");
  ck_assert(       psl                   != NULL           );
  ck_assert_int_eq(psl->num_param_specs,    1              );
  ck_assert_int_eq(psl->return_param_index, 0              );
  ps = psl->param_specs[0];
  ck_assert_int_eq(ps.ref_par,              0              );
  ck_assert_int_eq(ps.num_dims_spec,        0              );
  ck_assert(       ps.dims_spec          == NULL           );
  ck_assert_int_eq(ps.is_optional,          0              );
  ck_assert_int_eq(ps.remaining_dims,       PS_ABSENT      );
  ck_assert_int_eq(ps.logical_type,         PS_RETURN      );
  ck_assert_int_eq(ps.data_type_limit,      PS_EXACT       );
  ck_assert_int_eq(ps.data_type,            LUX_TEMP_STRING);
  free_param_spec_list(psl);
}
END_TEST

Suite *standardLoop_suite(void)
{
  Suite *s = suite_create("standardLoop test suite");

  TCase *tc_axis = tcase_create("axis");
  tcase_add_test(tc_axis, axis_012);
  tcase_add_test(tc_axis, axis_102);
  tcase_add_test(tc_axis, axis_201);
  suite_add_tcase(s, tc_axis);

  TCase *tc_axis_eachrow = tcase_create("axis eachrow");
  tcase_add_test(tc_axis_eachrow, axis_012_eachrow);
  tcase_add_test(tc_axis_eachrow, axis_102_eachrow);
  tcase_add_test(tc_axis_eachrow, axis_201_eachrow);
  suite_add_tcase(s, tc_axis_eachrow);

  TCase *tc_axis_edge = tcase_create("axis edge");
  tcase_add_test(tc_axis_edge, edge);
  suite_add_tcase(s, tc_axis_edge);

  return s;
}

Suite *standard_args_suite(void)
{
  Suite *s = suite_create("standard args suite");

  TCase *tc_standard_args = tcase_create("parse standard_args");
  tcase_add_test(tc_standard_args, simple_args);

  suite_add_tcase(s, tc_standard_args);
  return s;
}

int main(void)
{
  SRunner *sr = srunner_create(standardLoop_suite());
  srunner_add_suite(sr, standard_args_suite());
  srunner_run_all(sr, CK_NORMAL);
  int number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0)? EXIT_SUCCESS: EXIT_FAILURE;
}
