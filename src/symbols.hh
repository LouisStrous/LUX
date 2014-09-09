/* This is file symbols.hh.

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
/* File symbols.h */
/* Macro definitions that specify various parts of LUX symbols */
#define array_data(symbol)	/* void * */ ((void *) (array_header(symbol) + 1))
#define array_dims(symbol)	/* int32_t * */ ((int32_t *)(array_header(symbol)->dims))
#define array_facts(symbol)	/* arrayFacts */ (array_header(symbol)->facts)
#define array_header(symbol)	/* array * */ (sym[symbol].spec.array.ptr)
#define array_num_dims(symbol)	/* uint8_t */ (array_header(symbol)->ndim)
#define array_num_facts(symbol)	/* uint8_t */ (array_header(symbol)->nfacts)
#define array_size(symbol)	/* int32_t */ ((sym[symbol].spec.array.bstore - sizeof(array))/lux_type_size[array_type(symbol)])
#define array_type(symbol)	/* uint8_t */ (sym[symbol].type)
#define assoc_dims(symbol)	/* int32_t * */(((array *) sym[symbol].spec.array.ptr)->dims)
#define assoc_lun(symbol)	/* uint8_t */(((array *) sym[symbol].spec.array.ptr)->c1)
#define assoc_num_dims(symbol)	/* uint8_t */(((array *) sym[symbol].spec.array.ptr)->ndim)
#define assoc_type(symbol)	/* uint8_t */(sym[symbol].type)
#define assoc_has_offset(symbol) /* uint8_t */(((array *) sym[symbol].spec.array.ptr)->c2 & 1)
#define set_assoc_has_offset(symbol)    ((array *) sym[symbol].spec.array.ptr)->c2 |= 1
#define assoc_offset(symbol)	/* int32_t */(*(int32_t *) ((array *) sym[symbol].spec.array.ptr + 1))
#define bin_op_lhs(symbol)	/* uint16_t */(sym[symbol].spec.uevb.args[0])
#define bin_op_rhs(symbol)	/* uint16_t */(sym[symbol].spec.uevb.args[1])
#define bin_op_type(symbol)	/* int16_t */(sym[symbol].xx)
#define block_num_statements(symbol) /* int32_t */(sym[symbol].spec.wlist.bstore/sizeof(int16_t))
#define block_statements(symbol) /* int16_t * */(sym[symbol].spec.wlist.ptr)
#define case_num_statements(symbol) /* int32_t */(sym[symbol].spec.wlist.bstore/sizeof(int16_t))
#define case_statements(symbol) /* int16_t * */(sym[symbol].spec.wlist.ptr)
#define clist_depth(symbol)	/* uint8_t */(sym[symbol].type)
#define clist_num_symbols(symbol) /* int32_t */(sym[symbol].spec.wlist.bstore/sizeof(int16_t))
#define clist_symbols(symbol)	/* int16_t * */(sym[symbol].spec.wlist.ptr)
#define complex_array_data(symbol) /* void * */ ((void *) (array_header(symbol) + 1))
#define complex_array_dims(symbol) /* int32_t * */(array_header(symbol)->dims)
#define complex_array_header(symbol) /* array * */(sym[symbol].spec.array.ptr)
#define complex_array_num_dims(symbol) /* int32_t */(array_header(symbol)->ndim)
#define complex_array_size(symbol) /* int32_t */((symbol_memory(symbol) - sizeof(array))/lux_type_size[complex_array_type(symbol)])
#define complex_array_type(symbol) /* uint8_t */(sym[symbol].type)
#define complex_scalar_type(symbol) /* uint8_t */(sym[symbol].type)
#define complex_scalar_data(symbol) /* pointer */sym[symbol].spec.dpointer
#define complex_scalar_memory(symbol) /* int32_t */(sym[symbol].spec.general.bstore)
#define deferred_routine_filename(symbol) /* char * */(sym[symbol].spec.name.ptr)
#define do_while_body(symbol)	/* int16_t */(sym[symbol].spec.evb.args[0])
#define do_while_condition(symbol) /* int16_t */(sym[symbol].spec.evb.args[1])
#define enum_list(symbol)	/* enumElem * */(sym[symbol].spec.enumElem.ptr)
#define enum_key(symbol,i)	/* char * */ (sym[symbol].spec.enumElem.ptr[i].key)
#define enum_value(symbol,i)	/* int32_t */ (sym[symbol].spec.enumElem.ptr[i].value)
#define enum_num_elements(symbol) /* int32_t */(sym[symbol].spec.enumElem.bstore/sizeof(enumElem))
#define enum_type(symbol)	/* uint8_t */(sym[symbol].type)
#define evb_type(symbol)	/* uint8_t */(EVBclass) (sym[symbol].type)
#define evb_num_elements(symbol) /* int32_t */(sym[symbol].spec.wlist.bstore/sizeof(int16_t))
#define evb_elements(symbol)	/* int16_t * */(sym[symbol].spec.wlist.ptr)
#define evb_args(symbol)	/* int16_t * */(sym[symbol].spec.evb.args)
#define extract_target(symbol)	/* int16_t */(sym[symbol].xx)
#define extract_num_sec(symbol)	/* uint8_t */(sym[symbol].type)
#define extract_ptr(symbol)	/* extractSec * */(sym[symbol].spec.extract.ptr)
#define file_include_type(symbol) /* INCLUDE or REPORT */(sym[symbol].xx)
#define file_map_dims(symbol) /* int32_t * */(((array *) sym[symbol].spec.array.ptr)->dims)
#define file_map_file_name(symbol) /* char * */((char *) file_map_header(symbol) + sizeof(array) + file_map_has_offset(symbol)*sizeof(int32_t))
#define file_map_num_dims(symbol) /* uint8_t */(((array *) sym[symbol].spec.array.ptr)->ndim)
#define file_map_header(symbol) /* array * */ (sym[symbol].spec.array.ptr)
#define file_map_has_offset(symbol) /* uint8_t */(file_map_header(symbol)->c2 & 1)
#define file_map_swap(symbol) (file_map_header(symbol)->c2 & 4)
#define set_file_map_has_offset(symbol)	  file_map_header(symbol)->c2 |= 1
#define file_map_offset(symbol) /* int32_t */ (*(int32_t *) (file_map_header(symbol) + 1))
#define file_map_readonly(symbol) /* uint8_t */(file_map_header(symbol)->c2 & 2)
#define set_file_map_readonly(symbol)      file_map_header(symbol)->c2 |= 2
#define set_file_map_readwrite(symbol)     file_map_header(symbol)->c2 &= ~2
#define set_file_map_swap(symbol)  file_map_header(symbol)->c2 |= 4
/* file_map_size(symbol) is a function defined in symbols.c */
int32_t	file_map_size(int32_t symbol);
#define file_map_type(symbol) /* uint8_t */(sym[symbol].type)
#define file_name(symbol)	/* char * */(sym[symbol].spec.name.ptr)
#define file_name_size1(symbol) /* int32_t */(sym[symbol].spec.name.bstore)
#define for_body(symbol)	/* int16_t */(sym[symbol].xx)
#define for_end(symbol)	/* int16_t */(sym[symbol].spec.evb.args[2])
#define for_loop_symbol(symbol) /* int16_t */(sym[symbol].spec.evb.args[0])
#define for_start(symbol)	/* int16_t */(sym[symbol].spec.evb.args[1])
#define for_step(symbol)	/* int16_t */(sym[symbol].spec.evb.args[3])
#define func_ptr_routine_num(symbol) /* int16_t; negative if internal */(sym[symbol].spec.evb.args[0])
#define func_ptr_type(symbol)	/* int16_t */(sym[symbol].type) /* either LUX_FUNCTION or LUX_SUBROUTINE */
#define if_condition(symbol)	/* int16_t */(sym[symbol].spec.evb.args[0])
#define if_false_body(symbol)	/* int16_t */(sym[symbol].spec.evb.args[2])
#define if_true_body(symbol)	/* int16_t */(sym[symbol].spec.evb.args[1])
#define insert_num_target_indices(symbol) /* int32_t */(sym[symbol].spec.wlist.bstore/sizeof(int16_t) - 1)
#define insert_source(symbol)	/* int16_t */(insert_target_indices(symbol)[insert_num_target_indices(symbol)])
#define insert_target(symbol)	/* int16_t */(sym[symbol].xx)
#define insert_target_indices(symbol) /* int16_t * */(sym[symbol].spec.wlist.ptr)
#define int_func_arguments(symbol) /* int16_t * */(sym[symbol].spec.wlist.ptr)
#define int_func_num_arguments(symbol)	/* int32_t */(sym[symbol].spec.wlist.bstore/sizeof(int16_t))
#define int_func_number(symbol) /* int16_t */(sym[symbol].xx)
#define int_sub_arguments(symbol) /* int16_t * */(sym[symbol].spec.wlist.ptr)
#define int_sub_num_arguments(symbol) /* int32_t */(sym[symbol].spec.wlist.bstore/sizeof(int16_t))
#define int_sub_routine_num(symbol) /* int16_t */(sym[symbol].xx)
#define keyword_name(symbol)	/* char * */(string_value(keyword_name_symbol(symbol)))
#define keyword_name_symbol(symbol) /* int16_t */(sym[symbol].spec.evb.args[0])
#define keyword_value(symbol)	/* int16_t */(sym[symbol].spec.evb.args[1])
#define list_depth(symbol)	/* uint8_t */(sym[symbol].type)
#define list_symbols(symbol) /* listElem * */(sym[symbol].spec.listElem.ptr)
#define list_key(symbol,i)	/* char * */(list_symbols(symbol)[i].key)
#define list_num_symbols(symbol) /* int32_t */(sym[symbol].spec.listElem.bstore/sizeof(listElem))
#define list_symbol(symbol,i) /* int16_t */(list_symbols(symbol)[i].value)
#define meta_target(symbol)	/* int16_t */(sym[symbol].spec.evb.args[0])
#define ncase_else(symbol)	/* int16_t */(ncase_statements(symbol)[ncase_num_statements(symbol)])
#define ncase_num_statements(symbol) /* int32_t */(sym[symbol].spec.wlist.bstore/sizeof(int16_t) - 2)
#define ncase_statements(symbol) /* int16_t * */((int16_t *) sym[symbol].spec.wlist.ptr)
#define ncase_switch_value(symbol) /* int16_t */(*((int16_t *) sym[symbol].spec.wlist.ptr + sym[symbol].spec.wlist.bstore/sizeof(int16_t) - 1))
#define pre_clist_num_symbols(symbol) /* int32_t */(sym[symbol].spec.wlist.bstore/sizeof(int16_t))
#define pre_clist_symbols(symbol)	/* int16_t * */(sym[symbol].spec.wlist.ptr)
#define pre_extract_data(symbol) /* preExtract * */(sym[symbol].spec.preExtract.ptr)
#define pre_extract_name(symbol)	/* char */(sym[symbol].spec.preExtract.ptr->name)
#define pre_extract_num_sec(symbol)	/* uint8_t */(sym[symbol].type)
#define pre_extract_ptr(symbol)	/* extractSec * */(sym[symbol].spec.preExtract.ptr->extract)
#define pre_range_end(symbol)	/* int16_t */(sym[symbol].spec.evb.args[1])
#define pre_range_redirect(symbol) /* int16_t */(sym[symbol].spec.evb.args[3])
#define pre_range_scalar(symbol) /* uint8_t */(sym[symbol].type)
#define pre_range_start(symbol) /* int16_t */(sym[symbol].spec.evb.args[0])
#define pre_range_sum(symbol)	/* int16_t */(sym[symbol].spec.evb.args[2])
#define pre_list_symbols(symbol) /* listElem * */(sym[symbol].spec.listElem.ptr)
#define pre_list_key(symbol,i) /* char * */(pre_list_symbols(symbol)[i].key)
#define pre_list_num_symbols(symbol) /* int32_t */(sym[symbol].spec.listElem.bstore/sizeof(listElem))
#define pre_list_symbol(symbol,i) /* int16_t */(pre_list_symbols(symbol)[i].value)
#define range_end(symbol)	/* int16_t */(sym[symbol].spec.evb.args[1])
#define range_redirect(symbol)	/* int16_t */(sym[symbol].spec.evb.args[3])
#define range_scalar(symbol)	/* uint8_t */(sym[symbol].type)
#define range_start(symbol)	/* int16_t */(sym[symbol].spec.evb.args[0])
#define range_sum(symbol)	/* int16_t */(sym[symbol].spec.evb.args[2])
#define repeat_body(symbol)	/* int16_t */(sym[symbol].spec.evb.args[0])
#define repeat_condition(symbol) /* int16_t */(sym[symbol].spec.evb.args[1])
#define replace_lhs(symbol)	/* int16_t */(sym[symbol].spec.evb.args[0])
#define replace_rhs(symbol)	/* int16_t */(sym[symbol].spec.evb.args[1])
#define return_value(symbol)	/* int16_t */(sym[symbol].spec.evb.args[0])
#define routine_num_parameters(symbol) /* uint8_t */(sym[symbol].spec.routine.narg)
#define routine_num_statements(symbol) /* uint16_t */(sym[symbol].spec.routine.nstmnt)
#define routine_parameter_names(symbol) /* char ** */(sym[symbol].spec.routine.keys)
#define routine_parameters(symbol) /* int16_t * */(sym[symbol].spec.routine.ptr)
#define routine_statements(symbol) /* int16_t * */(routine_parameters(symbol) + routine_num_parameters(symbol))
#define routine_has_extended_param(symbol) /* uint8_t */(sym[symbol].spec.routine.extend)
#define run_block_number(symbol) /* int16_t */(sym[symbol].xx)
#define scal_ptr_pointer(symbol) /* pointer */(sym[symbol].spec.dpointer)
#define scal_ptr_type(symbol)	/* uint8_t */(sym[symbol].type)
#define scalar_type(symbol)	/* uint8_t */(sym[symbol].type)
#define scalar_value(symbol)	/* Scalar */(sym[symbol].spec.scalar)
#define string_size(symbol)	/* int32_t */(sym[symbol].spec.name.bstore - (int32_t) 1)
#define string_type(symbol)	/* uint8_t */(sym[symbol].type)
#define string_value(symbol)	/* char * */(sym[symbol].spec.name.ptr)
#define list_ptr_tag_number(symbol) /* int32_t */(sym[symbol].spec.scalar.l)
#define list_ptr_tag_string(symbol) /* char * */(sym[symbol].spec.name.ptr)
#define list_ptr_tag_size(symbol) /* int32_t */ (sym[symbol].spec.name.bstore)
#define list_ptr_target(symbol) /* int16_t */(sym[symbol].xx)
#define struct_depth(symbol)	/* uint8_t */(sym[symbol].type)
#define struct_num_top_elements(symbol) /* int32_t */(struct_elements(symbol)[0].u.first.nelem)
#define struct_num_all_elements(symbol) /* int32_t */(sym[symbol].spec.intList.ptr[0])
#define struct_total_size(symbol) /* int32_t */(struct_elements(symbol)[0].u.first.size)
#define struct_elements(symbol)	/* structElem * */((structElem *) (sym[symbol].spec.intList.ptr + 1))
#define struct_data(symbol)	/* void * */((void *)(sym[symbol].spec.name.ptr + (struct_num_all_elements(symbol)*sizeof(structElem) + sizeof(int32_t))))
#define struct_ptr_n_elements(symbol) /* int16_t */(sym[symbol].spec.structPtr.bstore/sizeof(structPtr))
#define struct_ptr_elements(symbol)	/* structPtr * */(sym[symbol].spec.structPtr.ptr)
#define struct_ptr_target(symbol) /* int16_t */(sym[symbol].xx)
#define subsc_ptr_end(symbol)	/* int32_t */((sym[symbol].spec.intList.ptr)[1])
#define subsc_ptr_redirect(symbol) /* int32_t */((sym[symbol].spec.intList.ptr)[3])
#define subsc_ptr_start(symbol) /* int32_t */((sym[symbol].spec.intList.ptr)[0])
#define subsc_ptr_sum(symbol)	/* int32_t */((sym[symbol].spec.intList.ptr)[2])
#define symbol_class(symbol)	/* uint8_t */ (sym[symbol].sclass)
#define symbol_context(symbol)	/* int16_t */(sym[symbol].context)
#define symbol_data(symbol)	/* void * */(sym[symbol].spec.general.ptr)
#define symbol_memory(symbol)	/* int32_t */(sym[symbol].spec.general.bstore)
#define symbol_type(symbol)	/* Symboltype */(sym[symbol].type)
#define symbol_line(symbol)	/* int32_t */(sym[symbol].line)
#define symbol_extra(symbol)	/* union specUnion */(sym[symbol].spec)
#define transfer_is_parameter(symbol) /* uint8_t */(sym[symbol].type)
#define transfer_target(symbol) /* int16_t */(sym[symbol].spec.evb.args[0])
#define transfer_temp_param(symbol) /* int16_t */(sym[symbol].spec.evb.args[1])
#define undefined_par(symbol)	/* char */(sym[symbol].type)
#define usr_code_routine_num(symbol) /* int16_t */(sym[symbol].xx)
#define usr_func_arguments(symbol) /* int16_t * */(sym[symbol].spec.wlist.ptr)
#define usr_func_num_arguments(symbol) /* int32_t */(sym[symbol].spec.wlist.bstore/sizeof(int16_t))
#define usr_func_number(symbol) /* int16_t */(sym[symbol].xx)
#define usr_routine_arguments(symbol) /* int16_t * */(sym[symbol].spec.wlist.ptr)
#define usr_routine_num_arguments(symbol) /* int32_t */(sym[symbol].spec.wlist.bstore/sizeof(int16_t))
#define usr_routine_num(symbol) /* int16_t */(sym[symbol].xx)
#define usr_routine_recursion(symbol) /* uint8_t */(sym[symbol].type)
#define usr_sub_arguments(symbol) /* int16_t * */(sym[symbol].spec.wlist.ptr)
#define usr_sub_num_arguments(symbol) /* int32_t */(sym[symbol].spec.wlist.bstore/sizeof(int16_t))
#define usr_sub_routine_num(symbol) /* int16_t */(sym[symbol].xx)
#define usr_sub_is_deferred(symbol) /* int32_t */(symbol_class(usr_sub_routine_num(symbol)) == LUX_STRING)
#define while_do_body(symbol)	/* int16_t */(sym[symbol].spec.evb.args[1])
#define while_do_condition(symbol) /* int16_t */(sym[symbol].spec.evb.args[0])


