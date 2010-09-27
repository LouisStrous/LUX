
/*  A Bison parser, made from calculator.c
 by  GNU Bison version 1.27
  */

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse calcparse
#define yylex calclex
#define yyerror calcerror
#define yylval calclval
#define yychar calcchar
#define yydebug calcdebug
#define yynerrs calcnerrs
#define	NEWLINE	999
#define	C_ID	257
#define	S_ID	258
#define	NUMBER	259
#define	STR	260
#define	INCLUDE	261
#define	REPORT	262
#define	IF	263
#define	THEN	264
#define	ELSE	265
#define	FOR	266
#define	REPEAT	267
#define	UNTIL	268
#define	WHILE	269
#define	DO	270
#define	CASE	271
#define	ENDCASE	272
#define	NCASE	273
#define	SUBR	274
#define	ENDSUBR	275
#define	FUNC	276
#define	ENDFUNC	277
#define	BLOCK	278
#define	ENDBLOCK	279
#define	RETURN	280
#define	BREAK	281
#define	CONTINUE	282
#define	RUN	283
#define	BEGIN	284
#define	END	285
#define	PLUSIS	286
#define	MINUSIS	287
#define	TIMESIS	288
#define	DIVIDEIS	289
#define	POWERIS	290
#define	RETALL	291
#define	STRUCTTAG	292
#define	ERRORSTATE	293
#define	ELLIPSIS	294
#define	ANDIF	295
#define	ORIF	296
#define	AND	297
#define	OR	298
#define	XOR	299
#define	GE	300
#define	LE	301
#define	GT	302
#define	LT	303
#define	EQ	304
#define	NE	305
#define	UMINUS	306

#line 1 "calculator.c"


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include "editor.h"
#include "action.h"

extern int	ans;

#define startList(x)	{ pushList(ANA_NEW_LIST); pushList(x); }
				/* start a new list */
void	pushList(word symNum),	/* push symbol number onto list stack */
	swapList(int, int),	/* swap items in the list stack */
	away(void);
word	popList(void);		/* pop an item from the list stack's top */
int	stackListLength(void),	/* return length of list at top of stack */
	isInternalSubr(int),	/* 1 if symbol is internal subroutine */
	installExec(void),
	findSym(int, hashTableEntry *[], int),
	installSubsc(int),
	anaerror(char *, int, ...), ana_replace(int, int), ana_type(int, int []),
	newSymbol(int, ...);
int	yyerror(char *), yylex(YYSTYPE *);
#ifndef YYSTYPE
#define YYSTYPE int
#endif
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		172
#define	YYFLAG		-32768
#define	YYNTBASE	70

#define YYTRANSLATE(x) ((unsigned)(x) <= 999 ? yytranslate[x] : 81)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,    60,     2,     2,    64,
    67,    58,    56,    65,    57,    63,    59,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    66,     2,    54,
    41,    55,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    68,     2,    69,    62,     2,     2,     2,     2,     2,     2,
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
     2,     2,     2,     2,     2,     1,     4,     5,     6,     7,
     8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
    18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
    28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
    38,    39,    40,    42,    43,    44,    45,    46,    47,    48,
    49,    50,    51,    52,    53,    61,     2,     2,     2,     2,
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
     2,     2,     2,     2,     2,     2,     2,     2,     3
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     5,     8,    13,    17,    21,    25,    29,    33,
    37,    41,    45,    49,    53,    57,    61,    65,    69,    73,
    77,    81,    85,    89,    92,    96,    99,   101,   103,   105,
   107,   111,   113,   115,   117,   119,   123,   128,   130,   133,
   138,   142,   146,   152,   156,   162,   170,   176,   180,   182,
   186,   189,   191,   193,   195,   197,   202,   206,   210,   214,
   218,   222,   226,   230,   234,   238,   242,   246,   250,   254,
   258,   262,   266,   270,   274,   278,   282,   286,   291,   294,
   296
};

