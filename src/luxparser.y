/* LUX parser specification (bison) and auxilliary routines */
/* This parser is re-entrant and this feature is not supported by yacc. */
/* Therefore either use bison or use the provided anaparser.c and */
/* anaparser.h.  LS 10nov97 */
%{ 
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
#if HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "action.h"
#include "luxparser.h"
#include "editor.h"
#define YYERROR_VERBOSE
#define startList(x)	{ pushList(LUX_NEW_LIST); pushList(x); }
				/* start a new list */
extern Int	scrat[],	/* general-purpose scratch storage (once.h) */
		compileLevel,	/* number of nested open input files */
		executeLevel,	/* number of nested execution items */
		symbolStackIndex, /* next free slot in symbol stack */
  setup, LUX_MATMUL_FUN;
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
	luxerror(char *, Int, ...);
Int	statementDepth = 0, keepEVB = 0;
Int	yyerror(const char *), yylex(YYSTYPE *);
%}

%pure_parser			/* reentrant parser, so we can compile a
				 new routine we encounter while we're already
				 compiling something else */

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
%left	AND OR XOR '&' '|'
%left	GE LE GT LT EQ NE
%left	'<' '>'
%left	'+' '-'
%left	'*' '/' '\\' '%' '#' SMOD
%left	UMINUS			/* "dummy" to give unitary minus proper */
				/* precedence */
%right	'^'
%left	'('

%start lines

%%

/* List of rules and actions that define LUX.  Routine yyparse() calls */
/* routine yylex() which gets and dissects user input and returns a */
/* token (i.e. syntactic code, e.g. "LUX_STRING" or "NUMBER") directly, and */
/* a semantic value (i.e. a pointer to the actual data) in variable *lvalp. */
/* yyparse() then tries to match the syntactic code to the list of parser */
/* patterns given below.  By repeated substitution according to the parser */
/* description given below, yyparse() attempts to reduce the token list to */
/* the topmost unit in the parser description, which indicates that the */
/* input is valid LUX code.  When a parser pattern is recognized, then */
/* the associated action (between {} to the right of the pattern) is */
/* executed.  In the actions, $n stands for the semantic value of the n-th */
/* item in the corresponding pattern, with n starting at 1.  For each */
/* action, the semantic value of the whole action (to be used in subsequent */
/* reductions by other patterns) must be specified by the user in variable */
/* $$.  If no pattern is specified, then { $$ = $1; } is assumed. */
/* For more info, see the manual pages, manuals, or info files for BISON. */
/* BISON is mostly the same as YACC, except that BISON allows recursive
   parsers where YACC does not.*/

lines:				/* an LUX parsing session */
next_line {			/* a statement or a newline */
  if (compileOnly && $1)	/* if we're just compiling (and not
				   immediately executing, and the statement
				   is not a newline or an error, then store
				   it at the start of a new list */
    startList($1);
  /*  if (debugLine) */		/* if we're in debugging mode (dbg> prompt)
				   then we return after the first statement */
  /* YYACCEPT; */			/* return */
}

| lines next_line {		/* next member in a set of statements and
				   newlines */
  if (compileOnly && $2)	/* if we're just compiling and the statement
				   is not a newline or error, then add to
				   the list */
    pushList($2);
}
;

next_line:			/* one complete LUX statement, or an */
				/* empty line */
  statement {			/* $1 > 0 indicates succesful execution.
				   $1 < 0 indicates an error or a premature
				   end to a loop structure (CONTINUE,
				   BREAK, RETURN) */
  if (!compileOnly) {		/* not just compiling */
    if ($1 > 0) {		/* statement OK */
      $1 = execute($1);		/* execute it */
    }
    cleanUp(-compileLevel, 0);	/* clean up temp storage and such */
    if ($1 == LOOP_RETALL	/* RETALL statement */
	&& compileLevel) {	/* not at the main execution level */
      puts("RETALL - return control to user");
      away();			/* clean up aborted execution */
      YYABORT;
    }
  }
  if ($1 < 0 && compileLevel) {	/* generic break or error condition */
    puts("Aborting");
    away();
    YYABORT;
  }
}
| NEWLINE {	/* a newline; newlines are only passed to the parser by
		   yylex() if disableNewline is equal to 0 */
    if (debugLine)		/* if this is a dbg> line then we quit after
				   the first line */
      YYACCEPT;
    $$ = 0;			/* else we ignore it */
}	
;

