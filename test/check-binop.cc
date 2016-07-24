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
#include <cppunit/extensions/HelperMacros.h>
#include <stdlib.h>		/* for EXIT_SUCCESS, EXIT_FAILURE */
#include <string.h>             // for memcpy

#include "config.h"
#include "luxparser.hh"
#include "action.hh"
#include "axis.hh"

class BinopTest
  : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BinopTest);
  CPPUNIT_TEST(binop_array);
  CPPUNIT_TEST_SUITE_END();
public:
  void binop_array();
};

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

#define ck_assert(C) CPPUNIT_ASSERT(C)
#define ck_assert_msg(T,M) CPPUNIT_ASSERT(T)
#define ck_assert_int_eq(L,R) CPPUNIT_ASSERT((L) == (R))
#define ck_assert_uint_eq(L,R) CPPUNIT_ASSERT((L) == (R))
#define ck_assert_int_ne(L,R) CPPUNIT_ASSERT((L) != (R))
#define fail_unless(C,M,...) CPPUNIT_ASSERT(C)

int32_t newSymbol(Symbolclass, ...);
int32_t lux_dump_one(int32_t, int32_t);

/* These are basic tests of binary operators */
void
BinopTest::binop_array()
{
  /* We test with arrays of 2 elements, so we can check that the
     correct step size is used to step through the array.  The
     elements are such that their square is small enough to fit in the
     smallest data type (LUX_INT8), so that none of the arithmetic
     binary operators overflows and the results can be fairly easily
     checked without needing code for each combination of types for
     the left-hand and right-hand arguments, and their power-of is
     small enough to fit in an int64_t, so that the expected results
     can be represented exactly.  All numbers are positive integers,
     because LUX_INT8 is an unsigned type and the current testing
     scheme doesn't easily incorporate both unsigned and negative
     integers. */

  int64_t lv_aa[] = { 97, 13, 47 };
  int64_t rv_aa[] = {  2, 16,  5 };

  int64_t lv_as[] = { 44, 11, 3 };
  int64_t rv_as = 11;

  int64_t lv_sa = 7;
  int64_t rv_sa[] = { 20, 7, 5 };

  struct test {
    binaryOp op;
    int64_t expected_values_aa[3];
    int64_t expected_values_as[3];
    int64_t expected_values_sa[3];
   } tests[19];

  int t = 0;
  tests[t].op = LUX_ADD;
  int i;
  for (i = 0; i < 3; ++i) {
    tests[t].expected_values_aa[i] = lv_aa[i] + rv_aa[i];
    tests[t].expected_values_as[i] = lv_as[i] + rv_as;
    tests[t].expected_values_sa[i] = lv_sa + rv_sa[i];
  }

  tests[++t].op = LUX_SUB;
  for (i = 0; i < 3; ++i) {
    tests[t].expected_values_aa[i] = lv_aa[i] - rv_aa[i];
    tests[t].expected_values_as[i] = lv_as[i] - rv_as;
    tests[t].expected_values_sa[i] = lv_sa - rv_sa[i];
  }

  tests[++t].op = LUX_MUL;
  for (i = 0; i < 3; ++i) {
    tests[t].expected_values_aa[i] = lv_aa[i] * rv_aa[i];
    tests[t].expected_values_as[i] = lv_as[i] * rv_as;
    tests[t].expected_values_sa[i] = lv_sa * rv_sa[i];
  }

  tests[++t].op = LUX_DIV;
  for (i = 0; i < 3; ++i) {
    tests[t].expected_values_aa[i] = lv_aa[i] / rv_aa[i];
    tests[t].expected_values_as[i] = lv_as[i] / rv_as;
    tests[t].expected_values_sa[i] = lv_sa / rv_sa[i];
  }

  tests[++t].op = LUX_IDIV;
  for (i = 0; i < 3; ++i) {
    tests[t].expected_values_aa[i] = lv_aa[i] / rv_aa[i];
    tests[t].expected_values_as[i] = lv_as[i] / rv_as;
    tests[t].expected_values_sa[i] = lv_sa / rv_sa[i];
  }

  tests[++t].op = LUX_MOD;
  for (i = 0; i < 3; ++i) {
    tests[t].expected_values_aa[i] = lv_aa[i] % rv_aa[i];
    tests[t].expected_values_as[i] = lv_as[i] % rv_as;
    tests[t].expected_values_sa[i] = lv_sa % rv_sa[i];
  }

  tests[++t].op = LUX_SMOD;
  for (i = 0; i < 3; ++i) {
    int32_t v = lv_aa[i] % rv_aa[i];
    if (2*v > rv_aa[i])
      v -= rv_aa[i];
    tests[t].expected_values_aa[i] = v;
    v = lv_as[i] % rv_as;
    if (2*v > rv_as)
      v -= rv_as;
    tests[t].expected_values_as[i] = v;
    v = lv_sa % rv_sa[i];
    if (2*v > rv_sa[i])
      v -= rv_sa[i];
    tests[t].expected_values_sa[i] = v;
  }

  tests[++t].op = LUX_MAX;
  for (i = 0; i < 3; ++i) {
    tests[t].expected_values_aa[i] = (lv_aa[i] > rv_aa[i]? lv_aa[i]: rv_aa[i]);
    tests[t].expected_values_as[i] = (lv_as[i] > rv_as? lv_as[i]: rv_as);
    tests[t].expected_values_sa[i] = (lv_sa > rv_sa[i]? lv_sa: rv_sa[i]);
  }

  tests[++t].op = LUX_MIN;
  for (i = 0; i < 3; ++i) {
    tests[t].expected_values_aa[i] = (lv_aa[i] < rv_aa[i]? lv_aa[i]: rv_aa[i]);
    tests[t].expected_values_as[i] = (lv_as[i] < rv_as? lv_as[i]: rv_as);
    tests[t].expected_values_sa[i] = (lv_sa < rv_sa[i]? lv_sa: rv_sa[i]);
  }

  tests[++t].op = LUX_EQ;
  for (i = 0; i < 3; ++i) {
    tests[t].expected_values_aa[i] = (lv_aa[i] == rv_aa[i]);
    tests[t].expected_values_as[i] = (lv_as[i] == rv_as);
    tests[t].expected_values_sa[i] = (lv_sa == rv_sa[i]);
  }

  tests[++t].op = LUX_GT;
  for (i = 0; i < 3; ++i) {
    tests[t].expected_values_aa[i] = (lv_aa[i] > rv_aa[i]);
    tests[t].expected_values_as[i] = (lv_as[i] > rv_as);
    tests[t].expected_values_sa[i] = (lv_sa > rv_sa[i]);
  }

  tests[++t].op = LUX_GE;
  for (i = 0; i < 3; ++i) {
    tests[t].expected_values_aa[i] = (lv_aa[i] >= rv_aa[i]);
    tests[t].expected_values_as[i] = (lv_as[i] >= rv_as);
    tests[t].expected_values_sa[i] = (lv_sa >= rv_sa[i]);
  }

  tests[++t].op = LUX_LT;
  for (i = 0; i < 3; ++i) {
    tests[t].expected_values_aa[i] = (lv_aa[i] < rv_aa[i]);
    tests[t].expected_values_as[i] = (lv_as[i] < rv_as);
    tests[t].expected_values_sa[i] = (lv_sa < rv_sa[i]);
  }

  tests[++t].op = LUX_LE;
  for (i = 0; i < 3; ++i) {
    tests[t].expected_values_aa[i] = (lv_aa[i] <= rv_aa[i]);
    tests[t].expected_values_as[i] = (lv_as[i] <= rv_as);
    tests[t].expected_values_sa[i] = (lv_sa <= rv_sa[i]);
  }

  tests[++t].op = LUX_NE;
  for (i = 0; i < 3; ++i) {
    tests[t].expected_values_aa[i] = (lv_aa[i] != rv_aa[i]);
    tests[t].expected_values_as[i] = (lv_as[i] != rv_as);
    tests[t].expected_values_sa[i] = (lv_sa != rv_sa[i]);
  }

  tests[++t].op = LUX_OR;
  for (i = 0; i < 3; ++i) {
    tests[t].expected_values_aa[i] = (lv_aa[i] | rv_aa[i]);
    tests[t].expected_values_as[i] = (lv_as[i] | rv_as);
    tests[t].expected_values_sa[i] = (lv_sa | rv_sa[i]);
  }

  tests[++t].op = LUX_AND;
  for (i = 0; i < 3; ++i) {
    tests[t].expected_values_aa[i] = (lv_aa[i] & rv_aa[i]);
    tests[t].expected_values_as[i] = (lv_as[i] & rv_as);
    tests[t].expected_values_sa[i] = (lv_sa & rv_sa[i]);
  }

  tests[++t].op = LUX_XOR;
  for (i = 0; i < 3; ++i) {
    tests[t].expected_values_aa[i] = (lv_aa[i] ^ rv_aa[i]);
    tests[t].expected_values_as[i] = (lv_as[i] ^ rv_as);
    tests[t].expected_values_sa[i] = (lv_sa ^ rv_sa[i]);
  }

  tests[++t].op = LUX_POW;
  tests[t].expected_values_aa[0] = 9409;                /* 97^2 */
  tests[t].expected_values_aa[1] = 665416609183179841;  /* 13^16 */
  tests[t].expected_values_aa[2] = 229345007;           /* 47^5 */
  tests[t].expected_values_as[0] = 1196683881290399744; /* 44^11 */
  tests[t].expected_values_as[1] = 285311670611;        /* 11^11 */
  tests[t].expected_values_as[2] = 177147;              /* 3^11 */
  tests[t].expected_values_sa[0] = 79792266297612001;   /* 7^20 */
  tests[t].expected_values_sa[1] = 823543;              /* 7^7 */
  tests[t].expected_values_sa[2] = 16807;               /* 7^5 */

  size_t n = sizeof(lv_aa)/sizeof(*lv_aa);

  Symboltype types[] = { LUX_INT8, LUX_INT16, LUX_INT32, LUX_INT64,
                         LUX_FLOAT, LUX_DOUBLE, LUX_CFLOAT };

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

      int32_t lhs_basic_sym_sa = prepare_scalar(lv_sa);
      int32_t rhs_basic_sym_sa = prepare_array(n, rv_sa);
      ck_assert_int_ne(lhs_basic_sym_sa, -1);
      ck_assert_int_ne(rhs_basic_sym_sa, -1);

      Symboltype lhs_type = types[lhs_itype];

      int32_t lhs_sym_aa = lux_converts[lhs_type](1, &lhs_basic_sym_aa);
      ck_assert_int_ne(lhs_sym_aa, -1);

      int32_t lhs_sym_as = lux_converts[lhs_type](1, &lhs_basic_sym_as);
      ck_assert_int_ne(lhs_sym_as, -1);

      int32_t lhs_sym_sa = lux_converts[lhs_type](1, &lhs_basic_sym_sa);
      ck_assert_int_ne(lhs_sym_sa, -1);

      Symboltype rhs_type = types[rhs_itype];

      int32_t rhs_sym_aa = lux_converts[rhs_type](1, &rhs_basic_sym_aa);
      ck_assert_int_ne(rhs_sym_aa, -1);

      int32_t op_sym_aa = newSymbol(LUX_BIN_OP, 0, lhs_sym_aa, rhs_sym_aa);
      ck_assert_int_ne(op_sym_aa, -1);

      int32_t rhs_sym_as = lux_converts[rhs_type](1, &rhs_basic_sym_as);
      ck_assert_int_ne(rhs_sym_as, -1);

      int32_t op_sym_as = newSymbol(LUX_BIN_OP, 0, lhs_sym_as, rhs_sym_as);
      ck_assert_int_ne(op_sym_as, -1);

      int32_t rhs_sym_sa = lux_converts[rhs_type](1, &rhs_basic_sym_sa);
      ck_assert_int_ne(rhs_sym_sa, -1);

      int32_t op_sym_sa = newSymbol(LUX_BIN_OP, 0, lhs_sym_sa, rhs_sym_sa);
      ck_assert_int_ne(op_sym_sa, -1);

      int32_t iop;
      for (iop = 0; iop < sizeof(tests)/sizeof(*tests); ++iop) {
        switch (tests[iop].op) {
        case LUX_OR: case LUX_AND: case LUX_XOR:
          if (!isIntegerType(lhs_type) || !isIntegerType(rhs_type))
            continue;
          break;
        case LUX_IDIV:
          if (isComplexType(lhs_type) || isComplexType(rhs_type))
            continue;
          break;
        }

        extern char* binOpName[];
        //printf("%s %s %s\n", typeName(lhs_type), binOpName[iop], typeName(rhs_type));

        bin_op_type(op_sym_aa) = tests[iop].op;
        bin_op_type(op_sym_as) = tests[iop].op;
        bin_op_type(op_sym_sa) = tests[iop].op;

        int32_t result_sym_aa = eval(op_sym_aa);    /* evaluate operation */
        ck_assert_int_ne(result_sym_aa, -1);
        ck_assert_int_eq(symbolIsArray(result_sym_aa), 1);

        int32_t result_sym_as = eval(op_sym_as);    /* evaluate operation */
        ck_assert_int_ne(result_sym_as, -1);
        ck_assert_int_eq(symbolIsArray(result_sym_as), 1);

        int32_t result_sym_sa = eval(op_sym_sa);    /* evaluate operation */
        ck_assert_int_ne(result_sym_sa, -1);
        ck_assert_int_eq(symbolIsArray(result_sym_sa), 1);

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
        ck_assert_int_eq(symbol_type(result_sym_sa), expected_type);

        pointer result_ptr_aa;
        result_ptr_aa.v = array_data(result_sym_aa);
        pointer result_ptr_as;
        result_ptr_as.v = array_data(result_sym_as);
        pointer result_ptr_sa;
        result_ptr_sa.v = array_data(result_sym_sa);

        int32_t expected_sym_aa = prepare_array(n, tests[iop].expected_values_aa);
        expected_sym_aa = lux_converts[expected_type](1, &expected_sym_aa);
        pointer expected_ptr_aa;
        expected_ptr_aa.v = array_data(expected_sym_aa);

        int32_t expected_sym_as = prepare_array(n, tests[iop].expected_values_as);
        expected_sym_as = lux_converts[expected_type](1, &expected_sym_as);
        pointer expected_ptr_as;
        expected_ptr_as.v = array_data(expected_sym_as);

        int32_t expected_sym_sa = prepare_array(n, tests[iop].expected_values_sa);
        expected_sym_sa = lux_converts[expected_type](1, &expected_sym_sa);
        pointer expected_ptr_sa;
        expected_ptr_sa.v = array_data(expected_sym_sa);

        switch (tests[iop].op) {
        case LUX_DIV:
          switch (expected_type) {
          case LUX_FLOAT:
            for (i = 0; i < 3; ++i) {
              expected_ptr_aa.f[i] = (float) lv_aa[i]/rv_aa[i];
              expected_ptr_as.f[i] = (float) lv_as[i]/rv_as;
              expected_ptr_sa.f[i] = (float) lv_sa/rv_sa[i];
            }
            break;
          case LUX_DOUBLE:
            for (i = 0; i < 3; ++i) {
              expected_ptr_aa.d[i] = (double) lv_aa[i]/rv_aa[i];
              expected_ptr_as.d[i] = (double) lv_as[i]/rv_as;
              expected_ptr_sa.d[i] = (double) lv_sa/rv_sa[i];
            }
            break;
          case LUX_CFLOAT:
            for (i = 0; i < 3; ++i) {
              expected_ptr_aa.cf[i].real = (float) lv_aa[i]/rv_aa[i];
              expected_ptr_aa.cf[i].imaginary = 0;
              expected_ptr_as.cf[i].real = (float) lv_as[i]/rv_as;
              expected_ptr_as.cf[i].imaginary = 0;
              expected_ptr_sa.cf[i].real = (float) lv_sa/rv_sa[i];
              expected_ptr_sa.cf[i].imaginary = 0;
            }
            break;
          case LUX_CDOUBLE:
            for (i = 0; i < 3; ++i) {
              expected_ptr_aa.cd[i].real = (double) lv_aa[i]/rv_aa[i];
              expected_ptr_aa.cd[i].imaginary = 0;
              expected_ptr_as.cd[i].real = (double) lv_as[i]/rv_as;
              expected_ptr_as.cd[i].imaginary = 0;
              expected_ptr_sa.cd[i].real = (double) lv_sa/rv_sa[i];
              expected_ptr_sa.cd[i].imaginary = 0;
            }
            break;
          }
          break;
        }

        switch (expected_type) {
        case LUX_INT8:
          switch (tests[iop].op) {
          case LUX_DIV: case LUX_IDIV:
            for (i = 0; i < 3; ++i) {
              expected_ptr_aa.b[i] = (uint8_t) lv_aa[i]/(uint8_t) rv_aa[i];
              expected_ptr_as.b[i] = (uint8_t) lv_as[i]/(uint8_t) rv_as;
              expected_ptr_sa.b[i] = (uint8_t) lv_sa/(uint8_t) rv_sa[i];
            }
            break;
          case LUX_MOD:
            for (i = 0; i < 3; ++i) {
              expected_ptr_aa.b[i] = (uint8_t) lv_aa[i] % (uint8_t) rv_aa[i];
              expected_ptr_as.b[i] = (uint8_t) lv_as[i] % (uint8_t) rv_as;
              expected_ptr_sa.b[i] = (uint8_t) lv_sa % (uint8_t) rv_sa[i];
            }
            break;
          case LUX_SMOD:
            for (i = 0; i < 3; ++i) {
              uint8_t v = (uint8_t) lv_aa[i] % (uint8_t) rv_aa[i];
              if (2*v > rv_aa[i])
                v -= rv_aa[i];
              expected_ptr_aa.b[i] = v;
              v = (uint8_t) lv_as[i] % (uint8_t) rv_as;
              if (2*v > rv_as)
                v -= rv_as;
              expected_ptr_as.b[i] = v;
              v = (uint8_t) lv_sa % (uint8_t) rv_sa[i];
              if (2*v > rv_sa[i])
                v -= rv_sa[i];
              expected_ptr_sa.b[i] = v;
            }
            break;
          }
          break;
        }

        /* if we compare the results byte-for-byte with the expected
           results then we may get into trouble because +0 and -0 are
           numerically equivalent but not byte-for-byte identical */

        const int eps_tol = 20;
        int32_t i;
        switch (expected_type) {
        case LUX_INT8:
          for (i = 0; i < n; ++i) {
            if (result_ptr_aa.b[i] != expected_ptr_aa.b[i]) {
              printf("Discrepancy for aa at #%d: got %u but expected %u\n",
                     i+1, result_ptr_aa.b[i], expected_ptr_aa.b[i]);
              printf("EXPECT: ");
              lux_dump_one(expected_sym_aa, 2);
              printf("GOT   : ");
              lux_dump_one(result_sym_aa, 2);
              ck_assert_int_eq(result_ptr_aa.b[i], expected_ptr_aa.b[i]);
            }
            if (result_ptr_as.b[i] != expected_ptr_as.b[i]) {
              printf("Discrepancy for as at #%d: got %u but expected %u\n",
                     i+1, result_ptr_as.b[i], expected_ptr_as.b[i]);
              printf("EXPECT: ");
              lux_dump_one(expected_sym_as, 2);
              printf("GOT   : ");
              lux_dump_one(result_sym_as, 2);
              ck_assert_int_eq(result_ptr_as.b[i], expected_ptr_as.b[i]);
            }
            if (result_ptr_sa.b[i] != expected_ptr_sa.b[i]) {
              printf("Discrepancy for sa at #%d: got %u but expected %u\n",
                     i+1, result_ptr_sa.b[i], expected_ptr_sa.b[i]);
              printf("EXPECT: ");
              lux_dump_one(expected_sym_sa, 2);
              printf("GOT   : ");
              lux_dump_one(result_sym_sa, 2);
              ck_assert_int_eq(result_ptr_sa.b[i], expected_ptr_sa.b[i]);
            }
          }
          break;
        case LUX_INT16:
          for (i = 0; i < n; ++i) {
            if (result_ptr_aa.w[i] != expected_ptr_aa.w[i]) {
              printf("Discrepancy for aa at #%d: got %d but expected %d\n",
                     i+1, result_ptr_aa.w[i], expected_ptr_aa.w[i]);
              printf("EXPECT: ");
              lux_dump_one(expected_sym_aa, 2);
              printf("GOT   : ");
              lux_dump_one(result_sym_aa, 2);
              ck_assert_int_eq(result_ptr_aa.w[i], expected_ptr_aa.w[i]);
            }
            if (result_ptr_as.w[i] != expected_ptr_as.w[i]) {
              printf("Discrepancy for as at #%d: got %d but expected %d\n",
                     i+1, result_ptr_as.w[i], expected_ptr_as.w[i]);
              printf("EXPECT: ");
              lux_dump_one(expected_sym_as, 2);
              printf("GOT   : ");
              lux_dump_one(result_sym_as, 2);
              ck_assert_int_eq(result_ptr_as.w[i], expected_ptr_as.w[i]);
            }
            if (result_ptr_sa.w[i] != expected_ptr_sa.w[i]) {
              printf("Discrepancy for sa at #%d: got %d but expected %d\n",
                     i+1, result_ptr_sa.w[i], expected_ptr_sa.w[i]);
              printf("EXPECT: ");
              lux_dump_one(expected_sym_sa, 2);
              printf("GOT   : ");
              lux_dump_one(result_sym_sa, 2);
              ck_assert_int_eq(result_ptr_sa.w[i], expected_ptr_sa.w[i]);
            }
          }
          break;
        case LUX_INT32:
          for (i = 0; i < n; ++i) {
            if (result_ptr_aa.l[i] != expected_ptr_aa.l[i]) {
              printf("Discrepancy for aa at #%d: got %d but expected %d\n",
                     i+1, result_ptr_aa.l[i], expected_ptr_aa.l[i]);
              printf("EXPECT: ");
              lux_dump_one(expected_sym_aa, 2);
              printf("GOT   : ");
              lux_dump_one(result_sym_aa, 2);
              ck_assert_uint_eq(result_ptr_aa.l[i], expected_ptr_aa.l[i]);
            }
            if (result_ptr_as.l[i] != expected_ptr_as.l[i]) {
              printf("Discrepancy for as at #%d: got %d but expected %d\n",
                     i+1, result_ptr_as.l[i], expected_ptr_as.l[i]);
              printf("EXPECT: ");
              lux_dump_one(expected_sym_as, 2);
              printf("GOT   : ");
              lux_dump_one(result_sym_as, 2);
              ck_assert_uint_eq(result_ptr_as.l[i], expected_ptr_as.l[i]);
            }
            if (result_ptr_sa.l[i] != expected_ptr_sa.l[i]) {
              printf("Discrepancy for sa at #%d: got %d but expected %d\n",
                     i+1, result_ptr_sa.l[i], expected_ptr_sa.l[i]);
              printf("EXPECT: ");
              lux_dump_one(expected_sym_sa, 2);
              printf("GOT   : ");
              lux_dump_one(result_sym_sa, 2);
              ck_assert_uint_eq(result_ptr_sa.l[i], expected_ptr_sa.l[i]);
            }
          }
          break;
        case LUX_INT64:
          for (i = 0; i < n; ++i) {
            if (result_ptr_aa.q[i] != expected_ptr_aa.q[i]) {
              printf("Discrepancy for aa at #%d: got %d but expected %d\n",
                     i+1, result_ptr_aa.q[i], expected_ptr_aa.q[i]);
              printf("EXPECT: ");
              lux_dump_one(expected_sym_aa, 2);
              printf("GOT   : ");
              lux_dump_one(result_sym_aa, 2);
              ck_assert_int_eq(result_ptr_aa.q[i], expected_ptr_aa.q[i]);
            }
            if (result_ptr_as.q[i] != expected_ptr_as.q[i]) {
              printf("Discrepancy for as at #%d: got %d but expected %d\n",
                     i+1, result_ptr_as.q[i], expected_ptr_as.q[i]);
              printf("EXPECT: ");
              lux_dump_one(expected_sym_as, 2);
              printf("GOT   : ");
              lux_dump_one(result_sym_as, 2);
              ck_assert_int_eq(result_ptr_as.q[i], expected_ptr_as.q[i]);
            }
            if (result_ptr_sa.q[i] != expected_ptr_sa.q[i]) {
              printf("Discrepancy for sa at #%d: got %d but expected %d\n",
                     i+1, result_ptr_sa.q[i], expected_ptr_sa.q[i]);
              printf("EXPECT: ");
              lux_dump_one(expected_sym_sa, 2);
              printf("GOT   : ");
              lux_dump_one(result_sym_sa, 2);
              ck_assert_int_eq(result_ptr_sa.q[i], expected_ptr_sa.q[i]);
            }
          }
          break;
        case LUX_FLOAT:
          for (i = 0; i < n; ++i) {
            if (!approximately_equal_f(result_ptr_aa.f[i],
                                       expected_ptr_aa.f[i], eps_tol)) {
              printf("Discrepancy for aa at #%d: got %.15g "
                     "but expected %.15g\n",
                     i+1, result_ptr_aa.f[i], expected_ptr_aa.f[i]);
              printf("EXPECT: ");
              lux_dump_one(expected_sym_aa, 2);
              printf("GOT   : ");
              lux_dump_one(result_sym_aa, 2);
              ck_assert(approximately_equal_f(result_ptr_aa.f[i],
                                              expected_ptr_aa.f[i], eps_tol));
            }
            if (!approximately_equal_f(result_ptr_as.f[i],
                                       expected_ptr_as.f[i], eps_tol)) {
              printf("Discrepancy for as at #%d: got %.15g "
                     "but expected %.15g\n",
                     i+1, result_ptr_as.f[i], expected_ptr_as.f[i]);
              printf("EXPECT: ");
              lux_dump_one(expected_sym_as, 2);
              printf("GOT   : ");
              lux_dump_one(result_sym_as, 2);
              ck_assert(approximately_equal_f(result_ptr_as.f[i],
                                              expected_ptr_as.f[i], eps_tol));
            }
            if (!approximately_equal_f(result_ptr_sa.f[i],
                                       expected_ptr_sa.f[i], eps_tol)) {
              printf("Discrepancy for sa at #%d: got %.15g "
                     "but expected %.15g\n",
                     i+1, result_ptr_sa.f[i], expected_ptr_sa.f[i]);
              printf("EXPECT: ");
              lux_dump_one(expected_sym_sa, 2);
              printf("GOT   : ");
              lux_dump_one(result_sym_sa, 2);
              ck_assert(approximately_equal_f(result_ptr_sa.f[i],
                                              expected_ptr_sa.f[i], eps_tol));
            }
          }
          break;
        case LUX_DOUBLE:
          for (i = 0; i < n; ++i) {
            if (!approximately_equal(result_ptr_aa.d[i],
                                     expected_ptr_aa.d[i], eps_tol)) {
              printf("Discrepancy for aa at #%d: got %.15g "
                     "but expected %.15g\n",
                     i+1, result_ptr_aa.d[i], expected_ptr_aa.d[i]);
              printf("EXPECT: ");
              lux_dump_one(expected_sym_aa, 2);
              printf("GOT   : ");
              lux_dump_one(result_sym_aa, 2);
              ck_assert(approximately_equal(result_ptr_aa.d[i],
                                            expected_ptr_aa.d[i], eps_tol));
            }
            if (!approximately_equal(result_ptr_as.d[i],
                                     expected_ptr_as.d[i], eps_tol)) {
              printf("Discrepancy for as at #%d: got %.15g "
                     "but expected %.15g\n",
                     i+1, result_ptr_as.d[i], expected_ptr_as.d[i]);
              printf("EXPECT: ");
              lux_dump_one(expected_sym_as, 2);
              printf("GOT   : ");
              lux_dump_one(result_sym_as, 2);
              ck_assert(approximately_equal(result_ptr_as.d[i],
                                            expected_ptr_as.d[i], eps_tol));
            }
            if (!approximately_equal(result_ptr_sa.d[i],
                                     expected_ptr_sa.d[i], eps_tol)) {
              printf("Discrepancy for sa at #%d: got %.15g "
                     "but expected %.15g\n",
                     i+1, result_ptr_sa.d[i], expected_ptr_sa.d[i]);
              printf("EXPECT: ");
              lux_dump_one(expected_sym_sa, 2);
              printf("GOT   : ");
              lux_dump_one(result_sym_sa, 2);
              ck_assert(approximately_equal(result_ptr_sa.d[i],
                                            expected_ptr_sa.d[i], eps_tol));
            }
          }
          break;
        case LUX_CFLOAT:
          for (i = 0; i < n; ++i) {
            if (!approximately_equal_z_f(result_ptr_aa.cf[i],
                                         expected_ptr_aa.cf[i],
                                         eps_tol)) {
              printf("Discrepancy for aa at #%d: got %.15g%+.15gi "
                     "but expected %.15g%+.15gi\n",
                     i+1, result_ptr_aa.cf[i].real,
                     result_ptr_aa.cf[i].imaginary,
                     expected_ptr_aa.cf[i].real,
                     expected_ptr_aa.cf[i].imaginary);
              printf("EXPECT: ");
              lux_dump_one(expected_sym_aa, 2);
              printf("GOT   : ");
              lux_dump_one(result_sym_aa, 2);
              ck_assert(approximately_equal_z_f(result_ptr_aa.cf[i],
                                                expected_ptr_aa.cf[i],
                                                eps_tol));
            }
            if (!approximately_equal_z_f(result_ptr_as.cf[i],
                                         expected_ptr_as.cf[i],
                                         eps_tol)) {
              printf("Discrepancy for as at #%d: got %.15g%+.15gi "
                     "but expected %.15g%+.15gi\n",
                     i+1, result_ptr_as.cf[i].real,
                     result_ptr_as.cf[i].imaginary,
                     expected_ptr_as.cf[i].real,
                     expected_ptr_as.cf[i].imaginary);
              printf("EXPECT: ");
              lux_dump_one(expected_sym_as, 2);
              printf("GOT   : ");
              lux_dump_one(result_sym_as, 2);
              ck_assert(approximately_equal_z_f(result_ptr_as.cf[i],
                                                expected_ptr_as.cf[i],
                                                eps_tol));
            }
            if (!approximately_equal_z_f(result_ptr_sa.cf[i],
                                         expected_ptr_sa.cf[i],
                                         eps_tol)) {
              printf("Discrepancy for sa at #%d: got %.15g%+.15gi "
                     "but expected %.15g%+.15gi\n",
                     i+1, result_ptr_sa.cf[i].real,
                     result_ptr_sa.cf[i].imaginary,
                     expected_ptr_sa.cf[i].real,
                     expected_ptr_sa.cf[i].imaginary);
              printf("EXPECT: ");
              lux_dump_one(expected_sym_sa, 2);
              printf("GOT   : ");
              lux_dump_one(result_sym_sa, 2);
              ck_assert(approximately_equal_z_f(result_ptr_sa.cf[i],
                                                expected_ptr_sa.cf[i],
                                                eps_tol));
            }
          }
          break;
        case LUX_CDOUBLE:
          for (i = 0; i < n; ++i) {
            if (!approximately_equal_z(result_ptr_aa.cd[i],
                                       expected_ptr_aa.cd[i],
                                       eps_tol)) {
              printf("Discrepancy for aa at #%d: got %.15g%+.15gi "
                     "but expected %.15g%+.15gi\n",
                     i+1, result_ptr_aa.cd[i].real,
                     result_ptr_aa.cd[i].imaginary,
                     expected_ptr_aa.cd[i].real,
                     expected_ptr_aa.cd[i].imaginary);
              printf("EXPECT: ");
              lux_dump_one(expected_sym_aa, 2);
              printf("GOT   : ");
              lux_dump_one(result_sym_aa, 2);
              ck_assert(approximately_equal_z(result_ptr_aa.cd[i],
                                              expected_ptr_aa.cd[i],
                                              eps_tol));
            }
            if (!approximately_equal_z(result_ptr_as.cd[i],
                                       expected_ptr_as.cd[i],
                                       eps_tol)) {
              printf("Discrepancy for as at #%d: got %.15g%+.15gi "
                     "but expected %.15g%+.15gi\n",
                     i+1, result_ptr_as.cd[i].real,
                     result_ptr_as.cd[i].imaginary,
                     expected_ptr_as.cd[i].real,
                     expected_ptr_as.cd[i].imaginary);
              printf("EXPECT: ");
              lux_dump_one(expected_sym_as, 2);
              printf("GOT   : ");
              lux_dump_one(result_sym_as, 2);
              ck_assert(approximately_equal_z(result_ptr_as.cd[i],
                                              expected_ptr_as.cd[i],
                                              eps_tol));
            }
            if (!approximately_equal_z(result_ptr_sa.cd[i],
                                       expected_ptr_sa.cd[i],
                                       eps_tol)) {
              printf("Discrepancy for sa at #%d: got %.15g%+.15gi "
                     "but expected %.15g%+.15gi\n",
                     i+1, result_ptr_sa.cd[i].real,
                     result_ptr_sa.cd[i].imaginary,
                     expected_ptr_sa.cd[i].real,
                     expected_ptr_sa.cd[i].imaginary);
              printf("EXPECT: ");
              lux_dump_one(expected_sym_sa, 2);
              printf("GOT   : ");
              lux_dump_one(result_sym_sa, 2);
              ck_assert(approximately_equal_z(result_ptr_sa.cd[i],
                                              expected_ptr_sa.cd[i],
                                              eps_tol));
            }
          }
          break;
        }

        zap(result_sym_aa);
        zap(result_sym_as);
        zap(result_sym_sa);
        zap(expected_sym_aa);
        zap(expected_sym_as);
        zap(expected_sym_sa);
      }
      zap(op_sym_aa);
      zap(op_sym_as);
      zap(op_sym_sa);
      zap(rhs_sym_aa);
      zap(rhs_sym_as);
      zap(rhs_sym_sa);
      zap(lhs_sym_aa);
      zap(lhs_sym_as);
      zap(lhs_sym_sa);
      zap(lhs_basic_sym_aa);
      zap(rhs_basic_sym_aa);
      zap(lhs_basic_sym_as);
      zap(rhs_basic_sym_as);
      zap(lhs_basic_sym_sa);
      zap(rhs_basic_sym_sa);
    }
  }
}

CPPUNIT_TEST_SUITE_REGISTRATION(BinopTest);
