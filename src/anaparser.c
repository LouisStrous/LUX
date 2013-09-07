/* A Bison parser, made by GNU Bison 2.6.1.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
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
#define YYBISON_VERSION "2.6.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
/* Line 336 of yacc.c  */
#line 6 "anaparser.y"
 
#if HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <gsl/gsl_spline.h>
#include "action.h"
#include "anaparser.c.tab.h"
#include "editor.h"
#include "gsl/gsl_errno.h"
#define YYERROR_VERBOSE
static char rcsid[] __attribute__ ((unused)) =
 "$Id: anaparser.c,v 4.0 2001/02/07 20:36:54 strous Exp $";
#define startList(x)	{ pushList(ANA_NEW_LIST); pushList(x); }
				/* start a new list */
extern Int	scrat[],	/* general-purpose scratch storage (once.h) */
		compileLevel,	/* number of nested open input files */
		executeLevel,	/* number of nested execution items */
		symbolStackIndex, /* next free slot in symbol stack */
  setup, ANA_MATMUL_FUN;
extern char	*symbolStack[],	/* stack of not-yet parsed symbols */
		line[],		/* raw user input */
		tLine[];	/* translated user input */
extern Word	curContext,	/* context of current execution */
		listStack[],	/* stack of unincorporated list items */
		*listStackItem;	/* next free list stack item */
extern hashTableEntry	*varHashTable[], /* name hash table for variables */
		*funcHashTable[], /* name hash table for functions */
		*blockHashTable[]; /* name hash table for block routines */
extern symTableEntry	sym[];	/* all symbols */
char	debugLine = 0,		/* we're not in a debugger line */
	errorState = 0,		/* we've not just experienced an error */
	compileOnly = 0;	/* not just compiling but also executing */
Byte disableNewline = 0;	/* disables NEWLINE token so that complex */
				/* structures can be parsed across newlines */
void	pushList(Word),		/* push symbol number onto list stack */
	swapList(Int, Int),	/* swap items in the list stack */
	cleanUp(Int, Int),
	away(void);
Word	popList(void);		/* pop an item from the list stack's top */
Int	stackListLength(void),	/* return length of list at top of stack */
	isInternalSubr(Int),	/* 1 if symbol is internal subroutine */
	installExec(void),
	findSym(Int, hashTableEntry *[], Int),
	installSubsc(Int),
	addSubsc(Int, Int, Int), newSubrSymbol(Int),
	newSymbol(Int, ...), newBlockSymbol(Int), copySym(Int),
	anaerror(char *, Int, ...);
Int	statementDepth = 0, keepEVB = 0;
Int	yyerror(char *), yylex(YYSTYPE *);

/* Line 336 of yacc.c  */
#line 122 "anaparser.c"

# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "".  */
#ifndef YY_
# define YY_
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

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
#define SMOD 1050
#define UMINUS 1051



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_  */

/* Copy the second part of user declarations.  */