statement:			/* one LUX statement */
  assignment			/* an assignment */
| routine_execution		/* a subroutine call */
| routine_definition		/* a subroutine definition */
| selection			/* a selection statement */
| loop				/* a loop statement */
| RETURN opt_arg {		/* a RETURN statement */
  $$ = newSymbol(LUX_EVB, EVB_RETURN, $2);
}
| begingroup { disableNewline++; } statement_list endgroup {
  /* a statement block */
  /* after the initial {, more input is needed to complete the statement.
     the disableNewline++ statement ensures that no newlines are recognized
     while the statement block is assembled. */
  if ($3 >= 0)			/* statement list is OK */
    $$ = newSymbol(LUX_EVB, EVB_BLOCK);
  else				/* statement list had some error */
    $$ = LUX_ERROR;
  statementDepth--;		/* was incremented by statement_list */
  disableNewline--;		/* back to initial */
}
| BREAK			{ $$ = LOOP_BREAK; }	/* a BREAK statement */
| CONTINUE		{ $$ = LOOP_CONTINUE; } /* a CONTINUE statement */
| INCLUDE {			/* a @file statement */
  $$ = newSymbol(LUX_EVB, EVB_FILE, $1, FILE_INCLUDE);
}
| REPORT {			/* a @@file statement */
  $$ = newSymbol(LUX_EVB, EVB_FILE, $1, FILE_REPORT);
}
| RETALL		{ $$ = LOOP_RETALL; }	/* a RETALL statement */
| RUN ',' var		{ $$ = newBlockSymbol($3); } /* a RUN statement */
| RUN var		{ $$ = newBlockSymbol($2); } /* Dick's RUN statement */
| error {			/* some error */
  puts("Illegal statement");	/* generate message */
  errorState = 1;		/* signal the error */
  YYABORT;			/* quite this parse */
}
| ERRORSTATE		{ YYABORT; }	/* some error in yylex()? */
;

opt_arg:
/* empty */ { $$ = 0; }
| ',' expr { $$ = $2; }
;

begingroup:
  BEGIN | '{'
;

endgroup:
  END | '}'
;

assignment:			/* an assignment statement */
  lhs '=' expr {		/* simple assignment */
  $$ = newSymbol(LUX_EVB, EVB_REPLACE, $1, $3);
}
| lhs op_assign expr {		/* an operator-assignment (e.g. X += 2) */
  if (symbol_class($1) == LUX_EXTRACT)
    $$ = newSymbol(LUX_EVB, EVB_REPLACE, copySym($1),
		   newSymbol(LUX_BIN_OP, $2, $1, $3));
  else
    $$ = newSymbol(LUX_EVB, EVB_REPLACE, $1,
		   newSymbol(LUX_BIN_OP, $2, $1, $3));
}
;

tag_list:
  STRUCTTAG		{ startList($1); }
| tag_list STRUCTTAG	{ pushList($2); }
;

member_spec:
/* $$ counts the number of separate element extraction lists */
  '(' subsc_list ')'	{ pushList(LUX_RANGE);  $$ = 1; }
| '(' subsc_list error {
  if ((setup & 1024) == 0)
    puts("Unbalanced ()");
  yyerrok;
  pushList(LUX_RANGE);
  $$ = 1;
}
| tag_list		{ pushList(LUX_LIST);  $$ = 1; }
| member_spec '(' subsc_list ')' { pushList(LUX_RANGE);  $$ = $1 + 1; }
| member_spec '(' subsc_list error {
  if ((setup & 1024) == 0)
    puts("Unbalanced ()");
  yyerrok;
  pushList(LUX_RANGE);
  $$ = $1 + 1;
}
| member_spec tag_list	{ pushList(LUX_LIST);  $$ = $1 + 1; }
;

