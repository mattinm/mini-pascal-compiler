#include "tokens.h"
#include <stdlib.h>

const char *pcsymstr[numsyms] = {
	/* End-of-Tokens */
	"oefsym",

	/* Operators */
	"idivsym",
	"modsym",
	"addsym",
	"minussym",
	"multsym",
	"divsym",

	/* Scopes */
	"programsym",
	"proceduresym",
	"functionsym",
	"beginsym",
	"endsym",

	/* Boolean operators */
	"andsym",
	"orsym",
	"notsym",
	"ltsym",
	"ltesym",
	"neqsym",
	"gtsym",
	"gtesym",
	"eqsym",

	/* Punctuation */
	"assignsym",
	"colonsym",
	"semicolonsym",
	"commasym",
	"dotsym",
	"lparensym",
	"rparensym",
	"lbracksym",
	"rbracksym",

	/* Control flow */
	"ifsym",
	"elsesym",
	"thensym",
	"dosym",
	"whilesym",

	/* Variables */
	"idsym",
	"arraysym",
	"ofsym",
	"charsym",
	"stringsym",
	"integersym",
	"realsym",
	"varsym",

	/* Constants */
	"integernosym",
	"realnosym",
	"stringvalsym",
	"charvalsym",

	/* Built-in functions */
	"chrsym",
	"ordsym",
	"readsym",
	"readlnsym",
	"writesym",
	"writelnsym",
};

pctoken *
pcnewtoken(pcsym sym, symval val, unsigned lineno) {
	pctoken *token;

	if (!(token = malloc(sizeof(*token)))) return NULL;
	token->sym = sym;
	token->val = val;
	token->lineno = lineno;

	return token;
}