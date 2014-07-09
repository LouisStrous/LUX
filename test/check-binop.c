/* This is file check-binop.c.

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
#include <check.h>
#include <stdlib.h>		/* for EXIT_SUCCESS, EXIT_FAILURE */

#include "config.h"
#include "luxparser.h"
#include "action.h"
#include "axis.h"

int32_t prepare_array(int32_t num_elem, int64_t *values)
{
  int32_t symbol = array_scratch(LUX_INT64, 1, &num_elem);
  int64_t* ptr = (int64_t*) array_data(symbol);
  memcpy(ptr, values, sizeof(*values)*num_elem);
  return symbol;
}

int32_t prepare_scalar(int64_t value)
{
  int32_t symbol = nextFreeNamedVariable();
  symbol_class(symbol) = LUX_SCALAR;
  symbol_type(symbol) = LUX_INT64;
  scalar_value(symbol).q = value;
  return symbol;
}

/* These are basic tests of binary operators */
START_TEST(binop_array)
{
  /* We test with arrays of 2 elements, so we can check that the
     correct step size is used to step through the array.  The
     elements are such that their square is small enough to fit in the
     smallest data type (LUX_INT8), so that none of the arithmetic
     binary operators overflows and the results can be fairly easily
     checked without needing code for each combination of types for
     the left-hand and right-hand arguments, and their power-of is
     small enough to fit in an int64_t, so that the expected results
     can be represented exactly. */

  int64_t lv_aa[2] = { 97, 13 };
  int64_t rv_aa[2] = { 2, 16 };

  int64_t lv_as[2] = { 44, 11 };
  int64_t rv_as = 11;

  struct test {
    binaryOp op;
    int64_t expected_values_aa[2];
    int64_t expected_values_as[2];
  } tests[] = {
    { LUX_ADD, { lv_aa[0] + rv_aa[0], lv_aa[1] + rv_aa[1] },
      { lv_as[0] + rv_as, lv_as[1] + rv_as } },
    { LUX_SUB, { lv_aa[0] - rv_aa[0], lv_aa[1] - rv_aa[1] },
      { lv_as[0] - rv_as, lv_as[1] - rv_as } },
    { LUX_MUL, { lv_aa[0] * rv_aa[0], lv_aa[1] * rv_aa[1] },
      { lv_as[0] * rv_as, lv_as[1] * rv_as } },
    { LUX_DIV, { lv_aa[0] / rv_aa[0], lv_aa[1] / rv_aa[1] },
      { lv_as[0] / rv_as, lv_as[1] / rv_as } },
    { LUX_IDIV, { lv_aa[0] / rv_aa[0], lv_aa[1] / rv_aa[1] },
      { lv_as[0] / rv_as, lv_as[1] / rv_as } },
    { LUX_MOD,  { lv_aa[0] % rv_aa[0], lv_aa[1] % rv_aa[1] },
      { lv_as[0] % rv_as, lv_as[1] % rv_as } },
    /* LUX_SMOD: symmetric modulus a smod b is like a%b but between
       -b/2 and +b/2 */
    { LUX_SMOD, { lv_aa[0] % rv_aa[0], lv_aa[1] % rv_aa[1] - rv_aa[1] },
      { lv_as[0] % rv_as, lv_as[1] % rv_as } },
    /* LUX_MAX: greatest of the arguments */
    { LUX_MAX, { (lv_aa[0] > rv_aa[0]? lv_aa[0]: rv_aa[0]),
                 (lv_aa[1] > rv_aa[1]? lv_aa[1]: rv_aa[1]) },
      { (lv_as[0] > rv_as? lv_as[0]: rv_as),
        (lv_as[1] > rv_as? lv_as[1]: rv_as) } },
    /* LUX_MIN: least of the arguments */
    { LUX_MIN, { (lv_aa[0] < rv_aa[0]? lv_aa[0]: rv_aa[0]),
                 (lv_aa[1] < rv_aa[1]? lv_aa[1]: rv_aa[1]) },
      { (lv_as[0] < rv_as? lv_as[0]: rv_as),
        (lv_as[1] < rv_as? lv_as[1]: rv_as) } },
    { LUX_EQ, { (lv_aa[0] == rv_aa[0]), (lv_aa[1] == rv_aa[1]) },
      { (lv_as[0] == rv_as), (lv_as[1] == rv_as) } },
    { LUX_GT, { (lv_aa[0] > rv_aa[0]), (lv_aa[1] > rv_aa[1]) },
      { (lv_as[0] > rv_as), (lv_as[1] > rv_as) } },
    { LUX_GE, { (lv_aa[0] >= rv_aa[0]), (lv_aa[1] >= rv_aa[1]) },
      { (lv_as[0] >= rv_as), (lv_as[1] >= rv_as) } },
    { LUX_LT, { (lv_aa[0] < rv_aa[0]), (lv_aa[1] < rv_aa[1]) },
      { (lv_as[0] < rv_as), (lv_as[1] < rv_as) } },
    { LUX_LE, { (lv_aa[0] <= rv_aa[0]), (lv_aa[1] <= rv_aa[1]) },
      { (lv_as[0] <= rv_as), (lv_as[1] <= rv_as) } },
    { LUX_NE, { (lv_aa[0] != rv_aa[0]), (lv_aa[1] != rv_aa[1]) },
      { (lv_as[0] != rv_as), (lv_as[1] != rv_as) } },
    { LUX_OR, { (lv_aa[0] | rv_aa[0]), (lv_aa[1] | rv_aa[1]) },
      { (lv_as[0] | rv_as), (lv_as[1] | rv_as) } },
    { LUX_AND, { (lv_aa[0] & rv_aa[0]), (lv_aa[1] & rv_aa[1]) },
      { (lv_as[0] & rv_as), (lv_as[1] & rv_as) } },
    { LUX_XOR, { (lv_aa[0] ^ rv_aa[0]), (lv_aa[1] ^ rv_aa[1]) },
      { (lv_as[0] ^ rv_as), (lv_as[1] ^ rv_as) } },
    { LUX_POW,  { 9409, 665416609183179841 },
      { 1196683881290399744, 285311670611 } },
  };

  size_t n = sizeof(lv_aa)/sizeof(*lv_aa);

  Symboltype types[] = { LUX_INT8, LUX_INT16, LUX_INT32, LUX_INT64,
                         LUX_FLOAT, LUX_DOUBLE };

  int num_types = sizeof(types)/sizeof(*types);

  int lhs_itype, rhs_itype;
  for (lhs_itype = 0; lhs_itype < num_types; ++lhs_itype) {
    for (rhs_itype = 0; rhs_itype < num_types; ++rhs_itype) {

      int32_t lhs_basic_sym_aa = prepare_array(n, lv_aa);
      int32_t rhs_basic_sym_aa = prepare_array(n, rv_aa);
      ck_assert_int_ne(lhs_basic_sym_aa, -1);
      ck_assert_int_ne(rhs_basic_sym_aa, -1);

      int32_t lhs_basic_sym_as = prepare_array(n, lv_as);
      int32_t rhs_basic_sym_as = prepare_scalar(rv_as);
      ck_assert_int_ne(lhs_basic_sym_as, -1);
      ck_assert_int_ne(rhs_basic_sym_as, -1);

      Symboltype lhs_type = types[lhs_itype];

      int32_t lhs_sym_aa = lux_converts[lhs_type](1, &lhs_basic_sym_aa);
      ck_assert_int_ne(lhs_sym_aa, -1);

      int32_t lhs_sym_as = lux_converts[lhs_type](1, &lhs_basic_sym_as);
      ck_assert_int_ne(lhs_sym_as, -1);

      Symboltype rhs_type = types[rhs_itype];

      int32_t rhs_sym_aa = lux_converts[rhs_type](1, &rhs_basic_sym_aa);
      ck_assert_int_ne(rhs_sym_aa, -1);

      int32_t op_sym_aa = newSymbol(LUX_BIN_OP, 0, lhs_sym_aa, rhs_sym_aa);
      ck_assert_int_ne(op_sym_aa, -1);

      /* prevent these symbols from being deleted when op_sym_aa is
         deleted */
      symbol_context(lhs_sym_aa) = curContext + 1;
      symbol_context(rhs_sym_aa) = curContext + 1;

      int32_t rhs_sym_as = lux_converts[rhs_type](1, &rhs_basic_sym_as);
      ck_assert_int_ne(rhs_sym_as, -1);

      int32_t op_sym_as = newSymbol(LUX_BIN_OP, 0, lhs_sym_as, rhs_sym_as);
      ck_assert_int_ne(op_sym_as, -1);

      /* prevent these symbols from being deleted when op_sym_aa is
         deleted */
      symbol_context(lhs_sym_as) = curContext + 1;
      symbol_context(rhs_sym_as) = curContext + 1;

      int32_t iop;
      for (iop = 0; iop < sizeof(tests)/sizeof(*tests); ++iop) {
        switch (tests[iop].op) {
        case LUX_OR: case LUX_AND: case LUX_XOR:
          if (!isIntegerType(lhs_type) || !isIntegerType(rhs_type))
            continue;
          break;
        }

        extern char* binOpName[];
        printf("%s %s %s\n", typeName(lhs_type), binOpName[iop], typeName(rhs_type));

        bin_op_type(op_sym_aa) = tests[iop].op;
        bin_op_type(op_sym_as) = tests[iop].op;

        int32_t result_sym_aa = eval(op_sym_aa);    /* evaluate operation */
        ck_assert_int_ne(result_sym_aa, -1);
        ck_assert_int_eq(symbol_class(result_sym_aa), LUX_ARRAY);

        int32_t result_sym_as = eval(op_sym_as);    /* evaluate operation */
        ck_assert_int_ne(result_sym_as, -1);
        ck_assert_int_eq(symbol_class(result_sym_as), LUX_ARRAY);

        Symboltype expected_type;
        switch (tests[iop].op) {
        case LUX_EQ: case LUX_NE: case LUX_GT: case LUX_LE:
        case LUX_LT: case LUX_GE:
          expected_type = LUX_INT32;
          break;
        case LUX_POW:
          expected_type = combinedType(lhs_type, rhs_type);
          expected_type = combinedType(expected_type, LUX_FLOAT);
          break;
        default:
          expected_type = combinedType(lhs_type, rhs_type);
          break;
        }
        ck_assert_int_eq(symbol_type(result_sym_aa), expected_type);
        ck_assert_int_eq(symbol_type(result_sym_as), expected_type);

        pointer result_ptr_aa;
        result_ptr_aa.v = array_data(result_sym_aa);
        pointer result_ptr_as;
        result_ptr_as.v = array_data(result_sym_as);

        int32_t expected_sym_aa = prepare_array(n, tests[iop].expected_values_aa);
        expected_sym_aa = lux_converts[expected_type](1, &expected_sym_aa);
        pointer expected_ptr_aa;
        expected_ptr_aa.v = array_data(expected_sym_aa);

        int32_t expected_sym_as = prepare_array(n, tests[iop].expected_values_as);
        expected_sym_as = lux_converts[expected_type](1, &expected_sym_as);
        pointer expected_ptr_as;
        expected_ptr_as.v = array_data(expected_sym_as);

        switch (tests[iop].op) {
        case LUX_DIV:
          switch (expected_type) {
          case LUX_FLOAT:
            expected_ptr_aa.f[0] = (float) lv_aa[0]/rv_aa[0];
            expected_ptr_aa.f[1] = (float) lv_aa[1]/rv_aa[1];
            expected_ptr_as.f[0] = (float) lv_as[0]/rv_as;
            expected_ptr_as.f[1] = (float) lv_as[1]/rv_as;
            break;
          case LUX_DOUBLE:
            expected_ptr_aa.d[0] = (double) lv_aa[0]/rv_aa[0];
            expected_ptr_aa.d[1] = (double) lv_aa[1]/rv_aa[1];
            expected_ptr_as.d[0] = (double) lv_as[0]/rv_as;
            expected_ptr_as.d[1] = (double) lv_as[1]/rv_as;
            break;
          }
          break;
        }

        int32_t i;
        for (i = 0; i < n*lux_type_size[expected_type]; ++i) {
          if (result_ptr_aa.b[i] != expected_ptr_aa.b[i]) {
            printf("Discrepancy at byte %d: expected %u but found %u\n",
                   i, result_ptr_aa.b[i], expected_ptr_aa.b[i]);
            printf("EXPECT: ");
            lux_dump_one(expected_sym_aa, 2);
            printf("GOT   : ");
            lux_dump_one(result_sym_aa, 2);
          }
          ck_assert_int_eq(result_ptr_aa.b[i], expected_ptr_aa.b[i]);

          if (result_ptr_as.b[i] != expected_ptr_as.b[i]) {
            printf("Discrepancy at byte %d: expected %u but found %u\n",
                   i, result_ptr_as.b[i], expected_ptr_as.b[i]);
            printf("EXPECT: ");
            lux_dump_one(expected_sym_as, 2);
            printf("GOT   : ");
            lux_dump_one(result_sym_as, 2);
          }
          ck_assert_int_eq(result_ptr_as.b[i], expected_ptr_as.b[i]);
        }
        zap(result_sym_aa);
        zap(result_sym_as);
        zap(expected_sym_aa);
        zap(expected_sym_as);
      }
      zap(op_sym_aa);
      zap(op_sym_as);
      zap(rhs_sym_aa);
      zap(rhs_sym_as);
      zap(lhs_sym_aa);
      zap(lhs_sym_as);
      zap(lhs_basic_sym_aa);
      zap(rhs_basic_sym_aa);
      zap(lhs_basic_sym_as);
      zap(rhs_basic_sym_as);
    }
  }
}
END_TEST

Suite *binop_suite(void)
{
  Suite *s = suite_create("binop suite");

  TCase *tc_binop = tcase_create("binop");
  tcase_add_test(tc_binop, binop_array);

  suite_add_tcase(s, tc_binop);
  return s;
}

int main(void)
{
  SRunner *sr = srunner_create(binop_suite());
  //  srunner_add_suite(sr, X_suite());
  srunner_run_all(sr, CK_NORMAL);
  int number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0)? EXIT_SUCCESS: EXIT_FAILURE;
}