lhs:				/* an expression that can be used as
				   the left-hand side in an assignment */
var member_spec {
  pushList($2);			/* the number of element extraction lists */
  pushList(-$1);		/* minus indicates "var" */
  $$ = newSymbol(LUX_EXTRACT);
}
| var			{ $$ = findVar($1, curContext); }
;

var:				/* a general variable; C_ID is a common
				   identifier, and S_ID a special one
				   (starting with # or ! or $) */
  C_ID
| S_ID
;

struct_list:			/* a list of structure elements */
  struct_element {
    pushList(LUX_NEW_LIST);	/* the stack contents is now:
				   key expr LUX_NEW_LIST */
    swapList(1, 2);		/* reverse stack contents to: */
    swapList(2, 3);		/* LUX_NEW_LIST key expr */
}

| struct_list ',' struct_element
;

struct_element:			/* a structure element: KEY:VALUE or VALUE */
  C_ID ':' expr {
    pushList($1);
    pushList($3);
}
| expr {
    pushList(-1);
    pushList($1);
}
;

expr:				/* a general expression */
  NUMBER			/* a number */
| STR {				/* a string */
  $$ = newSymbol(LUX_FIXED_STRING, $1);
}
| lhs
| var '(' ')' {			/* a function call without any arguments */
  startList(0);			/* no arguments */
  pushList(-$1);
  $$ = newSymbol(LUX_EXTRACT);
}
| expr member_spec {	/* expressions may be subscripted */
  pushList($2);			/* the number of element extraction lists */
  pushList($1);			/* the expression */
  $$ = newSymbol(LUX_EXTRACT);
}
| '&' var {		/* a variable or function/routine pointer */
  $$ = newSymbol(LUX_POINTER, $2);
}
| '(' range ')'	{		/* a range expression */
  $$ = $2;
}
| '(' range error {
  if ((setup & 1024) == 0)
    puts("Unbalanced ()");
  yyerrok;
  $$ = $2;
}
| '{' struct_list '}' {		/* a structure */
  $$ = newSymbol(LUX_PRE_LIST);
}
| '{' struct_list error {
  if ((setup & 1024) == 0)
    puts("Unbalanced {}");
  yyerrok;
  $$ = newSymbol(LUX_PRE_LIST);
}
| '{' '}' {			/* an empty list */
  pushList(LUX_NEW_LIST);
  $$ = newSymbol(LUX_PRE_LIST);
}
| '(' expr ')' {
  $$ = $2;
}
| '(' expr error {
  if ((setup & 1024) == 0)
    puts("Unbalanced ()");
  yyerrok;
  $$ = $2;
}
| '[' expr_list ']' {		/* concatenation */
  $$ = newSymbol(LUX_INT_FUNC, LUX_CONCAT_FUN);
}
| '[' expr_list error {
  if ((setup & 1024) == 0)
    puts("Unbalanced []");
  $$ = newSymbol(LUX_INT_FUNC, LUX_CONCAT_FUN);
  yyerrok;
}
/* note: if you express the following binary operations as expr bin_op expr
   with bin_op selecting between the various operators, then the rules of
   precedence don't work out the way you want.  LS 15sep98 */
