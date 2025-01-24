/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 6 "luxparser.yy"

/* This is file luxparser.c.

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
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "readline/readline.h"
#include "readline/history.h"
#include "action.hh"
#include "luxparser.hh"
#include "editor.hh"
#define YYERROR_VERBOSE
#define startList(x)    { pushList(LUX_NEW_LIST); pushList(x); }
                                /* start a new list */
extern int32_t  scrat[],        /* general-purpose scratch storage (once.h) */
                compileLevel,   /* number of nested open input files */
                executeLevel,   /* number of nested execution items */
                symbolStackIndex, /* next free slot in symbol stack */
  setup, LUX_MATMUL_FUN;
 extern char const* symbolStack[];      /* stack of not-yet parsed symbols */
 extern char  line[],           /* raw user input */
                tLine[];        /* translated user input */
extern int16_t  curContext,     /* context of current execution */
                listStack[],    /* stack of unincorporated list items */
                *listStackItem; /* next free list stack item */
extern HashTableEntry   *varHashTable[], /* name hash table for variables */
                *funcHashTable[], /* name hash table for functions */
                *blockHashTable[]; /* name hash table for block routines */
extern SymbolImpl sym[];  /* all symbols */
char    debugLine = 0,          /* we're not in a debugger line */
        errorState = 0,         /* we've not just experienced an error */
        compileOnly = 0;        /* not just compiling but also executing */
uint8_t disableNewline = 0;     /* disables NL token so that complex */
                                /* structures can be parsed across newlines */
void    pushList(int16_t),              /* push symbol number onto list stack */
        swapList(int32_t, int32_t),     /* swap items in the list stack */
        cleanUp(int32_t, int32_t),
        away(void);
int16_t popList(void);          /* pop an item from the list stack's top */
int32_t stackListLength(void),  /* return length of list at top of stack */
        isInternalSubr(int32_t),        /* 1 if symbol is internal subroutine */
        installExec(void),
        findSym(int32_t, HashTableEntry *[], int32_t),
        installSubsc(int32_t),
        addSubsc(int32_t, int32_t, int32_t), newSubrSymbol(int32_t),
        newSymbol(Symbolclass, ...), newBlockSymbol(int32_t), copySym(int32_t),
        luxerror(char const *, int32_t, ...);
int32_t statementDepth = 0, keepEVB = 0;
int32_t yyerror(const char *), yylex(YYSTYPE *);
int32_t installString(char const* string);

