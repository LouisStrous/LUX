/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

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