| expr '+' expr {
  $$ = newSymbol(LUX_BIN_OP, LUX_ADD, $1, $3);
}
| expr '-' expr {
  $$ = newSymbol(LUX_BIN_OP, LUX_SUB, $1, $3);
}
| expr '*' expr {
  $$ = newSymbol(LUX_BIN_OP, LUX_MUL, $1, $3);
}
| expr '/' expr {
  $$ = newSymbol(LUX_BIN_OP, LUX_DIV, $1, $3);
}
| expr '\\' expr {
  $$ = newSymbol(LUX_BIN_OP, LUX_IDIV, $1, $3);
}
| expr '%' expr {
  $$ = newSymbol(LUX_BIN_OP, LUX_MOD, $1, $3);
}
| expr SMOD expr {
  $$ = newSymbol(LUX_BIN_OP, LUX_SMOD, $1, $3);
}
| expr '^' expr {
  $$ = newSymbol(LUX_BIN_OP, LUX_POW, $1, $3);
}
| expr '>' expr {
  $$ = newSymbol(LUX_BIN_OP, LUX_MAX, $1, $3);
}
| expr '<' expr {
  $$ = newSymbol(LUX_BIN_OP, LUX_MIN, $1, $3);
}
| expr EQ expr {
  $$ = newSymbol(LUX_BIN_OP, LUX_EQ, $1, $3);
}
| expr NE expr {
  $$ = newSymbol(LUX_BIN_OP, LUX_NE, $1, $3);
}
| expr GE expr {
  $$ = newSymbol(LUX_BIN_OP, LUX_GE, $1, $3);
}
| expr GT expr {
  $$ = newSymbol(LUX_BIN_OP, LUX_GT, $1, $3);
}
| expr LT expr {
  $$ = newSymbol(LUX_BIN_OP, LUX_LT, $1, $3);
}
| expr LE expr {
  $$ = newSymbol(LUX_BIN_OP, LUX_LE, $1, $3);
}
| expr AND expr {
  $$ = newSymbol(LUX_BIN_OP, LUX_AND, $1, $3);
}
| expr OR expr {
  $$ = newSymbol(LUX_BIN_OP, LUX_OR, $1, $3);
}
| expr XOR expr {
  $$ = newSymbol(LUX_BIN_OP, LUX_XOR, $1, $3);
}
| expr '&' expr {		/* testing & for AND */
  $$ = newSymbol(LUX_BIN_OP, LUX_AND, $1, $3);
}
| expr '|' expr {		/* testing | for OR */
  $$ = newSymbol(LUX_BIN_OP, LUX_OR, $1, $3);
}
| expr ANDIF expr {		/* conditional and */
  $$ = newSymbol(LUX_IF_OP, LUX_ANDIF, $1, $3);
}
| expr ORIF expr {		/* conditional or */
  $$ = newSymbol(LUX_IF_OP, LUX_ORIF, $1, $3);
}
| expr '#' expr {		/* matrix multiplication */
  startList($1);
  pushList($3);
  $$ = newSymbol(LUX_INT_FUNC, LUX_MATMUL_FUN);
}
| '-' expr %prec UMINUS {
  startList($2);
  $$ = newSymbol(LUX_INT_FUNC, LUX_NEG_FUN);
}
| '+' expr %prec UMINUS {
  $$ = $2;
}
;

expr_list:			/* a list of expressions */
expr			{ startList($1); }
| expr_list ',' expr	{ pushList($3); }
;

range:				/* a range variable */
/* (limit1:limit2) or (single).  limit1 and limit2 may be expressions */
/* or  * - expression, where * points at one beyond the end of the */
/* data in the current dimension.  limit2 may also be *, which indicates */
/* the last element in the current dimension (i.e. equivalent to * - 1). */
/* single may be * or * - expr; the first indicates all elements in the */
/* current dimension, the second a specific single element. */
expr ':' expr {
  $$ = newSymbol(LUX_PRE_RANGE, $1, $3);
}
| expr ':' '*' '-' expr {
  $$ = newSymbol(LUX_PRE_RANGE, $1, -$5);
}
| expr ':' '*' {
  $$ = newSymbol(LUX_PRE_RANGE, $1, -LUX_ONE);
}
| '*' '-' expr ':' expr {
  $$ = newSymbol(LUX_PRE_RANGE, -$3, $5);
}
| '*' '-' expr ':' '*' '-' expr {
  $$ = newSymbol(LUX_PRE_RANGE, -$3, -$7);
}
| '*' '-' expr ':' '*' {
  $$ = newSymbol(LUX_PRE_RANGE, -$3, -LUX_ONE);
}
| '*' '-' expr {
  $$ = newSymbol(LUX_PRE_RANGE, -$3, LUX_ZERO);
}
| '*' {
  $$ = newSymbol(LUX_PRE_RANGE, -LUX_ONE, LUX_ZERO);
}
;
	
