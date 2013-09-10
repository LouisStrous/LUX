#include <check.h>
#include <stdlib.h>		/* for EXIT_SUCCESS, EXIT_FAILURE */

#include "config.h"
#include "anaparser.h"
#include "action.h"

START_TEST(first_axis)
{
  int dims[] = { 3, 4 };
  Int iq = array_scratch(ANA_LONG, 2, dims);
  iq = ana_indgen(1, &iq);
  ck_assert_msg(iq > 0, "Cannot create array");
  loopInfo srci;
  pointer srcp;
  ck_assert_int_eq(standardLoop(iq, 0, SL_ALLAXES, ANA_LONG, &srci, &srcp, NULL, NULL, NULL), ANA_OK);
  ck_assert_int_eq(srci.ndim, sizeof(dims)/sizeof(*dims));
  ck_assert_int_eq(srci.nelem, dims[0]*dims[1]);
  int i;
  for (i = 0; i < srci.ndim; i++)
    ck_assert_int_eq(srci.coords[i], 0);
  ck_assert_int_eq(srci.dims[0], dims[0]);
  ck_assert_int_eq(srci.dims[1], dims[1]);
  ck_assert_int_eq(srci.naxes, 2);
  ck_assert_int_eq(srci.axes[0], 0);
  ck_assert_int_eq(srci.axes[1], 1);
  ck_assert_int_eq(srci.singlestep[0], 1);
  ck_assert_int_eq(srci.singlestep[1], dims[0]);
  ck_assert_int_eq(srci.rndim, 2);
  ck_assert_int_eq(srci.rdims[0], dims[0]);
  ck_assert_int_eq(srci.rdims[1], dims[1]);
  ck_assert_int_eq(srci.rsinglestep[0], 1);
  ck_assert_int_eq(srci.rsinglestep[1], dims[0]);
  ck_assert_int_eq(srci.step[0], 1);
  ck_assert_int_eq(srci.step[1], 0);
  ck_assert_int_eq(srci.raxes[0], 0);
  ck_assert_int_eq(srci.raxes[1], 1);
  ck_assert_int_eq(srci.iraxes[0], 0);
  ck_assert_int_eq(srci.iraxes[1], 1);
  ck_assert_int_eq(srci.axisindex, 0);
  ck_assert_int_eq(srci.advanceaxis, 0);
  int count = 0;
  do {
    ck_assert_int_eq(*srcp.l, count); /* each element equal to its index */
    ++count;
  } while (advanceLoop(&srci, &srcp) < srci.rndim);
  ck_assert_int_eq(count, array_size(iq)); /* we did all elements */
}
END_TEST

START_TEST(second_axis)
{
  int dims[] = { 3, 4 };
  Int iq = array_scratch(ANA_LONG, 2, dims);
  iq = ana_indgen(1, &iq);
  ck_assert_msg(iq > 0, "Cannot create array");
  int one =  1;
  Int axes = array_scratch(ANA_LONG, 1, &one);
  pointer axesp;
  axesp.v = array_data(axes);
  *axesp.l = 1;
  loopInfo srci;
  pointer srcp;
  ck_assert_int_eq(standardLoop(iq, axes, 0, ANA_LONG, &srci, &srcp, NULL, NULL, NULL), ANA_OK);
  ck_assert_int_eq(srci.ndim, sizeof(dims)/sizeof(*dims));
  ck_assert_int_eq(srci.nelem, dims[0]*dims[1]);
  int i;
  for (i = 0; i < srci.ndim; i++)
    ck_assert_int_eq(srci.coords[i], 0);
  ck_assert_int_eq(srci.dims[0], dims[0]);
  ck_assert_int_eq(srci.dims[1], dims[1]);
  ck_assert_int_eq(srci.naxes, 1);
  ck_assert_int_eq(srci.axes[0], 1);
  ck_assert_int_eq(srci.singlestep[0], 1);
  ck_assert_int_eq(srci.singlestep[1], dims[0]);
  ck_assert_int_eq(srci.rndim, 2);
  ck_assert_int_eq(srci.rdims[0], dims[1]);
  ck_assert_int_eq(srci.rdims[1], dims[0]);
  ck_assert_int_eq(srci.rsinglestep[0], dims[0]);
  ck_assert_int_eq(srci.rsinglestep[1], 1);
  ck_assert_int_eq(srci.step[0], dims[0]);
  ck_assert_int_eq(srci.step[1], 1 - dims[0]*dims[1]);
  ck_assert_int_eq(srci.raxes[0], 1);
  ck_assert_int_eq(srci.raxes[1], 0);
  ck_assert_int_eq(srci.iraxes[0], 1);
  ck_assert_int_eq(srci.iraxes[1], 0);
  ck_assert_int_eq(srci.axisindex, 0);
  ck_assert_int_eq(srci.advanceaxis, 0);
  int count = 0;
  int expect[] = { 0, 3, 6, 9, 1, 4, 7, 10, 2, 5, 8, 11 };
  do {
    ck_assert_int_eq(*srcp.l, expect[count]);
    ++count;
  } while (advanceLoop(&srci, &srcp) < srci.rndim);
  ck_assert_int_eq(count, array_size(iq)); /* we did all elements */
}
END_TEST