/* Line 353 of yacc.c  */
#line 295 "anaparser.c"

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
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
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
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
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
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
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
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  53
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1319

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  76
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  65
/* YYNRULES -- Number of rules.  */
#define YYNRULES  183
/* YYNRULES -- Number of states.  */
#define YYNSTATES  299

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   1051

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,    64,     2,    63,    45,     2,
      68,    72,    60,    58,    69,    59,     2,    61,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    73,     2,
      56,    41,    57,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    74,    62,    75,    67,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    70,    46,    71,     2,     2,     2,     2,
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
      44,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      65,    66
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     8,    10,    12,    14,    16,    18,
      20,    22,    25,    26,    31,    33,    35,    37,    39,    41,
      45,    48,    50,    52,    53,    56,    58,    60,    62,    64,
      68,    72,    74,    77,    81,    85,    87,    92,    97,   100,
     103,   105,   107,   109,   111,   115,   119,   121,   123,   125,
     127,   131,   134,   137,   141,   145,   149,   153,   156,   160,
     164,   168,   172,   176,   180,   184,   188,   192,   196,   200,
     204,   208,   212,   216,   220,   224,   228,   232,   236,   240,
     244,   248,   252,   256,   260,   264,   268,   271,   274,   276,
     280,   284,   290,   294,   300,   308,   314,   318,   320,   322,
     326,   331,   333,   336,   341,   345,   347,   349,   351,   353,
     357,   359,   361,   363,   365,   368,   371,   372,   374,   378,
     380,   384,   387,   389,   391,   392,   393,   401,   402,   403,
     411,   412,   413,   420,   422,   424,   426,   428,   430,   432,
     434,   438,   439,   442,   444,   448,   450,   453,   456,   460,
     462,   463,   464,   472,   473,   479,   480,   487,   488,   490,
     491,   494,   495,   496,   500,   501,   506,   510,   515,   516,
     517,   529,   530,   531,   538,   539,   540,   547,   548,   549,
     556,   557,   560,   561
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      77,     0,    -1,    78,    -1,    77,    78,    -1,    79,    -1,
       3,    -1,    84,    -1,    98,    -1,   104,    -1,   119,    -1,
     130,    -1,    27,    81,    -1,    -1,    82,    80,   117,    83,
      -1,    28,    -1,    29,    -1,     8,    -1,     9,    -1,    38,
      -1,    30,    69,    88,    -1,    30,    88,    -1,     1,    -1,
      40,    -1,    -1,    69,    91,    -1,    31,    -1,    70,    -1,
      32,    -1,    71,    -1,    87,    41,    91,    -1,    87,    97,
      91,    -1,    39,    -1,    85,    39,    -1,    68,    96,    72,
      -1,    68,    96,     1,    -1,    85,    -1,    86,    68,    96,
      72,    -1,    86,    68,    96,     1,    -1,    86,    85,    -1,
      88,    86,    -1,    88,    -1,     4,    -1,     5,    -1,    90,
      -1,    89,    69,    90,    -1,     4,    73,    91,    -1,    91,
      -1,     6,    -1,     7,    -1,    87,    -1,    88,    68,    72,
      -1,    91,    86,    -1,    45,    88,    -1,    68,    93,    72,
      -1,    68,    93,     1,    -1,    70,    89,    71,    -1,    70,
      89,     1,    -1,    70,    71,    -1,    68,    91,    72,    -1,
      68,    91,     1,    -1,    74,    92,    75,    -1,    74,    92,
       1,    -1,    91,    58,    91,    -1,    91,    59,    91,    -1,
      91,    60,    91,    -1,    91,    61,    91,    -1,    91,    62,
      91,    -1,    91,    63,    91,    -1,    91,    65,    91,    -1,
      91,    67,    91,    -1,    91,    57,    91,    -1,    91,    56,
      91,    -1,    91,    51,    91,    -1,    91,    50,    91,    -1,
      91,    55,    91,    -1,    91,    53,    91,    -1,    91,    52,
      91,    -1,    91,    54,    91,    -1,    91,    49,    91,    -1,
      91,    48,    91,    -1,    91,    47,    91,    -1,    91,    45,
      91,    -1,    91,    46,    91,    -1,    91,    44,    91,    -1,
      91,    43,    91,    -1,    91,    64,    91,    -1,    59,    91,
      -1,    58,    91,    -1,    91,    -1,    92,    69,    91,    -1,
      91,    73,    91,    -1,    91,    73,    60,    59,    91,    -1,
      91,    73,    60,    -1,    60,    59,    91,    73,    91,    -1,
      60,    59,    91,    73,    60,    59,    91,    -1,    60,    59,
      91,    73,    60,    -1,    60,    59,    91,    -1,    60,    -1,
      93,    -1,    93,    73,    58,    -1,    93,    73,    57,    91,
      -1,    58,    -1,    57,    91,    -1,    91,    73,    57,    91,
      -1,    91,    73,    58,    -1,    94,    -1,    91,    -1,   102,
      -1,    95,    -1,    96,    69,    95,    -1,    33,    -1,    34,
      -1,    35,    -1,    36,    -1,     4,    99,    -1,    69,   100,
      -1,    -1,   103,    -1,   100,    69,   103,    -1,     4,    -1,
     101,    41,    91,    -1,    61,   101,    -1,    91,    -1,   102,
      -1,    -1,    -1,    21,   105,     4,   118,   106,   117,   111,
      -1,    -1,    -1,    23,   107,     4,   118,   108,   117,   112,
      -1,    -1,    -1,    25,   109,     4,   110,   117,   113,    -1,
      22,    -1,    32,    -1,    24,    -1,    32,    -1,    26,    -1,
      32,    -1,   116,    -1,   116,    69,    42,    -1,    -1,    69,
     114,    -1,     4,    -1,   116,    69,     4,    -1,    79,    -1,
     117,    79,    -1,    68,    72,    -1,    68,   114,    72,    -1,
     115,    -1,    -1,    -1,    10,   120,    91,   124,   121,    79,
     125,    -1,    -1,    18,   122,   129,   126,    19,    -1,    -1,
      20,   123,    91,   117,   126,    19,    -1,    -1,    11,    -1,
      -1,    12,    79,    -1,    -1,    -1,    12,   127,    79,    -1,
      -1,    12,   128,    73,    79,    -1,    91,    73,    79,    -1,
     129,    91,    73,    79,    -1,    -1,    -1,    13,   131,     4,
      41,    91,    69,    91,   139,   140,   132,    79,    -1,    -1,
      -1,    14,   133,    79,    15,   134,    91,    -1,    -1,    -1,
      17,   135,    79,    16,   136,    91,    -1,    -1,    -1,    16,
     137,    91,   138,   140,    79,    -1,    -1,    69,    91,    -1,
      -1,    17,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   111,   111,   122,   133,   155,   165,   166,   167,   168,
     169,   170,   173,   173,   185,   186,   187,   190,   193,   194,
     195,   196,   201,   205,   206,   210,   210,   214,   214,   218,
     221,   232,   233,   238,   239,   246,   247,   248,   255,   260,
     265,   271,   272,   276,   283,   287,   291,   298,   299,   302,
     303,   308,   313,   316,   319,   325,   328,   334,   338,   341,
     347,   350,   359,   362,   365,   368,   371,   374,   377,   380,
     383,   386,   389,   392,   395,   398,   401,   404,   407,   410,
     413,   416,   419,   422,   425,   428,   433,   437,   443,   444,
     454,   457,   460,   463,   466,   469,   472,   475,   486,   488,
     493,   498,   503,   508,   513,   521,   523,   525,   530,   534,
     541,   545,   549,   553,   560,   567,   568,   575,   579,   586,
     592,   596,   603,   605,   610,   612,   610,   620,   622,   620,
     630,   632,   630,   642,   642,   646,   646,   650,   650,   655,
     657,   665,   669,   674,   678,   685,   690,   700,   704,   706,
     711,   713,   711,   719,   719,   727,   727,   738,   741,   746,
     750,   757,   761,   761,   768,   768,   778,   783,   791,   793,
     791,   800,   802,   800,   808,   810,   808,   816,   818,   816,
     827,   831,   836,   840
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NEWLINE", "C_ID", "S_ID", "NUMBER",
  "STR", "INCLUDE", "REPORT", "IF", "THEN", "ELSE", "FOR", "REPEAT",
  "UNTIL", "WHILE", "DO", "CASE", "ENDCASE", "NCASE", "SUBR", "ENDSUBR",
  "FUNC", "ENDFUNC", "BLOCK", "ENDBLOCK", "RETURN", "BREAK", "CONTINUE",
  "RUN", "BEGIN", "END", "PLUSIS", "MINUSIS", "TIMESIS", "DIVIDEIS",
  "POWERIS", "RETALL", "STRUCTTAG", "ERRORSTATE", "'='", "ELLIPSIS",
  "ORIF", "ANDIF", "'&'", "'|'", "XOR", "OR", "AND", "NE", "EQ", "LT",
  "GT", "LE", "GE", "'<'", "'>'", "'+'", "'-'", "'*'", "'/'", "'\\\\'",
  "'%'", "'#'", "SMOD", "UMINUS", "'^'", "'('", "','", "'{'", "'}'", "')'",
  "':'", "'['", "']'", "$accept", "lines", "next_line", "statement", "$@1",
  "opt_arg", "begingroup", "endgroup", "assignment", "tag_list",
  "member_spec", "lhs", "var", "struct_list", "struct_element", "expr",
  "expr_list", "range", "subsc", "subsc_or_key", "subsc_list", "op_assign",
  "routine_execution", "s_arglist", "arglist", "key_param", "key", "arg",
  "routine_definition", "$@2", "@3", "$@4", "@5", "$@6", "@7", "endsubr",
  "endfunc", "endblock", "paramlist2", "s_paramlist", "paramlist",
  "statement_list", "f_paramlist", "selection", "$@8", "$@9", "$@10",
  "$@11", "opt_then", "opt_else", "opt_case_else", "$@12", "$@13",
  "case_list", "loop", "$@14", "$@15", "$@16", "$@17", "$@18", "$@19",
  "$@20", "$@21", "opt_step", "opt_do", YY_NULL
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
    1037,    61,  1038,  1039,  1040,    38,   124,  1041,  1042,  1043,
    1044,  1045,  1046,  1047,  1048,  1049,    60,    62,    43,    45,
      42,    47,    92,    37,    35,  1050,  1051,    94,    40,    44,
     123,   125,    41,    58,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    76,    77,    77,    78,    78,    79,    79,    79,    79,
      79,    79,    80,    79,    79,    79,    79,    79,    79,    79,
      79,    79,    79,    81,    81,    82,    82,    83,    83,    84,
      84,    85,    85,    86,    86,    86,    86,    86,    86,    87,
      87,    88,    88,    89,    89,    90,    90,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    91,    91,    92,    92,
      93,    93,    93,    93,    93,    93,    93,    93,    94,    94,
      94,    94,    94,    94,    94,    95,    95,    95,    96,    96,
      97,    97,    97,    97,    98,    99,    99,   100,   100,   101,
     102,   102,   103,   103,   105,   106,   104,   107,   108,   104,
     109,   110,   104,   111,   111,   112,   112,   113,   113,   114,
     114,   115,   115,   116,   116,   117,   117,   118,   118,   118,
     120,   121,   119,   122,   119,   123,   119,   124,   124,   125,
     125,   126,   127,   126,   128,   126,   129,   129,   131,   132,
     130,   133,   134,   130,   135,   136,   130,   137,   138,   130,
     139,   139,   140,   140
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     1,     1,     1,     1,     1,
       1,     2,     0,     4,     1,     1,     1,     1,     1,     3,
       2,     1,     1,     0,     2,     1,     1,     1,     1,     3,
       3,     1,     2,     3,     3,     1,     4,     4,     2,     2,
       1,     1,     1,     1,     3,     3,     1,     1,     1,     1,
       3,     2,     2,     3,     3,     3,     3,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     2,     1,     3,
       3,     5,     3,     5,     7,     5,     3,     1,     1,     3,
       4,     1,     2,     4,     3,     1,     1,     1,     1,     3,
       1,     1,     1,     1,     2,     2,     0,     1,     3,     1,
       3,     2,     1,     1,     0,     0,     7,     0,     0,     7,
       0,     0,     6,     1,     1,     1,     1,     1,     1,     1,
       3,     0,     2,     1,     3,     1,     2,     2,     3,     1,
       0,     0,     7,     0,     5,     0,     6,     0,     1,     0,
       2,     0,     0,     3,     0,     4,     3,     4,     0,     0,
      11,     0,     0,     6,     0,     0,     6,     0,     0,     6,
       0,     2,     0,     1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    21,     5,   116,    42,    16,    17,   150,   168,   171,
     177,   174,   153,   155,   124,   127,   130,    23,    14,    15,
       0,    25,    18,    22,    26,     0,     2,     4,    12,     6,
       0,    40,     7,     8,     9,    10,     0,   114,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      41,     0,    20,     1,     3,     0,   110,   111,   112,   113,
       0,     0,    31,     0,    35,    39,    41,    47,    48,     0,
       0,     0,     0,     0,     0,     0,    49,    40,   122,   115,
       0,   123,   117,   157,     0,     0,   178,     0,     0,   161,
       0,   141,   141,   131,    24,    19,   145,     0,    29,    30,
       0,   101,    97,   106,    98,   105,   108,     0,   107,    32,
       0,    38,    52,    87,    86,   119,   121,     0,     0,    41,
      57,     0,    43,    46,    88,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    51,     0,     0,   158,   151,     0,   172,   182,   175,
       0,   162,     0,     0,     0,     0,     0,   149,   125,   128,
       0,    27,    28,   146,    13,   102,     0,     0,     0,    34,
       0,    33,     0,    59,    58,     0,    54,    53,     0,    56,
       0,    55,    61,     0,    60,    50,    84,    83,    81,    82,
      80,    79,    78,    73,    72,    76,    75,    77,    74,    71,
      70,    62,    63,    64,    65,    66,    67,    85,    68,    69,
     118,   120,     0,     0,     0,   183,     0,     0,   166,     0,
       0,     0,   154,     0,   143,   147,     0,   139,   142,     0,
       0,     0,    96,     0,   104,    92,    90,     0,    99,   109,
      37,    36,    45,    44,    89,   159,     0,   173,   179,   176,
     163,     0,   167,   156,   148,     0,     0,     0,   137,   138,
     132,     0,   103,     0,   100,     0,   152,   180,   165,   144,
     140,   133,   134,   126,   135,   136,   129,    95,    93,    91,
     160,     0,   182,     0,   181,   169,    94,     0,   170
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    25,    26,    96,    55,    49,    28,   174,    29,    64,
     151,    76,    77,   121,   122,   103,   125,   104,   105,   106,
     107,    61,    32,    37,    79,    80,   108,    82,    33,    45,
     239,    46,   240,    47,   170,   283,   286,   270,   236,   167,
     237,    97,   168,    34,    38,   222,    43,    44,   155,   276,
     163,   229,   230,    89,    35,    39,   297,    40,   224,    42,
     227,    41,   158,   292,   226
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -144
static const yytype_int16 yypact[] =
{
     920,  -144,  -144,    -7,  -144,  -144,  -144,  -144,  -144,  -144,
    -144,  -144,  -144,  -144,  -144,  -144,  -144,   -56,  -144,  -144,
       6,  -144,  -144,  -144,  -144,   107,  -144,  -144,  -144,  -144,
     148,   -22,  -144,  -144,  -144,  -144,   190,  -144,   586,    20,
     958,   586,   958,   586,   586,    29,    32,    35,   586,  -144,
    -144,    18,  -144,  -144,  -144,   958,  -144,  -144,  -144,  -144,
     586,   586,  -144,   288,    -4,   -20,   -26,  -144,  -144,    18,
     586,   586,    45,   315,   488,   586,  -144,   -18,  1170,   -15,
      19,  -144,  -144,   962,    22,    42,  1170,    48,   992,   536,
     680,   -38,   -38,  -144,  1170,  -144,  -144,   496,  1170,  1170,
     586,   586,   -12,  1023,   -14,  -144,  -144,     4,  -144,  -144,
     288,    -4,  -144,   -30,   -30,  -144,  -144,   417,     7,    -8,
    -144,    11,  -144,  1170,  1170,     3,    97,   586,   586,   586,
     586,   586,   586,   586,   586,   586,   586,   586,   586,   586,
     586,   586,   586,   586,   586,   586,   586,   586,   586,   586,
     586,   -20,   190,   586,  -144,  -144,   586,  -144,    49,  -144,
     958,    -6,  1054,    65,   748,    14,    83,  -144,  -144,  -144,
     958,  -144,  -144,  -144,  -144,  1170,   586,   310,   -13,  -144,
     288,  -144,     5,  -144,  -144,   564,  -144,  -144,   586,  -144,
     612,  -144,  -144,   586,  -144,  -144,  1197,  1197,  1224,  1224,
    1224,  1224,  1224,   152,   152,   152,   152,   152,   152,  1251,
    1251,    31,    31,   -30,   -30,   -30,   -30,   -30,   -30,   -30,
    -144,  1170,   958,  1116,   586,  -144,   958,   586,  -144,   958,
      15,   958,  -144,    81,  -144,  -144,    33,    40,  -144,   958,
     958,   794,  1085,   586,   586,    55,  1170,   586,  -144,  -144,
    -144,  -144,  1170,  -144,  1170,   110,   586,  1170,  -144,  1170,
    -144,   958,  -144,  -144,  -144,    10,   838,   882,  -144,  -144,
    -144,   569,  1170,   586,  1170,   958,  -144,  1143,  -144,  -144,
    -144,  -144,  -144,  -144,  -144,  -144,  -144,    60,  1170,  1170,
    -144,   586,    49,   586,  1170,  -144,  1170,   958,  -144
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -144,  -144,   101,    43,  -144,  -144,  -144,  -144,  -144,   -62,
     -24,    16,     0,  -144,   -61,   264,  -144,    58,  -144,   -47,
      34,  -144,  -144,  -144,  -144,    67,   -34,   -11,  -144,  -144,
    -144,  -144,  -144,  -144,  -144,  -144,  -144,  -144,   -23,  -144,
    -144,   -89,    54,  -144,  -144,  -144,  -144,  -144,  -144,  -144,
     -16,  -144,  -144,  -144,  -144,  -144,  -144,  -144,  -144,  -144,
    -144,  -144,  -144,  -144,  -143
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -165
static const yytype_int16 yytable[] =
{
      31,   164,    81,   111,   192,   179,   250,    65,   186,    62,
      50,     4,   189,    48,   279,  -119,    30,    62,   234,    62,
      52,    62,    50,     4,    84,    31,   -41,   -41,   -41,   -41,
     165,   166,   -41,    91,   -41,   109,    92,   150,    63,    93,
      31,    30,    31,    27,   247,   248,    63,   176,   110,   115,
     126,    95,   280,    65,   152,    31,    30,   157,    30,   178,
     153,   -41,    36,   156,   159,   188,   225,  -164,    27,   112,
      62,    30,   193,   180,   180,    51,   181,   251,   194,   187,
     190,   241,   191,    85,   232,    87,   235,   234,   261,   111,
      31,   144,   145,   146,   147,   148,   149,    31,   150,    63,
     263,    66,     4,    67,    68,   264,    30,    53,     1,   265,
       2,     3,     4,    30,   273,     5,     6,     7,    81,   293,
       8,     9,   275,    10,    11,    12,    54,    13,    14,   253,
      15,   118,    16,   249,    17,    18,    19,    20,    21,   116,
     173,   220,    69,   238,   182,    22,   169,    23,   233,   295,
     266,   267,     0,     0,   100,   101,    71,   102,    72,     0,
      31,     0,     0,     0,    31,    73,     0,    74,     0,   195,
      31,    75,     0,     0,     0,     0,    30,    24,     0,     0,
      30,    56,    57,    58,    59,     0,    30,     0,     0,    60,
       0,    62,     0,     0,    66,     4,    67,    68,     0,     0,
       0,     0,     0,   228,     0,     0,     0,   173,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,     0,   150,
      63,     0,    31,     0,     0,     0,    31,     0,     0,    31,
       0,    31,     0,     0,     0,    69,     0,     0,    30,    31,
      31,    31,    30,     0,     0,    30,     0,    30,    70,    71,
       0,    72,     0,     0,     0,    30,    30,    30,    73,     0,
      74,    31,     0,     0,    75,   255,    31,    31,     0,   258,
       0,     0,   260,     0,   262,    31,     0,    30,     0,     0,
       0,     0,    30,    30,   173,     0,     0,     0,     0,     0,
       0,    30,    66,     4,    67,    68,     0,    31,     0,     0,
      78,     0,    83,     0,   278,    86,     0,    88,    90,   173,
     173,     0,    94,    30,    50,     4,    67,    68,   290,    50,
       4,    67,    68,     0,    98,    99,     0,     0,     0,     0,
       0,     0,     0,    69,   113,   114,     0,   117,   123,   124,
     298,     0,     0,     0,     0,   100,   101,    71,   102,    72,
       0,     0,     0,   162,     0,    69,    73,     0,    74,     0,
      69,     0,    75,     0,   175,   113,     0,   243,   244,    71,
     245,     0,     0,    70,    71,   102,     0,     0,    73,     0,
      74,     0,     0,    73,    75,    74,     0,     0,     0,    75,
       0,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,     0,    78,   221,   183,     0,
     223,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     242,   246,     0,     0,     0,     0,     0,     0,     0,   246,
       0,     0,   252,     0,   123,     0,    62,   254,     0,     0,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,     0,   150,    63,     0,     0,   257,   184,
     185,   259,   119,     4,    67,    68,     0,     1,     0,     0,
       3,     4,     0,     0,     5,     6,     7,   272,   113,     8,
       9,   274,    10,    11,    12,     0,    13,    14,     0,    15,
     277,    16,     0,    17,    18,    19,    20,    21,   171,     0,
       0,     0,     0,    69,    22,   288,    23,   289,     0,     0,
      50,     4,    67,    68,     0,     0,    70,    71,   161,     0,
       0,     0,     0,     0,     0,   294,    73,   296,    74,   120,
       0,     0,    75,     0,     0,     0,    24,   172,    50,     4,
      67,    68,     0,    50,     4,    67,    68,     0,     0,     0,
       0,    69,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     4,    67,    68,    70,    71,     0,     0,     0,     0,
       0,     0,     0,     0,    73,     0,    74,     0,     0,    69,
      75,     0,     0,     0,    69,     0,   119,     4,    67,    68,
       0,     0,    70,    71,   245,     0,     0,    70,    71,   287,
       0,    69,    73,     0,    74,     0,     0,    73,    75,    74,
       0,     0,     0,    75,    70,    71,     0,     0,     0,     0,
       0,     0,     0,     0,    73,     0,    74,    69,     0,     0,
      75,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      70,    71,     0,     0,     0,     0,     0,     0,     0,     0,
      73,     1,    74,     0,     3,     4,    75,     0,     5,     6,
       7,     0,     0,     8,     9,     0,    10,    11,    12,     0,
      13,    14,     0,    15,     0,    16,     0,    17,    18,    19,
      20,    21,     0,     0,     0,     0,     0,     0,    22,    62,
      23,     0,     0,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,     0,   150,    63,     1,
      24,     0,     3,     4,     0,     0,     5,     6,     7,     0,
     161,     8,     9,     0,    10,    11,    12,  -161,    13,    14,
       0,    15,     0,    16,     0,    17,    18,    19,    20,    21,
       0,     0,     0,     0,     0,     0,    22,     0,    23,     0,
       0,     0,     0,     0,     0,     1,     0,     0,     3,     4,
       0,     0,     5,     6,     7,     0,     0,     8,     9,     0,
      10,    11,    12,     0,    13,    14,     0,    15,    24,    16,
     268,    17,    18,    19,    20,    21,   269,     0,     0,     0,
       0,     0,    22,     0,    23,     0,     0,     0,     0,     1,
       0,     0,     3,     4,     0,     0,     5,     6,     7,     0,
       0,     8,     9,     0,    10,    11,    12,     0,    13,    14,
     281,    15,     0,    16,    24,    17,    18,    19,    20,    21,
     282,     0,     0,     0,     0,     0,    22,     0,    23,     0,
       0,     0,     0,     1,     0,     0,     3,     4,     0,     0,
       5,     6,     7,     0,     0,     8,     9,     0,    10,    11,
      12,     0,    13,    14,     0,    15,   284,    16,    24,    17,
      18,    19,    20,    21,   285,     0,     0,     0,     0,     0,
      22,     1,    23,     2,     3,     4,     0,     0,     5,     6,
       7,     0,     0,     8,     9,     0,    10,    11,    12,     0,
      13,    14,     0,    15,     0,    16,     0,    17,    18,    19,
      20,    21,    24,     0,     0,     0,     0,     0,    22,     1,
      23,     0,     3,     4,     0,     0,     5,     6,     7,     0,
       0,     8,     9,   154,    10,    11,    12,     0,    13,    14,
       0,    15,     0,    16,     0,    17,    18,    19,    20,    21,
      24,     0,     0,     0,     0,     0,    22,     0,    23,     0,
       0,    62,     0,     0,     0,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,    24,   150,
      63,    62,     0,     0,     0,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,     0,   150,
      63,     0,    62,     0,     0,   160,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,     0,
     150,    63,     0,    62,     0,     0,   177,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
       0,   150,    63,     0,    62,     0,     0,   231,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,     0,   150,    63,     0,    62,     0,     0,   271,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,    62,   150,    63,   256,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,    62,
     150,    63,   291,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,    62,   150,    63,     0,
       0,     0,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,    62,   150,    63,     0,     0,     0,     0,
       0,     0,     0,     0,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
      62,   150,    63,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   142,
     143,   144,   145,   146,   147,   148,   149,     0,   150,    63
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-144))