subsc:				/* a general subscript */
/* :+ indicates that the data in the current dimension must be summed */
/* and only the result returned.  :>n specifies that the data in the */
/* current dimension be redirected to the n-th dimension of the result */
/* note that + and > cannot be specified simultaneously.  LS 6aug96 */

range

| range ':' '+'	{
  sym[$1].spec.evb.args[2] = 1;
  $$ = $1;
}

| range ':' '>' expr {
  sym[$1].spec.evb.args[3] = $4;
  $$ = $1;
}

| '+' {
  $$ = newSymbol(LUX_PRE_RANGE, LUX_ZERO, -LUX_ONE);
  sym[$$].spec.evb.args[2] = 1;
}

| '>' expr {
  $$ = newSymbol(LUX_PRE_RANGE, LUX_ZERO, -LUX_ONE);
  sym[$$].spec.evb.args[3] = $2;
}

| expr ':' '>' expr {
  $$ = newSymbol(LUX_PRE_RANGE, $1, LUX_ZERO);
  sym[$$].spec.evb.args[3] = $4;
}

| expr ':' '+' {
  $$ = newSymbol(LUX_PRE_RANGE, $1, LUX_ZERO);
  sym[$$].spec.evb.args[2] = 1;
}
;

subsc_or_key:			/* possibly valid argument */

subsc

| expr

| key
;

subsc_list:			/* a list of subscripts */

subsc_or_key {
  startList($1);
}

| subsc_list ',' subsc_or_key {
  pushList($3);
}
;

op_assign:			/* assignments like A += B -> A = A + B */

PLUSIS {
  $$ = LUX_ADD;
}

| MINUSIS {
  $$ = LUX_SUB;
}

| TIMESIS {
  $$ = LUX_MUL;
}

| DIVIDEIS {
  $$ = LUX_DIV;
}
;

routine_execution:		/* execution of a subroutine */

C_ID s_arglist {
  $$ = newSubrSymbol($1);
}
;

s_arglist:			/* argument list for a subroutine */

',' arglist
| /* empty */ {
  pushList(LUX_NEW_LIST);
}
;

arglist:			/* general argument list */

arg {
  startList($1);
}

| arglist ',' arg {
  pushList($3);
}
;

key_param:			/* a valid key name: no # or ! or $ at start */

C_ID

;

key:				/* a key specification; /KEY is KEY=1 */
				/* and /NOKEY is KEY=0 */
key_param '=' expr {
  $$ = newSymbol(LUX_KEYWORD, $1, $3);
}

| '/' key_param	{
  $$ = newSymbol(LUX_KEYWORD, $2, LUX_ONE);
}
;

arg:				/* a general argument */

expr

| key
;

routine_definition:		/* definition of subroutine, function, */
				/* or blockroutine */
SUBR {
  disableNewline++;
} C_ID f_paramlist {
  $$ = newSymbol(LUX_SUBROUTINE, $3);
} statement_list endsubr {
  $$ = newSymbol(LUX_SUBROUTINE, -$5 - 1);
  statementDepth--;
  disableNewline--;
}

| FUNC {
  disableNewline++;
} C_ID f_paramlist {
  $$ = newSymbol(LUX_FUNCTION, $3);
} statement_list endfunc {
  $$ = newSymbol(LUX_FUNCTION, -$5 - 1);
  statementDepth--;
  disableNewline--;
}

| BLOCK {
  disableNewline++;
} C_ID {
  $$ = newSymbol(LUX_BLOCKROUTINE, $3);
} statement_list endblock {
  $$ = newSymbol(LUX_BLOCKROUTINE, -$4 - 1);
  statementDepth--;
  disableNewline--;
}
;

endsubr: 
  ENDSUBR | END
;

endfunc:
  ENDFUNC | END
; 

endblock:
  ENDBLOCK | END
;

paramlist2:

paramlist

| paramlist ',' ELLIPSIS {
  pushList(LUX_EXTEND);
}
;


s_paramlist:			/* a list of parameters for definitions */
				/* of subroutines */
/* empty */ {
  pushList(LUX_NEW_LIST);
}

| ',' paramlist2
;

paramlist:			/* a general list of parameters; */
				/* no expressions are allowed */