START_TEST(first_then_second_axis)
{
  int dims[] = { 3, 4 };
  Int iq = array_scratch(ANA_LONG, 2, dims);
  iq = ana_indgen(1, &iq);
  ck_assert_msg(iq > 0, "Cannot create array");
  loopInfo srci;
  pointer srcp;
  ck_assert_int_eq(standardLoop(iq, 0, SL_ALLAXES, ANA_LONG, &srci, &srcp, NULL, NULL, NULL), ANA_OK);
  ck_assert_int_eq(srci.ndim, sizeof(dims)/sizeof(*dims));
  ck_assert_int_eq(srci.nelem, dims[0]*dims[1]);
  int i;
  for (i = 0; i < srci.ndim; i++)
    ck_assert_int_eq(srci.coords[i], 0);
  ck_assert_int_eq(srci.dims[0], dims[0]);
  ck_assert_int_eq(srci.dims[1], dims[1]);
  ck_assert_int_eq(srci.naxes, 2);
  ck_assert_int_eq(srci.axes[0], 0);
  ck_assert_int_eq(srci.axes[1], 1);
  ck_assert_int_eq(srci.singlestep[0], 1);
  ck_assert_int_eq(srci.singlestep[1], dims[0]);
  ck_assert_int_eq(srci.rndim, 2);
  ck_assert_int_eq(srci.rdims[0], dims[0]);
  ck_assert_int_eq(srci.rdims[1], dims[1]);
  ck_assert_int_eq(srci.rsinglestep[0], 1);
  ck_assert_int_eq(srci.rsinglestep[1], dims[0]);
  ck_assert_int_eq(srci.step[0], 1);
  ck_assert_int_eq(srci.step[1], 0);
  ck_assert_int_eq(srci.raxes[0], 0);
  ck_assert_int_eq(srci.raxes[1], 1);
  ck_assert_int_eq(srci.iraxes[0], 0);
  ck_assert_int_eq(srci.iraxes[1], 1);
  ck_assert_int_eq(srci.axisindex, 0);
  ck_assert_int_eq(srci.advanceaxis, 0);
  int count = 0;
  do {
    ck_assert_int_eq(*srcp.l, count);
    ++count;
  } while (advanceLoop(&srci, &srcp) < srci.rndim);
  ck_assert_int_eq(count, array_size(iq)); /* we did all elements */
  int one =  1;
  setAxes(&srci, 1, &one, 0);
  ck_assert_int_eq(srci.ndim, sizeof(dims)/sizeof(*dims));
  ck_assert_int_eq(srci.nelem, dims[0]*dims[1]);
  for (i = 0; i < srci.ndim; i++)
    ck_assert_int_eq(srci.coords[i], 0);
  ck_assert_int_eq(srci.dims[0], dims[0]);
  ck_assert_int_eq(srci.dims[1], dims[1]);
  ck_assert_int_eq(srci.naxes, 1);
  ck_assert_int_eq(srci.axes[0], 1);
  ck_assert_int_eq(srci.singlestep[0], 1);
  ck_assert_int_eq(srci.singlestep[1], dims[0]);
  ck_assert_int_eq(srci.rndim, 2);
  ck_assert_int_eq(srci.rdims[0], dims[1]);
  ck_assert_int_eq(srci.rdims[1], dims[0]);
  ck_assert_int_eq(srci.rsinglestep[0], dims[0]);
  ck_assert_int_eq(srci.rsinglestep[1], 1);
  ck_assert_int_eq(srci.step[0], dims[0]);
  ck_assert_int_eq(srci.step[1], 1 - dims[0]*dims[1]);
  ck_assert_int_eq(srci.raxes[0], 1);
  ck_assert_int_eq(srci.raxes[1], 0);
  ck_assert_int_eq(srci.iraxes[0], 1);
  ck_assert_int_eq(srci.iraxes[1], 0);
  ck_assert_int_eq(srci.axisindex, 0);
  ck_assert_int_eq(srci.advanceaxis, 0);
  int expect[] = { 0, 3, 6, 9, 1, 4, 7, 10, 2, 5, 8, 11 };
  count = 0;
  do {
    ck_assert_int_eq(*srcp.l, expect[count]);
    ++count;
  } while (advanceLoop(&srci, &srcp) < srci.rndim);
  ck_assert_int_eq(count, array_size(iq)); /* we did all elements */
}
END_TEST