#line 141 "luxparser.cc"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_YY_LUXPARSER_HH_INCLUDED
# define YY_YY_LUXPARSER_HH_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 1000,                /* "invalid token"  */
    TOK_NL = 999,                  /* TOK_NL  */
    TOK_C_ID = 1001,               /* TOK_C_ID  */
    TOK_S_ID = 1002,               /* TOK_S_ID  */
    TOK_NUMBER = 1003,             /* TOK_NUMBER  */
    TOK_STR = 1004,                /* TOK_STR  */
    TOK_INCLUDE = 1005,            /* TOK_INCLUDE  */
    TOK_REPORT = 1006,             /* TOK_REPORT  */
    TOK_IF = 1007,                 /* TOK_IF  */
    TOK_THEN = 1008,               /* TOK_THEN  */
    TOK_ELSE = 1009,               /* TOK_ELSE  */
    TOK_FOR = 1010,                /* TOK_FOR  */
    TOK_REPEAT = 1011,             /* TOK_REPEAT  */
    TOK_UNTIL = 1012,              /* TOK_UNTIL  */
    TOK_WHILE = 1013,              /* TOK_WHILE  */
    TOK_DO = 1014,                 /* TOK_DO  */
    TOK_CASE = 1015,               /* TOK_CASE  */
    TOK_ENDCASE = 1016,            /* TOK_ENDCASE  */
    TOK_NCASE = 1017,              /* TOK_NCASE  */
    TOK_SUBR = 1018,               /* TOK_SUBR  */
    TOK_ENDSUBR = 1019,            /* TOK_ENDSUBR  */
    TOK_FUNC = 1020,               /* TOK_FUNC  */
    TOK_ENDFUNC = 1021,            /* TOK_ENDFUNC  */
    TOK_BLOCK = 1022,              /* TOK_BLOCK  */
    TOK_ENDBLOCK = 1023,           /* TOK_ENDBLOCK  */
    TOK_RETURN = 1024,             /* TOK_RETURN  */
    TOK_BREAK = 1025,              /* TOK_BREAK  */
    TOK_CONTINUE = 1026,           /* TOK_CONTINUE  */
    TOK_RUN = 1027,                /* TOK_RUN  */
    TOK_BEGIN = 1028,              /* TOK_BEGIN  */
    TOK_END = 1029,                /* TOK_END  */
    TOK_PLUSIS = 1030,             /* TOK_PLUSIS  */
    TOK_MINUSIS = 1031,            /* TOK_MINUSIS  */
    TOK_TIMESIS = 1032,            /* TOK_TIMESIS  */
    TOK_DIVIDEIS = 1033,           /* TOK_DIVIDEIS  */
    TOK_POWERIS = 1034,            /* TOK_POWERIS  */
    TOK_RETALL = 1035,             /* TOK_RETALL  */
    TOK_STRUCTTAG = 1036,          /* TOK_STRUCTTAG  */
    TOK_ERRORSTATE = 1037,         /* TOK_ERRORSTATE  */
    TOK_ELLIPSIS = 1038,           /* TOK_ELLIPSIS  */
    TOK_ANDIF = 1039,              /* TOK_ANDIF  */
    TOK_ORIF = 1040,               /* TOK_ORIF  */
    TOK_AND = 1041,                /* TOK_AND  */
    TOK_OR = 1042,                 /* TOK_OR  */
    TOK_XOR = 1043,                /* TOK_XOR  */
    TOK_GE = 1044,                 /* TOK_GE  */
    TOK_LE = 1045,                 /* TOK_LE  */
    TOK_GT = 1046,                 /* TOK_GT  */
    TOK_LT = 1047,                 /* TOK_LT  */
    TOK_EQ = 1048,                 /* TOK_EQ  */
    TOK_NE = 1049,                 /* TOK_NE  */
    TOK_SMOD = 1050,               /* TOK_SMOD  */
    TOK_UMINUS = 1051              /* TOK_UMINUS  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 1000
#define TOK_NL 999
#define TOK_C_ID 1001
#define TOK_S_ID 1002
#define TOK_NUMBER 1003
#define TOK_STR 1004
#define TOK_INCLUDE 1005
#define TOK_REPORT 1006
#define TOK_IF 1007
#define TOK_THEN 1008
#define TOK_ELSE 1009
#define TOK_FOR 1010
#define TOK_REPEAT 1011
#define TOK_UNTIL 1012
#define TOK_WHILE 1013
#define TOK_DO 1014
#define TOK_CASE 1015
#define TOK_ENDCASE 1016
#define TOK_NCASE 1017
#define TOK_SUBR 1018
#define TOK_ENDSUBR 1019
#define TOK_FUNC 1020
#define TOK_ENDFUNC 1021
#define TOK_BLOCK 1022
#define TOK_ENDBLOCK 1023
#define TOK_RETURN 1024
#define TOK_BREAK 1025
#define TOK_CONTINUE 1026
#define TOK_RUN 1027
#define TOK_BEGIN 1028
#define TOK_END 1029
#define TOK_PLUSIS 1030
#define TOK_MINUSIS 1031
#define TOK_TIMESIS 1032
#define TOK_DIVIDEIS 1033
#define TOK_POWERIS 1034
#define TOK_RETALL 1035
#define TOK_STRUCTTAG 1036
#define TOK_ERRORSTATE 1037
#define TOK_ELLIPSIS 1038
#define TOK_ANDIF 1039
#define TOK_ORIF 1040
#define TOK_AND 1041
#define TOK_OR 1042
#define TOK_XOR 1043
#define TOK_GE 1044
#define TOK_LE 1045
#define TOK_GT 1046
#define TOK_LT 1047
#define TOK_EQ 1048
#define TOK_NE 1049
#define TOK_SMOD 1050
#define TOK_UMINUS 1051

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif




int yyparse (void);


#endif /* !YY_YY_LUXPARSER_HH_INCLUDED  */
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_TOK_NL = 3,                     /* TOK_NL  */
  YYSYMBOL_TOK_C_ID = 4,                   /* TOK_C_ID  */
  YYSYMBOL_TOK_S_ID = 5,                   /* TOK_S_ID  */
  YYSYMBOL_TOK_NUMBER = 6,                 /* TOK_NUMBER  */
  YYSYMBOL_TOK_STR = 7,                    /* TOK_STR  */
  YYSYMBOL_TOK_INCLUDE = 8,                /* TOK_INCLUDE  */
  YYSYMBOL_TOK_REPORT = 9,                 /* TOK_REPORT  */
  YYSYMBOL_TOK_IF = 10,                    /* TOK_IF  */
  YYSYMBOL_TOK_THEN = 11,                  /* TOK_THEN  */
  YYSYMBOL_TOK_ELSE = 12,                  /* TOK_ELSE  */
  YYSYMBOL_TOK_FOR = 13,                   /* TOK_FOR  */
  YYSYMBOL_TOK_REPEAT = 14,                /* TOK_REPEAT  */
  YYSYMBOL_TOK_UNTIL = 15,                 /* TOK_UNTIL  */
  YYSYMBOL_TOK_WHILE = 16,                 /* TOK_WHILE  */
  YYSYMBOL_TOK_DO = 17,                    /* TOK_DO  */
  YYSYMBOL_TOK_CASE = 18,                  /* TOK_CASE  */
  YYSYMBOL_TOK_ENDCASE = 19,               /* TOK_ENDCASE  */
  YYSYMBOL_TOK_NCASE = 20,                 /* TOK_NCASE  */
  YYSYMBOL_TOK_SUBR = 21,                  /* TOK_SUBR  */
  YYSYMBOL_TOK_ENDSUBR = 22,               /* TOK_ENDSUBR  */
  YYSYMBOL_TOK_FUNC = 23,                  /* TOK_FUNC  */
  YYSYMBOL_TOK_ENDFUNC = 24,               /* TOK_ENDFUNC  */
  YYSYMBOL_TOK_BLOCK = 25,                 /* TOK_BLOCK  */
  YYSYMBOL_TOK_ENDBLOCK = 26,              /* TOK_ENDBLOCK  */
  YYSYMBOL_TOK_RETURN = 27,                /* TOK_RETURN  */
  YYSYMBOL_TOK_BREAK = 28,                 /* TOK_BREAK  */
  YYSYMBOL_TOK_CONTINUE = 29,              /* TOK_CONTINUE  */
  YYSYMBOL_TOK_RUN = 30,                   /* TOK_RUN  */
  YYSYMBOL_TOK_BEGIN = 31,                 /* TOK_BEGIN  */
  YYSYMBOL_TOK_END = 32,                   /* TOK_END  */
  YYSYMBOL_TOK_PLUSIS = 33,                /* TOK_PLUSIS  */
  YYSYMBOL_TOK_MINUSIS = 34,               /* TOK_MINUSIS  */
  YYSYMBOL_TOK_TIMESIS = 35,               /* TOK_TIMESIS  */
  YYSYMBOL_TOK_DIVIDEIS = 36,              /* TOK_DIVIDEIS  */
  YYSYMBOL_TOK_POWERIS = 37,               /* TOK_POWERIS  */
  YYSYMBOL_TOK_RETALL = 38,                /* TOK_RETALL  */
  YYSYMBOL_TOK_STRUCTTAG = 39,             /* TOK_STRUCTTAG  */
  YYSYMBOL_TOK_ERRORSTATE = 40,            /* TOK_ERRORSTATE  */
  YYSYMBOL_41_ = 41,                       /* '='  */
  YYSYMBOL_TOK_ELLIPSIS = 42,              /* TOK_ELLIPSIS  */
  YYSYMBOL_TOK_ANDIF = 43,                 /* TOK_ANDIF  */
  YYSYMBOL_TOK_ORIF = 44,                  /* TOK_ORIF  */
  YYSYMBOL_TOK_AND = 45,                   /* TOK_AND  */
  YYSYMBOL_TOK_OR = 46,                    /* TOK_OR  */
  YYSYMBOL_TOK_XOR = 47,                   /* TOK_XOR  */
  YYSYMBOL_48_ = 48,                       /* '&'  */
  YYSYMBOL_49_ = 49,                       /* '|'  */
  YYSYMBOL_TOK_GE = 50,                    /* TOK_GE  */
  YYSYMBOL_TOK_LE = 51,                    /* TOK_LE  */
  YYSYMBOL_TOK_GT = 52,                    /* TOK_GT  */
  YYSYMBOL_TOK_LT = 53,                    /* TOK_LT  */
  YYSYMBOL_TOK_EQ = 54,                    /* TOK_EQ  */
  YYSYMBOL_TOK_NE = 55,                    /* TOK_NE  */
  YYSYMBOL_56_ = 56,                       /* '<'  */
  YYSYMBOL_57_ = 57,                       /* '>'  */
  YYSYMBOL_58_ = 58,                       /* '+'  */
  YYSYMBOL_59_ = 59,                       /* '-'  */
  YYSYMBOL_60_ = 60,                       /* '*'  */
  YYSYMBOL_61_ = 61,                       /* '/'  */
  YYSYMBOL_62_ = 62,                       /* '\\'  */
  YYSYMBOL_63_ = 63,                       /* '%'  */
  YYSYMBOL_64_ = 64,                       /* '#'  */
  YYSYMBOL_TOK_SMOD = 65,                  /* TOK_SMOD  */
  YYSYMBOL_TOK_UMINUS = 66,                /* TOK_UMINUS  */
  YYSYMBOL_67_ = 67,                       /* '^'  */
  YYSYMBOL_68_ = 68,                       /* '('  */
  YYSYMBOL_69_ = 69,                       /* ','  */
  YYSYMBOL_70_ = 70,                       /* '{'  */
  YYSYMBOL_71_ = 71,                       /* '}'  */
  YYSYMBOL_72_ = 72,                       /* ')'  */
  YYSYMBOL_73_ = 73,                       /* ':'  */
  YYSYMBOL_74_ = 74,                       /* '['  */
  YYSYMBOL_75_ = 75,                       /* ']'  */
  YYSYMBOL_YYACCEPT = 76,                  /* $accept  */
  YYSYMBOL_lines = 77,                     /* lines  */
  YYSYMBOL_next_line = 78,                 /* next_line  */
  YYSYMBOL_statement = 79,                 /* statement  */
  YYSYMBOL_80_1 = 80,                      /* $@1  */
  YYSYMBOL_opt_arg = 81,                   /* opt_arg  */
  YYSYMBOL_begingroup = 82,                /* begingroup  */
  YYSYMBOL_endgroup = 83,                  /* endgroup  */
  YYSYMBOL_assignment = 84,                /* assignment  */
  YYSYMBOL_tag_list = 85,                  /* tag_list  */
  YYSYMBOL_member_spec = 86,               /* member_spec  */
  YYSYMBOL_lhs = 87,                       /* lhs  */
  YYSYMBOL_var = 88,                       /* var  */
  YYSYMBOL_struct_list = 89,               /* struct_list  */
  YYSYMBOL_struct_element = 90,            /* struct_element  */
  YYSYMBOL_value = 91,                     /* value  */
  YYSYMBOL_expr = 92,                      /* expr  */
  YYSYMBOL_expr_list = 93,                 /* expr_list  */
  YYSYMBOL_range = 94,                     /* range  */
  YYSYMBOL_subsc = 95,                     /* subsc  */
  YYSYMBOL_subsc_or_key = 96,              /* subsc_or_key  */
  YYSYMBOL_subsc_list = 97,                /* subsc_list  */
  YYSYMBOL_op_assign = 98,                 /* op_assign  */
  YYSYMBOL_routine_execution = 99,         /* routine_execution  */
  YYSYMBOL_s_arglist = 100,                /* s_arglist  */
  YYSYMBOL_arglist = 101,                  /* arglist  */
  YYSYMBOL_key_param = 102,                /* key_param  */
  YYSYMBOL_key = 103,                      /* key  */
  YYSYMBOL_arg = 104,                      /* arg  */
  YYSYMBOL_routine_definition = 105,       /* routine_definition  */
  YYSYMBOL_106_2 = 106,                    /* $@2  */
  YYSYMBOL_107_3 = 107,                    /* @3  */
  YYSYMBOL_108_4 = 108,                    /* $@4  */
  YYSYMBOL_109_5 = 109,                    /* @5  */
  YYSYMBOL_110_6 = 110,                    /* $@6  */
  YYSYMBOL_111_7 = 111,                    /* @7  */
  YYSYMBOL_endsubr = 112,                  /* endsubr  */
  YYSYMBOL_endfunc = 113,                  /* endfunc  */
  YYSYMBOL_endblock = 114,                 /* endblock  */
  YYSYMBOL_paramlist2 = 115,               /* paramlist2  */
  YYSYMBOL_s_paramlist = 116,              /* s_paramlist  */
  YYSYMBOL_paramlist = 117,                /* paramlist  */
  YYSYMBOL_statement_list = 118,           /* statement_list  */
  YYSYMBOL_f_paramlist = 119,              /* f_paramlist  */
  YYSYMBOL_selection = 120,                /* selection  */
  YYSYMBOL_121_8 = 121,                    /* $@8  */
  YYSYMBOL_122_9 = 122,                    /* $@9  */
  YYSYMBOL_123_10 = 123,                   /* $@10  */
  YYSYMBOL_124_11 = 124,                   /* $@11  */
  YYSYMBOL_opt_then = 125,                 /* opt_then  */
  YYSYMBOL_opt_else = 126,                 /* opt_else  */
  YYSYMBOL_opt_case_else = 127,            /* opt_case_else  */
  YYSYMBOL_128_12 = 128,                   /* $@12  */
  YYSYMBOL_129_13 = 129,                   /* $@13  */
  YYSYMBOL_case_list = 130,                /* case_list  */
  YYSYMBOL_loop = 131,                     /* loop  */
  YYSYMBOL_132_14 = 132,                   /* $@14  */
  YYSYMBOL_133_15 = 133,                   /* $@15  */
  YYSYMBOL_134_16 = 134,                   /* $@16  */
  YYSYMBOL_135_17 = 135,                   /* $@17  */
  YYSYMBOL_136_18 = 136,                   /* $@18  */
  YYSYMBOL_137_19 = 137,                   /* $@19  */
  YYSYMBOL_138_20 = 138,                   /* $@20  */
  YYSYMBOL_139_21 = 139,                   /* $@21  */
  YYSYMBOL_opt_step = 140,                 /* opt_step  */
  YYSYMBOL_opt_do = 141                    /* opt_do  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  53
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1211

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  76
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  66
/* YYNRULES -- Number of rules.  */
#define YYNRULES  184
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  300

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   1051


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,    64,     2,    63,    48,     2,
      68,    72,    60,    58,    69,    59,     2,    61,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    73,     2,
      56,    41,    57,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    74,    62,    75,    67,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    70,    49,    71,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     3,
       2,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    42,    43,
      44,    45,    46,    47,    50,    51,    52,    53,    54,    55,
      65,    66
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   129,   129,   137,   148,   170,   180,   181,   182,   183,
     184,   185,   188,   188,   200,   201,   202,   205,   208,   209,
     210,   211,   216,   220,   221,   225,   225,   229,   229,   233,
     236,   247,   248,   253,   254,   261,   262,   263,   270,   275,
     280,   286,   287,   291,   298,   302,   306,   313,   314,   320,
     321,   322,   327,   332,   335,   338,   344,   347,   353,   357,
     360,   366,   369,   378,   381,   384,   387,   390,   393,   396,
     399,   402,   405,   408,   411,   414,   417,   420,   423,   426,
     429,   432,   435,   438,   441,   444,   447,   452,   456,   462,
     463,   473,   476,   479,   482,   485,   488,   491,   494,   505,
     507,   512,   517,   522,   527,   532,   540,   542,   544,   549,
     553,   560,   564,   568,   572,   579,   586,   587,   594,   598,
     605,   611,   615,   622,   624,   629,   631,   629,   639,   641,
     639,   649,   651,   649,   661,   661,   665,   665,   669,   669,
     674,   676,   684,   688,   693,   697,   704,   709,   719,   723,
     725,   730,   732,   730,   738,   738,   746,   746,   757,   760,
     765,   769,   776,   780,   780,   787,   787,   797,   802,   810,
     812,   810,   819,   821,   819,   827,   829,   827,   835,   837,
     835,   846,   850,   855,   859
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "TOK_NL", "TOK_C_ID",
  "TOK_S_ID", "TOK_NUMBER", "TOK_STR", "TOK_INCLUDE", "TOK_REPORT",
  "TOK_IF", "TOK_THEN", "TOK_ELSE", "TOK_FOR", "TOK_REPEAT", "TOK_UNTIL",
  "TOK_WHILE", "TOK_DO", "TOK_CASE", "TOK_ENDCASE", "TOK_NCASE",
  "TOK_SUBR", "TOK_ENDSUBR", "TOK_FUNC", "TOK_ENDFUNC", "TOK_BLOCK",
  "TOK_ENDBLOCK", "TOK_RETURN", "TOK_BREAK", "TOK_CONTINUE", "TOK_RUN",
  "TOK_BEGIN", "TOK_END", "TOK_PLUSIS", "TOK_MINUSIS", "TOK_TIMESIS",
  "TOK_DIVIDEIS", "TOK_POWERIS", "TOK_RETALL", "TOK_STRUCTTAG",
  "TOK_ERRORSTATE", "'='", "TOK_ELLIPSIS", "TOK_ANDIF", "TOK_ORIF",
  "TOK_AND", "TOK_OR", "TOK_XOR", "'&'", "'|'", "TOK_GE", "TOK_LE",
  "TOK_GT", "TOK_LT", "TOK_EQ", "TOK_NE", "'<'", "'>'", "'+'", "'-'",
  "'*'", "'/'", "'\\\\'", "'%'", "'#'", "TOK_SMOD", "TOK_UMINUS", "'^'",
  "'('", "','", "'{'", "'}'", "')'", "':'", "'['", "']'", "$accept",
  "lines", "next_line", "statement", "$@1", "opt_arg", "begingroup",
  "endgroup", "assignment", "tag_list", "member_spec", "lhs", "var",
  "struct_list", "struct_element", "value", "expr", "expr_list", "range",
  "subsc", "subsc_or_key", "subsc_list", "op_assign", "routine_execution",
  "s_arglist", "arglist", "key_param", "key", "arg", "routine_definition",
  "$@2", "@3", "$@4", "@5", "$@6", "@7", "endsubr", "endfunc", "endblock",
  "paramlist2", "s_paramlist", "paramlist", "statement_list",
  "f_paramlist", "selection", "$@8", "$@9", "$@10", "$@11", "opt_then",
  "opt_else", "opt_case_else", "$@12", "$@13", "case_list", "loop", "$@14",
  "$@15", "$@16", "$@17", "$@18", "$@19", "$@20", "$@21", "opt_step",
  "opt_do", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-147)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-166)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     901,  -147,  -147,    -7,  -147,  -147,  -147,  -147,  -147,  -147,
    -147,  -147,  -147,  -147,  -147,  -147,  -147,   -16,  -147,  -147,
       6,  -147,  -147,  -147,  -147,   108,  -147,  -147,  -147,  -147,
     185,   -24,  -147,  -147,  -147,  -147,   284,  -147,   585,    29,
     939,   585,   939,    15,   585,    31,    35,    41,   585,  -147,
    -147,    19,  -147,  -147,  -147,   939,  -147,  -147,  -147,  -147,
     585,   585,  -147,   344,    18,   -22,    22,  -147,  -147,    19,
     585,   585,    56,   508,   513,   585,  -147,   -20,  -147,  1089,
      -5,    26,  -147,  -147,   943,    46,    32,  1089,    54,    20,
      24,   661,   -19,   -19,  -147,  1089,  -147,  -147,   466,  1089,
    1089,   585,   585,    25,   973,    21,  -147,  -147,     4,  -147,
    -147,   344,    18,  -147,   -30,   -30,  -147,  -147,   387,     7,
      23,  -147,    11,  -147,  1089,  1089,     3,    96,   585,   585,
     585,   585,   585,   585,   585,   585,   585,   585,   585,   585,
     585,   585,   585,   585,   585,   585,   585,   585,   585,   585,
     585,   585,   -22,   284,   585,  -147,  -147,   585,  -147,    71,
    -147,   939,    33,    37,    70,   729,    14,    88,  -147,  -147,
    -147,   939,  -147,  -147,  -147,  -147,  1089,   585,   495,     8,
    -147,   344,  -147,     5,  -147,  -147,   540,  -147,  -147,   585,
    -147,   590,  -147,  -147,   585,  -147,  -147,  1116,  1116,  1143,
    1143,  1143,  1143,  1143,   133,   133,   133,   133,   133,   133,
     262,   262,   149,   149,   -30,   -30,   -30,   -30,   -30,   -30,
     -30,  -147,  1089,   939,  1035,   585,  -147,   939,   585,  -147,
     939,    42,   939,  -147,    76,  -147,  -147,    27,    28,  -147,
     939,   939,   775,  1004,   585,   585,    45,  1089,   585,  -147,
    -147,  -147,  -147,  1089,  -147,  1089,    93,   585,  1089,  -147,
    1089,  -147,   939,  -147,  -147,  -147,    10,   819,   863,  -147,
    -147,  -147,   553,  1089,   585,  1089,   939,  -147,  1062,  -147,
    -147,  -147,  -147,  -147,  -147,  -147,  -147,  -147,    61,  1089,
    1089,  -147,   585,    71,   585,  1089,  -147,  1089,   939,  -147
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    21,     5,   117,    42,    16,    17,   151,   169,   172,
     178,   175,   154,   156,   125,   128,   131,    23,    14,    15,
       0,    25,    18,    22,    26,     0,     2,     4,    12,     6,
       0,    40,     7,     8,     9,    10,     0,   115,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      41,     0,    20,     1,     3,     0,   111,   112,   113,   114,
       0,     0,    31,     0,    35,    39,    41,    47,    48,     0,
       0,     0,     0,     0,     0,     0,    50,    40,    49,   123,
     116,     0,   124,   118,   158,     0,     0,   179,     0,     0,
     162,     0,   142,   142,   132,    24,    19,   146,     0,    29,
      30,     0,   102,    98,   107,    99,   106,   109,     0,   108,
      32,     0,    38,    53,    88,    87,   120,   122,     0,     0,
      41,    58,     0,    43,    46,    89,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    52,     0,     0,   159,   152,     0,   173,   183,
     176,     0,   163,     0,     0,     0,     0,     0,   150,   126,
     129,     0,    27,    28,   147,    13,   103,     0,     0,     0,
      34,     0,    33,     0,    60,    59,     0,    55,    54,     0,
      57,     0,    56,    62,     0,    61,    51,    84,    85,    79,
      80,    81,    82,    83,    75,    78,    76,    77,    73,    74,
      72,    71,    63,    64,    65,    66,    67,    68,    86,    69,
      70,   119,   121,     0,     0,     0,   184,     0,     0,   167,
       0,     0,     0,   155,     0,   144,   148,     0,   140,   143,
       0,     0,     0,    97,     0,   105,    93,    91,     0,   100,
     110,    37,    36,    45,    44,    90,   160,     0,   174,   180,
     177,   164,     0,   168,   157,   149,     0,     0,     0,   138,
     139,   133,     0,   104,     0,   101,     0,   153,   181,   166,
     145,   141,   134,   135,   127,   136,   137,   130,    96,    94,
      92,   161,     0,   183,     0,   182,   170,    95,     0,   171
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -147,  -147,    98,    43,  -147,  -147,  -147,  -147,  -147,   -62,
     -18,    16,     0,  -147,   -64,   -36,   233,  -147,    57,  -147,
     -49,    34,  -147,  -147,  -147,  -147,    62,   -34,   -13,  -147,
    -147,  -147,  -147,  -147,  -147,  -147,  -147,  -147,  -147,   -25,
    -147,  -147,   -90,    50,  -147,  -147,  -147,  -147,  -147,  -147,
    -147,    -6,  -147,  -147,  -147,  -147,  -147,  -147,  -147,  -147,
    -147,  -147,  -147,  -147,  -147,  -146
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    25,    26,    97,    55,    49,    28,   175,    29,    64,
     152,    76,    77,   122,   123,    78,   104,   126,   105,   106,
     107,   108,    61,    32,    37,    80,    81,   109,    83,    33,
      45,   240,    46,   241,    47,   171,   284,   287,   271,   237,
     168,   238,    98,   169,    34,    38,   223,    43,    44,   156,
     277,   164,   230,   231,    90,    35,    39,   298,    40,   225,
      42,   228,    41,   159,   293,   227
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      31,   165,    82,   112,   193,   180,   251,    89,   187,    62,
      50,     4,   190,    65,   280,    62,    30,    62,   235,    62,
      52,    67,    68,    50,     4,    31,   -41,   -41,   -41,   -41,
      67,    68,   -41,    85,   -41,    92,   162,   151,    63,    93,
      31,    30,    31,    27,    63,    94,   111,   158,   127,   166,
     167,    96,   281,    48,   163,    31,    30,   110,    30,    65,
     116,   -41,    36,  -120,   153,   248,   249,   154,    27,   113,
     160,    30,   194,   181,   181,    51,   182,   252,   195,   188,
     191,   242,   192,    86,   177,    88,   236,   157,   226,   233,
     112,    31,   235,   161,   179,   264,   189,   266,    31,   265,
      66,     4,    67,    68,   274,   276,  -165,    30,    53,     1,
     232,     2,     3,     4,    30,   262,     5,     6,     7,    82,
     294,     8,     9,    54,    10,    11,    12,   254,    13,    14,
     119,    15,   250,    16,   117,    17,    18,    19,    20,    21,
     221,   174,   239,   170,    69,   183,    22,   296,    23,     0,
     267,   268,     0,   101,   102,    71,   103,    72,     0,   234,
       0,    31,     0,     0,    73,    31,    74,     0,   196,     0,
      75,    31,    62,     0,     0,     0,     0,    30,    24,     0,
       0,    30,     0,     0,     0,     0,     0,    30,    62,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,     0,
     151,    63,     0,     0,   229,     0,     0,     0,   174,   145,
     146,   147,   148,   149,   150,     0,   151,    63,    56,    57,
      58,    59,     0,    31,     0,     0,    60,    31,     0,     0,
      31,     0,    31,     0,     0,     0,     0,     0,     0,    30,
      31,    31,    31,    30,     0,     0,    30,     0,    30,     0,
       0,     0,     0,     0,     0,     0,    30,    30,    30,     0,
       0,     0,    31,     0,     0,     0,   256,    31,    31,    79,
     259,    84,     0,   261,    87,   263,    31,    91,    30,     0,
       0,    95,     0,    30,    30,   174,     0,     0,    66,     4,
      67,    68,    30,    99,   100,     0,     0,     0,    31,     0,
       0,    62,     0,   114,   115,   279,   118,   124,   125,     0,
     174,   174,     0,     0,    30,     0,     0,     0,     0,   291,
     143,   144,   145,   146,   147,   148,   149,   150,     0,   151,
      63,     0,    69,     0,   176,   114,     0,     0,     0,     0,
       0,   299,    70,    71,     0,    72,     0,     0,    66,     4,
      67,    68,    73,     0,    74,     0,     0,     0,    75,     0,
       0,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,     0,    79,   222,   184,     0,
     224,     0,    69,     0,     0,     0,     0,     0,     0,     0,
       0,   101,   102,    71,   103,    72,     0,     0,     0,     0,
     243,   247,    73,     0,    74,     0,     0,     0,    75,   247,
       0,     0,   253,     0,   124,     0,    62,   255,     0,     0,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,     0,   151,    63,     0,     0,   258,   185,
     186,   260,     0,     0,     0,     0,     0,     1,     0,     0,
       3,     4,     0,     0,     5,     6,     7,   273,   114,     8,
       9,   275,    10,    11,    12,     0,    13,    14,     0,    15,
     278,    16,     0,    17,    18,    19,    20,    21,   172,    50,
       4,    67,    68,     0,    22,   289,    23,   290,     0,     0,
       0,     0,    50,     4,    67,    68,     0,   120,     4,    67,
      68,     0,     0,     0,     0,   295,     0,   297,     0,     0,
       0,     0,     0,     0,     0,     0,    24,   173,     0,     0,
       0,     0,     0,    69,    50,     4,    67,    68,     0,     0,
       0,     0,   244,   245,    71,   246,    69,    50,     4,    67,
      68,    69,     0,    73,     0,    74,    70,    71,   103,    75,
       0,    70,    71,     0,     0,     0,    73,     0,    74,     0,
       0,    73,    75,    74,   121,     0,     0,    75,    69,    50,
       4,    67,    68,     0,   120,     4,    67,    68,    70,    71,
     246,    69,     0,     0,     0,     0,     0,     0,    73,     0,
      74,    70,    71,   288,    75,     0,     0,     0,     0,     0,
       0,    73,     0,    74,     0,     0,     0,    75,     0,     0,
       0,     0,     0,    69,     0,     0,     0,     0,    69,     0,
       0,     0,     0,    70,    71,     0,     0,     0,    70,    71,
       0,     0,     0,    73,     0,    74,     0,     0,    73,    75,
      74,     0,     1,     0,    75,     3,     4,     0,     0,     5,
       6,     7,     0,     0,     8,     9,     0,    10,    11,    12,
       0,    13,    14,     0,    15,     0,    16,     0,    17,    18,
      19,    20,    21,     0,     0,     0,     0,     0,     0,    22,
      62,    23,     0,     0,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,     0,   151,    63,
       1,    24,     0,     3,     4,     0,     0,     5,     6,     7,
       0,   162,     8,     9,     0,    10,    11,    12,  -162,    13,
      14,     0,    15,     0,    16,     0,    17,    18,    19,    20,
      21,     0,     0,     0,     0,     0,     0,    22,     0,    23,
       0,     0,     0,     0,     0,     0,     1,     0,     0,     3,
       4,     0,     0,     5,     6,     7,     0,     0,     8,     9,
       0,    10,    11,    12,     0,    13,    14,     0,    15,    24,
      16,   269,    17,    18,    19,    20,    21,   270,     0,     0,
       0,     0,     0,    22,     0,    23,     0,     0,     0,     0,
       1,     0,     0,     3,     4,     0,     0,     5,     6,     7,
       0,     0,     8,     9,     0,    10,    11,    12,     0,    13,
      14,   282,    15,     0,    16,    24,    17,    18,    19,    20,
      21,   283,     0,     0,     0,     0,     0,    22,     0,    23,
       0,     0,     0,     0,     1,     0,     0,     3,     4,     0,
       0,     5,     6,     7,     0,     0,     8,     9,     0,    10,
      11,    12,     0,    13,    14,     0,    15,   285,    16,    24,
      17,    18,    19,    20,    21,   286,     0,     0,     0,     0,
       0,    22,     1,    23,     2,     3,     4,     0,     0,     5,
       6,     7,     0,     0,     8,     9,     0,    10,    11,    12,
       0,    13,    14,     0,    15,     0,    16,     0,    17,    18,
      19,    20,    21,    24,     0,     0,     0,     0,     0,    22,
       1,    23,     0,     3,     4,     0,     0,     5,     6,     7,
       0,     0,     8,     9,   155,    10,    11,    12,     0,    13,
      14,     0,    15,     0,    16,     0,    17,    18,    19,    20,
      21,    24,     0,     0,     0,     0,     0,    22,     0,    23,
       0,     0,    62,     0,     0,     0,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,    24,
     151,    63,    62,     0,     0,     0,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,     0,
     151,    63,     0,    62,     0,     0,   178,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
       0,   151,    63,     0,    62,     0,     0,   272,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,    62,   151,    63,   257,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,    62,   151,
      63,   292,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,    62,   151,    63,     0,     0,
       0,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,    62,   151,    63,     0,     0,     0,     0,     0,
       0,     0,     0,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,     0,
     151,    63
};

