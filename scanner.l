/*
scanner.l hold the regex information for creating the lexemes for our
compiler.
*/

%{
#include "compiler.h"	
#include "parser.tab.h"	

#include <math.h>		
#include <string.h>		
%}

%option yylineno 	
%option caseless	

%x MLCSTAR			
%x MLCBRACE 		

digit		[0-9]
real 		{digit}+\.{digit}+([eE][+-]?{digit}+)?
id 			[a-z][a-z0-9]*
whitespace	[ \t\n\r]+

%%

\( 				{DEBUG_PRINTF(("<, LPAREN>\n")); return LPAREN; }
\) 				{DEBUG_PRINTF(("<, RPAREN>\n")); return RPAREN; }
\[ 				{DEBUG_PRINTF(("<, LBRACK>\n")); return LBRACK; }
\] 				{DEBUG_PRINTF(("<, RBRACK>\n")); return RBRACK; }

\.				{DEBUG_PRINTF(("<, DOT>\n")); return DOT; }
\,				{DEBUG_PRINTF(("<, COMMA>\n")); return COMMA; }
\;				{DEBUG_PRINTF(("<, SEMICOLON>\n")); return SEMICOLON; }
\:				{DEBUG_PRINTF(("<, COLON>\n")); return COLON; }

":="			{DEBUG_PRINTF(("<, ASSIGNOP>\n")); return ASSIGNOP; }
"<"				{DEBUG_PRINTF(("LT\n")); return LT; }
">"				{DEBUG_PRINTF(("GT\n")); return GT; }
"<="			{DEBUG_PRINTF(("LTE\n")); return LTE; }
">="			{DEBUG_PRINTF(("GTE\n")); return GTE; }
"<>"			{DEBUG_PRINTF(("NEQ\n")); return NEQ; }
"="				{DEBUG_PRINTF(("EQ\n")); return EQ; }

"program"		{DEBUG_PRINTF(("PROGRAM\n")); return PROGRAM; }
"procedure"		{DEBUG_PRINTF(("PROCEDURE\n")); return PROCEDURE; }
"function"		{DEBUG_PRINTF(("FUNCTION\n")); return FUNCTION; }

"begin"			{DEBUG_PRINTF(("BEGINS\n")); return BEGINS; }
"end"			{DEBUG_PRINTF(("END\n")); return END; }

"do"			{DEBUG_PRINTF(("DO\n")); return DO; }
"while"			{DEBUG_PRINTF(("WHILE\n")); return WHILE; }

"if"			{DEBUG_PRINTF(("IF\n")); return IF; }
"then"			{DEBUG_PRINTF(("THEN\n")); return THEN; }
"else"			{DEBUG_PRINTF(("ELSE\n")); return ELSE; }

"and"			{DEBUG_PRINTF(("AND\n")); return AND; }
"or"			{DEBUG_PRINTF(("OR\n")); return OR; }
"not"			{DEBUG_PRINTF(("NOT\n")); return NOT; }

"var"			{DEBUG_PRINTF(("VAR\n")); return VAR; }
"array"			{DEBUG_PRINTF(("ARRAY\n")); return ARRAY; }

"read"			{DEBUG_PRINTF(("READ\n")); return READ; }
"readln"		{DEBUG_PRINTF(("READLN\n")); return READLN; }
"write"			{DEBUG_PRINTF(("WRITE\n")); return WRITE; }
"writeln"		{DEBUG_PRINTF(("WRITELN\n")); return WRITELN; }

"integer"		{DEBUG_PRINTF(("INTEGER\n")); return INTEGER; }
{digit}			{yylval.ival = atoi(yytext); DEBUG_PRINTF(("INTNO: %d\n", yylval.ival)); return INTNO; }

"real"			{DEBUG_PRINTF(("REAL\n")); return REAL; }
{real}			{yylval.rval = atof(yytext); DEBUG_PRINTF(("REALNO: %f\n", yylval.rval)); return REALNO; }

{id}			{yylval.id = strtolower(strdup(yytext)); DEBUG_PRINTF(("ID: %s\n", yylval.id)); return ID; }

{whitespace}	{ /* whitespace */ }

"//".*		{DEBUG_PRINT(("<%s, comment>", yytext)); /* skip comment to end of line */ }
"(*"		{BEGIN(MLCSTAR); }
"{"			{BEGIN(MLCBRACE); }		

<MLCSTAR> {
	"*)"	{BEGIN(INITIAL); }
	[^*\n]+	{ /* eat comment in chunks */ }
	"*"		{ /* eat the lone star */ }
	\n 		{ yylineno++; }
}

<MLCBRACE> {
	"}"		{BEGIN(INITIAL); }
	[^\n]+	{ /* eat comment in chunks */ }
	\n 		{ yylineno++; }
}

.				{printf("[%d] Unknown character: %s\n", yylineno, yytext); }

%%