START_TEST(first_axis_eachrow)
{
  int dims[] = { 3, 4 };
  Int iq = array_scratch(ANA_LONG, 2, dims);
  iq = ana_indgen(1, &iq);
  ck_assert_msg(iq > 0, "Cannot create array");
  loopInfo srci;
  pointer srcp;
  ck_assert_int_eq(standardLoop(iq, 0, SL_ALLAXES | SL_EACHROW, ANA_LONG, &srci, &srcp, NULL, NULL, NULL), ANA_OK);
  ck_assert_int_eq(srci.ndim, sizeof(dims)/sizeof(*dims));
  ck_assert_int_eq(srci.nelem, dims[0]*dims[1]);
  int i;
  for (i = 0; i < srci.ndim; i++)
    ck_assert_int_eq(srci.coords[i], 0);
  ck_assert_int_eq(srci.dims[0], dims[0]);
  ck_assert_int_eq(srci.dims[1], dims[1]);
  ck_assert_int_eq(srci.naxes, 2);
  ck_assert_int_eq(srci.axes[0], 0);
  ck_assert_int_eq(srci.axes[1], 1);
  ck_assert_int_eq(srci.singlestep[0], 1);
  ck_assert_int_eq(srci.singlestep[1], dims[0]);
  ck_assert_int_eq(srci.rndim, 2);
  ck_assert_int_eq(srci.rdims[0], dims[0]);
  ck_assert_int_eq(srci.rdims[1], dims[1]);
  ck_assert_int_eq(srci.rsinglestep[0], 1);
  ck_assert_int_eq(srci.rsinglestep[1], dims[0]);
  ck_assert_int_eq(srci.step[0], 1);
  ck_assert_int_eq(srci.step[1], 0);
  ck_assert_int_eq(srci.raxes[0], 0);
  ck_assert_int_eq(srci.raxes[1], 1);
  ck_assert_int_eq(srci.iraxes[0], 0);
  ck_assert_int_eq(srci.iraxes[1], 1);
  ck_assert_int_eq(srci.axisindex, 0);
  ck_assert_int_eq(srci.advanceaxis, 1);
  int count = 0;
  int expect[] = { 0, 3, 6, 9 };
  do {
    ck_assert_int_eq(*srcp.l, expect[count]);
    srcp.l += srci.rsinglestep[0]*srci.rdims[0];
    ++count;
  } while (advanceLoop(&srci, &srcp) < srci.rndim);
  ck_assert_int_eq(count, dims[1]); /* we did all rows */
}
END_TEST