static const short yyrhs[] = {    71,
     0,    70,    71,     0,    72,     3,     0,    72,    41,    79,
     3,     0,    56,    79,     3,     0,    57,    79,     3,     0,
    58,    79,     3,     0,    59,    79,     3,     0,    60,    79,
     3,     0,    62,    79,     3,     0,    55,    79,     3,     0,
    54,    79,     3,     0,    52,    79,     3,     0,    53,    79,
     3,     0,    48,    79,     3,     0,    50,    79,     3,     0,
    51,    79,     3,     0,    49,    79,     3,     0,    45,    79,
     3,     0,    46,    79,     3,     0,    47,    79,     3,     0,
    43,    79,     3,     0,    44,    79,     3,     0,    79,     3,
     0,    41,    79,     3,     0,    27,     3,     0,     3,     0,
     4,     0,     5,     0,    74,     0,    73,    65,    74,     0,
    79,     0,    75,     0,    77,     0,    76,     0,    76,    66,
    56,     0,    76,    66,    55,    79,     0,    56,     0,    55,
    79,     0,    79,    66,    55,    79,     0,    79,    66,    56,
     0,    79,    66,    79,     0,    79,    66,    58,    57,    79,
     0,    79,    66,    58,     0,    58,    57,    79,    66,    79,
     0,    58,    57,    79,    66,    58,    57,    79,     0,    58,
    57,    79,    66,    58,     0,    58,    57,    79,     0,    58,
     0,    78,    41,    79,     0,    59,    78,     0,     4,     0,
     6,     0,     7,     0,    72,     0,    72,    64,    73,    67,
     0,    79,    56,    79,     0,    79,    57,    79,     0,    79,
    58,    79,     0,    79,    59,    79,     0,    79,    60,    79,
     0,    79,    62,    79,     0,    79,    55,    79,     0,    79,
    54,    79,     0,    79,    52,    79,     0,    79,    53,    79,
     0,    79,    48,    79,     0,    79,    50,    79,     0,    79,
    51,    79,     0,    79,    49,    79,     0,    79,    45,    79,
     0,    79,    46,    79,     0,    79,    47,    79,     0,    79,
    43,    79,     0,    79,    44,    79,     0,    68,    80,    69,
     0,    64,    79,    67,     0,    79,    64,    73,    67,     0,
    57,    79,     0,    79,     0,    80,    65,    79,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    58,    60,    63,    70,    74,    78,    82,    86,    90,    94,
    98,   102,   106,   110,   114,   118,   122,   126,   130,   134,
   138,   142,   147,   152,   155,   158,   161,   165,   167,   170,
   173,   177,   179,   180,   183,   189,   190,   192,   194,   196,
   199,   203,   211,   213,   214,   216,   218,   220,   221,   224,
   227,   231,   235,   237,   239,   241,   243,   245,   247,   249,
   251,   253,   255,   257,   259,   261,   263,   265,   267,   269,
   271,   273,   275,   277,   280,   283,   285,   287,   290,   295,
   298
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","NEWLINE",
"C_ID","S_ID","NUMBER","STR","INCLUDE","REPORT","IF","THEN","ELSE","FOR","REPEAT",
"UNTIL","WHILE","DO","CASE","ENDCASE","NCASE","SUBR","ENDSUBR","FUNC","ENDFUNC",
"BLOCK","ENDBLOCK","RETURN","BREAK","CONTINUE","RUN","BEGIN","END","PLUSIS",
"MINUSIS","TIMESIS","DIVIDEIS","POWERIS","RETALL","STRUCTTAG","ERRORSTATE","'='",
"ELLIPSIS","ANDIF","ORIF","AND","OR","XOR","GE","LE","GT","LT","EQ","NE","'<'",
"'>'","'+'","'-'","'*'","'/'","'%'","UMINUS","'^'","'.'","'('","','","':'","')'",
"'['","']'","statements","statement","var","subsc_list","subsc_or_key","subsc",
"range","key","key_param","expr","expr_list", NULL
};
#endif

static const short yyr1[] = {     0,
    70,    70,    71,    71,    71,    71,    71,    71,    71,    71,
    71,    71,    71,    71,    71,    71,    71,    71,    71,    71,
    71,    71,    71,    71,    71,    71,    71,    72,    72,    73,
    73,    74,    74,    74,    75,    75,    75,    75,    75,    75,
    75,    76,    76,    76,    76,    76,    76,    76,    76,    77,
    77,    78,    79,    79,    79,    79,    79,    79,    79,    79,
    79,    79,    79,    79,    79,    79,    79,    79,    79,    79,
    79,    79,    79,    79,    79,    79,    79,    79,    79,    80,
    80
};