C_ID {
  startList($1);
}

| paramlist ',' C_ID {
  pushList($3);
}
;

statement_list:			/* a set of statements */

statement {
  startList($1);
  statementDepth++;
}

| statement_list statement {
  pushList($2);
  if ($2 == LUX_ERROR)
    $$ = LUX_ERROR;
}
;

f_paramlist:			/* a list of parameters for a function */
				/* definition; either FUNC F() or */
				/* FUNC F(x,y) or FUNC F,x,y */
'(' ')'	{			/* empty list */
  pushList(LUX_NEW_LIST);
}

| '(' paramlist2 ')'

| s_paramlist
;

selection:			/* selection statements */

IF {
  disableNewline++;
} expr opt_then {
  disableNewline--;
} statement opt_else {
  $$ = newSymbol(LUX_EVB, EVB_IF, $3, $6, $7);
}

| CASE {
  disableNewline++;
} case_list opt_case_else ENDCASE {
  pushList($4);
  $$ = newSymbol(LUX_EVB, EVB_CASE);
  disableNewline--;
}

| NCASE {
  disableNewline++;
} expr statement_list opt_case_else ENDCASE {
  pushList($5);
  pushList($3);
  statementDepth--;
  $$ = newSymbol(LUX_EVB, EVB_NCASE);
  disableNewline--;
}
;

opt_then:

/* empty */ 
| THEN
;

opt_else:			/* optional ELSE part to an IF statement */

/* empty */ {
  $$ = 0;
}

| ELSE statement {
  $$ = $2;
}
;

opt_case_else:

/* empty */ {
  $$ = 0;
}

| ELSE {
  disableNewline++;
} statement {
  $$ = $3;
  disableNewline--;
}

| ELSE {
  disableNewline++;
} ':' statement {		/* Dick's case else */
  $$ = $3;
  disableNewline--;
}
;

case_list:			/* set of cases for CASE */

expr ':' statement {
  startList($1);
  pushList($3);
}

| case_list expr ':' statement {
  pushList($2);
  pushList($4);
}
;

loop:				/* loop statements */

FOR {
  disableNewline++;
} C_ID '=' expr ',' expr opt_step opt_do {
  disableNewline--;
} statement {
  $$ = findVar($3, curContext);
  $$ = newSymbol(LUX_EVB, EVB_FOR, $$, $5, $7, $8, $11);
}

| REPEAT {
  disableNewline++;
} statement UNTIL {
  disableNewline--;
} expr {
  $$ = newSymbol(LUX_EVB, EVB_REPEAT, $3, $6);
}

| DO {
  disableNewline++;
} statement WHILE {
  disableNewline--;
} expr {
  $$ = newSymbol(LUX_EVB, EVB_DO_WHILE, $3, $6);
}

| WHILE {
  disableNewline++;
} expr {
  disableNewline--;
} opt_do statement {
  $$ = newSymbol(LUX_EVB, EVB_WHILE_DO, $3, $6);
}
;

opt_step:			/* optional STEP for FOR statement */

/* empty */ {
  $$ = LUX_ONE;
}

| ',' expr {
  $$ = $2;
}
;

opt_do:				/* optional DO Word in WHILE-DO statement */

/* empty */

| DO
;
	
%%

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

char	*ANAPrompts[] =	{	/* legal LUX prompts */
  "LUX>", "mor>", "ign>", "dbg>", "clc>"
};
#define N_ANAPROMPTS 5

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