static const yytype_int16 yycheck[] =
{
       0,    91,    36,    65,     1,     1,     1,    43,     1,    39,
       4,     5,     1,    31,     4,    39,     0,    39,     4,    39,
      20,     6,     7,     4,     5,    25,    33,    34,    35,    36,
       6,     7,    39,     4,    41,     4,    12,    67,    68,     4,
      40,    25,    42,     0,    68,     4,    68,    15,    68,    68,
      69,    51,    42,    69,    90,    55,    40,    39,    42,    77,
       4,    68,    69,    41,    69,    57,    58,    41,    25,    69,
      16,    55,    69,    69,    69,    69,    72,    72,    75,    72,
      69,   171,    71,    40,    59,    42,    72,    41,    17,    19,
     152,    91,     4,    73,    73,    19,    73,    69,    98,    72,
       4,     5,     6,     7,    59,    12,    73,    91,     0,     1,
      73,     3,     4,     5,    98,    73,     8,     9,    10,   153,
      59,    13,    14,    25,    16,    17,    18,   191,    20,    21,
      73,    23,   181,    25,    72,    27,    28,    29,    30,    31,
     153,    98,   167,    93,    48,   111,    38,   293,    40,    -1,
     240,   241,    -1,    57,    58,    59,    60,    61,    -1,   165,
      -1,   161,    -1,    -1,    68,   165,    70,    -1,    72,    -1,
      74,   171,    39,    -1,    -1,    -1,    -1,   161,    70,    -1,
      -1,   165,    -1,    -1,    -1,    -1,    -1,   171,    39,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      67,    68,    -1,    -1,   161,    -1,    -1,    -1,   165,    60,
      61,    62,    63,    64,    65,    -1,    67,    68,    33,    34,
      35,    36,    -1,   223,    -1,    -1,    41,   227,    -1,    -1,
     230,    -1,   232,    -1,    -1,    -1,    -1,    -1,    -1,   223,
     240,   241,   242,   227,    -1,    -1,   230,    -1,   232,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   240,   241,   242,    -1,
      -1,    -1,   262,    -1,    -1,    -1,   223,   267,   268,    36,
     227,    38,    -1,   230,    41,   232,   276,    44,   262,    -1,
      -1,    48,    -1,   267,   268,   242,    -1,    -1,     4,     5,
       6,     7,   276,    60,    61,    -1,    -1,    -1,   298,    -1,
      -1,    39,    -1,    70,    71,   262,    73,    74,    75,    -1,
     267,   268,    -1,    -1,   298,    -1,    -1,    -1,    -1,   276,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    67,
      68,    -1,    48,    -1,   101,   102,    -1,    -1,    -1,    -1,
      -1,   298,    58,    59,    -1,    61,    -1,    -1,     4,     5,
       6,     7,    68,    -1,    70,    -1,    -1,    -1,    74,    -1,
      -1,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,    -1,   153,   154,     1,    -1,
     157,    -1,    48,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    58,    59,    60,    61,    -1,    -1,    -1,    -1,
     177,   178,    68,    -1,    70,    -1,    -1,    -1,    74,   186,
      -1,    -1,   189,    -1,   191,    -1,    39,   194,    -1,    -1,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    67,    68,    -1,    -1,   225,    72,
      73,   228,    -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,
       4,     5,    -1,    -1,     8,     9,    10,   244,   245,    13,
      14,   248,    16,    17,    18,    -1,    20,    21,    -1,    23,
     257,    25,    -1,    27,    28,    29,    30,    31,    32,     4,
       5,     6,     7,    -1,    38,   272,    40,   274,    -1,    -1,
      -1,    -1,     4,     5,     6,     7,    -1,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,   292,    -1,   294,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    70,    71,    -1,    -1,
      -1,    -1,    -1,    48,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    57,    58,    59,    60,    48,     4,     5,     6,
       7,    48,    -1,    68,    -1,    70,    58,    59,    60,    74,
      -1,    58,    59,    -1,    -1,    -1,    68,    -1,    70,    -1,
      -1,    68,    74,    70,    71,    -1,    -1,    74,    48,     4,
       5,     6,     7,    -1,     4,     5,     6,     7,    58,    59,
      60,    48,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,
      70,    58,    59,    60,    74,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    -1,    70,    -1,    -1,    -1,    74,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    -1,    -1,    -1,    48,    -1,
      -1,    -1,    -1,    58,    59,    -1,    -1,    -1,    58,    59,
      -1,    -1,    -1,    68,    -1,    70,    -1,    -1,    68,    74,
      70,    -1,     1,    -1,    74,     4,     5,    -1,    -1,     8,
       9,    10,    -1,    -1,    13,    14,    -1,    16,    17,    18,
      -1,    20,    21,    -1,    23,    -1,    25,    -1,    27,    28,
      29,    30,    31,    -1,    -1,    -1,    -1,    -1,    -1,    38,
      39,    40,    -1,    -1,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    67,    68,
       1,    70,    -1,     4,     5,    -1,    -1,     8,     9,    10,
      -1,    12,    13,    14,    -1,    16,    17,    18,    19,    20,
      21,    -1,    23,    -1,    25,    -1,    27,    28,    29,    30,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,    40,
      -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,     4,
       5,    -1,    -1,     8,     9,    10,    -1,    -1,    13,    14,
      -1,    16,    17,    18,    -1,    20,    21,    -1,    23,    70,
      25,    26,    27,    28,    29,    30,    31,    32,    -1,    -1,
      -1,    -1,    -1,    38,    -1,    40,    -1,    -1,    -1,    -1,
       1,    -1,    -1,     4,     5,    -1,    -1,     8,     9,    10,
      -1,    -1,    13,    14,    -1,    16,    17,    18,    -1,    20,
      21,    22,    23,    -1,    25,    70,    27,    28,    29,    30,
      31,    32,    -1,    -1,    -1,    -1,    -1,    38,    -1,    40,
      -1,    -1,    -1,    -1,     1,    -1,    -1,     4,     5,    -1,
      -1,     8,     9,    10,    -1,    -1,    13,    14,    -1,    16,
      17,    18,    -1,    20,    21,    -1,    23,    24,    25,    70,
      27,    28,    29,    30,    31,    32,    -1,    -1,    -1,    -1,
      -1,    38,     1,    40,     3,     4,     5,    -1,    -1,     8,
       9,    10,    -1,    -1,    13,    14,    -1,    16,    17,    18,
      -1,    20,    21,    -1,    23,    -1,    25,    -1,    27,    28,
      29,    30,    31,    70,    -1,    -1,    -1,    -1,    -1,    38,
       1,    40,    -1,     4,     5,    -1,    -1,     8,     9,    10,
      -1,    -1,    13,    14,    11,    16,    17,    18,    -1,    20,
      21,    -1,    23,    -1,    25,    -1,    27,    28,    29,    30,
      31,    70,    -1,    -1,    -1,    -1,    -1,    38,    -1,    40,
      -1,    -1,    39,    -1,    -1,    -1,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    70,
      67,    68,    39,    -1,    -1,    -1,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      67,    68,    -1,    39,    -1,    -1,    73,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    67,    68,    -1,    39,    -1,    -1,    73,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    39,    67,    68,    69,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    39,    67,
      68,    69,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    39,    67,    68,    -1,    -1,
      -1,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    39,    67,    68,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      67,    68
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     3,     4,     5,     8,     9,    10,    13,    14,
      16,    17,    18,    20,    21,    23,    25,    27,    28,    29,
      30,    31,    38,    40,    70,    77,    78,    79,    82,    84,
      87,    88,    99,   105,   120,   131,    69,   100,   121,   132,
     134,   138,   136,   123,   124,   106,   108,   110,    69,    81,
       4,    69,    88,     0,    78,    80,    33,    34,    35,    36,
      41,    98,    39,    68,    85,    86,     4,     6,     7,    48,
      58,    59,    61,    68,    70,    74,    87,    88,    91,    92,
     101,   102,   103,   104,    92,     4,    79,    92,    79,    91,
     130,    92,     4,     4,     4,    92,    88,    79,   118,    92,
      92,    57,    58,    60,    92,    94,    95,    96,    97,   103,
      39,    68,    85,    88,    92,    92,     4,   102,    92,    94,
       4,    71,    89,    90,    92,    92,    93,    68,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    67,    86,    69,    41,    11,   125,    41,    15,   139,
      16,    73,    12,    91,   127,   118,    68,    69,   116,   119,
     119,   111,    32,    71,    79,    83,    92,    59,    73,    73,
       1,    69,    72,    97,     1,    72,    73,     1,    72,    73,
       1,    69,    71,     1,    69,    75,    72,    92,    92,    92,
      92,    92,    92,    92,    92,    92,    92,    92,    92,    92,
      92,    92,    92,    92,    92,    92,    92,    92,    92,    92,
      92,   104,    92,   122,    92,   135,    17,   141,   137,    79,
     128,   129,    73,    19,   127,     4,    72,   115,   117,   115,
     107,   109,   118,    92,    57,    58,    60,    92,    57,    58,
      96,     1,    72,    92,    90,    92,    79,    69,    92,    79,
      92,    79,    73,    79,    19,    72,    69,   118,   118,    26,
      32,   114,    73,    92,    59,    92,    12,   126,    92,    79,
       4,    42,    22,    32,   112,    24,    32,   113,    60,    92,
      92,    79,    69,   140,    59,    92,   141,    92,   133,    79
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    76,    77,    77,    78,    78,    79,    79,    79,    79,
      79,    79,    80,    79,    79,    79,    79,    79,    79,    79,
      79,    79,    79,    81,    81,    82,    82,    83,    83,    84,
      84,    85,    85,    86,    86,    86,    86,    86,    86,    87,
      87,    88,    88,    89,    89,    90,    90,    91,    91,    92,
      92,    92,    92,    92,    92,    92,    92,    92,    92,    92,
      92,    92,    92,    92,    92,    92,    92,    92,    92,    92,
      92,    92,    92,    92,    92,    92,    92,    92,    92,    92,
      92,    92,    92,    92,    92,    92,    92,    92,    92,    93,
      93,    94,    94,    94,    94,    94,    94,    94,    94,    95,
      95,    95,    95,    95,    95,    95,    96,    96,    96,    97,
      97,    98,    98,    98,    98,    99,   100,   100,   101,   101,
     102,   103,   103,   104,   104,   106,   107,   105,   108,   109,
     105,   110,   111,   105,   112,   112,   113,   113,   114,   114,
     115,   115,   116,   116,   117,   117,   118,   118,   119,   119,
     119,   121,   122,   120,   123,   120,   124,   120,   125,   125,
     126,   126,   127,   128,   127,   129,   127,   130,   130,   132,
     133,   131,   134,   135,   131,   136,   137,   131,   138,   139,
     131,   140,   140,   141,   141
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     2,     1,     1,     1,     1,     1,     1,
       1,     2,     0,     4,     1,     1,     1,     1,     1,     3,
       2,     1,     1,     0,     2,     1,     1,     1,     1,     3,
       3,     1,     2,     3,     3,     1,     4,     4,     2,     2,
       1,     1,     1,     1,     3,     3,     1,     1,     1,     1,
       1,     3,     2,     2,     3,     3,     3,     3,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     2,     1,
       3,     3,     5,     3,     5,     7,     5,     3,     1,     1,
       3,     4,     1,     2,     4,     3,     1,     1,     1,     1,
       3,     1,     1,     1,     1,     2,     2,     0,     1,     3,
       1,     3,     2,     1,     1,     0,     0,     7,     0,     0,
       7,     0,     0,     6,     1,     1,     1,     1,     1,     1,
       1,     3,     0,     2,     1,     3,     1,     2,     2,     3,
       1,     0,     0,     7,     0,     5,     0,     6,     0,     1,
       0,     2,     0,     0,     3,     0,     4,     3,     4,     0,
       0,    11,     0,     0,     6,     0,     0,     6,     0,     0,
       6,     0,     2,     0,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval);
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* lines: next_line  */
#line 129 "luxparser.yy"
          {                     /* a statement or a newline */
  if (compileOnly && yyvsp[0])        /* if we're just compiling (and not
                                   immediately executing, and the statement
                                   is not a newline or an error, then store
                                   it at the start of a new list */
    startList(yyvsp[0]);
}
#line 1911 "luxparser.cc"
    break;

  case 3: /* lines: lines next_line  */