#define yytable_value_is_error(yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
       0,    90,    36,    65,     1,     1,     1,    31,     1,    39,
       4,     5,     1,    69,     4,    41,     0,    39,     4,    39,
      20,    39,     4,     5,     4,    25,    33,    34,    35,    36,
      68,    69,    39,     4,    41,    39,     4,    67,    68,     4,
      40,    25,    42,     0,    57,    58,    68,    59,    68,     4,
      68,    51,    42,    77,    69,    55,    40,    15,    42,    73,
      41,    68,    69,    41,    16,    73,    17,    73,    25,    69,
      39,    55,    69,    69,    69,    69,    72,    72,    75,    72,
      69,   170,    71,    40,    19,    42,    72,     4,    73,   151,
      90,    60,    61,    62,    63,    64,    65,    97,    67,    68,
      19,     4,     5,     6,     7,    72,    90,     0,     1,    69,
       3,     4,     5,    97,    59,     8,     9,    10,   152,    59,
      13,    14,    12,    16,    17,    18,    25,    20,    21,   190,
      23,    73,    25,   180,    27,    28,    29,    30,    31,    72,
      97,   152,    45,   166,   110,    38,    92,    40,   164,   292,
     239,   240,    -1,    -1,    57,    58,    59,    60,    61,    -1,
     160,    -1,    -1,    -1,   164,    68,    -1,    70,    -1,    72,
     170,    74,    -1,    -1,    -1,    -1,   160,    70,    -1,    -1,
     164,    33,    34,    35,    36,    -1,   170,    -1,    -1,    41,
      -1,    39,    -1,    -1,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,   160,    -1,    -1,    -1,   164,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    67,
      68,    -1,   222,    -1,    -1,    -1,   226,    -1,    -1,   229,
      -1,   231,    -1,    -1,    -1,    45,    -1,    -1,   222,   239,
     240,   241,   226,    -1,    -1,   229,    -1,   231,    58,    59,
      -1,    61,    -1,    -1,    -1,   239,   240,   241,    68,    -1,
      70,   261,    -1,    -1,    74,   222,   266,   267,    -1,   226,
      -1,    -1,   229,    -1,   231,   275,    -1,   261,    -1,    -1,
      -1,    -1,   266,   267,   241,    -1,    -1,    -1,    -1,    -1,
      -1,   275,     4,     5,     6,     7,    -1,   297,    -1,    -1,
      36,    -1,    38,    -1,   261,    41,    -1,    43,    44,   266,
     267,    -1,    48,   297,     4,     5,     6,     7,   275,     4,
       5,     6,     7,    -1,    60,    61,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    70,    71,    -1,    73,    74,    75,
     297,    -1,    -1,    -1,    -1,    57,    58,    59,    60,    61,
      -1,    -1,    -1,    89,    -1,    45,    68,    -1,    70,    -1,
      45,    -1,    74,    -1,   100,   101,    -1,    57,    58,    59,
      60,    -1,    -1,    58,    59,    60,    -1,    -1,    68,    -1,
      70,    -1,    -1,    68,    74,    70,    -1,    -1,    -1,    74,
      -1,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,    -1,   152,   153,     1,    -1,
     156,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     176,   177,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   185,
      -1,    -1,   188,    -1,   190,    -1,    39,   193,    -1,    -1,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    67,    68,    -1,    -1,   224,    72,
      73,   227,     4,     5,     6,     7,    -1,     1,    -1,    -1,
       4,     5,    -1,    -1,     8,     9,    10,   243,   244,    13,
      14,   247,    16,    17,    18,    -1,    20,    21,    -1,    23,
     256,    25,    -1,    27,    28,    29,    30,    31,    32,    -1,
      -1,    -1,    -1,    45,    38,   271,    40,   273,    -1,    -1,
       4,     5,     6,     7,    -1,    -1,    58,    59,    12,    -1,
      -1,    -1,    -1,    -1,    -1,   291,    68,   293,    70,    71,
      -1,    -1,    74,    -1,    -1,    -1,    70,    71,     4,     5,
       6,     7,    -1,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       4,     5,     6,     7,    58,    59,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    -1,    70,    -1,    -1,    45,
      74,    -1,    -1,    -1,    45,    -1,     4,     5,     6,     7,
      -1,    -1,    58,    59,    60,    -1,    -1,    58,    59,    60,
      -1,    45,    68,    -1,    70,    -1,    -1,    68,    74,    70,
      -1,    -1,    -1,    74,    58,    59,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    -1,    70,    45,    -1,    -1,
      74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      58,    59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,     1,    70,    -1,     4,     5,    74,    -1,     8,     9,
      10,    -1,    -1,    13,    14,    -1,    16,    17,    18,    -1,
      20,    21,    -1,    23,    -1,    25,    -1,    27,    28,    29,
      30,    31,    -1,    -1,    -1,    -1,    -1,    -1,    38,    39,
      40,    -1,    -1,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    67,    68,     1,
      70,    -1,     4,     5,    -1,    -1,     8,     9,    10,    -1,
      12,    13,    14,    -1,    16,    17,    18,    19,    20,    21,
      -1,    23,    -1,    25,    -1,    27,    28,    29,    30,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,    40,    -1,
      -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,     4,     5,
      -1,    -1,     8,     9,    10,    -1,    -1,    13,    14,    -1,
      16,    17,    18,    -1,    20,    21,    -1,    23,    70,    25,
      26,    27,    28,    29,    30,    31,    32,    -1,    -1,    -1,
      -1,    -1,    38,    -1,    40,    -1,    -1,    -1,    -1,     1,
      -1,    -1,     4,     5,    -1,    -1,     8,     9,    10,    -1,
      -1,    13,    14,    -1,    16,    17,    18,    -1,    20,    21,
      22,    23,    -1,    25,    70,    27,    28,    29,    30,    31,
      32,    -1,    -1,    -1,    -1,    -1,    38,    -1,    40,    -1,
      -1,    -1,    -1,     1,    -1,    -1,     4,     5,    -1,    -1,
       8,     9,    10,    -1,    -1,    13,    14,    -1,    16,    17,
      18,    -1,    20,    21,    -1,    23,    24,    25,    70,    27,
      28,    29,    30,    31,    32,    -1,    -1,    -1,    -1,    -1,
      38,     1,    40,     3,     4,     5,    -1,    -1,     8,     9,
      10,    -1,    -1,    13,    14,    -1,    16,    17,    18,    -1,
      20,    21,    -1,    23,    -1,    25,    -1,    27,    28,    29,
      30,    31,    70,    -1,    -1,    -1,    -1,    -1,    38,     1,
      40,    -1,     4,     5,    -1,    -1,     8,     9,    10,    -1,
      -1,    13,    14,    11,    16,    17,    18,    -1,    20,    21,
      -1,    23,    -1,    25,    -1,    27,    28,    29,    30,    31,
      70,    -1,    -1,    -1,    -1,    -1,    38,    -1,    40,    -1,
      -1,    39,    -1,    -1,    -1,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    70,    67,
      68,    39,    -1,    -1,    -1,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    67,
      68,    -1,    39,    -1,    -1,    73,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      67,    68,    -1,    39,    -1,    -1,    73,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    67,    68,    -1,    39,    -1,    -1,    73,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    67,    68,    -1,    39,    -1,    -1,    73,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    39,    67,    68,    69,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    39,
      67,    68,    69,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    39,    67,    68,    -1,
      -1,    -1,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    39,    67,    68,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      39,    67,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    67,    68
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     3,     4,     5,     8,     9,    10,    13,    14,
      16,    17,    18,    20,    21,    23,    25,    27,    28,    29,
      30,    31,    38,    40,    70,    77,    78,    79,    82,    84,
      87,    88,    98,   104,   119,   130,    69,    99,   120,   131,
     133,   137,   135,   122,   123,   105,   107,   109,    69,    81,
       4,    69,    88,     0,    78,    80,    33,    34,    35,    36,
      41,    97,    39,    68,    85,    86,     4,     6,     7,    45,
      58,    59,    61,    68,    70,    74,    87,    88,    91,   100,
     101,   102,   103,    91,     4,    79,    91,    79,    91,   129,
      91,     4,     4,     4,    91,    88,    79,   117,    91,    91,
      57,    58,    60,    91,    93,    94,    95,    96,   102,    39,
      68,    85,    88,    91,    91,     4,   101,    91,    93,     4,
      71,    89,    90,    91,    91,    92,    68,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      67,    86,    69,    41,    11,   124,    41,    15,   138,    16,
      73,    12,    91,   126,   117,    68,    69,   115,   118,   118,
     110,    32,    71,    79,    83,    91,    59,    73,    73,     1,
      69,    72,    96,     1,    72,    73,     1,    72,    73,     1,
      69,    71,     1,    69,    75,    72,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    91,    91,    91,    91,
     103,    91,   121,    91,   134,    17,   140,   136,    79,   127,
     128,    73,    19,   126,     4,    72,   114,   116,   114,   106,
     108,   117,    91,    57,    58,    60,    91,    57,    58,    95,
       1,    72,    91,    90,    91,    79,    69,    91,    79,    91,
      79,    73,    79,    19,    72,    69,   117,   117,    26,    32,
     113,    73,    91,    59,    91,    12,   125,    91,    79,     4,
      42,    22,    32,   111,    24,    32,   112,    60,    91,    91,
      79,    69,   139,    59,    91,   140,    91,   132,    79
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
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
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
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (YYID (N))                                                     \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (YYID (0))
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])