static const short yyr2[] = {     0,
     1,     2,     2,     4,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     2,     3,     2,     1,     1,     1,     1,
     3,     1,     1,     1,     1,     3,     4,     1,     2,     4,
     3,     3,     5,     3,     5,     7,     5,     3,     1,     3,
     2,     1,     1,     1,     1,     4,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     4,     2,     1,
     3
};

static const short yydefact[] = {     0,
    27,    28,    29,    53,    54,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     1,
    55,     0,    26,     0,    55,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    79,     0,     0,     0,     0,     0,    80,     0,     2,     3,
     0,     0,    24,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    79,    25,    22,    23,    19,    20,    21,
    15,    18,    16,    17,    13,    14,    12,    11,     5,     6,
     7,     8,     9,    10,    77,     0,    76,     0,    28,     0,
    38,    49,     0,     0,    30,    33,    35,    34,     0,    32,
    74,    75,    71,    72,    73,    67,    70,    68,    69,    65,
    66,    64,    63,    57,    58,    59,    60,    61,    62,     0,
    81,     4,    39,     0,    52,    51,     0,    56,     0,     0,
     0,    78,    48,    31,     0,    36,    50,     0,    41,    44,
    42,     0,    37,    40,     0,    47,    45,    43,     0,    46,
     0,     0
};

static const short yydefgoto[] = {    29,
    30,    35,   114,   115,   116,   117,   118,   119,   120,    58
};

static const short yypact[] = {   193,
-32768,-32768,-32768,-32768,-32768,    30,    50,    50,    50,    50,
    50,    50,    50,    50,    50,    50,    50,    50,    50,    50,
    50,    50,    50,    50,    50,    50,    50,    50,   127,-32768,
    -2,   219,-32768,    50,   -17,   251,   277,   309,   335,   367,
   393,   425,   451,   483,   509,   541,   567,   599,   625,   657,
    41,   683,   715,   741,   773,   159,   869,    -6,-32768,-32768,
    50,    31,-32768,    50,    50,    50,    50,    50,    50,    50,
    50,    50,    50,    50,    50,    50,    50,    50,    50,    50,
    50,    50,    31,   -59,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,    50,-32768,   799,    11,    50,
-32768,    -4,    54,   -61,-32768,-32768,    17,-32768,    19,   821,
   889,   889,   906,   906,   906,    81,    81,    81,    81,    81,
    81,    64,    64,   169,   169,   -59,   -59,   -59,   -59,   -35,
   869,-32768,   869,    50,-32768,-32768,    31,-32768,   -10,    50,
    36,-32768,   845,-32768,    50,-32768,   869,    50,-32768,    27,
   869,    44,   869,   869,    50,    28,   869,   869,    50,   869,
    96,-32768
};

static const short yypgoto[] = {-32768,
    68,     2,    15,   -38,-32768,-32768,-32768,     3,     0,-32768
};


#define	YYLAST		970


