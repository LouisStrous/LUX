/* File symbols.h */
/* Macro definitions that specify various parts of ANA symbols */
/* $Id: symbols.h,v 4.1 2001/02/08 19:24:39 strous Exp $ */
#define array_data(symbol)	/* void * */ ((void *) (array_header(symbol) + 1))
#define array_dims(symbol)	/* int * */ ((int *)(array_header(symbol)->dims))
#define array_facts(symbol)	/* arrayFacts */ (array_header(symbol)->facts)
#define array_header(symbol)	/* array * */ (sym[symbol].spec.array.ptr)
#define array_num_dims(symbol)	/* byte */ (array_header(symbol)->ndim)
#define array_num_facts(symbol)	/* byte */ (array_header(symbol)->nfacts)
#define array_size(symbol)	/* int */ ((sym[symbol].spec.array.bstore - sizeof(array))/ana_type_size[array_type(symbol)])
#define array_type(symbol)	/* byte */ (sym[symbol].type)
#define assoc_dims(symbol)	/* int * */(((array *) sym[symbol].spec.array.ptr)->dims)
#define assoc_lun(symbol)	/* byte */(((array *) sym[symbol].spec.array.ptr)->c1)
#define assoc_num_dims(symbol)	/* byte */(((array *) sym[symbol].spec.array.ptr)->ndim)
#define assoc_type(symbol)	/* byte */(sym[symbol].type)
#define assoc_has_offset(symbol) /* byte */(((array *) sym[symbol].spec.array.ptr)->c2 & 1)
#define set_assoc_has_offset(symbol)    ((array *) sym[symbol].spec.array.ptr)->c2 |= 1
#define assoc_offset(symbol)	/* int */(*(int *) ((array *) sym[symbol].spec.array.ptr + 1))
#define bin_op_lhs(symbol)	/* uword */(sym[symbol].spec.uevb.args[0])
#define bin_op_rhs(symbol)	/* uword */(sym[symbol].spec.uevb.args[1])
#define bin_op_type(symbol)	/* word */(sym[symbol].xx)
#define block_num_statements(symbol) /* int */(sym[symbol].spec.wlist.bstore/sizeof(word))
#define block_statements(symbol) /* word * */(sym[symbol].spec.wlist.ptr)
#define case_num_statements(symbol) /* int */(sym[symbol].spec.wlist.bstore/sizeof(word))
#define case_statements(symbol) /* word * */(sym[symbol].spec.wlist.ptr)
#define clist_depth(symbol)	/* byte */(sym[symbol].type)
#define clist_num_symbols(symbol) /* int */(sym[symbol].spec.wlist.bstore/sizeof(word))
#define clist_symbols(symbol)	/* word * */(sym[symbol].spec.wlist.ptr)
#define complex_array_data(symbol) /* void * */ ((void *) (array_header(symbol) + 1))
#define complex_array_dims(symbol) /* int * */(array_header(symbol)->dims)
#define complex_array_header(symbol) /* array * */(sym[symbol].spec.array.ptr)
#define complex_array_num_dims(symbol) /* int */(array_header(symbol)->ndim)
#define complex_array_size(symbol) /* int */((symbol_memory(symbol) - sizeof(array))/ana_type_size[complex_array_type(symbol)])
#define complex_array_type(symbol) /* byte */(sym[symbol].type)
#define complex_scalar_type(symbol) /* byte */(sym[symbol].type)
#define complex_scalar_data(symbol) /* pointer */sym[symbol].spec.pointer
#define complex_scalar_memory(symbol) /* int */(sym[symbol].spec.general.bstore)
#define deferred_routine_filename(symbol) /* char * */(sym[symbol].spec.name.ptr)
#define do_while_body(symbol)	/* word */(sym[symbol].spec.evb.args[0])
#define do_while_condition(symbol) /* word */(sym[symbol].spec.evb.args[1])
#define enum_list(symbol)	/* enumElem * */(sym[symbol].spec.enumElem.ptr)
#define enum_key(symbol,i)	/* char * */ (sym[symbol].spec.enumElem.ptr[i].key)
#define enum_value(symbol,i)	/* int */ (sym[symbol].spec.enumElem.ptr[i].value)
#define enum_num_elements(symbol) /* int */(sym[symbol].spec.enumElem.bstore/sizeof(enumElem))
#define enum_type(symbol)	/* byte */(sym[symbol].type)
#define evb_type(symbol)	/* byte */(sym[symbol].type)
#define evb_num_elements(symbol) /* int */(sym[symbol].spec.wlist.bstore/sizeof(word))
#define evb_elements(symbol)	/* word * */(sym[symbol].spec.wlist.ptr)
#define evb_args(symbol)	/* word * */(sym[symbol].spec.evb.args)
#define extract_target(symbol)	/* word */(sym[symbol].xx)
#define extract_num_sec(symbol)	/* byte */(sym[symbol].type)
#define extract_ptr(symbol)	/* extractSec * */(sym[symbol].spec.extract.ptr)
#define file_include_type(symbol) /* INCLUDE or REPORT */(sym[symbol].xx)
#define file_map_dims(symbol) /* int * */(((array *) sym[symbol].spec.array.ptr)->dims)
#define file_map_file_name(symbol) /* char * */((char *) file_map_header(symbol) + sizeof(array) + file_map_has_offset(symbol)*sizeof(int))
#define file_map_num_dims(symbol) /* byte */(((array *) sym[symbol].spec.array.ptr)->ndim)
#define file_map_header(symbol) /* array * */ (sym[symbol].spec.array.ptr)
#define file_map_has_offset(symbol) /* byte */(file_map_header(symbol)->c2 & 1)
#define file_map_swap(symbol) (file_map_header(symbol)->c2 & 4)
#define set_file_map_has_offset(symbol)	  file_map_header(symbol)->c2 |= 1
#define file_map_offset(symbol) /* int */ (*(int *) (file_map_header(symbol) + 1))
#define file_map_readonly(symbol) /* byte */(file_map_header(symbol)->c2 & 2)
#define set_file_map_readonly(symbol)      file_map_header(symbol)->c2 |= 2
#define set_file_map_readwrite(symbol)     file_map_header(symbol)->c2 &= ~2
#define set_file_map_swap(symbol)  file_map_header(symbol)->c2 |= 4
/* file_map_size(symbol) is a function defined in symbols.c */
int	file_map_size(int symbol);
#define file_map_type(symbol) /* byte */(sym[symbol].type)
#define file_name(symbol)	/* char * */(sym[symbol].spec.name.ptr)
#define file_name_size1(symbol) /* int */(sym[symbol].spec.name.bstore)
#define for_body(symbol)	/* word */(sym[symbol].xx)
#define for_end(symbol)	/* word */(sym[symbol].spec.evb.args[2])
#define for_loop_symbol(symbol) /* word */(sym[symbol].spec.evb.args[0])
#define for_start(symbol)	/* word */(sym[symbol].spec.evb.args[1])
#define for_step(symbol)	/* word */(sym[symbol].spec.evb.args[3])
#define func_ptr_routine_num(symbol) /* word; negative if internal */(sym[symbol].spec.evb.args[0])
#define func_ptr_type(symbol)	/* word */(sym[symbol].type) /* either ANA_FUNCTION or ANA_SUBROUTINE */
#define if_condition(symbol)	/* word */(sym[symbol].spec.evb.args[0])
#define if_false_body(symbol)	/* word */(sym[symbol].spec.evb.args[2])
#define if_true_body(symbol)	/* word */(sym[symbol].spec.evb.args[1])
#define insert_num_target_indices(symbol) /* int */(sym[symbol].spec.wlist.bstore/sizeof(word) - 1)
#define insert_source(symbol)	/* word */(insert_target_indices(symbol)[insert_num_target_indices(symbol)])
#define insert_target(symbol)	/* word */(sym[symbol].xx)
#define insert_target_indices(symbol) /* word * */(sym[symbol].spec.wlist.ptr)
#define int_func_arguments(symbol) /* word * */(sym[symbol].spec.wlist.ptr)
#define int_func_num_arguments(symbol)	/* int */(sym[symbol].spec.wlist.bstore/sizeof(word))
#define int_func_number(symbol) /* word */(sym[symbol].xx)
#define int_sub_arguments(symbol) /* word * */(sym[symbol].spec.wlist.ptr)
#define int_sub_num_arguments(symbol) /* int */(sym[symbol].spec.wlist.bstore/sizeof(word))
#define int_sub_routine_num(symbol) /* word */(sym[symbol].xx)
#define keyword_name(symbol)	/* char * */(string_value(keyword_name_symbol(symbol)))
#define keyword_name_symbol(symbol) /* word */(sym[symbol].spec.evb.args[0])
#define keyword_value(symbol)	/* word */(sym[symbol].spec.evb.args[1])
#define list_depth(symbol)	/* byte */(sym[symbol].type)
#define list_symbols(symbol) /* listElem * */(sym[symbol].spec.listElem.ptr)
#define list_key(symbol,i)	/* char * */(list_symbols(symbol)[i].key)
#define list_num_symbols(symbol) /* int */(sym[symbol].spec.listElem.bstore/sizeof(listElem))
#define list_symbol(symbol,i) /* word */(list_symbols(symbol)[i].value)
#define meta_target(symbol)	/* word */(sym[symbol].spec.evb.args[0])
#define ncase_else(symbol)	/* word */(ncase_statements(symbol)[ncase_num_statements(symbol)])
#define ncase_num_statements(symbol) /* int */(sym[symbol].spec.wlist.bstore/sizeof(word) - 2)
#define ncase_statements(symbol) /* word * */((word *) sym[symbol].spec.wlist.ptr)
#define ncase_switch_value(symbol) /* word */(*((word *) sym[symbol].spec.wlist.ptr + sym[symbol].spec.wlist.bstore/sizeof(word) - 1))
#define pre_clist_num_symbols(symbol) /* int */(sym[symbol].spec.wlist.bstore/sizeof(word))
#define pre_clist_symbols(symbol)	/* word * */(sym[symbol].spec.wlist.ptr)
#define pre_extract_data(symbol) /* preExtract * */(sym[symbol].spec.preExtract.ptr)
#define pre_extract_name(symbol)	/* char */(sym[symbol].spec.preExtract.ptr->name)
#define pre_extract_num_sec(symbol)	/* byte */(sym[symbol].type)
#define pre_extract_ptr(symbol)	/* extractSec * */(sym[symbol].spec.preExtract.ptr->extract)
#define pre_range_end(symbol)	/* word */(sym[symbol].spec.evb.args[1])
#define pre_range_redirect(symbol) /* word */(sym[symbol].spec.evb.args[3])
#define pre_range_scalar(symbol) /* byte */(sym[symbol].type)
#define pre_range_start(symbol) /* word */(sym[symbol].spec.evb.args[0])
#define pre_range_sum(symbol)	/* word */(sym[symbol].spec.evb.args[2])
#define pre_list_symbols(symbol) /* listElem * */(sym[symbol].spec.listElem.ptr)
#define pre_list_key(symbol,i) /* char * */(pre_list_symbols(symbol)[i].key)
#define pre_list_num_symbols(symbol) /* int */(sym[symbol].spec.listElem.bstore/sizeof(listElem))
#define pre_list_symbol(symbol,i) /* word */(pre_list_symbols(symbol)[i].value)
#define range_end(symbol)	/* word */(sym[symbol].spec.evb.args[1])
#define range_redirect(symbol)	/* word */(sym[symbol].spec.evb.args[3])
#define range_scalar(symbol)	/* byte */(sym[symbol].type)
#define range_start(symbol)	/* word */(sym[symbol].spec.evb.args[0])
#define range_sum(symbol)	/* word */(sym[symbol].spec.evb.args[2])
#define repeat_body(symbol)	/* word */(sym[symbol].spec.evb.args[0])
#define repeat_condition(symbol) /* word */(sym[symbol].spec.evb.args[1])
#define replace_lhs(symbol)	/* word */(sym[symbol].spec.evb.args[0])
#define replace_rhs(symbol)	/* word */(sym[symbol].spec.evb.args[1])
#define return_value(symbol)	/* word */(sym[symbol].spec.evb.args[0])
#define routine_num_parameters(symbol) /* byte */(sym[symbol].spec.routine.narg)
#define routine_num_statements(symbol) /* uword */(sym[symbol].spec.routine.nstmnt)
#define routine_parameter_names(symbol) /* char ** */(sym[symbol].spec.routine.keys)
#define routine_parameters(symbol) /* word * */(sym[symbol].spec.routine.ptr)
#define routine_statements(symbol) /* word * */(routine_parameters(symbol) + routine_num_parameters(symbol))
#define routine_has_extended_param(symbol) /* byte */(sym[symbol].spec.routine.extend)
#define run_block_number(symbol) /* word */(sym[symbol].xx)
#define scal_ptr_pointer(symbol) /* pointer */(sym[symbol].spec.pointer)
#define scal_ptr_type(symbol)	/* byte */(sym[symbol].type)
#define scalar_type(symbol)	/* byte */(sym[symbol].type)
#define scalar_value(symbol)	/* scalar */(sym[symbol].spec.scalar)
#define string_size(symbol)	/* int */(sym[symbol].spec.name.bstore - (int) 1)
#define string_type(symbol)	/* byte */(sym[symbol].type)
#define string_value(symbol)	/* char * */(sym[symbol].spec.name.ptr)
#define list_ptr_tag_number(symbol) /* int */(sym[symbol].spec.scalar.l)
#define list_ptr_tag_string(symbol) /* char * */(sym[symbol].spec.name.ptr)
#define list_ptr_tag_size(symbol) /* int */ (sym[symbol].spec.name.bstore)
#define list_ptr_target(symbol) /* word */(sym[symbol].xx)
#define struct_depth(symbol)	/* byte */(sym[symbol].type)
#define struct_num_top_elements(symbol) /* int */(struct_elements(symbol)[0].u.first.nelem)
#define struct_num_all_elements(symbol) /* int */(sym[symbol].spec.intList.ptr[0])
#define struct_total_size(symbol) /* int */(struct_elements(symbol)[0].u.first.size)
#define struct_elements(symbol)	/* structElem * */((structElem *) (sym[symbol].spec.intList.ptr + 1))
#define struct_data(symbol)	/* void * */((void *)(sym[symbol].spec.name.ptr + (struct_num_all_elements(symbol)*sizeof(structElem) + sizeof(int))))
#define struct_ptr_n_elements(symbol) /* word */(sym[symbol].spec.structPtr.bstore/sizeof(structPtr))
#define struct_ptr_elements(symbol)	/* structPtr * */(sym[symbol].spec.structPtr.ptr)
#define struct_ptr_target(symbol) /* word */(sym[symbol].xx)
#define subsc_ptr_end(symbol)	/* int */((sym[symbol].spec.intList.ptr)[1])
#define subsc_ptr_redirect(symbol) /* int */((sym[symbol].spec.intList.ptr)[3])
#define subsc_ptr_start(symbol) /* int */((sym[symbol].spec.intList.ptr)[0])
#define subsc_ptr_sum(symbol)	/* int */((sym[symbol].spec.intList.ptr)[2])
#define symbol_class(symbol)	/* byte */ (sym[symbol].class)
#define symbol_context(symbol)	/* word */(sym[symbol].context)
#define symbol_data(symbol)	/* void * */(sym[symbol].spec.general.ptr)
#define symbol_memory(symbol)	/* int */(sym[symbol].spec.general.bstore)
#define symbol_type(symbol)	/* byte */(sym[symbol].type)
#define symbol_line(symbol)	/* int */(sym[symbol].line)
#define symbol_extra(symbol)	/* union specUnion */(sym[symbol].spec)
#define transfer_is_parameter(symbol) /* byte */(sym[symbol].type)
#define transfer_target(symbol) /* word */(sym[symbol].spec.evb.args[0])
#define transfer_temp_param(symbol) /* word */(sym[symbol].spec.evb.args[1])
#define undefined_par(symbol)	/* char */(sym[symbol].type)
#define usr_code_routine_num(symbol) /* word */(sym[symbol].xx)
#define usr_func_arguments(symbol) /* word * */(sym[symbol].spec.wlist.ptr)
#define usr_func_num_arguments(symbol) /* int */(sym[symbol].spec.wlist.bstore/sizeof(word))
#define usr_func_number(symbol) /* word */(sym[symbol].xx)
#define usr_routine_arguments(symbol) /* word * */(sym[symbol].spec.wlist.ptr)
#define usr_routine_num_arguments(symbol) /* int */(sym[symbol].spec.wlist.bstore/sizeof(word))
#define usr_routine_num(symbol) /* word */(sym[symbol].xx)
#define usr_routine_recursion(symbol) /* byte */(sym[symbol].type)
#define usr_sub_arguments(symbol) /* word * */(sym[symbol].spec.wlist.ptr)
#define usr_sub_num_arguments(symbol) /* int */(sym[symbol].spec.wlist.bstore/sizeof(word))
#define usr_sub_routine_num(symbol) /* word */(sym[symbol].xx)
#define usr_sub_is_deferred(symbol) /* int */(symbol_class(usr_sub_routine_num(symbol)) == ANA_STRING)
#define while_do_body(symbol)	/* word */(sym[symbol].spec.evb.args[1])
#define while_do_condition(symbol) /* word */(sym[symbol].spec.evb.args[0])