#define symbolIsArray(iq)	/* int32_t */ (symbol_class(iq) == LUX_ARRAY || symbol_class(iq) == LUX_CARRAY)
#define symbolIsNumericalArray(iq)	/* int32_t */ (symbolIsArray(iq) && symbolIsNumerical(iq))
#define symbolIsStringArray(iq)	/* int32_t */ (symbol_class(iq) == LUX_ARRAY && symbol_type(iq) == LUX_STRING_ARRAY)
#define symbolIsRealArray(iq)	/* int32_t */ (symbol_class(iq) == LUX_ARRAY && symbol_type(iq) <= LUX_DOUBLE)
#define symbolIsRealScalar(iq)	/* int32_t */ ((symbol_class(iq) == LUX_SCALAR || symbol_class(iq) == LUX_SCAL_PTR) && isRealType(symbol_type(iq)))
#define symbolIsComplexArray(iq) /* int32_t */ (symbol_class(iq) == LUX_CARRAY)
#define symbolIsComplexScalar(iq) /* int32_t */ (symbol_class(iq) == LUX_CSCALAR || (symbol_class(iq) == LUX_SCAL_PTR && isComplexType(symbol_type(iq))))
#define symbolIsNumerical(iq)	/* int32_t */ ((symbolIsArray(iq) || symbolIsScalar(iq)) && isNumericalType(symbol_type(iq)))
#define symbolIsReal(iq)        /* int32_t */ (symbolIsNumerical(iq) && isRealType(symbol_type(iq)))
#define symbolIsInteger(iq)     /* int32_t */ (symbolIsNumerical(iq) && isIntegerType(symbol_type(iq)))
#define symbolIsComplex(iq)     /* int32_t */ (symbolIsNumerical(iq) && isComplexType(symbol_type(iq)))
#define symbolIsString(iq)      /* int32_t */ (symbolIsStringScalar(iq) || symbolIsStringArray(iq))
#define symbolIsStringScalar(iq) /* int32_t */ (symbol_class(iq) == LUX_STRING)
#define symbolIsScalar(iq)		/* int32_t */ (symbol_class(iq) == LUX_SCALAR || symbol_class(iq) == LUX_SCAL_PTR || symbol_class(iq) == LUX_CSCALAR)
#define symbolIsUnitaryExpression(iq) /* int32_t */ (symbol_class(iq) < LUX_SUBROUTINE)
#define symbolIsNamed(iq)	/* int32_t */ (symbolProperName(iq) != NULL)
#define symbolIsModifiable(iq)  /* int32_t */ (symbolIsNamed((iq)) && (iq) > nFixed)
#define numericalSize(iq)	/* int32_t */ ((symbol_class(iq) == LUX_ARRAY)? array_size(iq): 1)
#define isNumericalType(type)	/* int32_t */ ((type) <= LUX_DOUBLE || (type) >= LUX_CFLOAT)
#define isStringType(type)	/* int32_t */ ((type) > LUX_DOUBLE && (type) < LUX_CFLOAT)
#define isRealType(type)	/* int32_t */ ((type) <= LUX_DOUBLE)
#define isComplexType(type)	/* int32_t */ ((type) >= LUX_CFLOAT)
#define isIntegerType(type)	/* int32_t */ ((type) <= LUX_INT64)
#define isFloatType(type)	/* int32_t */ ((type) == LUX_FLOAT || (type) == LUX_DOUBLE || (type) == LUX_CFLOAT || (type) == LUX_CDOUBLE)
#define isLegalType(type)	/* int32_t */ ((type) >= LUX_INT8 && (type) <= LUX_CDOUBLE)
/* realType(type) returns <type> if it is real, or the corresponding real */
/* type if it is complex. */
#define realType(type)		/* Symboltype */ (Symboltype)(((type) >= LUX_CFLOAT)? (type) + (LUX_FLOAT - LUX_CFLOAT): (type))
#define complexMag2(x) ((x).real*(x).real + (x).imaginary*(x).imaginary)

#define INFTY (1.0/0.0)