static const short yytable[] = {    32,
    60,    31,    82,   147,    83,   148,    36,    37,    38,    39,
    40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
    50,    51,    52,    53,    54,    55,    56,    57,    32,   147,
    31,   152,    33,    84,   109,     3,     4,     5,    61,     2,
     3,     4,     5,   100,   155,   156,    62,     2,     3,     4,
     5,   -52,   144,     2,     3,     4,     5,   145,   106,   150,
   108,    62,   107,   121,   122,   123,   124,   125,   126,   127,
   128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
   138,   139,   149,   165,   169,   110,   111,    34,   112,   113,
   158,   159,    34,   160,    27,   172,    59,   140,    28,    27,
    34,   166,    82,    28,    83,   141,    34,    27,   154,   143,
     0,    28,     0,    27,     0,   146,     0,    28,     0,    77,
    78,    79,    80,    81,     0,    82,   171,    83,     0,     1,
     2,     3,     4,     5,    75,    76,    77,    78,    79,    80,
    81,     0,    82,   153,    83,     0,     0,     0,     0,   157,
   161,     0,     0,     6,   163,     0,     0,   164,     0,     0,
     0,   167,     0,     0,   168,     0,     0,     7,   170,     8,
     9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
    19,    20,    21,    22,    23,    24,    25,     0,    26,     0,
    27,     0,     0,     0,    28,     1,     2,     3,     4,     5,
     0,    64,    65,    66,    67,    68,    69,    70,    71,    72,
    73,    74,    75,    76,    77,    78,    79,    80,    81,     6,
    82,    63,    83,     0,     0,   105,    79,    80,    81,     0,
    82,     0,    83,     7,     0,     8,     9,    10,    11,    12,
    13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
    23,    24,    25,    85,    26,     0,    27,     0,     0,     0,
    28,    64,    65,    66,    67,    68,    69,    70,    71,    72,
    73,    74,    75,    76,    77,    78,    79,    80,    81,    86,
    82,     0,    83,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    64,    65,    66,    67,    68,    69,    70,
    71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
    81,    87,    82,     0,    83,     0,     0,     0,     0,    64,
    65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
    75,    76,    77,    78,    79,    80,    81,    88,    82,     0,
    83,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    64,    65,    66,    67,    68,    69,    70,    71,    72,
    73,    74,    75,    76,    77,    78,    79,    80,    81,    89,
    82,     0,    83,     0,     0,     0,     0,    64,    65,    66,
    67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
    77,    78,    79,    80,    81,    90,    82,     0,    83,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    64,
    65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
    75,    76,    77,    78,    79,    80,    81,    91,    82,     0,
    83,     0,     0,     0,     0,    64,    65,    66,    67,    68,
    69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
    79,    80,    81,    92,    82,     0,    83,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    64,    65,    66,
    67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
    77,    78,    79,    80,    81,    93,    82,     0,    83,     0,
     0,     0,     0,    64,    65,    66,    67,    68,    69,    70,
    71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
    81,    94,    82,     0,    83,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    64,    65,    66,    67,    68,
    69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
    79,    80,    81,    95,    82,     0,    83,     0,     0,     0,
     0,    64,    65,    66,    67,    68,    69,    70,    71,    72,
    73,    74,    75,    76,    77,    78,    79,    80,    81,    96,
    82,     0,    83,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    64,    65,    66,    67,    68,    69,    70,
    71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
    81,    97,    82,     0,    83,     0,     0,     0,     0,    64,
    65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
    75,    76,    77,    78,    79,    80,    81,    98,    82,     0,
    83,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    64,    65,    66,    67,    68,    69,    70,    71,    72,
    73,    74,    75,    76,    77,    78,    79,    80,    81,    99,
    82,     0,    83,     0,     0,     0,     0,    64,    65,    66,
    67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
    77,    78,    79,    80,    81,   101,    82,     0,    83,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    64,
    65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
    75,    76,    77,    78,    79,    80,    81,   102,    82,     0,
    83,     0,     0,     0,     0,    64,    65,    66,    67,    68,
    69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
    79,    80,    81,   103,    82,     0,    83,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    64,    65,    66,
    67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
    77,    78,    79,    80,    81,   104,    82,     0,    83,     0,
     0,     0,     0,    64,    65,    66,    67,    68,    69,    70,
    71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
    81,   142,    82,     0,    83,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    64,    65,    66,    67,    68,
    69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
    79,    80,    81,     0,    82,     0,    83,     0,     0,     0,
     0,    64,    65,    66,    67,    68,    69,    70,    71,    72,
    73,    74,    75,    76,    77,    78,    79,    80,    81,     0,
    82,     0,    83,    64,    65,    66,    67,    68,    69,    70,
    71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
    81,     0,    82,     0,    83,     0,   151,    64,    65,    66,
    67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
    77,    78,    79,    80,    81,     0,    82,     0,    83,     0,
   162,    64,    65,    66,    67,    68,    69,    70,    71,    72,
    73,    74,    75,    76,    77,    78,    79,    80,    81,     0,
    82,     0,    83,    66,    67,    68,    69,    70,    71,    72,
    73,    74,    75,    76,    77,    78,    79,    80,    81,     0,
    82,     0,    83,    69,    70,    71,    72,    73,    74,    75,
    76,    77,    78,    79,    80,    81,     0,    82,     0,    83
};

static const short yycheck[] = {     0,
     3,     0,    62,    65,    64,    67,     7,     8,     9,    10,
    11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
    21,    22,    23,    24,    25,    26,    27,    28,    29,    65,
    29,    67,     3,    34,     4,     5,     6,     7,    41,     4,
     5,     6,     7,     3,    55,    56,    64,     4,     5,     6,
     7,    41,    57,     4,     5,     6,     7,     4,    65,    41,
    61,    64,    69,    64,    65,    66,    67,    68,    69,    70,
    71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
    81,    82,    66,    57,    57,    55,    56,    57,    58,    59,
    55,    56,    57,    58,    64,     0,    29,    83,    68,    64,
    57,    58,    62,    68,    64,   106,    57,    64,   147,   110,
    -1,    68,    -1,    64,    -1,   113,    -1,    68,    -1,    56,
    57,    58,    59,    60,    -1,    62,     0,    64,    -1,     3,
     4,     5,     6,     7,    54,    55,    56,    57,    58,    59,
    60,    -1,    62,   144,    64,    -1,    -1,    -1,    -1,   150,
   151,    -1,    -1,    27,   155,    -1,    -1,   158,    -1,    -1,
    -1,   162,    -1,    -1,   165,    -1,    -1,    41,   169,    43,
    44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
    54,    55,    56,    57,    58,    59,    60,    -1,    62,    -1,
    64,    -1,    -1,    -1,    68,     3,     4,     5,     6,     7,
    -1,    43,    44,    45,    46,    47,    48,    49,    50,    51,
    52,    53,    54,    55,    56,    57,    58,    59,    60,    27,
    62,     3,    64,    -1,    -1,    67,    58,    59,    60,    -1,
    62,    -1,    64,    41,    -1,    43,    44,    45,    46,    47,
    48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
    58,    59,    60,     3,    62,    -1,    64,    -1,    -1,    -1,
    68,    43,    44,    45,    46,    47,    48,    49,    50,    51,
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
    56,    57,    58,    59,    60,     3,    62,    -1,    64,    -1,
    -1,    -1,    -1,    43,    44,    45,    46,    47,    48,    49,
    50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
    60,     3,    62,    -1,    64,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    43,    44,    45,    46,    47,
    48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
    58,    59,    60,    -1,    62,    -1,    64,    -1,    -1,    -1,
    -1,    43,    44,    45,    46,    47,    48,    49,    50,    51,
    52,    53,    54,    55,    56,    57,    58,    59,    60,    -1,
    62,    -1,    64,    43,    44,    45,    46,    47,    48,    49,
    50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
    60,    -1,    62,    -1,    64,    -1,    66,    43,    44,    45,
    46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
    56,    57,    58,    59,    60,    -1,    62,    -1,    64,    -1,
    66,    43,    44,    45,    46,    47,    48,    49,    50,    51,
    52,    53,    54,    55,    56,    57,    58,    59,    60,    -1,
    62,    -1,    64,    45,    46,    47,    48,    49,    50,    51,
    52,    53,    54,    55,    56,    57,    58,    59,    60,    -1,
    62,    -1,    64,    48,    49,    50,    51,    52,    53,    54,
    55,    56,    57,    58,    59,    60,    -1,    62,    -1,    64
};
#define YYPURE 1

/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison.simple"
/* This file comes from bison-1.27.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

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
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 216 "/usr/share/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
int yyparse (void *);
#else
int yyparse (void);
#endif
#endif

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
  int yyfree_stacks = 0;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 3:
#line 65 "calculator.c"
{ startList(ans);
		  /* ana_replace(ans, eval(installSubsc($1))); */
		  ana_replace(ans,
			      eval(newSymbol(ANA_EXTRACT, -ANA_RANGE, yyvsp[-1])));
		  ana_type(1, &ans); ;
    break;}
case 4:
#line 71 "calculator.c"
{ yyval = findVar(yyvsp[-3], 0);
		ana_replace(yyval, yyvsp[-1]);
		ana_type(1, &ans); ;
    break;}
case 5:
#line 75 "calculator.c"
{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_ADD,
		ans, yyvsp[-1])));
		ana_type(1, &ans); ;
    break;}
case 6:
#line 79 "calculator.c"
{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_SUB,
		ans, yyvsp[-1])));
		ana_type(1, &ans); ;
    break;}
case 7:
#line 83 "calculator.c"
{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_MUL,
		ans, yyvsp[-1])));
		ana_type(1, &ans); ;
    break;}
case 8:
#line 87 "calculator.c"
{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_DIV,
		ans, yyvsp[-1])));
		ana_type(1, &ans); ;
    break;}
case 9:
#line 91 "calculator.c"
{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_MOD,
		ans, yyvsp[-1]))); 
		ana_type(1, &ans); ;
    break;}
case 10:
#line 95 "calculator.c"
{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_POW,
		ans, yyvsp[-1]))); 
		ana_type(1, &ans); ;
    break;}
case 11:
#line 99 "calculator.c"
{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_MAX,
		ans, yyvsp[-1]))); 
		ana_type(1, &ans); ;
    break;}
case 12:
#line 103 "calculator.c"
{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_MIN,
		ans, yyvsp[-1]))); 
		ana_type(1, &ans); ;
    break;}
case 13:
#line 107 "calculator.c"
{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_EQ,
		ans, yyvsp[-1]))); 
		ana_type(1, &ans); ;
    break;}
case 14:
#line 111 "calculator.c"
{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_NE,
		ans, yyvsp[-1]))); 
		ana_type(1, &ans); ;
    break;}
case 15:
#line 115 "calculator.c"
{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_GE,
		ans, yyvsp[-1]))); 
		ana_type(1, &ans); ;
    break;}
case 16:
#line 119 "calculator.c"
{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_GT,
		ans, yyvsp[-1]))); 
		ana_type(1, &ans); ;
    break;}
case 17:
#line 123 "calculator.c"
{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_LT,
		ans, yyvsp[-1]))); 
		ana_type(1, &ans); ;
    break;}
case 18:
#line 127 "calculator.c"
{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_LE,
		ans, yyvsp[-1]))); 
		ana_type(1, &ans); ;
    break;}
case 19:
#line 131 "calculator.c"
{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_AND,
		ans, yyvsp[-1]))); 
		ana_type(1, &ans); ;
    break;}
case 20:
#line 135 "calculator.c"
{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_OR,
		ans, yyvsp[-1]))); 
		ana_type(1, &ans); ;
    break;}
case 21:
#line 139 "calculator.c"
{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_XOR,
		ans, yyvsp[-1]))); 
		ana_type(1, &ans); ;
    break;}
case 22:
#line 144 "calculator.c"
{ ana_replace(ans, eval(newSymbol(ANA_IF_OP,
		ANA_ANDIF, ans, yyvsp[-1]))); 
		ana_type(1, &ans); ;
    break;}
case 23:
#line 149 "calculator.c"
{ ana_replace(ans, eval(newSymbol(ANA_IF_OP, ANA_ORIF,
		ans, yyvsp[-1]))); 
		ana_type(1, &ans); ;
    break;}
case 24:
#line 153 "calculator.c"
{ ana_replace(ans, yyvsp[-1]);
		ana_type(1, &ans); ;
    break;}
case 25:
#line 156 "calculator.c"
{ ana_replace(ans, yyvsp[-1]);
		ana_type(1, &ans); ;
    break;}
case 26:
#line 159 "calculator.c"
{ puts("Returning from calculator mode...");
		YYACCEPT; ;
    break;}
case 27:
#line 162 "calculator.c"
{ ana_type(1, &ans); ;
    break;}
case 30:
#line 172 "calculator.c"
{ startList(yyvsp[0]); ;
    break;}
case 31:
#line 174 "calculator.c"
{ pushList(yyvsp[0]); ;
    break;}
case 36:
#line 189 "calculator.c"
{ sym[yyvsp[-2]].spec.evb.args[2] = 1;  yyval = yyvsp[-2]; ;
    break;}
case 37:
#line 191 "calculator.c"
{ sym[yyvsp[-3]].spec.evb.args[3] = yyvsp[0]; yyval = yyvsp[-3]; ;
    break;}
case 38:
#line 192 "calculator.c"
{ yyval = newSymbol(ANA_PRE_RANGE, ANA_ZERO, -ANA_ONE);
			  sym[yyval].spec.evb.args[2] = 1; ;
    break;}
case 39:
#line 194 "calculator.c"
{ yyval = newSymbol(ANA_PRE_RANGE, ANA_ZERO, -ANA_ONE);
			  sym[yyval].spec.evb.args[3] = yyvsp[0]; ;
    break;}
case 40:
#line 197 "calculator.c"
{ yyval = newSymbol(ANA_PRE_RANGE, yyvsp[-3], ANA_ZERO);
			  sym[yyval].spec.evb.args[3] = yyvsp[0]; ;
    break;}
case 41:
#line 199 "calculator.c"
{ yyval = newSymbol(ANA_PRE_RANGE, yyvsp[-2], ANA_ZERO);
			  sym[yyval].spec.evb.args[2] = 1; ;
    break;}
case 42:
#line 210 "calculator.c"
{ yyval = newSymbol(ANA_PRE_RANGE, yyvsp[-2], yyvsp[0]); ;
    break;}
case 43:
#line 212 "calculator.c"
{ yyval = newSymbol(ANA_PRE_RANGE, yyvsp[-4], -yyvsp[0]); ;
    break;}
case 44:
#line 213 "calculator.c"
{ yyval = newSymbol(ANA_PRE_RANGE, yyvsp[-2], -ANA_ONE); ;
    break;}
case 45:
#line 215 "calculator.c"
{ yyval = newSymbol(ANA_PRE_RANGE, -yyvsp[-2], yyvsp[0]); ;
    break;}
case 46:
#line 217 "calculator.c"
{ yyval = newSymbol(ANA_PRE_RANGE, -yyvsp[-4], -yyvsp[0]); ;
    break;}
case 47:
#line 219 "calculator.c"
{ yyval = newSymbol(ANA_PRE_RANGE, -yyvsp[-2], -ANA_ONE); ;
    break;}
case 48:
#line 220 "calculator.c"
{ yyval = newSymbol(ANA_PRE_RANGE, -yyvsp[0], ANA_ZERO); ;
    break;}
case 49:
#line 221 "calculator.c"
{ yyval = newSymbol(ANA_PRE_RANGE, -ANA_ONE, ANA_ZERO); ;
    break;}
case 50:
#line 226 "calculator.c"
{ yyval = newSymbol(ANA_KEYWORD, yyvsp[-2], yyvsp[0]); ;
    break;}
case 51:
#line 228 "calculator.c"
{ yyval = newSymbol(ANA_KEYWORD, yyvsp[0], ANA_ONE); ;
    break;}
case 54:
#line 238 "calculator.c"
{ yyval = newSymbol(ANA_FIXED_STRING, yyvsp[0]); ;
    break;}
case 55:
#line 240 "calculator.c"
{ yyval = findVar(yyvsp[0], 0); ;
    break;}
case 56:
#line 242 "calculator.c"
{ yyval = eval(newSymbol(ANA_EXTRACT, -ANA_RANGE, yyvsp[-3])); ;
    break;}
case 57:
#line 244 "calculator.c"
{ yyval = eval(newSymbol(ANA_BIN_OP, ANA_ADD, yyvsp[-2], yyvsp[0])); ;
    break;}
case 58:
#line 246 "calculator.c"
{ yyval = eval(newSymbol(ANA_BIN_OP, ANA_SUB, yyvsp[-2], yyvsp[0])); ;
    break;}
case 59:
#line 248 "calculator.c"
{ yyval = eval(newSymbol(ANA_BIN_OP, ANA_MUL, yyvsp[-2], yyvsp[0])); ;
    break;}
case 60:
#line 250 "calculator.c"
{ yyval = eval(newSymbol(ANA_BIN_OP, ANA_DIV, yyvsp[-2], yyvsp[0])); ;
    break;}
case 61:
#line 252 "calculator.c"
{ yyval = eval(newSymbol(ANA_BIN_OP, ANA_MOD, yyvsp[-2], yyvsp[0])); ;
    break;}
case 62:
#line 254 "calculator.c"
{ yyval = eval(newSymbol(ANA_BIN_OP, ANA_POW, yyvsp[-2], yyvsp[0])); ;
    break;}