#line 137 "luxparser.yy"
                  {             /* next member in a set of statements and
                                   newlines */
  if (compileOnly && yyvsp[0])        /* if we're just compiling and the statement
                                   is not a newline or error, then add to
                                   the list */
    pushList(yyvsp[0]);
}
#line 1923 "luxparser.cc"
    break;

  case 4: /* next_line: statement  */
#line 148 "luxparser.yy"
            {                   /* $1 > 0 indicates succesful execution.
                                   $1 < 0 indicates an error or a premature
                                   end to a loop structure (CONTINUE,
                                   BREAK, RETURN) */
  if (!compileOnly) {           /* not just compiling */
    if (yyvsp[0] > 0) {               /* statement OK */
      yyvsp[0] = execute(yyvsp[0]);         /* execute it */
    }
    cleanUp(-compileLevel, 0);  /* clean up temp storage and such */
    if (yyvsp[0] == LOOP_RETALL       /* RETALL statement */
        && compileLevel) {      /* not at the main execution level */
      puts("RETALL - return control to user");
      away();                   /* clean up aborted execution */
      YYABORT;
    }
  }
  if (yyvsp[0] < 0 && compileLevel) { /* generic break or error condition */
    puts("Aborting");
    away();
    YYABORT;
  }
}
#line 1950 "luxparser.cc"
    break;

  case 5: /* next_line: TOK_NL  */
#line 170 "luxparser.yy"
         {  /* a newline; newlines are only passed to the parser by
                   yylex() if disableNewline is equal to 0 */
    if (debugLine)              /* if this is a dbg> line then we quit after
                                   the first line */
      YYACCEPT;
    yyval = 0;                     /* else we ignore it */
}
#line 1962 "luxparser.cc"
    break;

  case 11: /* statement: TOK_RETURN opt_arg  */
#line 185 "luxparser.yy"
                     {          /* a RETURN statement */
  yyval = newSymbol(LUX_EVB, EVB_RETURN, yyvsp[0]);
}
#line 1970 "luxparser.cc"
    break;

  case 12: /* $@1: %empty  */
#line 188 "luxparser.yy"
             { disableNewline++; }
#line 1976 "luxparser.cc"
    break;

  case 13: /* statement: begingroup $@1 statement_list endgroup  */
#line 188 "luxparser.yy"
                                                           {
  /* a statement block */
  /* after the initial {, more input is needed to complete the statement.
     the disableNewline++ statement ensures that no newlines are recognized
     while the statement block is assembled. */
  if (yyvsp[-1] >= 0)                  /* statement list is OK */
    yyval = newSymbol(LUX_EVB, EVB_BLOCK);
  else                          /* statement list had some error */
    yyval = LUX_ERROR;
  statementDepth--;             /* was incremented by statement_list */
  disableNewline--;             /* back to initial */
}
#line 1993 "luxparser.cc"
    break;

  case 14: /* statement: TOK_BREAK  */
#line 200 "luxparser.yy"
                        { yyval = LOOP_BREAK; }
#line 1999 "luxparser.cc"
    break;

  case 15: /* statement: TOK_CONTINUE  */
#line 201 "luxparser.yy"
                        { yyval = LOOP_CONTINUE; }
#line 2005 "luxparser.cc"
    break;

  case 16: /* statement: TOK_INCLUDE  */
#line 202 "luxparser.yy"
              {                     /* a @file statement */
  yyval = newSymbol(LUX_EVB, EVB_FILE, yyvsp[0], FILE_INCLUDE);
}
#line 2013 "luxparser.cc"
    break;

  case 17: /* statement: TOK_REPORT  */
#line 205 "luxparser.yy"
             {                      /* a @@file statement */
  yyval = newSymbol(LUX_EVB, EVB_FILE, yyvsp[0], FILE_REPORT);
}
#line 2021 "luxparser.cc"
    break;

  case 18: /* statement: TOK_RETALL  */
#line 208 "luxparser.yy"
                        { yyval = LOOP_RETALL; }
#line 2027 "luxparser.cc"
    break;

  case 19: /* statement: TOK_RUN ',' var  */
#line 209 "luxparser.yy"
                        { yyval = newBlockSymbol(yyvsp[0]); }
#line 2033 "luxparser.cc"
    break;

  case 20: /* statement: TOK_RUN var  */
#line 210 "luxparser.yy"
                        { yyval = newBlockSymbol(yyvsp[0]); }
#line 2039 "luxparser.cc"
    break;

  case 21: /* statement: error  */
#line 211 "luxparser.yy"
        {                       /* some error */
  puts("Illegal statement");    /* generate message */
  errorState = 1;               /* signal the error */
  YYABORT;                      /* quite this parse */
}
#line 2049 "luxparser.cc"
    break;

  case 22: /* statement: TOK_ERRORSTATE  */
#line 216 "luxparser.yy"
                        { YYABORT; }
#line 2055 "luxparser.cc"
    break;

  case 23: /* opt_arg: %empty  */
#line 220 "luxparser.yy"
            { yyval = 0; }
#line 2061 "luxparser.cc"
    break;

  case 24: /* opt_arg: ',' expr  */
#line 221 "luxparser.yy"
           { yyval = yyvsp[0]; }
#line 2067 "luxparser.cc"
    break;

  case 29: /* assignment: lhs '=' expr  */
#line 233 "luxparser.yy"
               {                /* simple assignment */
  yyval = newSymbol(LUX_EVB, EVB_REPLACE, yyvsp[-2], yyvsp[0]);
}
#line 2075 "luxparser.cc"
    break;

  case 30: /* assignment: lhs op_assign expr  */
#line 236 "luxparser.yy"
                     {          /* an operator-assignment (e.g. X += 2) */
  if (symbol_class(yyvsp[-2]) == LUX_EXTRACT)
    yyval = newSymbol(LUX_EVB, EVB_REPLACE, copySym(yyvsp[-2]),
                   newSymbol(LUX_BIN_OP, yyvsp[-1], yyvsp[-2], yyvsp[0]));
  else
    yyval = newSymbol(LUX_EVB, EVB_REPLACE, yyvsp[-2],
                   newSymbol(LUX_BIN_OP, yyvsp[-1], yyvsp[-2], yyvsp[0]));
}
#line 2088 "luxparser.cc"
    break;

  case 31: /* tag_list: TOK_STRUCTTAG  */
#line 247 "luxparser.yy"
                            { startList(yyvsp[0]); }
#line 2094 "luxparser.cc"
    break;

  case 32: /* tag_list: tag_list TOK_STRUCTTAG  */
#line 248 "luxparser.yy"
                            { pushList(yyvsp[0]); }
#line 2100 "luxparser.cc"
    break;

  case 33: /* member_spec: '(' subsc_list ')'  */
#line 253 "luxparser.yy"
                        { pushList(LUX_RANGE);  yyval = 1; }
#line 2106 "luxparser.cc"
    break;

  case 34: /* member_spec: '(' subsc_list error  */
#line 254 "luxparser.yy"
                       {
  if ((setup & 1024) == 0)
    puts("Unbalanced ()");
  yyerrok;
  pushList(LUX_RANGE);
  yyval = 1;
}
#line 2118 "luxparser.cc"
    break;

  case 35: /* member_spec: tag_list  */
#line 261 "luxparser.yy"
                        { pushList(LUX_LIST);  yyval = 1; }
#line 2124 "luxparser.cc"
    break;

  case 36: /* member_spec: member_spec '(' subsc_list ')'  */
#line 262 "luxparser.yy"
                                 { pushList(LUX_RANGE);  yyval = yyvsp[-3] + 1; }
#line 2130 "luxparser.cc"
    break;

  case 37: /* member_spec: member_spec '(' subsc_list error  */
#line 263 "luxparser.yy"
                                   {
  if ((setup & 1024) == 0)
    puts("Unbalanced ()");
  yyerrok;
  pushList(LUX_RANGE);
  yyval = yyvsp[-3] + 1;
}
#line 2142 "luxparser.cc"
    break;

  case 38: /* member_spec: member_spec tag_list  */
#line 270 "luxparser.yy"
                        { pushList(LUX_LIST);  yyval = yyvsp[-1] + 1; }
#line 2148 "luxparser.cc"
    break;

  case 39: /* lhs: var member_spec  */
#line 275 "luxparser.yy"
                {
  pushList(yyvsp[0]);                 /* the number of element extraction lists */
  pushList(-yyvsp[-1]);                /* minus indicates "var" */
  yyval = newSymbol(LUX_EXTRACT);
}
#line 2158 "luxparser.cc"
    break;

  case 40: /* lhs: var  */
#line 280 "luxparser.yy"
                        { yyval = findVar(yyvsp[0], curContext); }
#line 2164 "luxparser.cc"
    break;

  case 43: /* struct_list: struct_element  */
#line 291 "luxparser.yy"
                 {
    pushList(LUX_NEW_LIST);     /* the stack contents is now:
                                   key expr LUX_NEW_LIST */
    swapList(1, 2);             /* reverse stack contents to: */
    swapList(2, 3);             /* LUX_NEW_LIST key expr */
}
#line 2175 "luxparser.cc"
    break;

  case 45: /* struct_element: TOK_C_ID ':' expr  */
#line 302 "luxparser.yy"
                    {
    pushList(yyvsp[-2]);
    pushList(yyvsp[0]);
}
#line 2184 "luxparser.cc"
    break;

  case 46: /* struct_element: expr  */
