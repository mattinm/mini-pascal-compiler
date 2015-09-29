%{
#include "compiler.h"
#include <stdio.h>

void yyerror (const char *s) {
	fprintf(stderr, "%s\n", s);
}
%}

/* our lval types */
%union {
	int ival;
	double rval;
	char *id;
	char *string;
	char chval;
}

/* our tokens */
%start program
%token LPAREN RPAREN LBRACK RBRACK /* ( | ) | [ | ] */
%token DOT COMMA SEMICOLON COLON /* . | , | ; | : */
%token ASSIGNOP /* := */
%token PROGRAM PROCEDURE FUNCTION /* program | procedure | function */
%token BEGINS END /* begin | end */
%token DO WHILE /* do | while */
%token IF THEN ELSE /* if | then | else */
%token AND OR NOT /* AND | OR | NOT */ 
%token VAR ARRAY /* var | ARRAY */
%token READ READLN WRITE WRITELN /* read | readln | write | writeln */
%token <chval> ADDOP MULOP /* + - | * / m d */
%token <string> RELOP /* < > <= >= <> = */
%token <ival> INTEGER INTNO /* integer */
%token <rval> REAL REALNO /* real */
%token <id> ID /* id */

%%

program:
PROGRAM ID
;

%%