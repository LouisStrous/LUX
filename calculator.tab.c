/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse calcparse
#define yylex   calclex
#define yyerror calcerror
#define yylval  calclval
#define yychar  calcchar
#define yydebug calcdebug
#define yynerrs calcnerrs


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
     UMINUS = 1050
   };
#endif
/* Tokens.  */
#define NEWLINE 999
#define C_ID 1001
#define S_ID 1002
#define NUMBER 1003
#define STR 1004
#define INCLUDE 1005
#define REPORT 1006
#define IF 1007
#define THEN 1008
#define ELSE 1009
#define FOR 1010
#define REPEAT 1011
#define UNTIL 1012
#define WHILE 1013
#define DO 1014
#define CASE 1015
#define ENDCASE 1016
#define NCASE 1017
#define SUBR 1018
#define ENDSUBR 1019
#define FUNC 1020
#define ENDFUNC 1021
#define BLOCK 1022
#define ENDBLOCK 1023
#define RETURN 1024
#define BREAK 1025
#define CONTINUE 1026
#define RUN 1027
#define BEGIN 1028
#define END 1029
#define PLUSIS 1030
#define MINUSIS 1031
#define TIMESIS 1032
#define DIVIDEIS 1033
#define POWERIS 1034
#define RETALL 1035
#define STRUCTTAG 1036
#define ERRORSTATE 1037
#define ELLIPSIS 1038
#define ORIF 1039
#define ANDIF 1040
#define XOR 1041
#define OR 1042
#define AND 1043
#define NE 1044
#define EQ 1045
#define LT 1046
#define GT 1047
#define LE 1048
#define GE 1049
#define UMINUS 1050




/* Copy the first part of user declarations.  */
#line 1 "calculator.c"


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include "editor.h"
#include "action.h"
#include "anaparser.c.tab.h"

extern Int	ans;

#define startList(x)	{ pushList(ANA_NEW_LIST); pushList(x); }
				/* start a new list */
void	pushList(Word symNum),	/* push symbol number onto list stack */
	swapList(Int, Int),	/* swap items in the list stack */
	away(void);
Word	popList(void);		/* pop an item from the list stack's top */
Int	stackListLength(void),	/* return length of list at top of stack */
	isInternalSubr(Int),	/* 1 if symbol is internal subroutine */
	installExec(void),
	findSym(Int, hashTableEntry *[], Int),
	installSubsc(Int),
	anaerror(char *, Int, ...), ana_replace(Int, Int), ana_type(Int, Int []),
	newSymbol(Int, ...);
Int	yyerror(char *), yylex(YYSTYPE *);


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef Int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 249 "calculator.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef int16_t yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef uint16_t yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef int16_t yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T uint32_t
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static Int
YYID (Int i)
#else
static Int
YYID (i)
    Int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

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
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
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
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  59
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   971

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  70
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  12
/* YYNRULES -- Number of rules.  */
#define YYNRULES  82
/* YYNRULES -- Number of states.  */
#define YYNSTATES  172

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   1050