#line 306 "luxparser.yy"
       {
    pushList(-1);
    pushList(yyvsp[0]);
}
#line 2193 "luxparser.cc"
    break;

  case 48: /* value: TOK_STR  */
#line 314 "luxparser.yy"
            {                   // a string
    yyval = newSymbol(LUX_FIXED_STRING, yyvsp[0]);
  }
#line 2201 "luxparser.cc"
    break;

  case 51: /* expr: var '(' ')'  */
#line 322 "luxparser.yy"
              {                 /* a function call without any arguments */
  startList(0);                 /* no arguments */
  pushList(-yyvsp[-2]);
  yyval = newSymbol(LUX_EXTRACT);
}
#line 2211 "luxparser.cc"
    break;

  case 52: /* expr: expr member_spec  */
#line 327 "luxparser.yy"
                   {    /* expressions may be subscripted */
  pushList(yyvsp[0]);                 /* the number of element extraction lists */
  pushList(yyvsp[-1]);                 /* the expression */
  yyval = newSymbol(LUX_EXTRACT);
}
#line 2221 "luxparser.cc"
    break;

  case 53: /* expr: '&' var  */
#line 332 "luxparser.yy"
          {             /* a variable or function/routine pointer */
  yyval = newSymbol(LUX_POINTER, yyvsp[0]);
}
#line 2229 "luxparser.cc"
    break;

  case 54: /* expr: '(' range ')'  */
#line 335 "luxparser.yy"
                {               /* a range expression */
  yyval = yyvsp[-1];
}
#line 2237 "luxparser.cc"
    break;

  case 55: /* expr: '(' range error  */
#line 338 "luxparser.yy"
                  {
  if ((setup & 1024) == 0)
    puts("Unbalanced ()");
  yyerrok;
  yyval = yyvsp[-1];
}
#line 2248 "luxparser.cc"
    break;

  case 56: /* expr: '{' struct_list '}'  */
#line 344 "luxparser.yy"
                      {         /* a structure */
  yyval = newSymbol(LUX_PRE_LIST);
}
#line 2256 "luxparser.cc"
    break;

  case 57: /* expr: '{' struct_list error  */
#line 347 "luxparser.yy"
                        {
  if ((setup & 1024) == 0)
    puts("Unbalanced {}");
  yyerrok;
  yyval = newSymbol(LUX_PRE_LIST);
}
#line 2267 "luxparser.cc"
    break;

  case 58: /* expr: '{' '}'  */
#line 353 "luxparser.yy"
          {                     /* an empty list */
  pushList(LUX_NEW_LIST);
  yyval = newSymbol(LUX_PRE_LIST);
}
#line 2276 "luxparser.cc"
    break;

  case 59: /* expr: '(' expr ')'  */
#line 357 "luxparser.yy"
               {
  yyval = yyvsp[-1];
}
#line 2284 "luxparser.cc"
    break;

  case 60: /* expr: '(' expr error  */
#line 360 "luxparser.yy"
                 {
  if ((setup & 1024) == 0)
    puts("Unbalanced ()");
  yyerrok;
  yyval = yyvsp[-1];
}
#line 2295 "luxparser.cc"
    break;

  case 61: /* expr: '[' expr_list ']'  */
#line 366 "luxparser.yy"
                    {           /* concatenation */
  yyval = newSymbol(LUX_INT_FUNC, LUX_CONCAT_FUN);
}
#line 2303 "luxparser.cc"
    break;

  case 62: /* expr: '[' expr_list error  */
#line 369 "luxparser.yy"
                      {
  if ((setup & 1024) == 0)
    puts("Unbalanced []");
  yyval = newSymbol(LUX_INT_FUNC, LUX_CONCAT_FUN);
  yyerrok;
}
#line 2314 "luxparser.cc"
    break;

  case 63: /* expr: expr '+' expr  */
#line 378 "luxparser.yy"
                {
  yyval = newSymbol(LUX_BIN_OP, LUX_ADD, yyvsp[-2], yyvsp[0]);
}
#line 2322 "luxparser.cc"
    break;

  case 64: /* expr: expr '-' expr  */
#line 381 "luxparser.yy"
                {
  yyval = newSymbol(LUX_BIN_OP, LUX_SUB, yyvsp[-2], yyvsp[0]);
}
#line 2330 "luxparser.cc"
    break;

  case 65: /* expr: expr '*' expr  */
#line 384 "luxparser.yy"
                {
  yyval = newSymbol(LUX_BIN_OP, LUX_MUL, yyvsp[-2], yyvsp[0]);
}
#line 2338 "luxparser.cc"
    break;

  case 66: /* expr: expr '/' expr  */
#line 387 "luxparser.yy"
                {
  yyval = newSymbol(LUX_BIN_OP, LUX_DIV, yyvsp[-2], yyvsp[0]);
}
#line 2346 "luxparser.cc"
    break;

  case 67: /* expr: expr '\\' expr  */
#line 390 "luxparser.yy"
                 {
  yyval = newSymbol(LUX_BIN_OP, LUX_IDIV, yyvsp[-2], yyvsp[0]);
}
#line 2354 "luxparser.cc"
    break;

  case 68: /* expr: expr '%' expr  */
#line 393 "luxparser.yy"
                {
  yyval = newSymbol(LUX_BIN_OP, LUX_MOD, yyvsp[-2], yyvsp[0]);
}
#line 2362 "luxparser.cc"
    break;

  case 69: /* expr: expr TOK_SMOD expr  */
#line 396 "luxparser.yy"
                     {
  yyval = newSymbol(LUX_BIN_OP, LUX_SMOD, yyvsp[-2], yyvsp[0]);
}
#line 2370 "luxparser.cc"
    break;

  case 70: /* expr: expr '^' expr  */
#line 399 "luxparser.yy"
                {
  yyval = newSymbol(LUX_BIN_OP, LUX_POW, yyvsp[-2], yyvsp[0]);
}
#line 2378 "luxparser.cc"
    break;

  case 71: /* expr: expr '>' expr  */
#line 402 "luxparser.yy"
                {
  yyval = newSymbol(LUX_BIN_OP, LUX_MAX, yyvsp[-2], yyvsp[0]);
}
#line 2386 "luxparser.cc"
    break;

  case 72: /* expr: expr '<' expr  */
#line 405 "luxparser.yy"
                {
  yyval = newSymbol(LUX_BIN_OP, LUX_MIN, yyvsp[-2], yyvsp[0]);
}
#line 2394 "luxparser.cc"
    break;

  case 73: /* expr: expr TOK_EQ expr  */
#line 408 "luxparser.yy"
                   {
  yyval = newSymbol(LUX_BIN_OP, LUX_EQ, yyvsp[-2], yyvsp[0]);
}
#line 2402 "luxparser.cc"
    break;

  case 74: /* expr: expr TOK_NE expr  */
#line 411 "luxparser.yy"
                   {
  yyval = newSymbol(LUX_BIN_OP, LUX_NE, yyvsp[-2], yyvsp[0]);
}
#line 2410 "luxparser.cc"
    break;

  case 75: /* expr: expr TOK_GE expr  */
#line 414 "luxparser.yy"
                   {
  yyval = newSymbol(LUX_BIN_OP, LUX_GE, yyvsp[-2], yyvsp[0]);
}
#line 2418 "luxparser.cc"
    break;

  case 76: /* expr: expr TOK_GT expr  */
#line 417 "luxparser.yy"
                   {
  yyval = newSymbol(LUX_BIN_OP, LUX_GT, yyvsp[-2], yyvsp[0]);
}
#line 2426 "luxparser.cc"
    break;

  case 77: /* expr: expr TOK_LT expr  */
#line 420 "luxparser.yy"
                   {
  yyval = newSymbol(LUX_BIN_OP, LUX_LT, yyvsp[-2], yyvsp[0]);
}
#line 2434 "luxparser.cc"
    break;

  case 78: /* expr: expr TOK_LE expr  */
#line 423 "luxparser.yy"
                   {
  yyval = newSymbol(LUX_BIN_OP, LUX_LE, yyvsp[-2], yyvsp[0]);
}
#line 2442 "luxparser.cc"
    break;

  case 79: /* expr: expr TOK_AND expr  */
#line 426 "luxparser.yy"
                    {
  yyval = newSymbol(LUX_BIN_OP, LUX_AND, yyvsp[-2], yyvsp[0]);
}
#line 2450 "luxparser.cc"
    break;

  case 80: /* expr: expr TOK_OR expr  */
#line 429 "luxparser.yy"
                   {
  yyval = newSymbol(LUX_BIN_OP, LUX_OR, yyvsp[-2], yyvsp[0]);
}
#line 2458 "luxparser.cc"
    break;

  case 81: /* expr: expr TOK_XOR expr  */
#line 432 "luxparser.yy"
                    {
  yyval = newSymbol(LUX_BIN_OP, LUX_XOR, yyvsp[-2], yyvsp[0]);
}
#line 2466 "luxparser.cc"
    break;

  case 82: /* expr: expr '&' expr  */
#line 435 "luxparser.yy"
                {               /* testing & for AND */
  yyval = newSymbol(LUX_BIN_OP, LUX_AND, yyvsp[-2], yyvsp[0]);
}
#line 2474 "luxparser.cc"
    break;

  case 83: /* expr: expr '|' expr  */
#line 438 "luxparser.yy"
                {               /* testing | for OR */
  yyval = newSymbol(LUX_BIN_OP, LUX_OR, yyvsp[-2], yyvsp[0]);
}
#line 2482 "luxparser.cc"
    break;

  case 84: /* expr: expr TOK_ANDIF expr  */
#line 441 "luxparser.yy"
                      {             /* conditional and */
  yyval = newSymbol(LUX_IF_OP, LUX_ANDIF, yyvsp[-2], yyvsp[0]);
}
#line 2490 "luxparser.cc"
    break;

  case 85: /* expr: expr TOK_ORIF expr  */
#line 444 "luxparser.yy"
                     {              /* conditional or */
  yyval = newSymbol(LUX_IF_OP, LUX_ORIF, yyvsp[-2], yyvsp[0]);
}
#line 2498 "luxparser.cc"
    break;

  case 86: /* expr: expr '#' expr  */
#line 447 "luxparser.yy"
                {               /* matrix multiplication */
  startList(yyvsp[-2]);
  pushList(yyvsp[0]);
  yyval = newSymbol(LUX_INT_FUNC, LUX_MATMUL_FUN);
}
#line 2508 "luxparser.cc"
    break;

  case 87: /* expr: '-' expr  */
#line 452 "luxparser.yy"
                            {
  startList(yyvsp[0]);
  yyval = newSymbol(LUX_INT_FUNC, LUX_NEG_FUN);
}
#line 2517 "luxparser.cc"
    break;

  case 88: /* expr: '+' expr  */
#line 456 "luxparser.yy"
                            {
  yyval = yyvsp[0];
}
#line 2525 "luxparser.cc"
    break;

  case 89: /* expr_list: expr  */
#line 462 "luxparser.yy"
                        { startList(yyvsp[0]); }
#line 2531 "luxparser.cc"
    break;

  case 90: /* expr_list: expr_list ',' expr  */
#line 463 "luxparser.yy"
                        { pushList(yyvsp[0]); }
#line 2537 "luxparser.cc"
    break;

  case 91: /* range: expr ':' expr  */
#line 473 "luxparser.yy"
              {
  yyval = newSymbol(LUX_PRE_RANGE, yyvsp[-2], yyvsp[0]);
}
#line 2545 "luxparser.cc"
    break;

  case 92: /* range: expr ':' '*' '-' expr  */
#line 476 "luxparser.yy"
                        {
  yyval = newSymbol(LUX_PRE_RANGE, yyvsp[-4], -yyvsp[0]);
}
#line 2553 "luxparser.cc"
    break;

  case 93: /* range: expr ':' '*'  */
#line 479 "luxparser.yy"
               {
  yyval = newSymbol(LUX_PRE_RANGE, yyvsp[-2], -LUX_ONE);
}
#line 2561 "luxparser.cc"
    break;

  case 94: /* range: '*' '-' expr ':' expr  */
#line 482 "luxparser.yy"
                        {
  yyval = newSymbol(LUX_PRE_RANGE, -yyvsp[-2], yyvsp[0]);
}
#line 2569 "luxparser.cc"
    break;

  case 95: /* range: '*' '-' expr ':' '*' '-' expr  */
#line 485 "luxparser.yy"
                                {
  yyval = newSymbol(LUX_PRE_RANGE, -yyvsp[-4], -yyvsp[0]);
}
#line 2577 "luxparser.cc"
    break;

  case 96: /* range: '*' '-' expr ':' '*'  */
#line 488 "luxparser.yy"
                       {
  yyval = newSymbol(LUX_PRE_RANGE, -yyvsp[-2], -LUX_ONE);
}
#line 2585 "luxparser.cc"
    break;

  case 97: /* range: '*' '-' expr  */
#line 491 "luxparser.yy"
               {
  yyval = newSymbol(LUX_PRE_RANGE, -yyvsp[0], LUX_ZERO);
}
#line 2593 "luxparser.cc"
    break;

  case 98: /* range: '*'  */
#line 494 "luxparser.yy"
      {
  yyval = newSymbol(LUX_PRE_RANGE, -LUX_ONE, LUX_ZERO);
}
#line 2601 "luxparser.cc"
    break;

  case 100: /* subsc: range ':' '+'  */
#line 507 "luxparser.yy"
                {
  pre_range_sum(yyvsp[-2]) = 1;
  yyval = yyvsp[-2];
}
#line 2610 "luxparser.cc"
    break;

  case 101: /* subsc: range ':' '>' expr  */
#line 512 "luxparser.yy"
                     {
  pre_range_redirect(yyvsp[-3]) = yyvsp[0];
  yyval = yyvsp[-3];
}
#line 2619 "luxparser.cc"
    break;

  case 102: /* subsc: '+'  */
#line 517 "luxparser.yy"
      {
  yyval = newSymbol(LUX_PRE_RANGE, LUX_ZERO, -LUX_ONE);
  pre_range_sum(yyval) = 1;
}
#line 2628 "luxparser.cc"
    break;

  case 103: /* subsc: '>' expr  */
#line 522 "luxparser.yy"
           {
  yyval = newSymbol(LUX_PRE_RANGE, LUX_ZERO, -LUX_ONE);
  pre_range_redirect(yyval) = yyvsp[0];
}
#line 2637 "luxparser.cc"
    break;

  case 104: /* subsc: expr ':' '>' expr  */