/* This macro is provided for backward compatibility. */

#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
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
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
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
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
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
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
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
   heuristic is that double-quoting is unnecessary unless the string
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

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULL, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULL;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                yysize1 = yysize + yytnamerr (YY_NULL, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
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




/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

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
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

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

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
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

  /* Discard the shifted token.  */
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
        case 2:
/* Line 1787 of yacc.c  */
#line 111 "anaparser.y"
    {			/* a statement or a newline */
  if (compileOnly && (yyvsp[(1) - (1)]))	/* if we're just compiling (and not
				   immediately executing, and the statement
				   is not a newline or an error, then store
				   it at the start of a new list */
    startList((yyvsp[(1) - (1)]));
  /*  if (debugLine) */		/* if we're in debugging mode (dbg> prompt)
				   then we return after the first statement */
  /* YYACCEPT; */			/* return */
}
    break;

  case 3:
/* Line 1787 of yacc.c  */
#line 122 "anaparser.y"
    {		/* next member in a set of statements and
				   newlines */
  if (compileOnly && (yyvsp[(2) - (2)]))	/* if we're just compiling and the statement
				   is not a newline or error, then add to
				   the list */
    pushList((yyvsp[(2) - (2)]));
}
    break;

  case 4:
/* Line 1787 of yacc.c  */
#line 133 "anaparser.y"
    {			/* $1 > 0 indicates succesful execution.
				   $1 < 0 indicates an error or a premature
				   end to a loop structure (CONTINUE,
				   BREAK, RETURN) */
  if (!compileOnly) {		/* not just compiling */
    if ((yyvsp[(1) - (1)]) > 0) {		/* statement OK */
      (yyvsp[(1) - (1)]) = execute((yyvsp[(1) - (1)]));		/* execute it */
    }
    cleanUp(-compileLevel, 0);	/* clean up temp storage and such */
    if ((yyvsp[(1) - (1)]) == LOOP_RETALL	/* RETALL statement */
	&& compileLevel) {	/* not at the main execution level */
      puts("RETALL - return control to user");
      away();			/* clean up aborted execution */
      YYABORT;
    }
  }
  if ((yyvsp[(1) - (1)]) < 0 && compileLevel) {	/* generic break or error condition */
    puts("Aborting");
    away();
    YYABORT;
  }
}
    break;

  case 5:
/* Line 1787 of yacc.c  */
#line 155 "anaparser.y"
    {	/* a newline; newlines are only passeed to the parser by
		   yylex() if disableNewline is equal to 0 */
    if (debugLine)		/* if this is a dbg> line then we quit after
				   the first line */
      YYACCEPT;
    (yyval) = 0;			/* else we ignore it */
}
    break;

  case 11:
/* Line 1787 of yacc.c  */
#line 170 "anaparser.y"
    {		/* a RETURN statement */
  (yyval) = newSymbol(ANA_EVB, EVB_RETURN, (yyvsp[(2) - (2)]));
}
    break;

  case 12:
/* Line 1787 of yacc.c  */
#line 173 "anaparser.y"
    { disableNewline++; }
    break;

  case 13:
/* Line 1787 of yacc.c  */
#line 173 "anaparser.y"
    {
  /* a statement block */
  /* after the initial {, more input is needed to complete the statement.
     the disableNewline++ statement ensures that no newlines are recognized
     while the statement block is assembled. */
  if ((yyvsp[(3) - (4)]) >= 0)			/* statement list is OK */
    (yyval) = newSymbol(ANA_EVB, EVB_BLOCK);
  else				/* statement list had some error */
    (yyval) = ANA_ERROR;
  statementDepth--;		/* was incremented by statement_list */
  disableNewline--;		/* back to initial */
}
    break;

  case 14:
/* Line 1787 of yacc.c  */
#line 185 "anaparser.y"
    { (yyval) = LOOP_BREAK; }
    break;

  case 15:
/* Line 1787 of yacc.c  */
#line 186 "anaparser.y"
    { (yyval) = LOOP_CONTINUE; }
    break;

  case 16:
/* Line 1787 of yacc.c  */
#line 187 "anaparser.y"
    {			/* a @file statement */
  (yyval) = newSymbol(ANA_EVB, EVB_FILE, (yyvsp[(1) - (1)]), FILE_INCLUDE);
}
    break;

  case 17:
/* Line 1787 of yacc.c  */
#line 190 "anaparser.y"
    {			/* a @@file statement */
  (yyval) = newSymbol(ANA_EVB, EVB_FILE, (yyvsp[(1) - (1)]), FILE_REPORT);
}
    break;

  case 18:
/* Line 1787 of yacc.c  */
#line 193 "anaparser.y"
    { (yyval) = LOOP_RETALL; }
    break;

  case 19:
/* Line 1787 of yacc.c  */
#line 194 "anaparser.y"
    { (yyval) = newBlockSymbol((yyvsp[(3) - (3)])); }
    break;

  case 20:
/* Line 1787 of yacc.c  */
#line 195 "anaparser.y"
    { (yyval) = newBlockSymbol((yyvsp[(2) - (2)])); }
    break;

  case 21:
/* Line 1787 of yacc.c  */
#line 196 "anaparser.y"
    {			/* some error */
  puts("Illegal statement");	/* generate message */
  errorState = 1;		/* signal the error */
  YYABORT;			/* quite this parse */
}
    break;

  case 22:
/* Line 1787 of yacc.c  */
#line 201 "anaparser.y"
    { YYABORT; }
    break;

  case 23:
/* Line 1787 of yacc.c  */
#line 205 "anaparser.y"
    { (yyval) = 0; }
    break;

  case 24:
/* Line 1787 of yacc.c  */
#line 206 "anaparser.y"
    { (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 29:
/* Line 1787 of yacc.c  */
#line 218 "anaparser.y"
    {		/* simple assignment */
  (yyval) = newSymbol(ANA_EVB, EVB_REPLACE, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
}
    break;

  case 30:
/* Line 1787 of yacc.c  */
#line 221 "anaparser.y"
    {		/* an operator-assignment (e.g. X += 2) */
  if (symbol_class((yyvsp[(1) - (3)])) == ANA_EXTRACT)
    (yyval) = newSymbol(ANA_EVB, EVB_REPLACE, copySym((yyvsp[(1) - (3)])),
		   newSymbol(ANA_BIN_OP, (yyvsp[(2) - (3)]), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])));
  else
    (yyval) = newSymbol(ANA_EVB, EVB_REPLACE, (yyvsp[(1) - (3)]),
		   newSymbol(ANA_BIN_OP, (yyvsp[(2) - (3)]), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])));
}
    break;

  case 31:
/* Line 1787 of yacc.c  */
#line 232 "anaparser.y"
    { startList((yyvsp[(1) - (1)])); }
    break;

  case 32:
/* Line 1787 of yacc.c  */
#line 233 "anaparser.y"
    { pushList((yyvsp[(2) - (2)])); }
    break;

  case 33:
/* Line 1787 of yacc.c  */
#line 238 "anaparser.y"
    { pushList(ANA_RANGE);  (yyval) = 1; }
    break;

  case 34:
/* Line 1787 of yacc.c  */
#line 239 "anaparser.y"
    {
  if ((setup & 1024) == 0)
    puts("Unbalanced ()");
  yyerrok;
  pushList(ANA_RANGE);
  (yyval) = 1;
}
    break;

  case 35:
/* Line 1787 of yacc.c  */
#line 246 "anaparser.y"
    { pushList(ANA_LIST);  (yyval) = 1; }
    break;

  case 36:
/* Line 1787 of yacc.c  */
#line 247 "anaparser.y"
    { pushList(ANA_RANGE);  (yyval) = (yyvsp[(1) - (4)]) + 1; }
    break;

  case 37:
/* Line 1787 of yacc.c  */
#line 248 "anaparser.y"
    {
  if ((setup & 1024) == 0)
    puts("Unbalanced ()");
  yyerrok;
  pushList(ANA_RANGE);
  (yyval) = (yyvsp[(1) - (4)]) + 1;
}
    break;

  case 38:
/* Line 1787 of yacc.c  */
#line 255 "anaparser.y"
    { pushList(ANA_LIST);  (yyval) = (yyvsp[(1) - (2)]) + 1; }
    break;

  case 39:
/* Line 1787 of yacc.c  */
#line 260 "anaparser.y"
    {
  pushList((yyvsp[(2) - (2)]));			/* the number of element extraction lists */
  pushList(-(yyvsp[(1) - (2)]));		/* minus indicates "var" */
  (yyval) = newSymbol(ANA_EXTRACT);
}
    break;

  case 40:
/* Line 1787 of yacc.c  */
#line 265 "anaparser.y"
    { (yyval) = findVar((yyvsp[(1) - (1)]), curContext); }
    break;

  case 43:
/* Line 1787 of yacc.c  */
#line 276 "anaparser.y"
    {
    pushList(ANA_NEW_LIST);	/* the stack contents is now:
				   key expr ANA_NEW_LIST */
    swapList(1, 2);		/* reverse stack contents to: */
    swapList(2, 3);		/* ANA_NEW_LIST key expr */
}
    break;

  case 45:
/* Line 1787 of yacc.c  */
#line 287 "anaparser.y"
    {
    pushList((yyvsp[(1) - (3)]));
    pushList((yyvsp[(3) - (3)]));
}
    break;

  case 46:
/* Line 1787 of yacc.c  */
#line 291 "anaparser.y"
    {
    pushList(-1);
    pushList((yyvsp[(1) - (1)]));
}
    break;

  case 48:
/* Line 1787 of yacc.c  */
#line 299 "anaparser.y"
    {				/* a string */
  (yyval) = newSymbol(ANA_FIXED_STRING, (yyvsp[(1) - (1)]));
}
    break;

  case 50:
/* Line 1787 of yacc.c  */
#line 303 "anaparser.y"
    {			/* a function call without any arguments */
  startList(0);			/* no arguments */
  pushList(-(yyvsp[(1) - (3)]));
  (yyval) = newSymbol(ANA_EXTRACT);
}
    break;

  case 51:
/* Line 1787 of yacc.c  */
#line 308 "anaparser.y"
    {	/* expressions may be subscripted */
  pushList((yyvsp[(2) - (2)]));			/* the number of element extraction lists */
  pushList((yyvsp[(1) - (2)]));			/* the expression */
  (yyval) = newSymbol(ANA_EXTRACT);
}
    break;

  case 52:
/* Line 1787 of yacc.c  */
#line 313 "anaparser.y"
    {		/* a variable or function/routine pointer */
  (yyval) = newSymbol(ANA_POINTER, (yyvsp[(2) - (2)]));
}
    break;

  case 53:
/* Line 1787 of yacc.c  */
#line 316 "anaparser.y"
    {		/* a range expression */
  (yyval) = (yyvsp[(2) - (3)]);
}
    break;

  case 54:
/* Line 1787 of yacc.c  */
#line 319 "anaparser.y"
    {
  if ((setup & 1024) == 0)
    puts("Unbalanced ()");
  yyerrok;
  (yyval) = (yyvsp[(2) - (3)]);
}
    break;

  case 55:
/* Line 1787 of yacc.c  */
#line 325 "anaparser.y"
    {		/* a structure */
  (yyval) = newSymbol(ANA_PRE_LIST);
}
    break;

  case 56:
/* Line 1787 of yacc.c  */
#line 328 "anaparser.y"
    {
  if ((setup & 1024) == 0)
    puts("Unbalanced {}");
  yyerrok;
  (yyval) = newSymbol(ANA_PRE_LIST);
}
    break;

  case 57:
/* Line 1787 of yacc.c  */
#line 334 "anaparser.y"
    {			/* an empty list */
  pushList(ANA_NEW_LIST);
  (yyval) = newSymbol(ANA_PRE_LIST);
}
    break;

  case 58:
/* Line 1787 of yacc.c  */
#line 338 "anaparser.y"
    {
  (yyval) = (yyvsp[(2) - (3)]);
}
    break;

  case 59:
/* Line 1787 of yacc.c  */
#line 341 "anaparser.y"
    {
  if ((setup & 1024) == 0)
    puts("Unbalanced ()");
  yyerrok;
  (yyval) = (yyvsp[(2) - (3)]);
}
    break;

  case 60:
/* Line 1787 of yacc.c  */
#line 347 "anaparser.y"
    {		/* concatenation */
  (yyval) = newSymbol(ANA_INT_FUNC, ANA_CONCAT_FUN);
}
    break;

  case 61:
/* Line 1787 of yacc.c  */
#line 350 "anaparser.y"
    {
  if ((setup & 1024) == 0)
    puts("Unbalanced []");
  (yyval) = newSymbol(ANA_INT_FUNC, ANA_CONCAT_FUN);
  yyerrok;
}
    break;

  case 62:
/* Line 1787 of yacc.c  */
#line 359 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_BIN_OP, ANA_ADD, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
}
    break;

  case 63:
/* Line 1787 of yacc.c  */
#line 362 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_BIN_OP, ANA_SUB, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
}
    break;

  case 64:
/* Line 1787 of yacc.c  */
#line 365 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_BIN_OP, ANA_MUL, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
}
    break;

  case 65:
/* Line 1787 of yacc.c  */
#line 368 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_BIN_OP, ANA_DIV, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
}
    break;

  case 66:
/* Line 1787 of yacc.c  */
#line 371 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_BIN_OP, ANA_IDIV, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
}
    break;

  case 67:
/* Line 1787 of yacc.c  */
#line 374 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_BIN_OP, ANA_MOD, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
}
    break;

  case 68:
/* Line 1787 of yacc.c  */
#line 377 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_BIN_OP, ANA_SMOD, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
}
    break;

  case 69:
/* Line 1787 of yacc.c  */
#line 380 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_BIN_OP, ANA_POW, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
}
    break;

  case 70:
/* Line 1787 of yacc.c  */
#line 383 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_BIN_OP, ANA_MAX, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
}
    break;

  case 71:
/* Line 1787 of yacc.c  */
#line 386 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_BIN_OP, ANA_MIN, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
}
    break;

  case 72:
/* Line 1787 of yacc.c  */
#line 389 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_BIN_OP, ANA_EQ, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
}
    break;

  case 73:
/* Line 1787 of yacc.c  */
#line 392 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_BIN_OP, ANA_NE, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
}
    break;

  case 74:
/* Line 1787 of yacc.c  */
#line 395 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_BIN_OP, ANA_GE, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
}
    break;

  case 75:
/* Line 1787 of yacc.c  */
#line 398 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_BIN_OP, ANA_GT, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
}
    break;

  case 76:
/* Line 1787 of yacc.c  */
#line 401 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_BIN_OP, ANA_LT, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
}
    break;

  case 77:
/* Line 1787 of yacc.c  */
#line 404 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_BIN_OP, ANA_LE, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
}
    break;

  case 78:
/* Line 1787 of yacc.c  */
#line 407 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_BIN_OP, ANA_AND, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
}
    break;

  case 79:
/* Line 1787 of yacc.c  */
#line 410 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_BIN_OP, ANA_OR, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
}
    break;

  case 80:
/* Line 1787 of yacc.c  */
#line 413 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_BIN_OP, ANA_XOR, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
}
    break;

  case 81:
/* Line 1787 of yacc.c  */
#line 416 "anaparser.y"
    {		/* testing & for AND */
  (yyval) = newSymbol(ANA_BIN_OP, ANA_AND, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
}
    break;

  case 82:
/* Line 1787 of yacc.c  */
#line 419 "anaparser.y"
    {		/* testing | for OR */
  (yyval) = newSymbol(ANA_BIN_OP, ANA_OR, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
}
    break;

  case 83:
/* Line 1787 of yacc.c  */
#line 422 "anaparser.y"
    {		/* conditional and */
  (yyval) = newSymbol(ANA_IF_OP, ANA_ANDIF, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
}
    break;

  case 84:
/* Line 1787 of yacc.c  */
#line 425 "anaparser.y"
    {		/* conditional or */
  (yyval) = newSymbol(ANA_IF_OP, ANA_ORIF, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
}
    break;

  case 85:
/* Line 1787 of yacc.c  */
#line 428 "anaparser.y"
    {		/* matrix multiplication */
  startList((yyvsp[(1) - (3)]));
  pushList((yyvsp[(3) - (3)]));
  (yyval) = newSymbol(ANA_INT_FUNC, ANA_MATMUL_FUN);
}
    break;

  case 86:
/* Line 1787 of yacc.c  */
#line 433 "anaparser.y"
    {
  startList((yyvsp[(2) - (2)]));
  (yyval) = newSymbol(ANA_INT_FUNC, ANA_NEG_FUN);
}
    break;

  case 87:
/* Line 1787 of yacc.c  */
#line 437 "anaparser.y"
    {
  (yyval) = (yyvsp[(2) - (2)]);
}
    break;

  case 88:
/* Line 1787 of yacc.c  */
#line 443 "anaparser.y"
    { startList((yyvsp[(1) - (1)])); }
    break;

  case 89:
/* Line 1787 of yacc.c  */
#line 444 "anaparser.y"
    { pushList((yyvsp[(3) - (3)])); }
    break;

  case 90:
/* Line 1787 of yacc.c  */
#line 454 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_PRE_RANGE, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
}
    break;

  case 91:
/* Line 1787 of yacc.c  */
#line 457 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_PRE_RANGE, (yyvsp[(1) - (5)]), -(yyvsp[(5) - (5)]));
}
    break;

  case 92:
/* Line 1787 of yacc.c  */
#line 460 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_PRE_RANGE, (yyvsp[(1) - (3)]), -ANA_ONE);
}
    break;

  case 93:
/* Line 1787 of yacc.c  */
#line 463 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_PRE_RANGE, -(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));
}
    break;

  case 94:
/* Line 1787 of yacc.c  */
#line 466 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_PRE_RANGE, -(yyvsp[(3) - (7)]), -(yyvsp[(7) - (7)]));
}
    break;

  case 95:
/* Line 1787 of yacc.c  */
#line 469 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_PRE_RANGE, -(yyvsp[(3) - (5)]), -ANA_ONE);
}
    break;

  case 96:
/* Line 1787 of yacc.c  */
#line 472 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_PRE_RANGE, -(yyvsp[(3) - (3)]), ANA_ZERO);
}
    break;

  case 97:
/* Line 1787 of yacc.c  */
#line 475 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_PRE_RANGE, -ANA_ONE, ANA_ZERO);
}
    break;

  case 99:
/* Line 1787 of yacc.c  */
#line 488 "anaparser.y"
    {
  sym[(yyvsp[(1) - (3)])].spec.evb.args[2] = 1;
  (yyval) = (yyvsp[(1) - (3)]);
}
    break;

  case 100:
/* Line 1787 of yacc.c  */
#line 493 "anaparser.y"
    {
  sym[(yyvsp[(1) - (4)])].spec.evb.args[3] = (yyvsp[(4) - (4)]);
  (yyval) = (yyvsp[(1) - (4)]);
}
    break;

  case 101:
/* Line 1787 of yacc.c  */
#line 498 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_PRE_RANGE, ANA_ZERO, -ANA_ONE);
  sym[(yyval)].spec.evb.args[2] = 1;
}
    break;

  case 102:
/* Line 1787 of yacc.c  */
#line 503 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_PRE_RANGE, ANA_ZERO, -ANA_ONE);
  sym[(yyval)].spec.evb.args[3] = (yyvsp[(2) - (2)]);
}
    break;

  case 103:
/* Line 1787 of yacc.c  */
#line 508 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_PRE_RANGE, (yyvsp[(1) - (4)]), ANA_ZERO);
  sym[(yyval)].spec.evb.args[3] = (yyvsp[(4) - (4)]);
}
    break;

  case 104:
/* Line 1787 of yacc.c  */
#line 513 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_PRE_RANGE, (yyvsp[(1) - (3)]), ANA_ZERO);
  sym[(yyval)].spec.evb.args[2] = 1;
}
    break;

  case 108:
/* Line 1787 of yacc.c  */
#line 530 "anaparser.y"
    {
  startList((yyvsp[(1) - (1)]));
}
    break;

  case 109:
/* Line 1787 of yacc.c  */
#line 534 "anaparser.y"
    {
  pushList((yyvsp[(3) - (3)]));
}
    break;

  case 110:
/* Line 1787 of yacc.c  */
#line 541 "anaparser.y"
    {
  (yyval) = ANA_ADD;
}
    break;

  case 111:
/* Line 1787 of yacc.c  */
#line 545 "anaparser.y"
    {
  (yyval) = ANA_SUB;
}
    break;

  case 112:
/* Line 1787 of yacc.c  */
#line 549 "anaparser.y"
    {
  (yyval) = ANA_MUL;
}
    break;

  case 113:
/* Line 1787 of yacc.c  */
#line 553 "anaparser.y"
    {
  (yyval) = ANA_DIV;
}
    break;

  case 114:
/* Line 1787 of yacc.c  */
#line 560 "anaparser.y"
    {
  (yyval) = newSubrSymbol((yyvsp[(1) - (2)]));
}
    break;

  case 116:
/* Line 1787 of yacc.c  */
#line 568 "anaparser.y"
    {
  pushList(ANA_NEW_LIST);
}
    break;

  case 117:
/* Line 1787 of yacc.c  */
#line 575 "anaparser.y"
    {
  startList((yyvsp[(1) - (1)]));
}
    break;

  case 118:
/* Line 1787 of yacc.c  */
#line 579 "anaparser.y"
    {
  pushList((yyvsp[(3) - (3)]));
}
    break;

  case 120:
/* Line 1787 of yacc.c  */
#line 592 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_KEYWORD, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
}
    break;

  case 121:
/* Line 1787 of yacc.c  */
#line 596 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_KEYWORD, (yyvsp[(2) - (2)]), ANA_ONE);
}
    break;

  case 124:
/* Line 1787 of yacc.c  */
#line 610 "anaparser.y"
    {
  disableNewline++;
}
    break;

  case 125:
/* Line 1787 of yacc.c  */
#line 612 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_SUBROUTINE, (yyvsp[(3) - (4)]));
}
    break;

  case 126:
/* Line 1787 of yacc.c  */
#line 614 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_SUBROUTINE, -(yyvsp[(5) - (7)]) - 1);
  statementDepth--;
  disableNewline--;
}
    break;

  case 127:
/* Line 1787 of yacc.c  */
#line 620 "anaparser.y"
    {
  disableNewline++;
}
    break;

  case 128:
/* Line 1787 of yacc.c  */
#line 622 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_FUNCTION, (yyvsp[(3) - (4)]));
}
    break;

  case 129:
/* Line 1787 of yacc.c  */
#line 624 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_FUNCTION, -(yyvsp[(5) - (7)]) - 1);
  statementDepth--;
  disableNewline--;
}
    break;

  case 130:
/* Line 1787 of yacc.c  */
#line 630 "anaparser.y"
    {
  disableNewline++;
}
    break;

  case 131:
/* Line 1787 of yacc.c  */
#line 632 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_BLOCKROUTINE, (yyvsp[(3) - (3)]));
}
    break;

  case 132:
/* Line 1787 of yacc.c  */
#line 634 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_BLOCKROUTINE, -(yyvsp[(4) - (6)]) - 1);
  statementDepth--;
  disableNewline--;
}
    break;

  case 140:
/* Line 1787 of yacc.c  */
#line 657 "anaparser.y"
    {
  pushList(ANA_EXTEND);
}
    break;

  case 141:
/* Line 1787 of yacc.c  */
#line 665 "anaparser.y"
    {
  pushList(ANA_NEW_LIST);
}
    break;

  case 143:
/* Line 1787 of yacc.c  */
#line 674 "anaparser.y"
    {
  startList((yyvsp[(1) - (1)]));
}
    break;

  case 144:
/* Line 1787 of yacc.c  */
#line 678 "anaparser.y"
    {
  pushList((yyvsp[(3) - (3)]));
}
    break;

  case 145:
/* Line 1787 of yacc.c  */
#line 685 "anaparser.y"
    {
  startList((yyvsp[(1) - (1)]));
  statementDepth++;
}
    break;

  case 146:
/* Line 1787 of yacc.c  */
#line 690 "anaparser.y"
    {
  pushList((yyvsp[(2) - (2)]));
  if ((yyvsp[(2) - (2)]) == ANA_ERROR)
    (yyval) = ANA_ERROR;
}
    break;

  case 147:
/* Line 1787 of yacc.c  */
#line 700 "anaparser.y"
    {			/* empty list */
  pushList(ANA_NEW_LIST);
}
    break;

  case 150:
/* Line 1787 of yacc.c  */
#line 711 "anaparser.y"
    {
  disableNewline++;
}
    break;

  case 151:
/* Line 1787 of yacc.c  */
#line 713 "anaparser.y"
    {
  disableNewline--;
}
    break;

  case 152:
/* Line 1787 of yacc.c  */
#line 715 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_EVB, EVB_IF, (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), (yyvsp[(7) - (7)]));
}
    break;

  case 153:
/* Line 1787 of yacc.c  */
#line 719 "anaparser.y"
    {
  disableNewline++;
}
    break;

  case 154:
/* Line 1787 of yacc.c  */
#line 721 "anaparser.y"
    {
  pushList((yyvsp[(4) - (5)]));
  (yyval) = newSymbol(ANA_EVB, EVB_CASE);
  disableNewline--;
}
    break;

  case 155:
/* Line 1787 of yacc.c  */
#line 727 "anaparser.y"
    {
  disableNewline++;
}
    break;

  case 156:
/* Line 1787 of yacc.c  */
#line 729 "anaparser.y"
    {
  pushList((yyvsp[(5) - (6)]));
  pushList((yyvsp[(3) - (6)]));
  statementDepth--;
  (yyval) = newSymbol(ANA_EVB, EVB_NCASE);
  disableNewline--;
}
    break;

  case 159:
/* Line 1787 of yacc.c  */
#line 746 "anaparser.y"
    {
  (yyval) = 0;
}
    break;

  case 160:
/* Line 1787 of yacc.c  */
#line 750 "anaparser.y"
    {
  (yyval) = (yyvsp[(2) - (2)]);
}
    break;

  case 161:
/* Line 1787 of yacc.c  */
#line 757 "anaparser.y"
    {
  (yyval) = 0;
}
    break;

  case 162:
/* Line 1787 of yacc.c  */
#line 761 "anaparser.y"
    {
  disableNewline++;
}
    break;

  case 163:
/* Line 1787 of yacc.c  */
#line 763 "anaparser.y"
    {
  (yyval) = (yyvsp[(3) - (3)]);
  disableNewline--;
}
    break;

  case 164:
/* Line 1787 of yacc.c  */
#line 768 "anaparser.y"
    {
  disableNewline++;
}
    break;

  case 165:
/* Line 1787 of yacc.c  */
#line 770 "anaparser.y"
    {		/* Dick's case else */
  (yyval) = (yyvsp[(3) - (4)]);
  disableNewline--;
}
    break;

  case 166:
/* Line 1787 of yacc.c  */
#line 778 "anaparser.y"
    {
  startList((yyvsp[(1) - (3)]));
  pushList((yyvsp[(3) - (3)]));
}
    break;

  case 167:
/* Line 1787 of yacc.c  */
#line 783 "anaparser.y"
    {
  pushList((yyvsp[(2) - (4)]));
  pushList((yyvsp[(4) - (4)]));
}
    break;

  case 168:
/* Line 1787 of yacc.c  */
#line 791 "anaparser.y"
    {
  disableNewline++;
}
    break;

  case 169:
/* Line 1787 of yacc.c  */
#line 793 "anaparser.y"
    {
  disableNewline--;
}
    break;

  case 170:
/* Line 1787 of yacc.c  */
#line 795 "anaparser.y"
    {
  (yyval) = findVar((yyvsp[(3) - (11)]), curContext);
  (yyval) = newSymbol(ANA_EVB, EVB_FOR, (yyval), (yyvsp[(5) - (11)]), (yyvsp[(7) - (11)]), (yyvsp[(8) - (11)]), (yyvsp[(11) - (11)]));
}
    break;

  case 171:
/* Line 1787 of yacc.c  */
#line 800 "anaparser.y"
    {
  disableNewline++;
}
    break;

  case 172:
/* Line 1787 of yacc.c  */
#line 802 "anaparser.y"
    {
  disableNewline--;
}
    break;

  case 173:
/* Line 1787 of yacc.c  */
#line 804 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_EVB, EVB_REPEAT, (yyvsp[(3) - (6)]), (yyvsp[(6) - (6)]));
}
    break;

  case 174:
/* Line 1787 of yacc.c  */
#line 808 "anaparser.y"
    {
  disableNewline++;
}
    break;

  case 175:
/* Line 1787 of yacc.c  */
#line 810 "anaparser.y"
    {
  disableNewline--;
}
    break;

  case 176:
/* Line 1787 of yacc.c  */
#line 812 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_EVB, EVB_DO_WHILE, (yyvsp[(3) - (6)]), (yyvsp[(6) - (6)]));
}
    break;

  case 177:
/* Line 1787 of yacc.c  */
#line 816 "anaparser.y"
    {
  disableNewline++;
}
    break;

  case 178:
/* Line 1787 of yacc.c  */
#line 818 "anaparser.y"
    {
  disableNewline--;
}
    break;

  case 179:
/* Line 1787 of yacc.c  */
#line 820 "anaparser.y"
    {
  (yyval) = newSymbol(ANA_EVB, EVB_WHILE_DO, (yyvsp[(3) - (6)]), (yyvsp[(6) - (6)]));
}
    break;

  case 180:
/* Line 1787 of yacc.c  */
#line 827 "anaparser.y"
    {
  (yyval) = ANA_ONE;
}
    break;

  case 181:
/* Line 1787 of yacc.c  */
#line 831 "anaparser.y"
    {
  (yyval) = (yyvsp[(2) - (2)]);
}
    break;


/* Line 1787 of yacc.c  */
#line 3329 "anaparser.c"
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
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
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
      if (!yypact_value_is_default (yyn))
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

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
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


/* Line 2048 of yacc.c  */
#line 843 "anaparser.y"


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <termio.h>	/* for unbuffered input */
#include <unistd.h>	/* for unbuffered input */
/*#include "editor_once.h"*/
#include <setjmp.h>
#include <ctype.h>
#include "once.h"
#include "editorcharclass.h"

jmp_buf	jmpenv;			/* environment storage for long jumps */
char	*currentChar,		/* the character currently being parsed */
	inHistoryBuffer = 1,	/* input is copied into history buffer */
	reportBody = 0;		/* user routines are compiled, */
				/* not just reported.*/
static char	oldChar = '\0';
extern FILE	*inputStream;		/* source of input, if file */
extern char	*inputString;		/* source of input, if string */
Int	ignoreInput = 0,	/* nesting level of IGNORE-RESUME pairs */
        findBody = 0,		/* nonzero if a specific user-routine */
				/* is sought for recompilation */
	calculatorMode = 0;	/* nonzero if in calculator mode
				 (see calculator.c) */

char	*ANAPrompts[] =	{	/* legal ANA prompts */
  "ANA>", "mor>", "ign>", "dbg>", "clc>"
};
#define N_ANAPROMPTS 5

Int	rawIo(void),		/* selects untreated key-by-key input */
  cookedIo(void);		/* selects treated line-by-line input */
extern char	line[],		/* raw user input */
	tLine[];		/* translated user input */
void	symbolInitialization(void);

static char	continuation = 0; /* indicates whether the current line */
				/* ends with a continuation character (-) */
				/* outside of a comment. */

void away(void)
     /* clean up the symbol stack and reinitialize various counters */
{
  char	**p;
  extern Int	keepEVB, (*getChar)(void), nSymbolStack;
  Int	getStreamChar(void);

  /* statementDepth = disableNewline = curContext = keepEVB = 0; */
  curContext = keepEVB = 0;
  p = &symbolStack[nSymbolStack - 1];
  while (nSymbolStack > 0) {
    if (*p)
      free(*p);
    *p-- = 0;
    nSymbolStack--;
  }
  if (symbolStack[0]) {
    free(symbolStack[0]);
    symbolStack[0] = 0;
  }
  symbolStackIndex = nSymbolStack = 0;
  getChar = getStreamChar;
}