START_TEST(second_axis_eachrow)
{
  int dims[] = { 3, 4 };
  Int iq = array_scratch(ANA_LONG, 2, dims);
  iq = ana_indgen(1, &iq);
  ck_assert_msg(iq > 0, "Cannot create array");
  int one =  1;
  Int axes = array_scratch(ANA_LONG, 1, &one);
  pointer axesp;
  axesp.v = array_data(axes);
  *axesp.l = 1;
  loopInfo srci;
  pointer srcp;
  ck_assert_int_eq(standardLoop(iq, axes, SL_EACHROW, ANA_LONG, &srci, &srcp, NULL, NULL, NULL), ANA_OK);
  ck_assert_int_eq(srci.ndim, sizeof(dims)/sizeof(*dims));
  ck_assert_int_eq(srci.nelem, dims[0]*dims[1]);
  int i;
  for (i = 0; i < srci.ndim; i++)
    ck_assert_int_eq(srci.coords[i], 0);
  ck_assert_int_eq(srci.dims[0], dims[0]);
  ck_assert_int_eq(srci.dims[1], dims[1]);
  ck_assert_int_eq(srci.naxes, 1);
  ck_assert_int_eq(srci.axes[0], 1);
  ck_assert_int_eq(srci.singlestep[0], 1);
  ck_assert_int_eq(srci.singlestep[1], dims[0]);
  ck_assert_int_eq(srci.rndim, 2);
  ck_assert_int_eq(srci.rdims[0], dims[1]);
  ck_assert_int_eq(srci.rdims[1], dims[0]);
  ck_assert_int_eq(srci.rsinglestep[0], dims[0]);
  ck_assert_int_eq(srci.rsinglestep[1], 1);
  ck_assert_int_eq(srci.step[0], dims[0]);
  ck_assert_int_eq(srci.step[1], 1 - dims[0]*dims[1]);
  ck_assert_int_eq(srci.raxes[0], 1);
  ck_assert_int_eq(srci.raxes[1], 0);
  ck_assert_int_eq(srci.iraxes[0], 1);
  ck_assert_int_eq(srci.iraxes[1], 0);
  ck_assert_int_eq(srci.axisindex, 0);
  ck_assert_int_eq(srci.advanceaxis, 1);
  int count = 0;
  int expect[] = { 0, 1, 2 };
  do {
    ck_assert_int_eq(*srcp.l, expect[count]);
    srcp.l += srci.rsinglestep[0]*srci.rdims[0];
    ++count;
  } while (advanceLoop(&srci, &srcp) < srci.rndim);
  ck_assert_int_eq(count, dims[0]); /* we did all columns */
}
END_TEST