#define YYTRANSLATE(YYX)						\
  ((uint32_t) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    60,     2,     2,
      64,    67,    58,    56,    65,    57,    63,    59,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    66,     2,
      54,    40,    55,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    68,     2,    69,    62,     2,     2,     2,     2,     2,
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
      33,    34,    35,    36,    37,    38,    39,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      61
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     8,    11,    16,    20,    24,    28,
      32,    36,    40,    44,    48,    52,    56,    60,    64,    68,
      72,    76,    80,    84,    88,    92,    95,    99,   102,   104,
     106,   108,   110,   114,   116,   118,   120,   122,   126,   131,
     133,   136,   141,   145,   149,   155,   159,   165,   173,   179,
     183,   185,   189,   192,   194,   196,   198,   200,   205,   209,
     213,   217,   221,   225,   229,   233,   237,   241,   245,   249,
     253,   257,   261,   265,   269,   273,   277,   281,   285,   289,
     294,   297,   299
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      71,     0,    -1,    72,    -1,    71,    72,    -1,    73,     3,
      -1,    73,    40,    80,     3,    -1,    56,    80,     3,    -1,
      57,    80,     3,    -1,    58,    80,     3,    -1,    59,    80,
       3,    -1,    60,    80,     3,    -1,    62,    80,     3,    -1,
      55,    80,     3,    -1,    54,    80,     3,    -1,    49,    80,
       3,    -1,    48,    80,     3,    -1,    53,    80,     3,    -1,
      51,    80,     3,    -1,    50,    80,     3,    -1,    52,    80,
       3,    -1,    47,    80,     3,    -1,    46,    80,     3,    -1,
      45,    80,     3,    -1,    44,    80,     3,    -1,    43,    80,
       3,    -1,    80,     3,    -1,    40,    80,     3,    -1,    27,
       3,    -1,     3,    -1,     4,    -1,     5,    -1,    75,    -1,
      74,    65,    75,    -1,    80,    -1,    76,    -1,    78,    -1,
      77,    -1,    77,    66,    56,    -1,    77,    66,    55,    80,
      -1,    56,    -1,    55,    80,    -1,    80,    66,    55,    80,
      -1,    80,    66,    56,    -1,    80,    66,    80,    -1,    80,
      66,    58,    57,    80,    -1,    80,    66,    58,    -1,    58,
      57,    80,    66,    80,    -1,    58,    57,    80,    66,    58,
      57,    80,    -1,    58,    57,    80,    66,    58,    -1,    58,
      57,    80,    -1,    58,    -1,    79,    40,    80,    -1,    59,
      79,    -1,     4,    -1,     6,    -1,     7,    -1,    73,    -1,
      73,    64,    74,    67,    -1,    80,    56,    80,    -1,    80,
      57,    80,    -1,    80,    58,    80,    -1,    80,    59,    80,
      -1,    80,    60,    80,    -1,    80,    62,    80,    -1,    80,
      55,    80,    -1,    80,    54,    80,    -1,    80,    49,    80,
      -1,    80,    48,    80,    -1,    80,    53,    80,    -1,    80,
      51,    80,    -1,    80,    50,    80,    -1,    80,    52,    80,
      -1,    80,    47,    80,    -1,    80,    46,    80,    -1,    80,
      45,    80,    -1,    80,    44,    80,    -1,    80,    43,    80,
      -1,    68,    81,    69,    -1,    64,    80,    67,    -1,    80,
      64,    74,    67,    -1,    57,    80,    -1,    80,    -1,    81,
      65,    80,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,    60,    60,    61,    65,    71,    75,    79,    83,    87,
      91,    95,    99,   103,   107,   111,   115,   119,   123,   127,
     131,   135,   139,   143,   148,   153,   156,   159,   162,   167,
     168,   172,   174,   179,   180,   181,   189,   190,   191,   193,
     195,   197,   200,   211,   212,   214,   215,   217,   219,   221,
     222,   226,   228,   233,   237,   238,   240,   242,   244,   246,
     248,   250,   252,   254,   256,   258,   260,   262,   264,   266,
     268,   270,   272,   274,   276,   278,   281,   284,   286,   288,
     291,   297,   299
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NEWLINE", "C_ID", "S_ID", "NUMBER",
  "STR", "INCLUDE", "REPORT", "IF", "THEN", "ELSE", "FOR", "REPEAT",
  "UNTIL", "WHILE", "DO", "CASE", "ENDCASE", "NCASE", "SUBR", "ENDSUBR",
  "FUNC", "ENDFUNC", "BLOCK", "ENDBLOCK", "RETURN", "BREAK", "CONTINUE",
  "RUN", "BEGIN", "END", "PLUSIS", "MINUSIS", "TIMESIS", "DIVIDEIS",
  "POWERIS", "RETALL", "STRUCTTAG", "'='", "ERRORSTATE", "ELLIPSIS",
  "ORIF", "ANDIF", "XOR", "OR", "AND", "NE", "EQ", "LT", "GT", "LE", "GE",
  "'<'", "'>'", "'+'", "'-'", "'*'", "'/'", "'%'", "UMINUS", "'^'", "'.'",
  "'('", "','", "':'", "')'", "'['", "']'", "$accept", "statements",
  "statement", "var", "subsc_list", "subsc_or_key", "subsc", "range",
  "key", "key_param", "expr", "expr_list", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,  1000,   999,  1001,  1002,  1003,  1004,  1005,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,
    1027,  1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,  1036,
      61,  1037,  1038,  1039,  1040,  1041,  1042,  1043,  1044,  1045,
    1046,  1047,  1048,  1049,    60,    62,    43,    45,    42,    47,
      37,  1050,    94,    46,    40,    44,    58,    41,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    70,    71,    71,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    73,
      73,    74,    74,    75,    75,    75,    76,    76,    76,    76,
      76,    76,    76,    77,    77,    77,    77,    77,    77,    77,
      77,    78,    78,    79,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    81,    81
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     2,     4,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     3,     2,     1,     1,
       1,     1,     3,     1,     1,     1,     1,     3,     4,     1,
       2,     4,     3,     3,     5,     3,     5,     7,     5,     3,
       1,     3,     2,     1,     1,     1,     1,     4,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     4,
       2,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    28,    29,    30,    54,    55,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       2,    56,     0,    27,     0,    56,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    80,     0,     0,     0,     0,     0,    81,     0,     1,
       3,     4,     0,     0,    25,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    80,    26,    24,    23,    22,
      21,    20,    15,    14,    18,    17,    19,    16,    13,    12,
       6,     7,     8,     9,    10,    11,    78,     0,    77,     0,
      29,     0,    39,    50,     0,     0,    31,    34,    36,    35,
       0,    33,    76,    75,    74,    73,    72,    67,    66,    70,
      69,    71,    68,    65,    64,    58,    59,    60,    61,    62,
      63,     0,    82,     5,    40,     0,    53,    52,     0,    57,
       0,     0,     0,    79,    49,    32,     0,    37,    51,     0,
      42,    45,    43,     0,    38,    41,     0,    48,    46,    44,
       0,    47
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,    29,    30,    35,   115,   116,   117,   118,   119,   120,
     121,    58
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -62
static const yytype_int16 yypact[] =
{
     194,   -62,   -62,   -62,   -62,   -62,    29,    49,    49,    49,
      49,    49,    49,    49,    49,    49,    49,    49,    49,    49,
      49,    49,    49,    49,    49,    49,    49,    49,    49,   128,
     -62,    -1,   220,   -62,    49,   -17,   252,   278,   310,   336,
     368,   394,   426,   452,   484,   510,   542,   568,   600,   626,
     658,    41,   684,   716,   742,   774,   160,   870,    -8,   -62,
     -62,   -62,    49,    31,   -62,    49,    49,    49,    49,    49,
      49,    49,    49,    49,    49,    49,    49,    49,    49,    49,
      49,    49,    49,    49,    31,   -59,   -62,   -62,   -62,   -62,
     -62,   -62,   -62,   -62,   -62,   -62,   -62,   -62,   -62,   -62,
     -62,   -62,   -62,   -62,   -62,   -62,   -62,    49,   -62,   800,
      12,    49,   -62,     2,    54,   -61,   -62,   -62,    -6,   -62,
      24,   822,   890,   890,   907,   907,   907,    65,    65,    65,
      65,    65,    65,    80,    80,   171,   171,   -59,   -59,   -59,
     -59,   -34,   870,   -62,   870,    49,   -62,   -62,    31,   -62,
     -10,    49,    36,   -62,   846,   -62,    49,   -62,   870,    49,
     -62,    27,   870,    44,   870,   870,    49,    28,   870,   870,
      49,   870
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -62,   -62,    67,     1,    13,   -50,   -62,   -62,   -62,    -5,
       0,   -62
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -54
static const yytype_int16 yytable[] =
{
      32,    31,    61,    83,   148,    84,   149,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    32,
      31,   148,    33,   153,    85,   110,     3,     4,     5,    62,
       2,     3,     4,     5,   101,   156,   157,    63,     2,     3,
       4,     5,   -53,     2,     3,     4,     5,   107,   146,   145,
     150,   108,   109,    63,   151,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   166,   170,   111,   112,    34,   113,
     114,   159,   160,    34,   161,    27,    60,   141,   155,    28,
      27,    34,   167,    83,    28,    84,    34,   142,    27,   147,
       0,   144,    28,    27,     0,     0,     0,    28,     0,    76,
      77,    78,    79,    80,    81,    82,     0,    83,    59,    84,
       0,     1,     2,     3,     4,     5,    78,    79,    80,    81,
      82,     0,    83,     0,    84,   154,     0,     0,     0,     0,
       0,   158,   162,     0,     0,     6,   164,     0,     0,   165,
       0,     0,     0,   168,     0,     0,   169,     0,     7,     0,
     171,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,     0,
      26,     0,    27,     0,     0,     0,    28,     1,     2,     3,
       4,     5,     0,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,     6,    83,    64,    84,     0,     0,   106,     0,    80,
      81,    82,     0,    83,     7,    84,     0,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    86,    26,     0,    27,     0,
       0,     0,    28,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    87,    83,     0,    84,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    88,    83,     0,    84,     0,     0,     0,
       0,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    89,
      83,     0,    84,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    90,    83,     0,    84,     0,     0,     0,     0,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    91,    83,     0,
      84,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    92,
      83,     0,    84,     0,     0,     0,     0,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    93,    83,     0,    84,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    94,    83,     0,
      84,     0,     0,     0,     0,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    95,    83,     0,    84,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    96,    83,     0,    84,     0,
       0,     0,     0,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    97,    83,     0,    84,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    98,    83,     0,    84,     0,     0,     0,
       0,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    99,
      83,     0,    84,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,   100,    83,     0,    84,     0,     0,     0,     0,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,   102,    83,     0,
      84,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,   103,
      83,     0,    84,     0,     0,     0,     0,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,   104,    83,     0,    84,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,   105,    83,     0,
      84,     0,     0,     0,     0,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,   143,    83,     0,    84,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,     0,    83,     0,    84,     0,
       0,     0,     0,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,     0,    83,     0,    84,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,     0,    83,     0,    84,     0,   152,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,     0,    83,     0,
      84,     0,   163,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,     0,    83,     0,    84,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,     0,    83,     0,    84,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,     0,    83,
       0,    84
};

static const yytype_int16 yycheck[] =
{
       0,     0,     3,    62,    65,    64,    67,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      29,    65,     3,    67,    34,     4,     5,     6,     7,    40,
       4,     5,     6,     7,     3,    55,    56,    64,     4,     5,
       6,     7,    40,     4,     5,     6,     7,    65,     4,    57,
      66,    69,    62,    64,    40,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    57,    57,    55,    56,    57,    58,
      59,    55,    56,    57,    58,    64,    29,    84,   148,    68,
      64,    57,    58,    62,    68,    64,    57,   107,    64,   114,
      -1,   111,    68,    64,    -1,    -1,    -1,    68,    -1,    54,
      55,    56,    57,    58,    59,    60,    -1,    62,     0,    64,
      -1,     3,     4,     5,     6,     7,    56,    57,    58,    59,
      60,    -1,    62,    -1,    64,   145,    -1,    -1,    -1,    -1,
      -1,   151,   152,    -1,    -1,    27,   156,    -1,    -1,   159,
      -1,    -1,    -1,   163,    -1,    -1,   166,    -1,    40,    -1,
     170,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    -1,
      62,    -1,    64,    -1,    -1,    -1,    68,     3,     4,     5,
       6,     7,    -1,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    27,    62,     3,    64,    -1,    -1,    67,    -1,    58,
      59,    60,    -1,    62,    40,    64,    -1,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,     3,    62,    -1,    64,    -1,
      -1,    -1,    68,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,     3,    62,    -1,    64,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,     3,    62,    -1,    64,    -1,    -1,    -1,
      -1,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,     3,
      62,    -1,    64,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,     3,    62,    -1,    64,    -1,    -1,    -1,    -1,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,     3,    62,    -1,
      64,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,     3,
      62,    -1,    64,    -1,    -1,    -1,    -1,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,     3,    62,    -1,    64,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,     3,    62,    -1,
      64,    -1,    -1,    -1,    -1,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,     3,    62,    -1,    64,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,     3,    62,    -1,    64,    -1,
      -1,    -1,    -1,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,     3,    62,    -1,    64,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,     3,    62,    -1,    64,    -1,    -1,    -1,
      -1,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,     3,
      62,    -1,    64,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,     3,    62,    -1,    64,    -1,    -1,    -1,    -1,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,     3,    62,    -1,
      64,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,     3,
      62,    -1,    64,    -1,    -1,    -1,    -1,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,     3,    62,    -1,    64,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,     3,    62,    -1,
      64,    -1,    -1,    -1,    -1,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,     3,    62,    -1,    64,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    -1,    62,    -1,    64,    -1,
      -1,    -1,    -1,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    -1,    62,    -1,    64,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    -1,    62,    -1,    64,    -1,    66,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    -1,    62,    -1,
      64,    -1,    66,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    -1,    62,    -1,    64,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    -1,    62,    -1,    64,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    -1,    62,
      -1,    64
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     7,    27,    40,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    62,    64,    68,    71,
      72,    73,    80,     3,    57,    73,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    81,     0,
      72,     3,    40,    64,     3,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    62,    64,    80,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,    67,    65,    69,    80,
       4,    55,    56,    58,    59,    74,    75,    76,    77,    78,
      79,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    74,    80,     3,    80,    57,     4,    79,    65,    67,
      66,    40,    66,    67,    80,    75,    55,    56,    80,    55,
      56,    58,    80,    66,    80,    80,    57,    58,    80,    80,
      57,    80
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, Int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    Int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, Int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    Int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, Int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    Int yyrule;
#endif
{
  Int yynrhs = yyr2[yyrule];
  Int yyi;
  unsigned int64_t yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
Int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
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



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that Double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null Byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, Int yystate, Int yychar)
{
  Int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      Int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      Int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      Int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      Int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      Int yychecklim = YYLAST - yyn + 1;
      Int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      Int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  Int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, Int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    Int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
Int yyparse (void *YYPARSE_PARAM);
#else
Int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
Int yyparse (void);
#else
Int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */






/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
Int
yyparse (void *YYPARSE_PARAM)
#else
Int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
Int
yyparse (void)
#else
Int
yyparse ()

#endif
#endif
{
  /* The look-ahead symbol.  */
Int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
Int yynerrs;

  Int yystate;
  Int yyn;
  Int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  Int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  Int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  Int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned int64_t) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
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
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

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
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 4:
#line 66 "calculator.c"
    { startList(ans);
		  /* ana_replace(ans, eval(installSubsc($1))); */
		  ana_replace(ans,
			      eval(newSymbol(ANA_EXTRACT, -ANA_RANGE, (yyvsp[(1) - (2)]))));
		  ana_type(1, &ans); ;}
    break;

  case 5:
#line 72 "calculator.c"
    { (yyval) = findVar((yyvsp[(1) - (4)]), 0);
		ana_replace((yyval), (yyvsp[(3) - (4)]));
		ana_type(1, &ans); ;}
    break;

  case 6:
#line 76 "calculator.c"
    { ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_ADD,
		ans, (yyvsp[(2) - (3)]))));
		ana_type(1, &ans); ;}
    break;

  case 7:
#line 80 "calculator.c"
    { ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_SUB,
		ans, (yyvsp[(2) - (3)]))));
		ana_type(1, &ans); ;}
    break;

  case 8:
#line 84 "calculator.c"
    { ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_MUL,
		ans, (yyvsp[(2) - (3)]))));
		ana_type(1, &ans); ;}
    break;

  case 9:
#line 88 "calculator.c"
    { ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_DIV,
		ans, (yyvsp[(2) - (3)]))));
		ana_type(1, &ans); ;}
    break;

  case 10:
#line 92 "calculator.c"
    { ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_MOD,
		ans, (yyvsp[(2) - (3)])))); 
		ana_type(1, &ans); ;}
    break;

  case 11:
#line 96 "calculator.c"
    { ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_POW,
		ans, (yyvsp[(2) - (3)])))); 
		ana_type(1, &ans); ;}
    break;

  case 12:
#line 100 "calculator.c"
    { ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_MAX,
		ans, (yyvsp[(2) - (3)])))); 
		ana_type(1, &ans); ;}
    break;

  case 13:
#line 104 "calculator.c"
    { ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_MIN,
		ans, (yyvsp[(2) - (3)])))); 
		ana_type(1, &ans); ;}
    break;

  case 14:
#line 108 "calculator.c"
    { ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_EQ,
		ans, (yyvsp[(2) - (3)])))); 
		ana_type(1, &ans); ;}
    break;

  case 15:
#line 112 "calculator.c"
    { ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_NE,
		ans, (yyvsp[(2) - (3)])))); 
		ana_type(1, &ans); ;}
    break;

  case 16:
#line 116 "calculator.c"
    { ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_GE,
		ans, (yyvsp[(2) - (3)])))); 
		ana_type(1, &ans); ;}
    break;

  case 17:
#line 120 "calculator.c"
    { ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_GT,
		ans, (yyvsp[(2) - (3)])))); 
		ana_type(1, &ans); ;}
    break;

  case 18:
#line 124 "calculator.c"
    { ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_LT,
		ans, (yyvsp[(2) - (3)])))); 
		ana_type(1, &ans); ;}
    break;

  case 19:
#line 128 "calculator.c"
    { ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_LE,
		ans, (yyvsp[(2) - (3)])))); 
		ana_type(1, &ans); ;}
    break;

  case 20:
#line 132 "calculator.c"
    { ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_AND,
		ans, (yyvsp[(2) - (3)])))); 
		ana_type(1, &ans); ;}
    break;

  case 21:
#line 136 "calculator.c"
    { ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_OR,
		ans, (yyvsp[(2) - (3)])))); 
		ana_type(1, &ans); ;}
    break;

  case 22:
#line 140 "calculator.c"
    { ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_XOR,
		ans, (yyvsp[(2) - (3)])))); 
		ana_type(1, &ans); ;}
    break;

  case 23:
#line 145 "calculator.c"
    { ana_replace(ans, eval(newSymbol(ANA_IF_OP,
		ANA_ANDIF, ans, (yyvsp[(2) - (3)])))); 
		ana_type(1, &ans); ;}
    break;

  case 24:
#line 150 "calculator.c"
    { ana_replace(ans, eval(newSymbol(ANA_IF_OP, ANA_ORIF,
		ans, (yyvsp[(2) - (3)])))); 
		ana_type(1, &ans); ;}
    break;

  case 25:
#line 154 "calculator.c"
    { ana_replace(ans, (yyvsp[(1) - (2)]));
		ana_type(1, &ans); ;}
    break;

  case 26:
#line 157 "calculator.c"
    { ana_replace(ans, (yyvsp[(2) - (3)]));
		ana_type(1, &ans); ;}
    break;

  case 27:
#line 160 "calculator.c"
    { puts("Returning from calculator mode...");
		YYACCEPT; ;}
    break;

  case 28:
#line 163 "calculator.c"
    { ana_type(1, &ans); ;}
    break;

  case 31:
#line 173 "calculator.c"
    { startList((yyvsp[(1) - (1)])); ;}
    break;

  case 32:
#line 175 "calculator.c"
    { pushList((yyvsp[(3) - (3)])); ;}
    break;

  case 37:
#line 190 "calculator.c"
    { sym[(yyvsp[(1) - (3)])].spec.evb.args[2] = 1;  (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 38:
#line 192 "calculator.c"
    { sym[(yyvsp[(1) - (4)])].spec.evb.args[3] = (yyvsp[(4) - (4)]); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 39:
#line 193 "calculator.c"
    { (yyval) = newSymbol(ANA_PRE_RANGE, ANA_ZERO, -ANA_ONE);
			  sym[(yyval)].spec.evb.args[2] = 1; ;}
    break;

  case 40:
#line 195 "calculator.c"
    { (yyval) = newSymbol(ANA_PRE_RANGE, ANA_ZERO, -ANA_ONE);
			  sym[(yyval)].spec.evb.args[3] = (yyvsp[(2) - (2)]); ;}
    break;

  case 41:
#line 198 "calculator.c"
    { (yyval) = newSymbol(ANA_PRE_RANGE, (yyvsp[(1) - (4)]), ANA_ZERO);
			  sym[(yyval)].spec.evb.args[3] = (yyvsp[(4) - (4)]); ;}
    break;

  case 42:
#line 200 "calculator.c"
    { (yyval) = newSymbol(ANA_PRE_RANGE, (yyvsp[(1) - (3)]), ANA_ZERO);
			  sym[(yyval)].spec.evb.args[2] = 1; ;}
    break;

  case 43:
#line 211 "calculator.c"
    { (yyval) = newSymbol(ANA_PRE_RANGE, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 44:
#line 213 "calculator.c"
    { (yyval) = newSymbol(ANA_PRE_RANGE, (yyvsp[(1) - (5)]), -(yyvsp[(5) - (5)])); ;}
    break;

  case 45:
#line 214 "calculator.c"
    { (yyval) = newSymbol(ANA_PRE_RANGE, (yyvsp[(1) - (3)]), -ANA_ONE); ;}
    break;

  case 46:
#line 216 "calculator.c"
    { (yyval) = newSymbol(ANA_PRE_RANGE, -(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 47:
#line 218 "calculator.c"
    { (yyval) = newSymbol(ANA_PRE_RANGE, -(yyvsp[(3) - (7)]), -(yyvsp[(7) - (7)])); ;}
    break;

  case 48:
#line 220 "calculator.c"
    { (yyval) = newSymbol(ANA_PRE_RANGE, -(yyvsp[(3) - (5)]), -ANA_ONE); ;}
    break;

  case 49:
#line 221 "calculator.c"
    { (yyval) = newSymbol(ANA_PRE_RANGE, -(yyvsp[(3) - (3)]), ANA_ZERO); ;}
    break;

  case 50:
#line 222 "calculator.c"
    { (yyval) = newSymbol(ANA_PRE_RANGE, -ANA_ONE, ANA_ZERO); ;}
    break;

  case 51:
#line 227 "calculator.c"
    { (yyval) = newSymbol(ANA_KEYWORD, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 52:
#line 229 "calculator.c"
    { (yyval) = newSymbol(ANA_KEYWORD, (yyvsp[(2) - (2)]), ANA_ONE); ;}
    break;

  case 55:
#line 239 "calculator.c"
    { (yyval) = newSymbol(ANA_FIXED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 56:
#line 241 "calculator.c"
    { (yyval) = findVar((yyvsp[(1) - (1)]), 0); ;}
    break;

  case 57:
#line 243 "calculator.c"
    { (yyval) = eval(newSymbol(ANA_EXTRACT, -ANA_RANGE, (yyvsp[(1) - (4)]))); ;}
    break;

  case 58:
#line 245 "calculator.c"
    { (yyval) = eval(newSymbol(ANA_BIN_OP, ANA_ADD, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]))); ;}
    break;

  case 59:
#line 247 "calculator.c"
    { (yyval) = eval(newSymbol(ANA_BIN_OP, ANA_SUB, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]))); ;}
    break;

  case 60:
#line 249 "calculator.c"
    { (yyval) = eval(newSymbol(ANA_BIN_OP, ANA_MUL, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]))); ;}
    break;

  case 61:
#line 251 "calculator.c"
    { (yyval) = eval(newSymbol(ANA_BIN_OP, ANA_DIV, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]))); ;}
    break;

  case 62:
#line 253 "calculator.c"
    { (yyval) = eval(newSymbol(ANA_BIN_OP, ANA_MOD, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]))); ;}
    break;

  case 63:
#line 255 "calculator.c"
    { (yyval) = eval(newSymbol(ANA_BIN_OP, ANA_POW, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]))); ;}
    break;

  case 64:
#line 257 "calculator.c"
    { (yyval) = eval(newSymbol(ANA_BIN_OP, ANA_MAX, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]))); ;}
    break;

  case 65:
#line 259 "calculator.c"
    { (yyval) = eval(newSymbol(ANA_BIN_OP, ANA_MIN, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]))); ;}
    break;

  case 66:
#line 261 "calculator.c"
    { (yyval) = eval(newSymbol(ANA_BIN_OP, ANA_EQ, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]))); ;}
    break;

  case 67:
#line 263 "calculator.c"
    { (yyval) = eval(newSymbol(ANA_BIN_OP, ANA_NE, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]))); ;}
    break;

  case 68:
#line 265 "calculator.c"
    { (yyval) = eval(newSymbol(ANA_BIN_OP, ANA_GE, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]))); ;}
    break;

  case 69:
#line 267 "calculator.c"
    { (yyval) = eval(newSymbol(ANA_BIN_OP, ANA_GT, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]))); ;}
    break;

  case 70:
#line 269 "calculator.c"
    { (yyval) = eval(newSymbol(ANA_BIN_OP, ANA_LT, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]))); ;}
    break;

  case 71:
#line 271 "calculator.c"
    { (yyval) = eval(newSymbol(ANA_BIN_OP, ANA_LE, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]))); ;}
    break;

  case 72:
#line 273 "calculator.c"
    { (yyval) = eval(newSymbol(ANA_BIN_OP, ANA_AND, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]))); ;}
    break;

  case 73:
#line 275 "calculator.c"
    { (yyval) = eval(newSymbol(ANA_BIN_OP, ANA_OR, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]))); ;}
    break;

  case 74:
#line 277 "calculator.c"
    { (yyval) = eval(newSymbol(ANA_BIN_OP, ANA_XOR, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]))); ;}
    break;

  case 75:
#line 280 "calculator.c"
    { (yyval) = eval(newSymbol(ANA_IF_OP, ANA_ANDIF, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]))); ;}
    break;

  case 76:
#line 283 "calculator.c"
    { (yyval) = eval(newSymbol(ANA_IF_OP, ANA_ORIF, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]))); ;}
    break;

  case 77:
#line 285 "calculator.c"
    { (yyval) = eval(newSymbol(ANA_INT_FUNC, ANA_CONCAT_FUN)); ;}
    break;

  case 78:
#line 287 "calculator.c"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 79:
#line 289 "calculator.c"
    { pushList((yyvsp[(1) - (4)]));
		  (yyval) = eval(newSymbol(ANA_INT_FUNC, ANA_SUBSC_FUN)); ;}
    break;

  case 80:
#line 292 "calculator.c"
    { startList((yyvsp[(2) - (2)]));
		(yyval) = eval(newSymbol(ANA_INT_FUNC, ANA_NEG_FUN)); ;}
    break;

  case 81:
#line 298 "calculator.c"
    { startList((yyvsp[(1) - (1)])); ;}
    break;

  case 82:
#line 300 "calculator.c"
    { pushList((yyvsp[(3) - (3)])); ;}
    break;


/* Line 1267 of yacc.c.  */
#line 2240 "calculator.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
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

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
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
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
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
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 303 "calculator.c"


static char rcsid[] __attribute__ ((unused)) =
  "$Id";
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Int	ans;

Int calcerror(char *s)
/* reports parser errors - required by calcparse() */
{
  extern Int	calc_error(char *);

  return calc_error(s);
}
/*----------------------------------------------------------------------*/
Int calclex(YYSTYPE *lvalp)
/* returns semantic value of next red token in *lvalp and the lexical
 value as function return value */
{
  extern Int	calc_lex(YYSTYPE *);

  return calc_lex(lvalp);
}
/*----------------------------------------------------------------------*/
Int ana_calculator(Int narg, Int ps[])
/* go into calculator mode */
{
  extern Int	calculatorMode;
  extern char	inHistoryBuffer;
  Int	oldhb;
  Int	calcparse(void);

  if (curContext)
    return anaerror("Can only enter calculator mode from main execution level.",
		 0);
  puts("Going into calculator mode...");
  calculatorMode = 1;
  oldhb = inHistoryBuffer;
  inHistoryBuffer = 0;
  ans = lookForVarName("$", 0);
  if (ans < 0) {		/* not found */
    ans = findVarName("$", 0);
    symbol_class(ans) = ANA_SCALAR;
    scalar_type(ans) = ANA_LONG;
    scalar_value(ans).l = 0;
  }
  ana_type(1, &ans);
  calcparse();
  calculatorMode = 0;
  inHistoryBuffer = oldhb;
  return ANA_ONE;
}

