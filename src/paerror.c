/* This is file paerror.c.

Copyright 2013 Louis Strous, Richard Shine

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
#include <stdio.h>
#include "ana_structures.h"

Int execute_error(n)		/* handle errors */
 Int n;
 {
 printf("execution error: ");
 switch (n) {
 case 1: printf("invalid EDB passed to EXECUTE\n"); break;
 case 2: printf("# of levels exceeds 100\n"); break;
 case 3: printf("(X) an impossible error\n"); break;
 case 4: printf("lhs of REPLACE is a constant\n"); break;
 case 5: printf("illegal symbol on RHS of REPLACE\n"); break;
 case 6: printf("conditional does not evaluate to a scalar\n"); break;
 case 7: printf("undefined symbol in expression (eval)\n"); break;
 case 8: printf("illegal symbol class in expression (eval)\n"); break;
 case 9: printf("bad binary op in eval (this is impossible)\n"); break;
 case 10: printf("argument was not a scalar\n"); break;
 case 11: printf("more than 8 dimensions specified\n"); break;
 case 12: printf("illegal scalar type (impossible?)\n"); break;
 case 13: printf("memory allocation failure\n"); break;
 case 14: printf("problem initializing counter in FOR loop\n"); break;
 case 15: printf("problem with final counter value in FOR loop\n"); break;
 case 16: printf("problem with increment in FOR loop\n"); break;
 case 17: printf("problem deleting a symbol (impossible?)\n"); break;
 case 18: printf("illegal variable type (string?) for unary negative\n"); break;
 case 19: printf("(X) symbol passed to array_clone not an array\n"); break;
 case 20: printf("(X) no memory allocated to symbol in array_clone\n"); break;
 case 21: printf("(X) symbol passed to string_clone not a string\n"); break;
 case 22: printf("INDGEN accepts only arrays or scalars\n"); break;
 case 23: printf("(X) malloc failure creating argument list (how did you do this?)\n");
		 break;
 case 24: printf("incompatible arrays in expression\n"); break;
 case 25: printf("(X) error in fixed_string count\n"); break;
 case 26: printf("illegal action with a string (kinky?)\n"); break;
 case 27: printf("(X) mysterious class in binary operation or function\n"); break;
 case 28: printf("(X) illegal variable type in dump\n"); break;
 case 29: printf("floating point values illegal in logical expression\n");
			 break;
 case 30: printf("illegal variable type (string?) in math function\n"); break;
 case 31: printf("subscripted variable is not an array or string\n"); break;
 case 32: printf("illegal variable type\n"); break;
 case 33: printf("symbol table overflow for subscript pointers\n"); break;
 case 34: printf("symbol table overflow for temporaries\n"); break;
 case 35: printf("subscript out of range\n"); break;
 case 36: printf("too many subscripts\n"); break;
 case 37: printf("invalid rearrange attempted\n"); break;
 case 38: printf("(X)impossible error in ana_sub_arg\n"); break;
 case 39: printf("illegal subscript type\n"); break;
 case 40: printf("(X)scalar_scratch_copy got a non-scalar\n"); break;
 case 41: printf("undefined subroutine\n"); break;
 case 42: printf("recursive subroutine calls not allowed\n"); break;
 case 43: printf("too many arguments passed to subroutine\n"); break;
 case 44: printf("(X)bad return status handed to escaper\n"); break;
 case 45: printf("in user subroutine "); break;
 case 46: printf("(X) illegal (impossible) scalar type\n"); break;
 case 47: printf("illegal variable type (string?) in function\n"); break;
 case 48: printf("illegal variable type (undefined?) in function\n"); break;
 case 49: printf("illegal variable type (string?) in DSUM function\n"); break;
 case 50: printf("illegal variable type (string?) in RUNSUM function\n"); break;
 case 51: printf("illegal variable type (string?) in SDEV function\n"); break;
 case 52: printf("illegal variable type (string?) in SWAB function\n"); break;
 case 53: printf("undefined argument in TYPE\n"); break;
 case 54: printf("error in argument for DUMP\n"); break;
 case 55: printf("wrong number of arguments passed\n"); break;
 case 56: printf("(X) bad rhs in replacement statement\n"); break;
 case 57: printf("bad argument class in CONCATENATION\n"); break;
 case 58:
 printf("illegal combination of string with non-string in CONCATENATION\n");
 break;
 case 59:printf("illegal combination of dimensions in CONCATENATION\n");
 break;
 case 60: printf("undefined named block: "); break;
 case 61: printf("within named block: "); break;
 case 62: printf("no dimensions specified for REDIM\n"); break;
 case 63: printf("too many dimensions specified\n"); break;
 case 64: printf("array too small for requested REDIM\n"); break;
 case 65: printf("zero or negative dimension\n"); break;
 case 66: printf("argument not an array\n"); break;
 case 67: printf("first argument in REDIM is not an array\n"); break;
 case 68: printf("undefined function: "); break;
 case 69: printf("too many arguments passed to user function\n"); break;
 case 70: printf("string argument required\n"); break;
 case 71: printf("subscripted variable on lhs is not an array\n"); break;
 case 72: printf("too many subscripts in lhs of INSERT statement\n"); break;
 case 73: printf("illegal subscript in lhs of INSERT statement\n"); break;
 case 74: printf("negative subscript\n"); break;
 case 75: printf("illegal string/non-string mixture in INSERT statement\n");
  break;
 case 76: printf("argument is not a square 2-D matrix\n"); break;
 case 77: printf("dimensions of arrays are incompatiable\n"); break;
 case 78: printf("illegal power specified for poly. fit (range 1-10)\n"); break;
 case 79: printf("incompatiable LU decomp and rhs in dsolve\n"); break;
 case 80:
 printf("inner dimen. Byte count isn't a multiple of element length \n");
 break;
 case 81: printf("reversal specified for non-existent dimension\n"); break;
 case 82: printf("duplicate index in REVERSE\n"); break;
 case 83: printf("illegal variable type\n"); break;
 case 84: printf("error opening file\n"); break;
 case 85: printf("out of range lun\n"); break;
 case 86: printf("lun already in use\n"); break;
 case 87: printf("more than 1 dimension implied for a string\n"); break;
 case 88: printf("compress or rearrange not supported for strings\n"); break;
 case 89: printf("non-integer counter in NCASE\n"); break;
 case 90: printf("error writing file\n"); break;
 case 91: printf("hit EOF while reading\n"); break;
 case 92: printf("compress accepts only 1-D or 2-D arrays\n"); break;
 case 93: printf("malloc failure for pointer array in COMPRESS\n"); break;
 case 94: printf("LUN not open\n"); break;
 case 95: printf("REGRID array must be a non-trivial 2-D array\n"); break;
 case 96: printf("REGRID grids must be non-trivial 2-D arrays\n"); break;
 case 97: printf("x and y grids must be of same size\n"); break;
 case 98: printf("requested index exceeds dimension of array\n"); break;
 case 99: printf("error in argument list\n"); break;
 case 100: printf("argument must be a 3x3 array\n"); break;
 case 101: printf("argument not a 2-D array\n"); break;
 case 102: printf("displacement grid not in correct format\n"); break;
 case 103: printf("input arrays must match\n"); break;
 case 104: printf("argument destined to be returned is not atomic\n"); break;
 case 105: printf("fcwrite only accepts I*1 or I*2 arrays (I*4 on some)\n"); break;
 case 106: printf("file open for read only\n"); break;
 case 107: printf("argument must be a 1-D vector\n"); break;
 case 108: printf("bad expression\n"); break;
 case 109: printf("bad $CONTOURS array\n"); break;
 case 110: printf("INSERT only accepts 1 or 2 D arrays\n"); break;
 case 111: printf("I*2 input array required\n"); break;
 case 112: printf("error in associated variable index\n"); break;
 case 113: printf("failure in assoc var input (record too big?)\n");
	  break;
 case 114: printf("assoc. var (output) does not match array\n"); break;
 case 115: printf("failure in assoc var output\n");
	  break;
 case 116: printf("attempt to redefine a protected symbol\n"); break;
 case 117: printf("bad year in ephemeris call, must be 19xx or xx\n"); break;
 case 118: printf("I*1 input array required\n"); break;
 case 119: printf("I*2 or I*1 input array required\n"); break;
 case 120: printf("F*4 input array required\n"); break;
 case 121: printf("illegal format specified\n"); break;
 case 122: printf("malloc failure in ck_format_vars\n"); break;
 case 123: printf("malloc failure while creating temporary\n"); break;
 case 124: printf("arguments must be of same size\n"); break;
 case 125: printf("lookup table must be 256 long\n"); break;
 case 126: printf("\n"); break;
 default: printf("(X)undefined error code\n"); break;
 }
 return -1;
 }						/*end of execute_error */

Int file_open_error()
{
  puts("Error opening file");
  return -1;
}