#define symbolIsArray(iq)	/* int */ (symbol_class(iq) == ANA_ARRAY || symbol_class(iq) == ANA_CARRAY)
#define symbolIsNumericalArray(iq)	/* int */ (symbolIsArray(iq) && symbolIsNumerical(iq))
#define symbolIsStringArray(iq)	/* int */ (symbol_class(iq) == ANA_ARRAY && symbol_type(iq) == ANA_STRING_ARRAY)
#define symbolIsRealArray(iq)	/* int */ (symbol_class(iq) == ANA_ARRAY && symbol_type(iq) <= ANA_DOUBLE)
#define symbolIsRealScalar(iq)	/* int */ ((symbol_class(iq) == ANA_SCALAR || symbol_class(iq) == ANA_SCAL_PTR) && isRealType(symbol_type(iq)))
#define symbolIsComplexArray(iq) /* int */ (symbol_class(iq) == ANA_CARRAY)
#define symbolIsComplexScalar(iq) /* int */ (symbol_class(iq) == ANA_CSCALAR || (symbol_class(iq) == ANA_SCAL_PTR && isComplexType(symbol_type(iq))))
#define symbolIsNumerical(iq)	/* int */ ((symbolIsArray(iq) || symbolIsScalar(iq)) && isNumericalType(symbol_type(iq)))
#define symbolIsReal(iq)        /* int */ (symbolIsNumerical(iq) && isRealType(symbol_type(iq)))
#define symbolIsInteger(iq)     /* int */ (symbolIsNumerical(iq) && isIntegerType(symbol_type(iq)))
#define symbolIsComplex(iq)     /* int */ (symbolIsNumerical(iq) && isComplexType(symbol_type(iq)))
#define symbolIsString(iq)      /* int */ (symbolIsStringScalar(iq) || symbolIsStringArray(iq))
#define symbolIsStringScalar(iq) /* int */ (symbol_class(iq) == ANA_STRING)
#define symbolIsScalar(iq)		/* int */ (symbol_class(iq) == ANA_SCALAR || symbol_class(iq) == ANA_SCAL_PTR || symbol_class(iq) == ANA_CSCALAR)
#define symbolIsUnitaryExpression(iq) /* int */ (symbol_class(iq) < ANA_SUBROUTINE)
#define symbolIsNamed(iq)	/* int */ (symbolProperName(iq) != NULL)
#define symbolIsModifiable(iq)  /* int */ (symbolIsNamed((iq)) && (iq) > nFixed)
#define numericalSize(iq)	/* int */ ((symbol_class(iq) == ANA_ARRAY)? array_size(iq): 1)
#define isNumericalType(type)	/* int */ (type <= ANA_DOUBLE || type >= ANA_CFLOAT)
#define isStringType(type)	/* int */ (type > ANA_DOUBLE && type < ANA_CFLOAT)
#define isRealType(type)	/* int */ (type <= ANA_DOUBLE)
#define isComplexType(type)	/* int */ (type >= ANA_CFLOAT)
#define isIntegerType(type)	/* int */ (type <= ANA_LONG)
#define isFloatType(type)	/* int */ (type == ANA_FLOAT || type == ANA_DOUBLE || type == ANA_CFLOAT || type == ANA_CDOUBLE)
#define isLegalType(type)	/* int */ (type >= ANA_BYTE && type <= ANA_CDOUBLE)
/* highestType(type1,type2) returns the highest type among its arguments.
   If no complex data types are involved, then it is straightforward:
   Return whichever type is numerically greater.  If complex data types
   are involved, then the same trick will do, except if type1 == ANA_CFLOAT
   and type2 == ANA_DOUBLE: then the simple comparison would return
   ANA_CFLOAT, while we want ANA_CDOUBLE (i.e., ANA_DOUBLE promoted to
   a complex type). */
#define highestType(type1,type2) /* int */ ((type1 == ANA_CFLOAT && type2 == ANA_DOUBLE)? ANA_CDOUBLE: (type2 > type1)? type2: type1)
/* realType(type) returns <type> if it is real, or the corresponding real */
/* type if it is complex. */
#define realType(type)		/* int */ ((type >= ANA_CFLOAT)? type + (ANA_FLOAT - ANA_CFLOAT): type)
#define complexMag2(x) ((x).real*(x).real + (x).imaginary*(x).imaginary)