#line 527 "luxparser.yy"
                    {
  yyval = newSymbol(LUX_PRE_RANGE, yyvsp[-3], LUX_ZERO);
  pre_range_redirect(yyval) = yyvsp[0];
}
#line 2646 "luxparser.cc"
    break;

  case 105: /* subsc: expr ':' '+'  */
#line 532 "luxparser.yy"
               {
  yyval = newSymbol(LUX_PRE_RANGE, yyvsp[-2], LUX_ZERO);
  pre_range_sum(yyval) = 1;
}
#line 2655 "luxparser.cc"
    break;

  case 109: /* subsc_list: subsc_or_key  */
#line 549 "luxparser.yy"
             {
  startList(yyvsp[0]);
}
#line 2663 "luxparser.cc"
    break;

  case 110: /* subsc_list: subsc_list ',' subsc_or_key  */
#line 553 "luxparser.yy"
                              {
  pushList(yyvsp[0]);
}
#line 2671 "luxparser.cc"
    break;

  case 111: /* op_assign: TOK_PLUSIS  */
#line 560 "luxparser.yy"
           {
  yyval = LUX_ADD;
}
#line 2679 "luxparser.cc"
    break;

  case 112: /* op_assign: TOK_MINUSIS  */
#line 564 "luxparser.yy"
              {
  yyval = LUX_SUB;
}
#line 2687 "luxparser.cc"
    break;

  case 113: /* op_assign: TOK_TIMESIS  */
#line 568 "luxparser.yy"
              {
  yyval = LUX_MUL;
}
#line 2695 "luxparser.cc"
    break;

  case 114: /* op_assign: TOK_DIVIDEIS  */
#line 572 "luxparser.yy"
               {
  yyval = LUX_DIV;
}
#line 2703 "luxparser.cc"
    break;

  case 115: /* routine_execution: TOK_C_ID s_arglist  */
#line 579 "luxparser.yy"
                   {
  yyval = newSubrSymbol(yyvsp[-1]);
}
#line 2711 "luxparser.cc"
    break;

  case 117: /* s_arglist: %empty  */
#line 587 "luxparser.yy"
              {
  pushList(LUX_NEW_LIST);
}
#line 2719 "luxparser.cc"
    break;

  case 118: /* arglist: arg  */
#line 594 "luxparser.yy"
    {
  startList(yyvsp[0]);
}
#line 2727 "luxparser.cc"
    break;

  case 119: /* arglist: arglist ',' arg  */
#line 598 "luxparser.yy"
                  {
  pushList(yyvsp[0]);
}
#line 2735 "luxparser.cc"
    break;

  case 121: /* key: key_param '=' expr  */
#line 611 "luxparser.yy"
                   {
  yyval = newSymbol(LUX_KEYWORD, yyvsp[-2], yyvsp[0]);
}
#line 2743 "luxparser.cc"
    break;

  case 122: /* key: '/' key_param  */
#line 615 "luxparser.yy"
                {
  yyval = newSymbol(LUX_KEYWORD, yyvsp[0], LUX_ONE);
}
#line 2751 "luxparser.cc"
    break;

  case 125: /* $@2: %empty  */
#line 629 "luxparser.yy"
         {
  disableNewline++;
}
#line 2759 "luxparser.cc"
    break;

  case 126: /* @3: %empty  */
#line 631 "luxparser.yy"
                       {
  yyval = newSymbol(LUX_SUBROUTINE, yyvsp[-1]);
}
#line 2767 "luxparser.cc"
    break;

  case 127: /* routine_definition: TOK_SUBR $@2 TOK_C_ID f_paramlist @3 statement_list endsubr  */
#line 633 "luxparser.yy"
                         {
  yyval = newSymbol(LUX_SUBROUTINE, -yyvsp[-2] - 1);
  statementDepth--;
  disableNewline--;
}
#line 2777 "luxparser.cc"
    break;

  case 128: /* $@4: %empty  */
#line 639 "luxparser.yy"
           {
  disableNewline++;
}
#line 2785 "luxparser.cc"
    break;

  case 129: /* @5: %empty  */
#line 641 "luxparser.yy"
                       {
  yyval = newSymbol(LUX_FUNCTION, yyvsp[-1]);
}
#line 2793 "luxparser.cc"
    break;

  case 130: /* routine_definition: TOK_FUNC $@4 TOK_C_ID f_paramlist @5 statement_list endfunc  */
#line 643 "luxparser.yy"
                         {
  yyval = newSymbol(LUX_FUNCTION, -yyvsp[-2] - 1);
  statementDepth--;
  disableNewline--;
}
#line 2803 "luxparser.cc"
    break;

  case 131: /* $@6: %empty  */
#line 649 "luxparser.yy"
            {
  disableNewline++;
}
#line 2811 "luxparser.cc"
    break;

  case 132: /* @7: %empty  */
#line 651 "luxparser.yy"
           {
  yyval = newSymbol(LUX_BLOCKROUTINE, yyvsp[0]);
}
#line 2819 "luxparser.cc"
    break;

  case 133: /* routine_definition: TOK_BLOCK $@6 TOK_C_ID @7 statement_list endblock  */
#line 653 "luxparser.yy"
                          {
  yyval = newSymbol(LUX_BLOCKROUTINE, -yyvsp[-2] - 1);
  statementDepth--;
  disableNewline--;
}
#line 2829 "luxparser.cc"
    break;

  case 141: /* paramlist2: paramlist ',' TOK_ELLIPSIS  */
#line 676 "luxparser.yy"
                             {
  pushList(LUX_EXTEND);
}
#line 2837 "luxparser.cc"
    break;

  case 142: /* s_paramlist: %empty  */
#line 684 "luxparser.yy"
            {
  pushList(LUX_NEW_LIST);
}
#line 2845 "luxparser.cc"
    break;

  case 144: /* paramlist: TOK_C_ID  */
#line 693 "luxparser.yy"
         {
  startList(yyvsp[0]);
}
#line 2853 "luxparser.cc"
    break;

  case 145: /* paramlist: paramlist ',' TOK_C_ID  */
#line 697 "luxparser.yy"
                         {
  pushList(yyvsp[0]);
}
#line 2861 "luxparser.cc"
    break;

  case 146: /* statement_list: statement  */
#line 704 "luxparser.yy"
          {
  startList(yyvsp[0]);
  statementDepth++;
}
#line 2870 "luxparser.cc"
    break;

  case 147: /* statement_list: statement_list statement  */
#line 709 "luxparser.yy"
                           {
  pushList(yyvsp[0]);
  if (yyvsp[0] == LUX_ERROR)
    yyval = LUX_ERROR;
}
#line 2880 "luxparser.cc"
    break;

  case 148: /* f_paramlist: '(' ')'  */
#line 719 "luxparser.yy"
        {                       /* empty list */
  pushList(LUX_NEW_LIST);
}
#line 2888 "luxparser.cc"
    break;

  case 151: /* $@8: %empty  */
#line 730 "luxparser.yy"
       {
  disableNewline++;
}
#line 2896 "luxparser.cc"
    break;

  case 152: /* $@9: %empty  */
#line 732 "luxparser.yy"
                {
  disableNewline--;
}
#line 2904 "luxparser.cc"
    break;

  case 153: /* selection: TOK_IF $@8 expr opt_then $@9 statement opt_else  */
#line 734 "luxparser.yy"
                     {
  yyval = newSymbol(LUX_EVB, EVB_IF, yyvsp[-4], yyvsp[-1], yyvsp[0]);
}
#line 2912 "luxparser.cc"
    break;

  case 154: /* $@10: %empty  */
#line 738 "luxparser.yy"
           {
  disableNewline++;
}
#line 2920 "luxparser.cc"
    break;

  case 155: /* selection: TOK_CASE $@10 case_list opt_case_else TOK_ENDCASE  */
#line 740 "luxparser.yy"
                                      {
  pushList(yyvsp[-1]);
  yyval = newSymbol(LUX_EVB, EVB_CASE);
  disableNewline--;
}
#line 2930 "luxparser.cc"
    break;

  case 156: /* $@11: %empty  */
#line 746 "luxparser.yy"
            {
  disableNewline++;
}
#line 2938 "luxparser.cc"
    break;

  case 157: /* selection: TOK_NCASE $@11 expr statement_list opt_case_else TOK_ENDCASE  */
#line 748 "luxparser.yy"
                                                {
  pushList(yyvsp[-1]);
  pushList(yyvsp[-3]);
  statementDepth--;
  yyval = newSymbol(LUX_EVB, EVB_NCASE);
  disableNewline--;
}
#line 2950 "luxparser.cc"
    break;

  case 160: /* opt_else: %empty  */
#line 765 "luxparser.yy"
            {
  yyval = 0;
}
#line 2958 "luxparser.cc"
    break;

  case 161: /* opt_else: TOK_ELSE statement  */
#line 769 "luxparser.yy"
                     {
  yyval = yyvsp[0];
}
#line 2966 "luxparser.cc"
    break;

  case 162: /* opt_case_else: %empty  */
#line 776 "luxparser.yy"
            {
  yyval = 0;
}
#line 2974 "luxparser.cc"
    break;

  case 163: /* $@12: %empty  */
#line 780 "luxparser.yy"
           {
  disableNewline++;
}
#line 2982 "luxparser.cc"
    break;

  case 164: /* opt_case_else: TOK_ELSE $@12 statement  */
#line 782 "luxparser.yy"
            {
  yyval = yyvsp[0];
  disableNewline--;
}
#line 2991 "luxparser.cc"
    break;

  case 165: /* $@13: %empty  */
#line 787 "luxparser.yy"
           {
  disableNewline++;
}
#line 2999 "luxparser.cc"
    break;

  case 166: /* opt_case_else: TOK_ELSE $@13 ':' statement  */
#line 789 "luxparser.yy"
                {               /* Dick's case else */
  yyval = yyvsp[-1];
  disableNewline--;
}
#line 3008 "luxparser.cc"
    break;

  case 167: /* case_list: value ':' statement  */
#line 797 "luxparser.yy"
                    {
  startList(yyvsp[-2]);
  pushList(yyvsp[0]);
}
#line 3017 "luxparser.cc"
    break;

  case 168: /* case_list: case_list value ':' statement  */
#line 802 "luxparser.yy"
                                {
  pushList(yyvsp[-2]);
  pushList(yyvsp[0]);
}
#line 3026 "luxparser.cc"
    break;

  case 169: /* $@14: %empty  */
#line 810 "luxparser.yy"
        {
  disableNewline++;
}
#line 3034 "luxparser.cc"
    break;

  case 170: /* $@15: %empty  */
#line 812 "luxparser.yy"
                                             {
  disableNewline--;
}
#line 3042 "luxparser.cc"
    break;

  case 171: /* loop: TOK_FOR $@14 TOK_C_ID '=' expr ',' expr opt_step opt_do $@15 statement  */
#line 814 "luxparser.yy"
            {
  yyval = findVar(yyvsp[-8], curContext);
  yyval = newSymbol(LUX_EVB, EVB_FOR, yyval, yyvsp[-6], yyvsp[-4], yyvsp[-3], yyvsp[0]);
}
#line 3051 "luxparser.cc"
    break;

  case 172: /* $@16: %empty  */
#line 819 "luxparser.yy"
             {
  disableNewline++;
}
#line 3059 "luxparser.cc"
    break;

  case 173: /* $@17: %empty  */
#line 821 "luxparser.yy"
                      {
  disableNewline--;
}
#line 3067 "luxparser.cc"
    break;

  case 174: /* loop: TOK_REPEAT $@16 statement TOK_UNTIL $@17 expr  */
#line 823 "luxparser.yy"
       {
  yyval = newSymbol(LUX_EVB, EVB_REPEAT, yyvsp[-3], yyvsp[0]);
}
#line 3075 "luxparser.cc"
    break;

  case 175: /* $@18: %empty  */
#line 827 "luxparser.yy"
         {
  disableNewline++;
}
#line 3083 "luxparser.cc"
    break;

  case 176: /* $@19: %empty  */
#line 829 "luxparser.yy"
                      {
  disableNewline--;
}
#line 3091 "luxparser.cc"
    break;

  case 177: /* loop: TOK_DO $@18 statement TOK_WHILE $@19 expr  */
#line 831 "luxparser.yy"
       {
  yyval = newSymbol(LUX_EVB, EVB_DO_WHILE, yyvsp[-3], yyvsp[0]);
}
#line 3099 "luxparser.cc"
    break;

  case 178: /* $@20: %empty  */
#line 835 "luxparser.yy"
            {
  disableNewline++;
}
#line 3107 "luxparser.cc"
    break;

  case 179: /* $@21: %empty  */
#line 837 "luxparser.yy"
       {
  disableNewline--;
}
#line 3115 "luxparser.cc"
    break;

  case 180: /* loop: TOK_WHILE $@20 expr $@21 opt_do statement  */
#line 839 "luxparser.yy"
                   {
  yyval = newSymbol(LUX_EVB, EVB_WHILE_DO, yyvsp[-3], yyvsp[0]);
}
#line 3123 "luxparser.cc"
    break;

  case 181: /* opt_step: %empty  */
#line 846 "luxparser.yy"
            {
  yyval = LUX_ONE;
}
#line 3131 "luxparser.cc"
    break;

  case 182: /* opt_step: ',' expr  */
#line 850 "luxparser.yy"
           {
  yyval = yyvsp[0];
}
#line 3139 "luxparser.cc"
    break;


#line 3143 "luxparser.cc"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 862 "luxparser.yy"


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <termio.h>     /* for unbuffered input */
#include <unistd.h>     /* for unbuffered input */
/*#include "editor_once.h"*/
#include <setjmp.h>
#include <ctype.h>
#include "once.hh"
#include "editorcharclass.hh"

jmp_buf jmpenv;                 /* environment storage for long jumps */
char    *currentChar,           /* the character currently being parsed */
        inHistoryBuffer = 1,    /* input is copied into history buffer */
        reportBody = 0;         /* user routines are compiled, */
                                /* not just reported.*/
static char     oldChar = '\0';
extern FILE     *inputStream;           /* source of input, if file */
extern char     *inputString;           /* source of input, if string */
int32_t ignoreInput = 0,        /* nesting level of IGNORE-RESUME pairs */
        findBody = 0,           /* nonzero if a specific user-routine */
                                /* is sought for recompilation */
        calculatorMode = 0;     /* nonzero if in calculator mode
                                 (see calculator.c) */

char const* LUXPrompts[] =      {       /* legal LUX prompts */
  "LUX>", "mor>", "ign>", "dbg>", "clc>"
};
#define N_LUXPROMPTS 5

extern char     line[],         /* raw user input */
        tLine[];                /* translated user input */
void    symbolInitialization(void);