case 63:
#line 256 "calculator.c"
{ yyval = eval(newSymbol(ANA_BIN_OP, ANA_MAX, yyvsp[-2], yyvsp[0])); ;
    break;}
case 64:
#line 258 "calculator.c"
{ yyval = eval(newSymbol(ANA_BIN_OP, ANA_MIN, yyvsp[-2], yyvsp[0])); ;
    break;}
case 65:
#line 260 "calculator.c"
{ yyval = eval(newSymbol(ANA_BIN_OP, ANA_EQ, yyvsp[-2], yyvsp[0])); ;
    break;}
case 66:
#line 262 "calculator.c"
{ yyval = eval(newSymbol(ANA_BIN_OP, ANA_NE, yyvsp[-2], yyvsp[0])); ;
    break;}
case 67:
#line 264 "calculator.c"
{ yyval = eval(newSymbol(ANA_BIN_OP, ANA_GE, yyvsp[-2], yyvsp[0])); ;
    break;}
case 68:
#line 266 "calculator.c"
{ yyval = eval(newSymbol(ANA_BIN_OP, ANA_GT, yyvsp[-2], yyvsp[0])); ;
    break;}
case 69:
#line 268 "calculator.c"
{ yyval = eval(newSymbol(ANA_BIN_OP, ANA_LT, yyvsp[-2], yyvsp[0])); ;
    break;}
case 70:
#line 270 "calculator.c"
{ yyval = eval(newSymbol(ANA_BIN_OP, ANA_LE, yyvsp[-2], yyvsp[0])); ;
    break;}
case 71:
#line 272 "calculator.c"
{ yyval = eval(newSymbol(ANA_BIN_OP, ANA_AND, yyvsp[-2], yyvsp[0])); ;
    break;}
case 72:
#line 274 "calculator.c"
{ yyval = eval(newSymbol(ANA_BIN_OP, ANA_OR, yyvsp[-2], yyvsp[0])); ;
    break;}
case 73:
#line 276 "calculator.c"
{ yyval = eval(newSymbol(ANA_BIN_OP, ANA_XOR, yyvsp[-2], yyvsp[0])); ;
    break;}
case 74:
#line 279 "calculator.c"
{ yyval = eval(newSymbol(ANA_IF_OP, ANA_ANDIF, yyvsp[-2], yyvsp[0])); ;
    break;}
case 75:
#line 282 "calculator.c"
{ yyval = eval(newSymbol(ANA_IF_OP, ANA_ORIF, yyvsp[-2], yyvsp[0])); ;
    break;}
case 76:
#line 284 "calculator.c"
{ yyval = eval(newSymbol(ANA_INT_FUNC, ANA_CONCAT_FUN)); ;
    break;}
case 77:
#line 286 "calculator.c"
{ yyval = yyvsp[-1]; ;
    break;}
case 78:
#line 288 "calculator.c"
{ pushList(yyvsp[-3]);
		  yyval = eval(newSymbol(ANA_INT_FUNC, ANA_SUBSC_FUN)); ;
    break;}
case 79:
#line 291 "calculator.c"
{ startList(yyvsp[0]);
		yyval = eval(newSymbol(ANA_INT_FUNC, ANA_NEG_FUN)); ;
    break;}
case 80:
#line 297 "calculator.c"
{ startList(yyvsp[0]); ;
    break;}
case 81:
#line 299 "calculator.c"
{ pushList(yyvsp[0]); ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 542 "/usr/share/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 1;
}
#line 302 "calculator.c"


static char rcsid[] __attribute__ ((unused)) =
  "$Id";
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int	ans;

int calcerror(char *s)
/* reports parser errors - required by calcparse() */
{
  extern int	anaerror(char *);

  return anaerror(s);
}
/*----------------------------------------------------------------------*/
int calclex(YYSTYPE *lvalp)
/* returns semantic value of next red token in *lvalp and the lexical
 value as function return value */
{
  extern int	analex(YYSTYPE *);

  return analex(lvalp);
}
/*----------------------------------------------------------------------*/
int ana_calculator(int narg, int ps[])
/* go into calculator mode */
{
  extern int	calculatorMode;
  extern char	inHistoryBuffer;
  int	oldhb;
  int	calcparse(void);

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