START_TEST(first_then_second_axis_eachrow)
{
  int dims[] = { 3, 4 };
  Int iq = array_scratch(ANA_LONG, 2, dims);
  iq = ana_indgen(1, &iq);
  ck_assert_msg(iq > 0, "Cannot create array");
  loopInfo srci;
  pointer srcp;
  ck_assert_int_eq(standardLoop(iq, 0, SL_ALLAXES | SL_EACHROW, ANA_LONG, &srci, &srcp, NULL, NULL, NULL), ANA_OK);
  ck_assert_int_eq(srci.ndim, sizeof(dims)/sizeof(*dims));
  ck_assert_int_eq(srci.nelem, dims[0]*dims[1]);
  int i;
  for (i = 0; i < srci.ndim; i++)
    ck_assert_int_eq(srci.coords[i], 0);
  ck_assert_int_eq(srci.dims[0], dims[0]);
  ck_assert_int_eq(srci.dims[1], dims[1]);
  ck_assert_int_eq(srci.naxes, 2);
  ck_assert_int_eq(srci.axes[0], 0);
  ck_assert_int_eq(srci.axes[1], 1);
  ck_assert_int_eq(srci.singlestep[0], 1);
  ck_assert_int_eq(srci.singlestep[1], dims[0]);
  ck_assert_int_eq(srci.rndim, 2);
  ck_assert_int_eq(srci.rdims[0], dims[0]);
  ck_assert_int_eq(srci.rdims[1], dims[1]);
  ck_assert_int_eq(srci.rsinglestep[0], 1);
  ck_assert_int_eq(srci.rsinglestep[1], dims[0]);
  ck_assert_int_eq(srci.step[0], 1);
  ck_assert_int_eq(srci.step[1], 0);
  ck_assert_int_eq(srci.raxes[0], 0);
  ck_assert_int_eq(srci.raxes[1], 1);
  ck_assert_int_eq(srci.iraxes[0], 0);
  ck_assert_int_eq(srci.iraxes[1], 1);
  ck_assert_int_eq(srci.axisindex, 0);
  ck_assert_int_eq(srci.advanceaxis, 1);
  int count = 0;
  int expect1[] = { 0, 3, 6, 9 };
  do {
    ck_assert_int_eq(*srcp.l, expect1[count]);
    srcp.l += srci.rsinglestep[0]*srci.rdims[0];
    ++count;
  } while (advanceLoop(&srci, &srcp) < srci.rndim);
  ck_assert_int_eq(count, dims[1]); /* we did all rows */
  int one =  1;
  setAxes(&srci, 1, &one, SL_EACHROW);
  ck_assert_int_eq(srci.ndim, sizeof(dims)/sizeof(*dims));
  ck_assert_int_eq(srci.nelem, dims[0]*dims[1]);
  for (i = 0; i < srci.ndim; i++)
    ck_assert_int_eq(srci.coords[i], 0);
  ck_assert_int_eq(srci.dims[0], dims[0]);
  ck_assert_int_eq(srci.dims[1], dims[1]);
  ck_assert_int_eq(srci.naxes, 1);
  ck_assert_int_eq(srci.axes[0], 1);
  ck_assert_int_eq(srci.singlestep[0], 1);
  ck_assert_int_eq(srci.singlestep[1], dims[0]);
  ck_assert_int_eq(srci.rndim, 2);
  ck_assert_int_eq(srci.rdims[0], dims[1]);
  ck_assert_int_eq(srci.rdims[1], dims[0]);
  ck_assert_int_eq(srci.rsinglestep[0], dims[0]);
  ck_assert_int_eq(srci.rsinglestep[1], 1);
  ck_assert_int_eq(srci.step[0], dims[0]);
  ck_assert_int_eq(srci.step[1], 1 - dims[0]*dims[1]);
  ck_assert_int_eq(srci.raxes[0], 1);
  ck_assert_int_eq(srci.raxes[1], 0);
  ck_assert_int_eq(srci.iraxes[0], 1);
  ck_assert_int_eq(srci.iraxes[1], 0);
  ck_assert_int_eq(srci.axisindex, 0);
  ck_assert_int_eq(srci.advanceaxis, 1);
  count = 0;
  int expect2[] = { 0, 1, 2 };
  do {
    ck_assert_int_eq(*srcp.l, expect2[count]);
    srcp.l += srci.rsinglestep[0]*srci.rdims[0];
    ++count;
  } while (advanceLoop(&srci, &srcp) < srci.rndim);
  ck_assert_int_eq(count, dims[0]); /* we did all columns */
}
END_TEST

Suite *test_suite(void)
{
  Suite *s = suite_create(PACKAGE " test suite");

  TCase *tc_axis = tcase_create("axis");
  tcase_add_test(tc_axis, first_axis);
  tcase_add_test(tc_axis, second_axis);
  tcase_add_test(tc_axis, first_then_second_axis);
  suite_add_tcase(s, tc_axis);

  TCase *tc_axis_eachrow = tcase_create("axis eachrow");
  tcase_add_test(tc_axis_eachrow, first_axis_eachrow);
  tcase_add_test(tc_axis_eachrow, second_axis_eachrow);
  tcase_add_test(tc_axis_eachrow, first_then_second_axis_eachrow);
  suite_add_tcase(s, tc_axis_eachrow);
  return s;
}

int main(void)
{
  Suite *s = test_suite();
  SRunner *sr = srunner_create(s);
  srunner_run_all(sr, CK_NORMAL);
  int number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0)? EXIT_SUCCESS: EXIT_FAILURE;
}
