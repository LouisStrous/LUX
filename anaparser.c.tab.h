
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     NEWLINE = 999,
     C_ID = 1001,
     S_ID = 1002,
     NUMBER = 1003,
     STR = 1004,
     INCLUDE = 1005,
     REPORT = 1006,
     IF = 1007,
     THEN = 1008,
     ELSE = 1009,
     FOR = 1010,
     REPEAT = 1011,
     UNTIL = 1012,
     WHILE = 1013,
     DO = 1014,
     CASE = 1015,
     ENDCASE = 1016,
     NCASE = 1017,
     SUBR = 1018,
     ENDSUBR = 1019,
     FUNC = 1020,
     ENDFUNC = 1021,
     BLOCK = 1022,
     ENDBLOCK = 1023,
     RETURN = 1024,
     BREAK = 1025,
     CONTINUE = 1026,
     RUN = 1027,
     BEGIN = 1028,
     END = 1029,
     PLUSIS = 1030,
     MINUSIS = 1031,
     TIMESIS = 1032,
     DIVIDEIS = 1033,
     POWERIS = 1034,
     RETALL = 1035,
     STRUCTTAG = 1036,
     ERRORSTATE = 1037,
     ELLIPSIS = 1038,
     ORIF = 1039,
     ANDIF = 1040,
     XOR = 1041,
     OR = 1042,
     AND = 1043,
     NE = 1044,
     EQ = 1045,
     LT = 1046,
     GT = 1047,
     LE = 1048,
     GE = 1049,
     SMOD = 1050,
     UMINUS = 1051
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif




