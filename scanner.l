%{
#include "compiler.h"
#include "parser.tab.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
%}

%option yylineno
%option caseless

DIGIT	[0-9]
ID 		[a-z][a-z0-9]*

%%

\( 									{DEBUG_PRINTF(("LPAREN\n")); return LPAREN; }
\) 									{DEBUG_PRINTF(("RPAREN\n")); return RPAREN; }
\[ 									{DEBUG_PRINTF(("LBRACK\n")); return LBRACK; }
\] 									{DEBUG_PRINTF(("RBRACK\n")); return RBRACK; }

\.									{DEBUG_PRINTF(("DOT\n")); return DOT; }
\,									{DEBUG_PRINTF(("COMMA\n")); return COMMA; }
\;									{DEBUG_PRINTF(("SEMICOLON\n")); return SEMICOLON; }
\:									{DEBUG_PRINTF(("COLON\n")); return COLON; }

":="								{DEBUG_PRINTF(("ASSIGNOP\n")); return ASSIGNOP; }

"program"							{DEBUG_PRINTF(("PROGRAM\n")); return PROGRAM; }
"procedure"							{DEBUG_PRINTF(("PROCEDURE\n")); return PROCEDURE; }
"function"							{DEBUG_PRINTF(("FUNCTION\n")); return FUNCTION; }

"begin"								{DEBUG_PRINTF(("BEGINS\n")); return BEGINS; }
"end"								{DEBUG_PRINTF(("END\n")); return END; }

"do"								{DEBUG_PRINTF(("DO\n")); return DO; }
"while"								{DEBUG_PRINTF(("WHILE\n")); return WHILE; }

"if"								{DEBUG_PRINTF(("IF\n")); return IF; }
"then"								{DEBUG_PRINTF(("THEN\n")); return THEN; }
"else"								{DEBUG_PRINTF(("ELSE\n")); return ELSE; }

"and"								{DEBUG_PRINTF(("AND\n")); return AND; }
"or"								{DEBUG_PRINTF(("OR\n")); return OR; }
"not"								{DEBUG_PRINTF(("NOT\n")); return NOT; }

"var"								{DEBUG_PRINTF(("VAR\n")); return VAR; }
"array"								{DEBUG_PRINTF(("ARRAY\n")); return ARRAY; }

"read"								{DEBUG_PRINTF(("READ\n")); return READ; }
"readln"							{DEBUG_PRINTF(("READLN\n")); return READLN; }
"write"								{DEBUG_PRINTF(("WRITE\n")); return WRITE; }
"writeln"							{DEBUG_PRINTF(("WRITELN\n")); return WRITELN; }

"<"|">"|"<="|">="|"<>"|"="			{yylval.string = strdup(yytext); DEBUG_PRINTF(("RELOP: %s\n", yylval.string)); return RELOP; }

"integer"							{DEBUG_PRINTF(("INTEGER\n")); return INTEGER; }
{DIGIT}+							{yylval.ival = atoi(yytext); DEBUG_PRINTF(("INTNO: %d\n", yylval.ival)); return INTNO; }

"real"								{DEBUG_PRINTF(("REAL\n")); return REAL; }
{DIGIT}+\.{DIGIT}+([eE][+-]?{DIGIT}+)?	{yylval.rval = atof(yytext); DEBUG_PRINTF(("REALNO: %f\n", yylval.rval)); return REALNO; }

{ID}								{yylval.id = strtolower(strdup(yytext)); DEBUG_PRINTF(("ID: %s\n", yylval.id)); return ID; }

[ \t\n\r]+							{ /* whitespace */ }
.									{printf("[%d] Unknown character: %s\n", yylineno, yytext); }

%%