static char     continuation = 0; /* indicates whether the current line */
                                /* ends with a continuation character (-) */
                                /* outside of a comment. */

void away(void)
     /* clean up the symbol stack and reinitialize various counters */
{
  char const**p;
  extern int32_t        keepEVB, (*getChar)(void), nSymbolStack;
  int32_t       getStreamChar(void);

  /* statementDepth = disableNewline = curContext = keepEVB = 0; */
  curContext = keepEVB = 0;
  p = &symbolStack[nSymbolStack - 1];
  while (nSymbolStack > 0) {
    if (*p)
      free((void*) *p);
    *p-- = 0;
    nSymbolStack--;
  }
  if (symbolStack[0]) {
    free((void*) symbolStack[0]);
    symbolStack[0] = 0;
  }
  symbolStackIndex = nSymbolStack = 0;
  getChar = getStreamChar;
}

int32_t yyerror(const char *s)
     /* reports errors occurring during parsing (required by yyparse()) */
{
 extern int32_t curLineNumber;  /* current line number */
 extern CompileInfo     *curCompileInfo;

 if (setup & 1024)      /* no parser warnings */
   return 0;
 if (!*line) {
   *line = oldChar;
   oldChar = '\0';
 } else
   oldChar = 1;
 printf("%s ", s);
 if (curCompileInfo && curCompileInfo->name)
   printf("in file %s\n", curCompileInfo->name);
 printf("in line %d: \"%s\"\n", curLineNumber, line);
 if (!oldChar) {
   luxerror("Error just before end of line", 0);
   *line = '\0';
 } else
   luxerror("Error just before \"%s\"", 0, currentChar); 
 return 0;
}
/*--------------------------------------------------------------*/
void translateLine(void)
/* translates line in line[] and stores result in tLine[]:
   - remove heading whitespace
   - translate control chars to whitespace
   - replace multiple whitespace by single whitespace
   - remove comments
   Moved treatment of literal strings to yylex because sometimes
   they must be ignored (e.g. when parsing nonrelevant code from
   a @@ file)
   string in line[] may be changed */
{
  char  *p, *tp, inString = 0;
  int32_t       nonWhite, i;
  extern char allowPromptInInput; /* allow prompts at start of line */

  p = line;                     /* user input */
  tp = tLine;                   /* translated input */
  if (allowPromptInInput)       /* allow legal LUX prompts at the very
                                   beginning of input lines; i.e. ignore when
                                   encountered */
    for (i = 0; i < N_LUXPROMPTS; i++)
      if (!strncmp(p, LUXPrompts[i], strlen(LUXPrompts[i]))) {
                                /* found an LUX prompt */
        p += strlen(LUXPrompts[i]);/* skip over it */
        break;
      }
  if (!inString)
    while (isspace((uint8_t) *p))
      p++;                      /* skip heading whitespace
                                   outside literal strings */
  nonWhite = 0;                 /* number of consecutive non-whitespaces */
  while (*p) {                  /* not at end of line */
    if (iscntrl((uint8_t) *p))
      *p = ' ';                 /* control characters are transformed into
                                   whitespace */
    switch (*p) {
      case '\'': case '"':      /* fixed literal string */
        if (inString) {         /* we are already inside a literal string */
          if (*p == inString) { /* active string quote */
            if (p[1] == *p) {   /* a quoted apostrophe, i.e. an apostrophe
                                   as part of the literal string */
              *tp++ = *p;       /* a single apostrophe into the translation */
              memcpy(p, p + 1, strlen(p));  /* "skip over" the second one */
            } else {            /* found the end of the literal string */
              *tp++ = TRANS_FIXED_STRING; /* signal end of literal string */
              inString = 0;     /* we're now outside the string */
            }
          } else                /* not an active string quote: just copy */
            *tp++ = *p;
        } else {                /* we're not now in a literal string */
          *tp++ = TRANS_FIXED_STRING; /* signal the string's start */
          inString = *p;        /* we're now inside the string */
        }
        break;
      case '@':
        if (!inString) {        /* we're not inside a literal string, so
                                   it must be a "@filename" */
          *tp++ = *p++;         /* @ goes into the translation */
          while (isFileNameChar((uint8_t) *p))
            *tp++ = *p++;       /* file name goes into the translation */
          p--;
        } else
          *tp++ = *p;           /* else include */
        break;
      case ';':                 /* possibly the start of a comment */
        if (inString) {         /* no, part of a literal string, so leave;
                                   otherwise treat as end of line */
          *tp++ = *p;
          break;
        }
        *tp = '\0';             /* a comment, so regard as an empty line. */
        return;
      case '\n':                /* end of the line */
        *tp = '\0';             /* terminate translated string */
        return;                 /* done. */
      case ' ':  case '\t':     /* whitespace */
        if (!inString) {        /* not inside a literal string */
          if (nonWhite)         /* we did not just copy whitespace */
            *tp++ = *p;         /* this whitespace into the translation */
          nonWhite = -1;        /* will be incremented below to zero
                                   consecutive non-whitespaces */
        } else
          *tp++ = *p;           /* inside a string, so just copy */
        break;
      case '\\':
        /* we must check for \' and \" inside strings because they must */
        /* not be interpreted as a backslash and a string-ending */
        /* character, but rather as introducing a literal character ' or ". */
        if (inString && p[1] == inString) {
          *tp++ = *p++;         /* copy the backslash; the next lines */
                                /* will get the quote character*/
        } else if (p[1] == '\\')
          *tp++ = *p++;         /* we copy both slashes; they'll be dealt */
                                /* with in installString(). */
        *tp++ = *p;             /* copy (also if the backslash does not */
                                /* introduce a literal quote */
        break;
      default:                  /* "ordinary" character */
        *tp++ = *p;           /* in string, so just copy */
        break;
    }
    p++;                        /* next character */
    nonWhite++;                 /* increment consecutive
                                   non-whitespace count */
  }
  *tp++ = '\0';                 /* sure close */
}
/*--------------------------------------------------------------*/
void Quit(int32_t result)
/* Quits the program */
{
  int32_t       saveHistory(void);

  saveHistory();
  printf("\nCPUtime: %g seconds\n", ((float) clock())/CLOCKS_PER_SEC);
  puts("Quitting... Bye!");     /* farewell message */
  exit(result);
}
/*--------------------------------------------------------------*/
/* NUMBERS

   LUX allows specification of numbers of six types (LUX_INT8,
   LUX_INT16, LUX_INT32, LUX_INT64, LUX_FLOAT, LUX_DOUBLE) and three
   bases (8, 10, 16).  Numbers must always start with a decimal digit.
   Fractional (i.e. LUX_FLOAT or LUX_DOUBLE) numbers can only be
   specified in base 10.  In integer numbers, a base specifier (if
   any) precedes a type specifier (if any).  By default, integers are
   in base 10 and of type LUX_INT32.  Octal numbers are followed by an
   O.  Hexadecimal numbers are either preceded by 0X or followed by an
   X.  A final B indicates a LUX_INT8 number, a final W a LUX_INT16, a
   final L a LUX_INT32 number, and a final Q a LUX_INT64 number.  A
   general LUX_FLOAT looks like this: 1.23E+4 Either the decimal point
   or the exponent indicator E must be present.  A plus sign is
   optional in the exponent.  A general LUX_DOUBLE looks like a
   general LUX_FLOAT, except that a D is used instead of an E.  The
   exponent indicator D must be present.

   Some examples:  0X1AB = 1AXB = LUX_INT8 hex 1A = LUX_INT8 26
                   0X1ABL = 1ABXL = LUX_INT32 hex 1AB = LUX_INT32 427
                   2E = 2. = LUX_FLOAT 2
                   2EX = 0X2E = LUX_INT32 hex 2E = LUX_INT32 46
*/
/*--------------------------------------------------------------*/
int32_t readNumber(YYSTYPE *lvalp)
/* reads the number at currentChar, puts it in a new sybol, assigns
   the symbol number to *lvalp, and returns the proper parser code */
{
  Symboltype    type;
  Scalar        v;
  void  read_a_number(char **, Scalar *, Symboltype *);

  read_a_number(&currentChar, &v, &type);
  if (!ignoreInput) {           /* we're not ignoring stuff */
    if ((*lvalp = newSymbol(LUX_FIXED_NUMBER, type)) < 0) /* could not get */
                                /* a new symbol for the number */
      return LUX_ERROR;         /* return error indication */
    if (*lvalp)
      switch(type) {            /* non-zero return value (??) */
                                /* insert value of proper type */
      case LUX_INT8:
        scalar_value(*lvalp).ui8 = (uint8_t) v.i64;
        break;
      case LUX_INT16:
        scalar_value(*lvalp).i16 = (int16_t) v.i64;
        break;
      case LUX_INT32:
        scalar_value(*lvalp).i32 = (int32_t) v.i64;
        break;
      case LUX_INT64:
        scalar_value(*lvalp).i64 = v.i64;
        break;
      case LUX_FLOAT:
        scalar_value(*lvalp).f = (float) v.d;
        break;
      case LUX_DOUBLE:
        scalar_value(*lvalp).d = v.d;
        break;
      case LUX_CFLOAT:
        complex_scalar_data(*lvalp).cf->real = 0.0;
        complex_scalar_data(*lvalp).cf->imaginary = v.d;
        break;
      case LUX_CDOUBLE:
        complex_scalar_data(*lvalp).cd->real = 0.0;
        complex_scalar_data(*lvalp).cd->imaginary = v.d;
        break;
      default:
        return cerror(ILL_TYPE, 0);
      }
    return TOK_NUMBER;
  } else
    return 0;
}
/*--------------------------------------------------------------*/
int32_t strcmp2(const void *key, const void *data)
/* compares key to data */
{
  return strcmp((char *) key, *(char **) data);
}
/*--------------------------------------------------------------*/
int32_t isKeyWord(void)
/* checks if the string starting at currentChar is a reserved (key) Word;
  if so, returns the keyword's code */
{
 static char const* keyWords[] = {
   "and", "andif", "begin", "block", "break", "case", "continue",
   "do", "else", "end", "endblock", "endcase", "endfunc",
   "endsubr", "eq", "for", "func", "function", "ge", "gt", "if", "le", "lt",
   "mod", "ncase", "ne", "or", "orif", "repeat", "retall", "return",
   "run", "smod", "subr", "subroutine", "then", "until", "while", "xor"
 };
 static int32_t keyCodes[] = {
   TOK_AND, TOK_ANDIF, TOK_BEGIN, TOK_BLOCK, TOK_BREAK, TOK_CASE,
   TOK_CONTINUE, TOK_DO, TOK_ELSE, TOK_END, TOK_ENDBLOCK, TOK_ENDCASE,
   TOK_ENDFUNC, TOK_ENDSUBR, TOK_EQ, TOK_FOR, TOK_FUNC, TOK_FUNC,
   TOK_GE, TOK_GT, TOK_IF, TOK_LE, TOK_LT, '%', TOK_NCASE, TOK_NE,
   TOK_OR, TOK_ORIF, TOK_REPEAT, TOK_RETALL, TOK_RETURN, TOK_RUN,
   TOK_SMOD, TOK_SUBR, TOK_SUBR, TOK_THEN, TOK_UNTIL, TOK_WHILE,
   TOK_XOR
 };
 char const** ptr;

 ptr = (char const**) bsearch(currentChar, keyWords, sizeof(keyWords)/sizeof(char *),
   sizeof(char const*), strcmp2);
 return (ptr)? keyCodes[ptr - keyWords]: 0;
}
/*--------------------------------------------------------------*/
int32_t readIdentifier(YYSTYPE *lvalp)
/* identifies keywords at currentChar and returns appropriate lexical number;
  otherwise reads identifier, stores in stack, returns index to
  stack in *lvalp, and returns appropriate lexical number. */
{
 char   *p, c;
 int32_t        n;

 p = currentChar + 1;           /* skip over first character, which */
                                /* is assumed to be OK by this routine */
 while (isNextChar((uint8_t) *p))
   p++;                         /* span identifier */
 c = *p;
 *p = '\0';                     /* terminate string, but save clobbered char */
 if ((n = isKeyWord())) {       /* a standard LUX-language key Word */
   *p = c;                      /* restore clobbered char */
   currentChar = p;             /* continue parsing beyond key Word */
   return n;                    /* return keyword index */
 }
 /* the identifier is not a standard key Word */
 if (!ignoreInput) {            /* we're not ignoring stuff, so need to save */
   *lvalp = installString(currentChar); /* save string, index in *lvalp */
   *p = c;                      /* restore clobbered char */
   n = isNextChar((uint8_t) *currentChar)? TOK_C_ID: TOK_S_ID;
   currentChar = p;             /* continue parse beyond string */
   if (*lvalp < 0)
     return LUX_ERROR;          /* some error occurred */
   return n;                    /* return "special identifier" token */
 } else {                       /* we are ignoring stuff, don't save */
   currentChar = p;             /* so continue parse beyond string */
   return 0;                    /* signal that nothing was saved */
 }
}
/*--------------------------------------------------------------*/
int32_t yylex(YYSTYPE *lvalp)
/* returns semantic value of next read token in *lvalp, and the
   lexical value as function return value */
{
 char   *p, c, *p2;
 char const* prompt;
 int32_t        i;
 static int32_t len = 0;
 extern char* currentInputFile;
 extern int32_t curLineNumber;  /* current line number */
 int32_t        getNewLine(char *buffer, size_t bufsize, char const* prompt, char historyFlag),
   showstats(int32_t narg, int32_t ps[]);

 if (errorState)
   return TOK_ERRORSTATE;       /* if a syntax error occurred, then */
                                /* tell the parser so. */
 /* now determine the appropriate LUX prompt to use */
 if (ignoreInput)               /* we're within a nested IGNORE-RESUME pair */
   prompt = LUXPrompts[2];      /* ign> */
 else if (debugLine)            /* debugger line (execute()) */
   prompt = LUXPrompts[3];      /* dbg> */
 else if (disableNewline)       /* need more input */
   prompt = LUXPrompts[1];      /* mor> */
 else if (calculatorMode)
   prompt = LUXPrompts[4];      /* clc> */
 else
   prompt = LUXPrompts[0];      /* default, LUX> */
 /* now get and treat the input */
 while (1) {                    /* keep cycling */
   if (!*line) {                /* nothing more in current input line */
     while (!*line) {           /* try until there's something to do */
       if (len < 0)             /* EOF reached */
         putchar('\r');         /* ensure cursor is at left */
       len = getNewLine(line, BUFSIZE, prompt, inHistoryBuffer);        /* read new line; */
                                /* the length (excluding final \0) into len */
       curLineNumber++;         /* update current line number */
       if (len < 0) {           /* found end of file (EOF) */
         if (ignoreInput) {
           /* ignoreInput is non-zero either because we're under influence */
           /* of one or more IGNOREs, or because we're skimming over the */
           /* body of a function or subroutine through a @@file command */
           if (!findBody)       /* dangling IGNORE(s) */
             printf("%1d IGNOREs still enforced at EOF!\n", ignoreInput); 
         }
         return 0;              /* signal end-of-file to yyparse() */
       }
     }
     translateLine();           /* translate input line for easier parsing */
     currentChar = tLine;       /* first character of translated input line */
                                /* is current one in parsing */
   }
   while (isspace((uint8_t) *currentChar))
     currentChar++;             /* skip leading spaces */

   /* we recognize RESUME and IGNORE only at the beginning of a line */
   if (currentChar == tLine) {
     if (!strncmp(currentChar, "ignore", 6)
         && !isNextChar((uint8_t) currentChar[6])) {
       ignoreInput++;
       prompt = LUXPrompts[2];  /* ign> */
     } else if (!strncmp(currentChar, "resume", 6)
                && !isNextChar((uint8_t) currentChar[6])) {
       if (!ignoreInput)
         puts("Unmatched resume ignored");
       else
         ignoreInput--;
       if (!ignoreInput) {
         if (disableNewline)    /* need more input */
           prompt = LUXPrompts[1];      /* mor> */
         else if (debugLine)    /* debugger line */
           prompt = LUXPrompts[3];      /* dbg> */
         else if (calculatorMode)
           prompt = LUXPrompts[4];      /* clc> */
         else
           prompt = LUXPrompts[0];      /* default, LUX> */
       }
       currentChar += 6;
     }
     while (isspace(*currentChar))
       ++currentChar;
   }

   if (ignoreInput && !findBody) { /* ignoring input and not looking for */
                                   /* a routine body */
     *line = '\0';
     continue;
   }

   if (*currentChar == TRANS_FIXED_STRING) { /* a literal string */
                                /* find the end of the string */
     for (p2 = p = currentChar + 1; *p2 && *p2 != TRANS_FIXED_STRING; p2++);
     if (!*p2) {                /* reached end-of-line before end of string */
       if ((setup & 1024) == 0) {
         printf("WARNING - unfinished literal string \"%s\" in line %d of",
                line, curLineNumber);
         if (currentInputFile)
           printf(" file %s\n", currentInputFile);
         else
           puts(" standard input");
       }
       currentChar = p2;
     } else {
       *p2 = '\0';              /* terminate string */
       currentChar = p2 + 1;    /* resume analysis beyond the string */
     }
     if (!ignoreInput) {        /* we're not IGNOREing stuff now */
       *lvalp = installString(p); /* save the string in the symbol stack */
       return TOK_STR;            /* token type is string */
     }
     continue;                  /* we are ignoring stuff, so continue */
   }

   if (*currentChar == '#' && !isNextChar(currentChar[1])) {
     /* it's the # operator, not an identifier starting with # */
     currentChar++;
     if (ignoreInput)
       continue;
     return '#';
   }

   if (isFirstChar((uint8_t) *currentChar)) { /* an identifier of some sort */
     i = readIdentifier(lvalp); /* which standard one or what kind is it? */
     switch (i) {
       case TOK_SUBR: case TOK_FUNC: case TOK_BLOCK: /* SUBR, FUNC, BLOCK: */
                                /* definition start */
         if (findBody > 0) {    /* we're looking for the body of some */
                                /* routine/function.  If the just */
                                /* encounterd routine/function is the */
                                /* sought one, then we need to compile it */
           for (p = currentChar + 2; isNextChar((uint8_t) *p); p++);
           /* end of name */
           c = *p;
           *p = '\0';           /* temporary termination */
                                /* get name of sought routine/function */
           {
             char const *cp;

             if (i == TOK_SUBR)
               cp = subrName(findBody);
             else if (i == TOK_FUNC)
               cp = funcName(findBody);
             else
               cp = blockName(findBody);
             /* we only accept a match if the routine name matches */
             /* exactly; i.e., no partial matches */

             if (!strcmp(currentChar + 1, cp)) { /* same */
               ignoreInput--;   /* yes: reduce IGNORE level, */
               *p = c;          /* restore */
               return i;
             }
             *p = c;            /* restore */
           }
         }
         if (ignoreInput)
           continue;            /* we're ignoring stuff, so get next */
         else
           return i;            /* not ignoring, so pass on to yyparse() */
       case TOK_ENDSUBR: case TOK_ENDFUNC: case TOK_ENDBLOCK: /* ENDSUBR, ENDFUNC, */
                                /* ENDBLOCK: end of definition */
         if (findBody && !ignoreInput) { /* we were looking for a routine/ */
           /* function body (findBody) and were not ignoring input */
           /* (!ignoreInput), so we must have just completed compilation */
           /* of the sought routine/function. */
           findBody = -findBody;/* flag completion of body */
           ignoreInput++;       /* ignore rest of the input, and */
           return i;            /* pass token on to yyparse() */
         }
         if (ignoreInput)
           continue;            /* wasn't sought routine, keep looking */
         else
           return i;            /* pass on to yyparse() */
       case 0:                  /* nothing was saved; stuff was ignored */
         continue;              /* so continue parsing */
       default:
         if (ignoreInput)
           continue;            /* ignoring, so continue parsing */
         return i;              /* some user-defined identifier, pass */
                                /* returned C_ID or S_ID on to yyparse() */
     }
   }

   if (isdigit((uint8_t) *currentChar)) {       /* a number */
     i = readNumber(lvalp);     /* read number into symbol */
     if (ignoreInput)
       continue;                /* we're ignoring stuff, so the number */
                                /* wasn't saved by readNumber() and we */
                                /* can continue parsing */
     else
       return i;                /* pass token NUM on to yyparse() */
   }

   if (isTwoOperator((uint8_t) *currentChar)) { /* possibly op-assignment */
     if (currentChar[1] == '=') { /* yes, an operation-assignment (e.g. +=) */
       currentChar += 2;        /* continue parse beyond operator */
       if (ignoreInput)
         continue;              /* ignoring stuff, so continue parse */
       switch (currentChar[-2]) { /* which operation? */
         case '+':
           return TOK_PLUSIS;       /* += */
         case '-':
           return TOK_MINUSIS;      /* -= */
         case '*':
           return TOK_TIMESIS;      /* *= */
         case '/':
           return TOK_DIVIDEIS;     /* /= */
         case '^':
           return TOK_POWERIS; }    /* ^= */
     }
   }

  if (!*currentChar) {          /* end-of-line */
    oldChar = *line;            /* (??) */
    *line = '\0';               /* request a new input line */
    if (!disableNewline && !ignoreInput) /* not ignoring things, and */
                                /* NLs are honored */
      return TOK_NL;
    else
     continue;                  /* else ignore NL and continue parse */
  }

  if (*currentChar == '@') {    /* file inclusion */
    /* If a file is included with the @@ prefix, then any SUBR, FUNC, or */
    /* BLOCK definition contained in the file is noted and the body of */
    /* that routine is parsed, but not yet compiled.  If the routine is */
    /* later referenced, then the file is read again to compile the routine. */
    /* This means that files with lots of routine definitions of which */
    /* only a few are going to be used don't clog up the symbol tables. */
    i = 0;                      /* signal @ */
    if (currentChar[1] == '@') { /* i.e., @@ */
      if (!ignoreInput)
        i = 1;                  /* no ignoring, signal @@ */
       currentChar++;           /* skip extra @ */
    }
    for (p = currentChar + 1; isFileNameChar((uint8_t) *p); p++); /* span name */
    if (!ignoreInput) {         /* not ignoring, so note file name */
      c = *p;                   /* save character beyond file name */
      *p = '\0';                /* temporary string terminator */
      *lvalp = installString(currentChar + 1);  /* save file name */
      *p = c;                   /* restore clobbered char */
    }
    currentChar = p;            /* continue parse beyond file name */
    if (ignoreInput)
      continue;                 /* ignoring stuff, so continue parse */
    else
      return i? TOK_REPORT: TOK_INCLUDE; /* @@ -> REPORT, @ -> INCLUDE */
  }

  if (*currentChar == '-'
      && !currentChar[1]) {     /* continuation character */
    continuation = 1;
    prompt = LUXPrompts[1];     /* mor> */
    *line = '\0';               /* request a new input line */
    continue;
  }

  if (*currentChar == '?') {    /* show vital statistics immediately, */
                                /* i.e. during parsing (!)*/
    showstats(0, NULL);
    /* now move over rest of line so '?' disappears */
    memcpy(currentChar, currentChar + 1, strlen(currentChar));
    continue;                   /* continue parse */
  }

  if (*currentChar == '.') {    /* a structure tag or ellipsis or a number */
    if (isdigit((uint8_t) currentChar[1])) { /* a number */
      i = readNumber(lvalp);
      if (ignoreInput)
        continue;
      else
        return i;
    } else if (isNextChar((uint8_t) currentChar[1])) { /* a string tag */
      p = ++currentChar;        /* skip . */
      while (isNextChar((uint8_t) *currentChar))
        currentChar++;          /* span identifier */
      if (ignoreInput)
        continue;
      c = *currentChar;
      *currentChar = '\0';      /* temporary string end */
      *lvalp = string_scratch(currentChar - p);
      strcpy(string_value(*lvalp), p);
      *currentChar = c;
      return TOK_STRUCTTAG;
    } else if (currentChar[1] == '.' && currentChar[2] == '.') { /* ellipsis */
      currentChar += 3;
      if (ignoreInput)
        continue;
      return TOK_ELLIPSIS;
    } else                      /* nothing yet; pass as is */
      currentChar--;
  }

  switch (*currentChar) {
    case '=':
      if (currentChar[1] == '=') { /* == corresponds to EQ */
        currentChar += 2;
        if (ignoreInput)
          continue;
        return TOK_EQ;
      }
      break;
    case '!':
      if (currentChar[1] == '=') { /* != corresponds to NE */
        currentChar += 2;
        if (ignoreInput)
          continue;
        return TOK_NE;
      }
      break;
    case '>':
      if (currentChar[1] == '=') { /* >= corresponds to GE */
        currentChar += 2;
        if (ignoreInput)
          continue;
        return TOK_GE;
      }
      break;
    case '<':
      if (currentChar[1] == '=') { /* <= corresponds to LE */
        currentChar += 2;
        if (ignoreInput)
          continue;
        return TOK_LE;
      }
      break;
    case '&':
      if (currentChar[1] == '&') { /* && corresponds to ANDIF */
        currentChar += 2;
        if (ignoreInput)
          continue;
        return TOK_ANDIF;
      }
      break;
    case '|':
      if (currentChar[1] == '|') { /* || corresponds to ORIF */
        currentChar += 2;
        if (ignoreInput)
          continue;
        return TOK_ORIF;
      }
      break;
  }

  /* only thing left: single char (e.g. + or [ ) */
  if (!ignoreInput)             /* not ignoring: pass on */
    return *currentChar++;
  else {                        /* ignoring, skip and continue parse */
    currentChar++;
    continue;
  }
 }
}
/*--------------------------------------------------------------*/
int32_t calc_lex(YYSTYPE *lvalp)
/* required to make ana_calculator() parsing work */
{
  return yylex(lvalp);
}
/*--------------------------------------------------------------*/
int32_t calc_error(char *s)
/* required to make ana_calculator() parsing work */
{
  return yyerror(s);
}
/*--------------------------------------------------------------*/
void gehandler(const char *reason, const char *file, int32_t line, int32_t gsl_errno)
{
  luxerror("GSL error %d (%s line %d): %s", 0, gsl_errno, file, line, reason);
}
/*--------------------------------------------------------------*/
#if HAVE_LIBGSL
# include <gsl/gsl_errno.h>      // for gsl_set_error_handler
#endif
char    *programName;
int do_main(int argc, char *argv[])
     /* main program */
{
  int32_t       site(int32_t, int32_t []), readHistory(void);
  char  *p;
  extern int32_t        nSymbolStack;
  extern void   getTerminalSize(void);
  void  pegParse(void), inHistory(char *), getTermCaps(void);
  FILE  *fp;
  int32_t       yyparse(void);
  void register_printf_extensions();
  void register_the_bindings();

  register_printf_extensions();
  register_the_bindings();

  programName = argv[0];
  getTerminalSize();
  site(0, NULL);                /* identify this version of LUX */
  fflush(stdin);
  symbolInitialization();
  readHistory();
  stifle_history(100);          /* limit history buffer to 100 lines */
  *line = '\0';                 /* start with an empty line */
  p = line;

#if HAVE_LIBGSL
  void gehandler(const char *, const char *, int32_t, int32_t);

  gsl_set_error_handler(&gehandler);
#endif
  /* seek .luxinit in home directory */
  fp = fopen(expand_name("~/.luxinit", NULL), "r");
  if (fp) {
    fclose(fp);
    strcpy(p, "@~/.luxinit");
    p += strlen(p);
  }
  /* now treat the command line "options" as if they were typed at */
  /* the LUX> prompt */
  argc--;  argv++;
  while (argc--) {
    *p++ = ' ';
    if (**argv == '/')
      *p++ = '@';
    strcpy(p, *argv++);
    p += strlen(p);
  }
  if (*line) {
    printf("%s%s\n", LUXPrompts[0], line);
    add_history(line + 1);
    translateLine();
    currentChar = tLine;
  }
  setjmp(jmpenv);
  clock();                      /* or it does not seem to start counting */
                                /* on some machines */
  /* the main parsing loop: if an error occurs, then clean up and restart */
  /* parsing */
  do
  { errorState = 0;             /* no syntax errors (yet) in this parse */
    pegParse();
    yyparse();                  /* parse input (calls yylex()) */
    if (errorState)             /* a syntax error occurred */
    { cleanUp(0, CLEANUP_ERROR);
      /* clean up stored but yet uninterpreted symbol names */
      while (symbolStackIndex)
        if (symbolStack[symbolStackIndex - 1])
          unlinkString(symbolStackIndex - 1);
      symbolStackIndex = nSymbolStack = 0;
      listStackItem = listStack; /* reset list stack */
      statementDepth = disableNewline = curContext = 0; /* various resets */
      *line = 0; }              /* clear input line */
  } while (errorState);         /* cycle if an error occurred */
  Quit(0);
  return 1;                     /* or the compiler may complain */
}
