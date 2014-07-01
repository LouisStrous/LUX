%{
/* This is file calculator.c.

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include "editor.h"
#include "action.h"
#include "anaparser.h"

extern Int	ans;

#define startList(x)	{ pushList(ANA_NEW_LIST); pushList(x); }
				/* start a new list */
void	pushList(int16_t symNum),	/* push symbol number onto list stack */
	swapList(Int, Int),	/* swap items in the list stack */
	away(void);
int16_t	popList(void);		/* pop an item from the list stack's top */
Int	stackListLength(void),	/* return length of list at top of stack */
	isInternalSubr(Int),	/* 1 if symbol is internal subroutine */
	installExec(void),
	findSym(Int, hashTableEntry *[], Int),
	installSubsc(Int),
	anaerror(char *, Int, ...), ana_replace(Int, Int), ana_type(Int, Int []),
	newSymbol(Int, ...);
Int	yyerror(char *), yylex(YYSTYPE *);
%}

%pure_parser

/* list of tokens that are returned by yylex and have no */
/* associativity */
%token	NEWLINE 999 C_ID S_ID NUMBER STR INCLUDE REPORT
%token	IF THEN ELSE FOR REPEAT UNTIL WHILE DO CASE
%token	ENDCASE NCASE SUBR ENDSUBR FUNC ENDFUNC BLOCK
%token	ENDBLOCK RETURN BREAK CONTINUE RUN BEGIN END
%token	PLUSIS MINUSIS TIMESIS DIVIDEIS POWERIS RETALL
%token  STRUCTTAG
%token	ERRORSTATE '=' ELLIPSIS

/* tokes returned by yylex that have associativity, in order of increasing */
/* precedence */
%left	ANDIF ORIF
%left	AND OR XOR
%left	GE LE GT LT EQ NE
%left	'<' '>'
%left	'+' '-'
%left	'*' '/' '%'
%left	UMINUS			/* "dummy" to give unitary minus proper */
				/* precedence */
%right	'^'
%left	'.' '('

%start statements

%%

statements:
	  statement
	| statements statement
;

statement:
	 var NEWLINE			/* single-argument function call */
		{ startList(ans);
		  /* ana_replace(ans, eval(installSubsc($1))); */
		  ana_replace(ans,
			      eval(newSymbol(ANA_EXTRACT, -ANA_RANGE, $1)));
		  ana_type(1, &ans); }
	| var '=' expr NEWLINE
		{ $$ = findVar($1, 0);
		ana_replace($$, $3);
		ana_type(1, &ans); }
	| '+' expr NEWLINE		/* add */
		{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_ADD,
		ans, $2)));
		ana_type(1, &ans); }
	| '-' expr NEWLINE		/* subtract */
		{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_SUB,
		ans, $2)));
		ana_type(1, &ans); }
	| '*' expr NEWLINE		/* multiply */
		{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_MUL,
		ans, $2)));
		ana_type(1, &ans); }
	| '/' expr NEWLINE		/* divide */
		{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_DIV,
		ans, $2)));
		ana_type(1, &ans); }
 	| '%' expr NEWLINE
		{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_MOD,
		ans, $2))); 
		ana_type(1, &ans); } 
	| '^' expr NEWLINE
		{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_POW,
		ans, $2))); 
		ana_type(1, &ans); } 
	| '>' expr NEWLINE
		{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_MAX,
		ans, $2))); 
		ana_type(1, &ans); } 
	| '<' expr NEWLINE
		{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_MIN,
		ans, $2))); 
		ana_type(1, &ans); } 
	| EQ expr NEWLINE	
		{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_EQ,
		ans, $2))); 
		ana_type(1, &ans); } 
	| NE expr NEWLINE	
		{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_NE,
		ans, $2))); 
		ana_type(1, &ans); } 
	| GE expr NEWLINE	
		{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_GE,
		ans, $2))); 
		ana_type(1, &ans); } 
	| GT expr NEWLINE	
		{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_GT,
		ans, $2))); 
		ana_type(1, &ans); } 
	| LT expr NEWLINE	
		{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_LT,
		ans, $2))); 
		ana_type(1, &ans); } 
	| LE expr NEWLINE	
		{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_LE,
		ans, $2))); 
		ana_type(1, &ans); } 
	| AND expr NEWLINE	
		{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_AND,
		ans, $2))); 
		ana_type(1, &ans); } 
	| OR expr NEWLINE	
		{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_OR,
		ans, $2))); 
		ana_type(1, &ans); } 
	| XOR expr NEWLINE	
		{ ana_replace(ans, eval(newSymbol(ANA_BIN_OP, ANA_XOR,
		ans, $2))); 
		ana_type(1, &ans); } 
	| ANDIF expr NEWLINE	/* if left expr equals zero, then right */
				/* one isn't evaluated */
		{ ana_replace(ans, eval(newSymbol(ANA_IF_OP,
		ANA_ANDIF, ans, $2))); 
		ana_type(1, &ans); } 
	| ORIF expr NEWLINE	/* if left expr is unequal to zero, */
				/* then right one isn't evaluated */
		{ ana_replace(ans, eval(newSymbol(ANA_IF_OP, ANA_ORIF,
		ans, $2))); 
		ana_type(1, &ans); } 
	| expr NEWLINE
		{ ana_replace(ans, $1);
		ana_type(1, &ans); }
	| '=' expr NEWLINE		/* assign value of variable */
		{ ana_replace(ans, $2);
		ana_type(1, &ans); }
	| RETURN NEWLINE		/* end calculator mode */
		{ puts("Returning from calculator mode...");
		YYACCEPT; }
	| NEWLINE
		{ ana_type(1, &ans); }
;

var:
	C_ID
	| S_ID
;

subsc_list:
	subsc_or_key
		{ startList($1); }
	| subsc_list ',' subsc_or_key
		{ pushList($3); }
;

subsc_or_key:
	expr
	| subsc
	| key
;

subsc:				/* a general subscript */
/* :+ indicates that the data in the current dimension must be summed */
/* and only the result returned.  :>n specifies that the data in the */
/* current dimension be redirected to the n-th dimension of the result */
/* note that + and > cannot be specified simultaneously.  LS 6aug96 */
	  range
	| range ':' '+'	{ sym[$1].spec.evb.args[2] = 1;  $$ = $1; }
	| range ':' '>' expr
			{ sym[$1].spec.evb.args[3] = $4; $$ = $1; }
	| '+'		{ $$ = newSymbol(ANA_PRE_RANGE, ANA_ZERO, -ANA_ONE);
			  sym[$$].spec.evb.args[2] = 1; }
	| '>' expr	{ $$ = newSymbol(ANA_PRE_RANGE, ANA_ZERO, -ANA_ONE);
			  sym[$$].spec.evb.args[3] = $2; }
	| expr ':' '>' expr
			{ $$ = newSymbol(ANA_PRE_RANGE, $1, ANA_ZERO);
			  sym[$$].spec.evb.args[3] = $4; }
	| expr ':' '+'  { $$ = newSymbol(ANA_PRE_RANGE, $1, ANA_ZERO);
			  sym[$$].spec.evb.args[2] = 1; }
;

range:				/* a range variable */
/* (limit1:limit2) or (single).  limit1 and limit2 may be expressions */
/* or  * - expression, where * points at one beyond the end of the */
/* data in the current dimension.  limit2 may also be *, which indicates */
/* the last element in the current dimension (i.e. equivalent to * - 1). */
/* single may be * or * - expr; the first indicates all elements in the */
/* current dimension, the second a specific single element. */
	  expr ':' expr { $$ = newSymbol(ANA_PRE_RANGE, $1, $3); }
	| expr ':' '*' '-' expr
			{ $$ = newSymbol(ANA_PRE_RANGE, $1, -$5); }
	| expr ':' '*'	{ $$ = newSymbol(ANA_PRE_RANGE, $1, -ANA_ONE); }
	| '*' '-' expr ':' expr
			{ $$ = newSymbol(ANA_PRE_RANGE, -$3, $5); }
	| '*' '-' expr ':' '*' '-' expr
			{ $$ = newSymbol(ANA_PRE_RANGE, -$3, -$7); }
	| '*' '-' expr ':' '*'
			{ $$ = newSymbol(ANA_PRE_RANGE, -$3, -ANA_ONE); }
	| '*' '-' expr	{ $$ = newSymbol(ANA_PRE_RANGE, -$3, ANA_ZERO); }
	| '*'		{ $$ = newSymbol(ANA_PRE_RANGE, -ANA_ONE, ANA_ZERO); }
;

key:
	key_param '=' expr
		{ $$ = newSymbol(ANA_KEYWORD, $1, $3); }
	| '/' key_param
		{ $$ = newSymbol(ANA_KEYWORD, $2, ANA_ONE); }
;

key_param:
	C_ID
;

expr:
	  NUMBER
	| STR
		{ $$ = newSymbol(ANA_FIXED_STRING, $1); }
	| var
		{ $$ = findVar($1, 0); }
	| var '(' subsc_list ')'	/* arbitrary-argument function call */
		{ $$ = eval(newSymbol(ANA_EXTRACT, -ANA_RANGE, $1)); }
	| expr '+' expr
		{ $$ = eval(newSymbol(ANA_BIN_OP, ANA_ADD, $1, $3)); }
        | expr '-' expr
		{ $$ = eval(newSymbol(ANA_BIN_OP, ANA_SUB, $1, $3)); }
	| expr '*' expr
		{ $$ = eval(newSymbol(ANA_BIN_OP, ANA_MUL, $1, $3)); }
	| expr '/' expr
		{ $$ = eval(newSymbol(ANA_BIN_OP, ANA_DIV, $1, $3)); }
 	| expr '%' expr
		{ $$ = eval(newSymbol(ANA_BIN_OP, ANA_MOD, $1, $3)); }
	| expr '^' expr
		{ $$ = eval(newSymbol(ANA_BIN_OP, ANA_POW, $1, $3)); }
	| expr '>' expr
		{ $$ = eval(newSymbol(ANA_BIN_OP, ANA_MAX, $1, $3)); }
	| expr '<' expr
		{ $$ = eval(newSymbol(ANA_BIN_OP, ANA_MIN, $1, $3)); }
	| expr EQ expr	
		{ $$ = eval(newSymbol(ANA_BIN_OP, ANA_EQ, $1, $3)); }
	| expr NE expr	
		{ $$ = eval(newSymbol(ANA_BIN_OP, ANA_NE, $1, $3)); }
	| expr GE expr	
		{ $$ = eval(newSymbol(ANA_BIN_OP, ANA_GE, $1, $3)); }
	| expr GT expr	
		{ $$ = eval(newSymbol(ANA_BIN_OP, ANA_GT, $1, $3)); }
	| expr LT expr	
		{ $$ = eval(newSymbol(ANA_BIN_OP, ANA_LT, $1, $3)); }
	| expr LE expr	
		{ $$ = eval(newSymbol(ANA_BIN_OP, ANA_LE, $1, $3)); }
	| expr AND expr	
		{ $$ = eval(newSymbol(ANA_BIN_OP, ANA_AND, $1, $3)); }
	| expr OR expr	
		{ $$ = eval(newSymbol(ANA_BIN_OP, ANA_OR, $1, $3)); }
	| expr XOR expr	
		{ $$ = eval(newSymbol(ANA_BIN_OP, ANA_XOR, $1, $3)); }
	| expr ANDIF expr	/* if left expr equals zero, then right */
				/* one isn't evaluated */
		{ $$ = eval(newSymbol(ANA_IF_OP, ANA_ANDIF, $1, $3)); }
	| expr ORIF expr	/* if left expr is unequal to zero, */
				/* then right one isn't evaluated */
		{ $$ = eval(newSymbol(ANA_IF_OP, ANA_ORIF, $1, $3)); }
	| '[' expr_list ']'
		{ $$ = eval(newSymbol(ANA_INT_FUNC, ANA_CONCAT_FUN)); }
	| '(' expr ')'
		{ $$ = $2; }
	| expr '(' subsc_list ')'
		{ pushList($1);
		  $$ = eval(newSymbol(ANA_INT_FUNC, ANA_SUBSC_FUN)); }
	| '-' expr	%prec UMINUS
		{ startList($2);
		$$ = eval(newSymbol(ANA_INT_FUNC, ANA_NEG_FUN)); }
;

expr_list:
	expr
		{ startList($1); }
	| expr_list ',' expr
		{ pushList($3); }
;

%%

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