Int yyerror(char *s)
     /* reports errors occurring during parsing (required by yyparse()) */
{
 extern Int	curLineNumber;	/* current line number */
 extern compileInfo	*curCompileInfo;

 if (setup & 1024)	/* no parser warnings */
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
   anaerror("Error just before end of line", 0);
   *line = '\0';
 } else
   anaerror("Error just before \"%s\"", 0, currentChar); 
 return 0;
}
/*--------------------------------------------------------------*/
void translateLine(void)
/* translates line in line[] and stores result in tLine[]:
   - remove heading whitespace
   - translate control chars to whitespace
   - replace multiple whitespace by single whitespace
   - remove comments
   - transform the rest to upper case (except literal strings)
   Moved treatment of literal strings to yylex because sometimes
   they must be ignored (e.g. when parsing nonrelevant code from
   a @@ file)
   string in line[] may be changed */
{
  char	*p, *tp, inString = 0;
  Int	nonWhite, i;
  extern char	*ANAPrompts[],
		allowPromptInInput; /* allow prompts at start of line */

  p = line;			/* user input */
  tp = tLine;			/* translated input */
  if (allowPromptInInput)	/* allow legal ANA prompts at the very
				   beginning of input lines; i.e. ignore when
				   encountered */
    for (i = 0; i < N_ANAPROMPTS; i++)
      if (!strncmp(p, ANAPrompts[i], strlen(ANAPrompts[i]))) {
				/* found an ANA prompt */
	p += strlen(ANAPrompts[i]);/* skip over it */
	break;
      }
  if (!inString)
    while (isspace((Byte) *p))
      p++;			/* skip heading whitespace
				   outside literal strings */
  nonWhite = 0;			/* number of consecutive non-whitespaces */
  while (*p) {			/* not at end of line */
    if (iscntrl((Byte) *p))
      *p = ' ';			/* control characters are transformed into
				   whitespace */
    switch (*p) {
      case '\'': case '"':	/* fixed literal string */
        if (inString) {		/* we are already inside a literal string */
	  if (*p == inString) {	/* active string quote */
	    if (p[1] == *p) {	/* a quoted apostrophe, i.e. an apostrophe
				   as part of the literal string */
	      *tp++ = *p;	/* a single apostrophe into the translation */
	      memcpy(p, p + 1, strlen(p));  /* "skip over" the second one */
	    } else {		/* found the end of the literal string */
	      *tp++ = TRANS_FIXED_STRING; /* signal end of literal string */
	      inString = 0; 	/* we're now outside the string */
	    }
	  } else		/* not an active string quote: just copy */
	    *tp++ = *p;
	} else {		/* we're not now in a literal string */
	  *tp++ = TRANS_FIXED_STRING; /* signal the string's start */
	  inString = *p;	/* we're now inside the string */
	}
        break;
      case '@':
	if (!inString) {	/* we're not inside a literal string, so
				   it must be a "@filename" */
	  *tp++ = *p++;		/* @ goes into the translation */
          while (isFileNameChar((Byte) *p))
	    *tp++ = *p++;	/* file name goes into the translation */
	  p--;
	} else
	  *tp++ = *p;		/* else include */
        break;
      case ';':			/* possibly the start of a comment */
        if (inString) {		/* no, part of a literal string, so leave;
				   otherwise treat as end of line */
	  *tp++ = *p;
	  break;
	}
	*tp = '\0';		/* a comment, so regard as an empty line. */
	return;
      case '\n':		/* end of the line */
        *tp = '\0';		/* terminate translated string */
        return;			/* done. */
      case ' ':  case '\t':	/* whitespace */
        if (!inString) {	/* not inside a literal string */
	  if (nonWhite)		/* we did not just copy whitespace */
	    *tp++ = *p;		/* this whitespace into the translation */
          nonWhite = -1;	/* will be incremented below to zero
				   consecutive non-whitespaces */
	} else
	  *tp++ = *p;		/* inside a string, so just copy */
        break;
      case '\\':
	/* we must check for \' and \" inside strings because they must */
	/* not be interpreted as a backslash and a string-ending */
	/* character, but rather as introducing a literal character ' or ". */
	if (inString && p[1] == inString) {
	  *tp++ = *p++;		/* copy the backslash; the next lines */
				/* will get the quote character*/
	} else if (p[1] == '\\')
	  *tp++ = *p++;		/* we copy both slashes; they'll be dealt */
				/* with in installString(). */
	*tp++ = *p;		/* copy (also if the backslash does not */
				/* introduce a literal quote */
	break;
      default:			/* "ordinary" character */
	if (inString)
	  *tp++ = *p;		/* in string, so just copy */
	else
	  *tp++ = toupper(*p);	/* else make upper case */
	break;
    }
    p++;			/* next character */
    nonWhite++;			/* increment consecutive
				   non-whitespace count */
  }
  *tp++ = '\0';			/* sure close */
}
/*--------------------------------------------------------------*/
void Quit(Int result)
/* Quits the program */
{
  Int	saveHistory(void);

  cookedIo();			/* back to line-by-line input */
  saveHistory();
  printf("\nCPUtime: %g seconds\n", ((Float) clock())/CLOCKS_PER_SEC);
  puts("Quitting... Bye!");	/* farewell message */
  exit(result);
}
/*--------------------------------------------------------------*/
/* NUMBERS
   ANA allows specification of numbers of five types (ANA_BYTE, ANA_WORD, ANA_LONG,
   ANA_FLOAT, ANA_DOUBLE) and three bases (8, 10, 16).  Numbers must always
   start with a decimal digit.
   Fractional (i.e. ANA_FLOAT or ANA_DOUBLE) numbers can only be specified
   in base 10.  In integer numbers, a base specifier (if any) precedes
   a type specifier (if any).  By default, integers are in base 10 and
   of type ANA_LONG.
   Octal numbers are followed by an O.  Hexadecimal numbers are either
   preceded by 0X or followed by an X.  A final B indicates a ANA_BYTE number,
   a final W a ANA_WORD, and a final L a ANA_LONG number.
   A general ANA_FLOAT looks like this:  1.23E+4
   Either the decimal point or the exponent indicator E must be present.
   A plus sign is optional in the exponent.  A general ANA_DOUBLE looks like
   a general ANA_FLOAT, except that a D is used instead of an E.  The exponent
   indicator D must be present.
   Some examples:  0X1AB = 1AXB = ANA_BYTE hex 1A = ANA_BYTE 26
                   0X1ABL = 1ABXL = ANA_LONG hex 1AB = ANA_LONG 427
                   2E = 2. = ANA_FLOAT 2
                   2EX = 0X2E = ANA_LONG hex 2E = ANA_LONG 46
*/
/*--------------------------------------------------------------*/
Int readNumber(YYSTYPE *lvalp)
/* reads the number at currentChar, puts it in a new sybol, assigns
   the symbol number to *lvalp, and returns the proper parser code */
{
  Int	type;
  scalar	v;
  void	read_a_number(char **, scalar *, Int *);

  read_a_number(&currentChar, &v, &type);
  if (!ignoreInput) {		/* we're not ignoring stuff */
    if ((*lvalp = newSymbol(ANA_FIXED_NUMBER, type)) < 0) /* could not get */
				/* a new symbol for the number */
      return ANA_ERROR;		/* return error indication */
    if (*lvalp)
      switch(type) {		/* non-zero return value (??) */
				/* insert value of proper type */
	case ANA_BYTE:
	  scalar_value(*lvalp).b = (Byte) v.l;
	  break;
	case ANA_WORD:
	  scalar_value(*lvalp).w = (Word) v.l;
	  break;
	case ANA_LONG:
	  scalar_value(*lvalp).l = v.l;
	  break;
	case ANA_FLOAT:
	  scalar_value(*lvalp).f = (Float) v.d;
	  break;
	case ANA_DOUBLE:
	  scalar_value(*lvalp).d = v.d;
	  break;
	case ANA_CFLOAT:
	  complex_scalar_data(*lvalp).cf->real = 0.0;
	  complex_scalar_data(*lvalp).cf->imaginary = v.d;
	  break;
	case ANA_CDOUBLE:
	  complex_scalar_data(*lvalp).cd->real = 0.0;
	  complex_scalar_data(*lvalp).cd->imaginary = v.d;
	  break;
	default:
	  return cerror(ILL_TYPE, 0);
      }
    return NUMBER;
  } else
    return 0;
}
/*--------------------------------------------------------------*/
Int strcmp2(const void *key, const void *data)
/* compares key to data */
{
  return strcmp((char *) key, *(char **) data);
}
/*--------------------------------------------------------------*/
Int isKeyWord(void)
/* checks if the string starting at currentChar is a reserved (key) Word;
  if so, returns the keyword's code */
{
 static char	*keyWords[] = {
   "AND", "ANDIF", "BEGIN", "BLOCK", "BREAK", "CASE", "CONTINUE", 
   "DO", "ELSE", "END", "ENDBLOCK", "ENDCASE", "ENDFUNC", 
   "ENDSUBR", "EQ", "FOR", "FUNC", "FUNCTION", "GE", "GT", "IF", "LE", "LT", 
   "MOD", "NCASE", "NE", "OR", "ORIF", "REPEAT", "RETALL", "RETURN", 
   "RUN", "SMOD", "SUBR", "SUBROUTINE", "THEN", "UNTIL", "WHILE", "XOR"
 };
 static Int	keyCodes[] = {
   AND, ANDIF, BEGIN, BLOCK, BREAK, CASE, CONTINUE, DO, ELSE,
   END, ENDBLOCK, ENDCASE, ENDFUNC, ENDSUBR, EQ, FOR, FUNC, FUNC, GE,
   GT, IF, LE, LT, '%', NCASE, NE, OR, ORIF, REPEAT, RETALL, RETURN, RUN,
   SMOD, SUBR, SUBR, THEN, UNTIL, WHILE, XOR
 };
 char	**ptr;

 ptr = bsearch(currentChar, keyWords, sizeof(keyWords)/sizeof(char *),
   sizeof(char *), strcmp2);
 return (ptr)? keyCodes[ptr - keyWords]: 0;
}
/*--------------------------------------------------------------*/
Int readIdentifier(YYSTYPE *lvalp)
/* identifies keywords at currentChar and returns appropriate lexical number;
  otherwise reads identifier, stores in stack, returns index to
  stack in *lvalp, and returns appropriate lexical number. */
{
 char	*p, c;
 Int	n;
 Int	installString(char *string);

 p = currentChar + 1;		/* skip over first character, which */
				/* is assumed to be OK by this routine */
 while (isNextChar((Byte) *p))
   p++;				/* span identifier */
 c = *p;
 *p = '\0';			/* terminate string, but save clobbered char */
 if ((n = isKeyWord())) {	/* a standard ANA-language key Word */
   *p = c;			/* restore clobbered char */
   currentChar = p;		/* continue parsing beyond key Word */
   return n;			/* return keyword index */
 }
 /* the identifier is not a standard key Word */
 if (!ignoreInput) {		/* we're not ignoring stuff, so need to save */
   *lvalp = installString(currentChar);	/* save string, index in *lvalp */
   *p = c;			/* restore clobbered char */
   n = isNextChar((Byte) *currentChar)? C_ID: S_ID;
   currentChar = p;		/* continue parse beyond string */
   if (*lvalp < 0)
     return ANA_ERROR;		/* some error occurred */
   return n;			/* return "special identifier" token */
 } else {			/* we are ignoring stuff, don't save */
   currentChar = p;		/* so continue parse beyond string */
   return 0; 			/* signal that nothing was saved */
 }
}
/*--------------------------------------------------------------*/
Int yylex(YYSTYPE *lvalp)
/* returns semantic value of next read token in *lvalp, and the
   lexical value as function return value */
{
 char	*p, c, *prompt, *p2;
 Int	i;
 static Int	len = 0;
 extern char	recording, *currentInputFile;
 extern Int	curLineNumber;	/* current line number */
 Int	getNewLine(char *buffer, char *prompt, char historyFlag),
   showstats(Int narg, Int ps[]), installString(char *);

 if (errorState)
   return ERRORSTATE;		/* if a syntax error occurred, then */
				/* tell the parser so. */
 /* now determine the appropriate ANA prompt to use */
 if (ignoreInput)		/* we're within a nested IGNORE-RESUME pair */
   prompt = ANAPrompts[2];	/* ign> */
 else if (debugLine)		/* debugger line (execute()) */
   prompt = ANAPrompts[3];	/* dbg> */
 else if (disableNewline)	/* need more input */
   prompt = ANAPrompts[1];	/* mor> */
 else if (calculatorMode)
   prompt = ANAPrompts[4];	/* clc> */
 else
   prompt = ANAPrompts[0];	/* default, ANA> */
 /* now get and treat the input */
 while (1) {			/* keep cycling */
   if (!*line) {		/* nothing more in current input line */
     while (!*line) {		/* try until there's something to do */
       if (len < 0)		/* EOF reached */
	 putchar('\r');		/* ensure cursor is at left */
       len = getNewLine(line, prompt, inHistoryBuffer);	/* read new line; */
	                        /* the length (excluding final \0) into len */
       curLineNumber++;		/* update current line number */
       if (len < 0) {		/* found end of file (EOF) */
	 if (ignoreInput) {
	   /* ignoreInput is non-zero either because we're under influence */
	   /* of one or more IGNOREs, or because we're skimming over the */
	   /* body of a function or subroutine through a @@file command */
	   if (!findBody)	/* dangling IGNORE(s) */
	     printf("%1d IGNOREs still enforced at EOF!\n", ignoreInput); 
	 }
	 return 0;		/* signal end-of-file to yyparse() */
       }
     }
     translateLine();		/* translate input line for easier parsing */
     currentChar = tLine; 	/* first character of translated input line */
				/* is current one in parsing */
   }
   while (isspace((Byte) *currentChar))
     currentChar++;		/* skip leading spaces */

   /* we recognize RESUME and IGNORE only at the beginning of a line */
   if (currentChar == tLine) {
     if (!strncmp(currentChar, "IGNORE", 6)
	 && !isNextChar((Byte) currentChar[6])) {
       ignoreInput++;
       prompt = ANAPrompts[2];	/* ign> */
     } else if (!strncmp(currentChar, "RESUME", 6)
		&& !isNextChar((Byte) currentChar[6])) {
       if (!ignoreInput)
	 puts("Unmatched RESUME ignored");
       else
	 ignoreInput--;
       if (!ignoreInput) {
	 if (disableNewline)	/* need more input */
	   prompt = ANAPrompts[1];	/* mor> */
	 else if (debugLine)	/* debugger line */
	   prompt = ANAPrompts[3];	/* dbg> */
	 else if (calculatorMode)
	   prompt = ANAPrompts[4];	/* clc> */
	 else
	   prompt = ANAPrompts[0];	/* default, ANA> */
       }
       currentChar += 6;
     }
   }

   if (ignoreInput && !findBody) { /* ignoring input and not looking for */
				   /* a routine body */
     *line = '\0';
     continue;
   }

   if (*currentChar == TRANS_FIXED_STRING) { /* a literal string */
   				/* find the end of the string */
     for (p2 = p = currentChar + 1; *p2 && *p2 != TRANS_FIXED_STRING; p2++);
     if (!*p2) {		/* reached end-of-line before end of string */
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
       *p2 = '\0';		/* terminate string */
       currentChar = p2 + 1; 	/* resume analysis beyond the string */
     }
     if (!ignoreInput) {	/* we're not IGNOREing stuff now */
       *lvalp = installString(p); /* save the string in the symbol stack */
       return STR; 		/* token type is string */
     }
     continue;			/* we are ignoring stuff, so continue */
   }

   if (*currentChar == '#' && !isNextChar(currentChar[1])) {
     /* it's the # operator, not an identifier starting with # */
     currentChar++;
     if (ignoreInput)
       continue;
     return '#';
   }

   if (isFirstChar((Byte) *currentChar)) { /* an identifier of some sort */
     i = readIdentifier(lvalp);	/* which standard one or what kind is it? */
     switch (i) {
       case SUBR: case FUNC: case BLOCK: /* SUBR, FUNC, BLOCK: */
				/* definition start */
	 if (findBody > 0) {	/* we're looking for the body of some */
				/* routine/function.  If the just */
				/* encounterd routine/function is the */
				/* sought one, then we need to compile it */
	   for (p = currentChar + 2; isNextChar((Byte) *p); p++);
	   /* end of name */
	   c = *p;
	   *p = '\0';		/* temporary termination */
				/* get name of sought routine/function */
	   if (i == SUBR)
	     p2 = subrName(findBody);
	   else if (i == FUNC)
	     p2 = funcName(findBody);
	   else
	     p2 = blockName(findBody);
	   /* we only accept a match if the routine name matches */
	   /* exactly; i.e., no partial matches */
	   
	   if (!strcmp(currentChar + 1, p2)) { /* same */
	     ignoreInput--;	/* yes: reduce IGNORE level, */
	     *p = c;		/* restore */
	     return i;
	   }
	   *p = c;		/* restore */
	 }
	 if (ignoreInput)
	   continue;		/* we're ignoring stuff, so get next */
	 else
	   return i;		/* not ignoring, so pass on to yyparse() */
       case ENDSUBR: case ENDFUNC: case ENDBLOCK: /* ENDSUBR, ENDFUNC, */
				/* ENDBLOCK: end of definition */
	 if (findBody && !ignoreInput) { /* we were looking for a routine/ */
	   /* function body (findBody) and were not ignoring input */
	   /* (!ignoreInput), so we must have just completed compilation */
	   /* of the sought routine/function. */
	   findBody = -findBody;/* flag completion of body */
	   ignoreInput++;	/* ignore rest of the input, and */
	   return i;		/* pass token on to yyparse() */
	 }
	 if (ignoreInput)
	   continue;		/* wasn't sought routine, keep looking */
	 else
	   return i;		/* pass on to yyparse() */
       case 0:			/* nothing was saved; stuff was ignored */
	 continue;		/* so continue parsing */
       default:
	 if (ignoreInput)
	   continue;		/* ignoring, so continue parsing */
	 return i; 		/* some user-defined identifier, pass */
				/* returned C_ID or S_ID on to yyparse() */
     }
   }

   if (isdigit((Byte) *currentChar)) {	/* a number */
     i = readNumber(lvalp);	/* read number into symbol */
     if (ignoreInput)
       continue;		/* we're ignoring stuff, so the number */
				/* wasn't saved by readNumber() and we */
				/* can continue parsing */
     else
       return i; 		/* pass token NUM on to yyparse() */
   }

   if (isTwoOperator((Byte) *currentChar)) { /* possibly op-assignment */
     if (currentChar[1] == '=')	{ /* yes, an operation-assignment (e.g. +=) */
       currentChar += 2;	/* continue parse beyond operator */
       if (ignoreInput)
	 continue;		/* ignoring stuff, so continue parse */
       switch (currentChar[-2])	{ /* which operation? */
	 case '+':
	   return PLUSIS;	/* += */
	 case '-':
	   return MINUSIS;	/* -= */
	 case '*':
	   return TIMESIS;	/* *= */
	 case '/':
	   return DIVIDEIS;	/* /= */
	 case '^':
	   return POWERIS; }	/* ^= */
     }
   }
 
  if (!*currentChar) {		/* end-of-line */
    oldChar = *line;		/* (??) */
    *line = '\0';		/* request a new input line */
    if (!disableNewline && !ignoreInput) /* not ignoring things, and */
				/* NEWLINEs are honored */
      return NEWLINE;
    else
     continue;			/* else ignore NEWLINE and continue parse */
  }

  if (*currentChar == '@') {	/* file inclusion */
    /* If a file is included with the @@ prefix, then any SUBR, FUNC, or */
    /* BLOCK definition contained in the file is noted and the body of */
    /* that routine is parsed, but not yet compiled.  If the routine is */
    /* later referenced, then the file is read again to compile the routine. */
    /* This means that files with lots of routine definitions of which */
    /* only a few are going to be used don't clog up the symbol tables. */
    i = 0;			/* signal @ */
    if (currentChar[1] == '@') { /* i.e., @@ */
      if (!ignoreInput)
	i = 1;			/* no ignoring, signal @@ */
       currentChar++; 		/* skip extra @ */
    }
    for (p = currentChar + 1; isFileNameChar((Byte) *p); p++); /* span name */
    if (!ignoreInput) {		/* not ignoring, so note file name */
      c = *p;			/* save character beyond file name */
      *p = '\0';		/* temporary string terminator */
      *lvalp = installString(currentChar + 1);	/* save file name */
      *p = c; 			/* restore clobbered char */
    }
    currentChar = p;		/* continue parse beyond file name */
    if (ignoreInput)
      continue;			/* ignoring stuff, so continue parse */
    else
      return i? REPORT: INCLUDE; /* @@ -> REPORT, @ -> INCLUDE */
  }

  if (*currentChar == '-'
      && !currentChar[1]) {	/* continuation character */
    continuation = 1;
    prompt = ANAPrompts[1];	/* mor> */
    *line = '\0';		/* request a new input line */
    continue;
  }

  if (*currentChar == '?') {	/* show vital statistics immediately, */
				/* i.e. during parsing (!)*/
    showstats(0, NULL);
    /* now move over rest of line so '?' disappears */
    memcpy(currentChar, currentChar + 1, strlen(currentChar));
    continue; 			/* continue parse */
  }

  if (*currentChar == '.') {	/* a structure tag or ellipsis or a number */
    if (isdigit((Byte) currentChar[1])) { /* a number */
      i = readNumber(lvalp);
      if (ignoreInput)
	continue;
      else
	return i;
    } else if (isNextChar((Byte) currentChar[1])) { /* a string tag */
      p = ++currentChar;	/* skip . */
      while (isNextChar((Byte) *currentChar))
	currentChar++;		/* span identifier */
      if (ignoreInput)
	continue;
      c = *currentChar;
      *currentChar = '\0';	/* temporary string end */
      *lvalp = string_scratch(currentChar - p);
      strcpy(string_value(*lvalp), p);
      p = string_value(*lvalp);
      while (*p) {
	*p = toupper(*p);
	p++;
      }
      *currentChar = c;
      return STRUCTTAG;
    } else if (currentChar[1] == '.' && currentChar[2] == '.') { /* ellipsis */
      currentChar += 3;
      if (ignoreInput)
	continue;
      return ELLIPSIS;
    } else			/* nothing yet; pass as is */
      currentChar--;
  }
    
  switch (*currentChar) {
    case '=':
      if (currentChar[1] == '=') { /* == corresponds to EQ */
	currentChar += 2;
	if (ignoreInput)
	  continue;
	return EQ;
      }
      break;
    case '!':
      if (currentChar[1] == '=') { /* != corresponds to NE */
	currentChar += 2;
	if (ignoreInput)
	  continue;
	return NE;
      }
      break;
    case '>':
      if (currentChar[1] == '=') { /* >= corresponds to GE */
	currentChar += 2;
	if (ignoreInput)
	  continue;
	return GE;
      }
      break;
    case '<':
      if (currentChar[1] == '=') { /* <= corresponds to LE */
	currentChar += 2;
	if (ignoreInput)
	  continue;
	return LE;
      }
      break;
    case '&':
      if (currentChar[1] == '&') { /* && corresponds to ANDIF */
	currentChar += 2;
	if (ignoreInput)
	  continue;
	return ANDIF;
      }
      break;
    case '|':
      if (currentChar[1] == '|') { /* || corresponds to ORIF */
	currentChar += 2;
	if (ignoreInput)
	  continue;
	return ORIF;
      }
      break;
  }

  /* only thing left: single char (e.g. + or [ ) */
  if (!ignoreInput)		/* not ignoring: pass on */
    return *currentChar++;
  else {			/* ignoring, skip and continue parse */
    currentChar++;
    continue;
  }
 }
}
/*--------------------------------------------------------------*/
Int calc_lex(YYSTYPE *lvalp)
/* required to make ana_calculator() parsing work */
{
  return yylex(lvalp);
}
/*--------------------------------------------------------------*/
Int calc_error(char *s)
/* required to make ana_calculator() parsing work */
{
  return yyerror(s);
}
/*--------------------------------------------------------------*/
void gehandler(const char *reason, const char *file, Int line, Int gsl_errno)
{
  anaerror("GSL error %d (%s line %d): %s", 0, gsl_errno, file, line, reason);
}
/*--------------------------------------------------------------*/
char	*programName;
Int do_main(Int argc, char *argv[])
     /* main program */
{
  Int	site(Int, Int []), readHistory(void);
  char	*p;
  extern Int	nSymbolStack;
  extern void	getTerminalSize(void);
  void	pegParse(void), inHistory(char *), getTermCaps(void);
  FILE	*fp;
  Int	yyparse(void);

  programName = argv[0];
  getTerminalSize();
  getTermCaps();
  site(0, NULL);		/* identify this version of ANA */
  fflush(stdin);
  rawIo();			/* get keystrokes one by one */
  symbolInitialization();
  readHistory();
  *line = '\0';			/* start with an empty line */
  p = line;

  void gehandler(const char *, const char *, Int, Int);

  gsl_set_error_handler(&gehandler);
  /* seek .anainit in home directory */
  fp = fopen(expand_name("~/.anainit", NULL), "r");
  if (fp) {
    fclose(fp);
    strcpy(p, "@~/.anainit");
    p += strlen(p);
  }
  /* now treat the command line "options" as if they were typed at */
  /* the ANA> prompt */
  argc--;  argv++;
  while (argc--) {
    *p++ = ' ';
    if (**argv == '/')
      *p++ = '@';
    strcpy(p, *argv++);
    p += strlen(p);
  }
  if (*line) {
    printf("%s%s\n", ANAPrompts[0], line);
    inHistory(line + 1);
    translateLine();
    currentChar = tLine;
  }
  setjmp(jmpenv);
  clock();			/* or it does not seem to start counting */
				/* on some machines */
  /* the main parsing loop: if an error occurs, then clean up and restart */
  /* parsing */
  do
  { errorState = 0;		/* no syntax errors (yet) in this parse */
    pegParse();
    yyparse();			/* parse input (calls yylex()) */
    if (errorState)		/* a syntax error occurred */
    { cleanUp(0, CLEANUP_ERROR);
      /* clean up stored but yet uninterpreted symbol names */
      while (symbolStackIndex)
	if (symbolStack[symbolStackIndex - 1])
	  unlinkString(symbolStackIndex - 1);
      symbolStackIndex = nSymbolStack = 0;
      listStackItem = listStack; /* reset list stack */
      statementDepth = disableNewline = curContext = 0;	/* various resets */
      *line = 0; }		/* clear input line */
  } while (errorState);		/* cycle if an error occurred */
  Quit(0);
  return 1;			/* or the compiler may complain */
}