Int yyerror(const char *s)
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
  if (allowPromptInInput)	/* allow legal LUX prompts at the very
				   beginning of input lines; i.e. ignore when
				   encountered */
    for (i = 0; i < N_ANAPROMPTS; i++)
      if (!strncmp(p, ANAPrompts[i], strlen(ANAPrompts[i]))) {
				/* found an LUX prompt */
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

  saveHistory();
  printf("\nCPUtime: %g seconds\n", ((float) clock())/CLOCKS_PER_SEC);
  puts("Quitting... Bye!");	/* farewell message */
  exit(result);
}
/*--------------------------------------------------------------*/
/* NUMBERS
   LUX allows specification of numbers of five types (LUX_BYTE, LUX_WORD, LUX_LONG,
   LUX_FLOAT, LUX_DOUBLE) and three bases (8, 10, 16).  Numbers must always
   start with a decimal digit.
   Fractional (i.e. LUX_FLOAT or LUX_DOUBLE) numbers can only be specified
   in base 10.  In integer numbers, a base specifier (if any) precedes
   a type specifier (if any).  By default, integers are in base 10 and
   of type LUX_LONG.
   Octal numbers are followed by an O.  Hexadecimal numbers are either
   preceded by 0X or followed by an X.  A final B indicates a LUX_BYTE number,
   a final W a LUX_WORD, and a final L a LUX_LONG number.
   A general LUX_FLOAT looks like this:  1.23E+4
   Either the decimal point or the exponent indicator E must be present.
   A plus sign is optional in the exponent.  A general LUX_DOUBLE looks like
   a general LUX_FLOAT, except that a D is used instead of an E.  The exponent
   indicator D must be present.
   Some examples:  0X1AB = 1AXB = LUX_BYTE hex 1A = LUX_BYTE 26
                   0X1ABL = 1ABXL = LUX_LONG hex 1AB = LUX_LONG 427
                   2E = 2. = LUX_FLOAT 2
                   2EX = 0X2E = LUX_LONG hex 2E = LUX_LONG 46
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
    if ((*lvalp = newSymbol(LUX_FIXED_NUMBER, type)) < 0) /* could not get */
				/* a new symbol for the number */
      return LUX_ERROR;		/* return error indication */
    if (*lvalp)
      switch(type) {		/* non-zero return value (??) */
				/* insert value of proper type */
	case LUX_BYTE:
	  scalar_value(*lvalp).b = (Byte) v.l;
	  break;
	case LUX_WORD:
	  scalar_value(*lvalp).w = (Word) v.l;
	  break;
	case LUX_LONG:
	  scalar_value(*lvalp).l = v.l;
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
 if ((n = isKeyWord())) {	/* a standard LUX-language key Word */
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
     return LUX_ERROR;		/* some error occurred */
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
 Int	getNewLine(char *buffer, size_t bufsize, char *prompt, char historyFlag),
   showstats(Int narg, Int ps[]), installString(char *);

 if (errorState)
   return ERRORSTATE;		/* if a syntax error occurred, then */
				/* tell the parser so. */
 /* now determine the appropriate LUX prompt to use */
 if (ignoreInput)		/* we're within a nested IGNORE-RESUME pair */
   prompt = ANAPrompts[2];	/* ign> */
 else if (debugLine)		/* debugger line (execute()) */
   prompt = ANAPrompts[3];	/* dbg> */
 else if (disableNewline)	/* need more input */
   prompt = ANAPrompts[1];	/* mor> */
 else if (calculatorMode)
   prompt = ANAPrompts[4];	/* clc> */
 else
   prompt = ANAPrompts[0];	/* default, LUX> */
 /* now get and treat the input */
 while (1) {			/* keep cycling */
   if (!*line) {		/* nothing more in current input line */
     while (!*line) {		/* try until there's something to do */
       if (len < 0)		/* EOF reached */
	 putchar('\r');		/* ensure cursor is at left */
       len = getNewLine(line, BUFSIZE, prompt, inHistoryBuffer);	/* read new line; */
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
	   prompt = ANAPrompts[0];	/* default, LUX> */
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
  luxerror("GSL error %d (%s line %d): %s", 0, gsl_errno, file, line, reason);
}
/*--------------------------------------------------------------*/
char	*programName;
int do_main(int argc, char *argv[])
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
  site(0, NULL);		/* identify this version of LUX */
  fflush(stdin);
  symbolInitialization();
  readHistory();
  stifle_history(100);          /* limit history buffer to 100 lines */
  *line = '\0';			/* start with an empty line */
  p = line;

  void gehandler(const char *, const char *, Int, Int);

  gsl_set_error_handler(&gehandler);
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
    printf("%s%s\n", ANAPrompts[0], line);
    add_history(line + 1);
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
