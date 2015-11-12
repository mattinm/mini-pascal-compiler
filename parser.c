#include "parser.h"
#include "symtab.h"
#include <stdlib.h>
#include <stdio.h>

int pcp_block();
int pcp_statement_part();
int pcp_application();
int pcp_constant_definition();
int pcp_expression();

FILE *fp = NULL;
pctoken *token = NULL;
pctoken *nexttoken = NULL;

#define NEXTTOKEN() if (!pcp_next()) return 0
#define ADDSYM(NAME, TYPE, TOKEN) if (!pcaddsym(NAME, TYPE, (TOKEN)->val, (TOKEN)->lineno)) return 0
#define ADDTOKEN(TAIL, TOKEN) if (!(TAIL = tokenlist_add(TAIL, TOKEN))) return 0

typedef struct pctokenlist {
	pctoken *token;
	struct pctokenlist *next;
} pctokenlist;

int
pcp_error(const char *str) {
	printf("\n[%u] PARSE ERROR: %s", token->lineno, str);
	return 0;
}

int
sym_to_type(pcsym sym, symtype *type) {
	if (sym == integersym) *type = integertype;
	else if (sym == realsym) *type = realtype;
	else if (sym == stringsym) *type = stringtype;
	else if (sym == charsym) *type = chartype;
	else return pcp_error("Unknown type. Arrays and custom types not yet supported.");

	return 1;
}

pctokenlist *
tokenlist_add(pctokenlist *tail, pctoken *token) {
	pctokenlist *next;

	if (!(next = malloc(sizeof(*next)))) {
		pcp_error("Out of memory.");
		return NULL;
	}
	next->token = token;
	next->next = NULL;

	tail->next = next;
	return next;
}

/* Updates the tokens */
int
pcp_next() {
	/* get the next token */
	token = nexttoken;
	nexttoken = pcgettoken(fp);

	if (!token) {
		return pcp_error("Unexpected end of tokens.");
	}

	/* print the line from the scanner */
	printf("< %s ", pcsymstr[token->sym]);

	if (token->sym == idsym) {
		printf(", %s ", token->val.id);
	} else if (token->sym == integernosym) {
		printf(", %d ", token->val.ival);
	} else if (token->sym == realnosym) {
		printf(", %f ", token->val.rval);
	} else if (token->sym == stringvalsym) {
		printf(", %s ", token->val.str);
	} else if (token->sym == charvalsym) {
		printf(", %c ", token->val.cval);
	}

	printf(">\n");
	return 1;
}

pctoken *
pcp_const_no_id(symtype *type) {
	if (token->sym == integernosym) 	 *type = integertype;
	else if (token->sym == realnosym) 	 *type = realtype;
	else if (token->sym == charvalsym)	 *type = chartype;
	else if (token->sym == stringvalsym) *type = stringtype;
	else return NULL;

	return token;
}

int
pcp_ord() {
	printf("## ENTERING pcp_ord ##\n");

	if (token->sym != ordsym) return 0;

	NEXTTOKEN();
	if (token->sym != lparensym) return pcp_error("Expected '('");

	NEXTTOKEN();
	if (!pcp_expression()) return 0;

	NEXTTOKEN();
	if (token->sym != rparensym) return pcp_error("Expected ')'");

	return 1;
}

int
pcp_chr() {
	printf("## ENTERING pcp_chr ##\n");

	if (token->sym != chrsym) return 0;

	NEXTTOKEN();
	if (token->sym != lparensym) return pcp_error("Expected '('");

	NEXTTOKEN();
	if (!pcp_expression()) return 0;

	NEXTTOKEN();
	if (token->sym != rparensym) return pcp_error("Expected ')'");

	return 1;
}

int
pcp_factor() {
	printf("## ENTERED pcp_factor ##\n");

	if (token->sym == notsym) {
		NEXTTOKEN();
		return pcp_factor(); 
	}

	if (token->sym == idsym) {
		if (nexttoken->sym == lparensym) {
			NEXTTOKEN();
			return pcp_application();
		} if (nexttoken->sym == lbracksym) {
			return pcp_error("Arrays not yet supported.");
		}
		
		return 1;
	}

	if (token->sym == ordsym) {
		return pcp_ord();
	}

	if (token->sym == chrsym) {
		return pcp_chr();
	}

	if (token->sym == lparensym) {
		NEXTTOKEN();
		if (!pcp_expression()) return 0;

		NEXTTOKEN();
		if (token->sym != rparensym) return pcp_error("Expected ')'.");

		return 1;
	}

	return (token->sym == integernosym || token->sym == realnosym || token->sym == stringvalsym || token->sym == charvalsym);
}

int
pcp_term() {
	printf("## ENTERED pcp_term ##\n");

	if (!pcp_factor()) return 0;

	while (nexttoken->sym == multsym || nexttoken->sym == idivsym || nexttoken->sym == divsym || nexttoken->sym == andsym) {
		NEXTTOKEN();
		NEXTTOKEN();
		if (!pcp_factor()) return 0;
	}

	return 1;
}

int
pcp_simple_expression() {
	printf("## ENTERED pcp_simple_expression ##\n");

	if (!pcp_term()) return 0;

	while (nexttoken->sym == addsym || nexttoken->sym == minussym || nexttoken->sym == orsym) {
		NEXTTOKEN();
		NEXTTOKEN();
		if (!pcp_term()) return 0;
	}

	return 1;
}

int
pcp_expression() {
	printf("## ENTERED pcp_expression ##\n");

	if (!pcp_simple_expression()) return 0;

	if (nexttoken->sym == eqsym || nexttoken->sym == neqsym || nexttoken->sym == ltsym || nexttoken->sym == ltesym || nexttoken->sym == gtesym || nexttoken->sym == gtsym) {
		NEXTTOKEN();
		NEXTTOKEN();
		return pcp_simple_expression();
	}

	return 1;
}

/*int
pcp_for() {
	NEXTTOKEN();
	if (token->sym != idsym) return pcp_error("Expected ID.");

	NEXTTOKEN();
	if (token->sym != assignsym) return pcp_error("Expected ':='.");

	NEXTTOKEN();
	if (!pcp_expression()) return 0;

	NEXTTOKEN();
	if (token->sym != tosym && token->sym != downtosym) return pcp_error("Expected to or downto.");

	NEXTTOKEN();
	if (!pcp_expression()) return 0;

	NEXTTOKEN();
	if (token->sym != dosym) return pcp_error("Expected do.");

	NEXTTOKEN();
	return pcp_statement_part();
}*/

int
pcp_while() {
	printf("## ENTERED pcp_while ##\n");

	NEXTTOKEN();
	if (!pcp_expression()) return 0;

	NEXTTOKEN();
	if (token->sym != dosym) return pcp_error("Expected 'do'.");

	NEXTTOKEN();
	return pcp_statement_part();
}

int
pcp_if() {
	printf("## ENTERED pcp_if ##\n");

	NEXTTOKEN();
	if (!pcp_expression()) return 0;

	NEXTTOKEN();
	if (token->sym != thensym) return pcp_error("Expected 'then'.");

	NEXTTOKEN();
	if (!pcp_statement_part()) return 0;

	if (nexttoken->sym == elsesym) {
		NEXTTOKEN();
		NEXTTOKEN();
		return pcp_statement_part();
	}

	return 1;
}

int
pcp_write() {
	printf("## ENTERED pcp_write ##\n");
	NEXTTOKEN();
	if (token->sym != lparensym) return pcp_error("Expected '('.");

	NEXTTOKEN();
	if (!pcp_expression()) return 0;

	NEXTTOKEN();
	while (token->sym == commasym) {
		NEXTTOKEN();
		if (!pcp_expression()) return 0;
		NEXTTOKEN();
	}

	if (token->sym != rparensym) return pcp_error("Expected ')'.");
	return 1;
}

int
pcp_read() {
	printf("## ENTERED pcp_read ##\n");
	NEXTTOKEN();
	if (token->sym != lparensym) return pcp_error("Expected '('.");

	NEXTTOKEN();
	if (token->sym != idsym) return pcp_error("Expected ID.");

	NEXTTOKEN();
	while (token->sym == commasym) {
		NEXTTOKEN();
		if (token->sym != idsym) return pcp_error("Expected ID.");
		NEXTTOKEN();
	}

	if (token->sym != rparensym) return pcp_error("Expected ')'.");
	return 1;
}

int
pcp_application(pctoken *ltoken) {
	symentry *entry = pclookupsym(ltoken->val.id);
	int params = 1;

	printf("## ENTERED pcp_application ##\n");

	if (!entry || entry->type != functiontype) {
		return pcp_error("Undefined ID or unexpected type.");
	}

	NEXTTOKEN();
	if (!pcp_expression()) return 0;

	NEXTTOKEN();
	while (token->sym == commasym) {
		NEXTTOKEN();

		if (!pcp_expression()) return 0;
		++params;

		NEXTTOKEN();
	}

	if (token->sym != rparensym) return pcp_error("Expected ')'.");
	return 1;
}

int
pcp_procedure_call(pctoken *ltoken) {
	symentry *entry = pclookupsym(ltoken->val.id);
	int params = 1;

	printf("## ENTERED pcp_procedure_call ##\n");

	if (!entry || entry->type != proceduretype) {
		return pcp_error("Undefined ID or unexpected type.");
	}

	NEXTTOKEN();
	if (!pcp_expression()) return 0;

	NEXTTOKEN();
	while (token->sym == commasym) {
		NEXTTOKEN();

		if (!pcp_expression()) return 0;
		++params;

		NEXTTOKEN();
	}

	if (token->sym != rparensym) return pcp_error("Expected ')'.");
	return 1;
}

int
pcp_procedure_call_or_application(pctoken *ltoken) {
	return pcp_procedure_call(ltoken) || pcp_application(ltoken);
}

int
pcp_assign(pctoken *ltoken) {
	symentry *entry = pclookupsym(ltoken->val.id);

	printf("## ENTERED pcp_assign ##\n");

	if (!entry || (entry->type != integertype && entry->type != realtype && entry->type != chartype && entry->type != stringtype)) {
		return pcp_error("Undefined ID or unexpected type.");
	}

	NEXTTOKEN();
	return pcp_expression();
}

int
pcp_statement() {
	int success = 0;

	printf("## ENTERED pcp_statement ##\n");

	/* procedure/function call or assignment */
	if (token->sym == idsym) {
		pctoken *oldtoken = token;

		NEXTTOKEN();
		if (token->sym == lparensym) success = pcp_procedure_call_or_application(oldtoken);
		else if (token->sym == assignsym) success = pcp_assign(oldtoken);
		else if (token->sym == lbracksym) return pcp_error("Arrays not yet supported.");
		else return pcp_error("Unexpected symbol.");
	} else if (token->sym == readsym || token->sym == readlnsym) {
		success = pcp_read();
	} else if (token->sym == writesym || token->sym == writelnsym) {
		success = pcp_write();
	} else if (token->sym == ifsym) {
		success = pcp_if();
	} else if (token->sym == whilesym) {
		success = pcp_while();
	} /*else if (token->sym == forsym) {
		success = pcp_for();
	}*/ else if (token->sym == beginsym) {
		success = pcp_statement_part();
	} else {
		return pcp_error("Unexpected statement.");
	}

	if (!success) return 0;

	NEXTTOKEN();
	if (token->sym != semicolonsym) return pcp_error("Expected ';'.");

	return 1;
}

int
pcp_statement_part() {
	printf("## ENTERING pcp_statement_part ##\n");

	if (token->sym != beginsym) return pcp_error("Expected 'BEGIN'.");

	/* go through all the statements until end */
	NEXTTOKEN();
	while (token->sym != endsym) {
		if (!pcp_statement()) return 0;
		NEXTTOKEN();
	}

	return 1;
}

int
pcp_formal_parameters() {
	pctokenlist tokens = {NULL, NULL};
	pctokenlist *tail = NULL;
	pctokenlist *cur;
	pctoken *val;
	symtype type;

	printf("## ENTERING pcp_formal_parameters ##\n");

	if (token->sym != idsym) return pcp_error("Expected ID.");
	tokens.token = token;
	tail = &tokens;

	NEXTTOKEN();
	while (token->sym == commasym) {
		NEXTTOKEN();

		if (token->sym != idsym) return pcp_error("Expected ID.");
		ADDTOKEN(tail, token);

		NEXTTOKEN();
	}

	if (token->sym != colonsym) return pcp_error("Expected ':'.");

	NEXTTOKEN();
	val = token;
	if (!sym_to_type(val->sym, &type)) return 0;	

	/* add all the id's to the symbol table */
	cur = &tokens;
	while (cur != NULL) {
		if (!pcaddsym(cur->token->val.id, type, (symval)0, cur->token->lineno)) return 0;
		tail = cur;
		cur = cur->next;
		if (tail != &tokens) free(tail);
	}

	return 1;
}

int
pcp_function_declaration() {
	symtype type;

	printf("## ENTERING pcp_function_declaration ##\n");

	/* enter our new scope for the function */
	if (token->sym != idsym) return pcp_error("Expected ID.");
	if (!pcenterscope(token->val.id, functiontype, token->lineno)) return 0;

	NEXTTOKEN();
	if (token->sym != lparensym) return pcp_error("Expected '('");

	NEXTTOKEN();
	if (!pcp_formal_parameters()) return 0;

	NEXTTOKEN();
	if (token->sym != rparensym) return pcp_error("Expected ')'");

	NEXTTOKEN();
	if (token->sym != colonsym) return pcp_error("Expected ':'");

	NEXTTOKEN();
	if (!sym_to_type(token->sym, &type)) return 0;

	/* update our return type */
	/*entry->returntype = type;*/

	NEXTTOKEN();
	if (token->sym != semicolonsym) return pcp_error("Expected ';'");

	NEXTTOKEN();
	if (!pcp_block()) return 0;

	/* leave the function scope */
	return pcleavescope();
}

int
pcp_procedure_declaration() {
	printf("## ENTERING pcp_procedure_declaration ##\n");

	/* create our new scope for the new variables */
	if (token->sym != idsym) return pcp_error("Expected ID.");
	if (!pcenterscope(token->val.id, proceduretype, token->lineno)) return 0;

	NEXTTOKEN();
	if (token->sym != lparensym) return pcp_error("Expected '('");

	NEXTTOKEN();
	if (!pcp_formal_parameters()) return 0;

	NEXTTOKEN();
	if (token->sym != rparensym) return pcp_error("Expected ')'");

	NEXTTOKEN();
	if (token->sym != semicolonsym) return pcp_error("Expected ';'");

	NEXTTOKEN();
	if (!pcp_block()) return 0;

	/* leave the procedure scope */
	return pcleavescope();
}

int
pcp_procedure_and_function_definition_part() {
	printf("## ENTERING pcp_procedure_and_function_definition_part ##\n");

	if (token->sym == proceduresym) {
		NEXTTOKEN();
		pcp_procedure_declaration();
	} else if (token->sym == functionsym) {
		NEXTTOKEN();
		pcp_function_declaration();
	} else return 1;

	NEXTTOKEN();
	if (token->sym != semicolonsym) return pcp_error("Expected ';'.");

	NEXTTOKEN();
	return pcp_procedure_and_function_definition_part();
}

int
pcp_variable_definition() {
	pctokenlist tokens = {NULL, NULL};
	pctokenlist *tail = NULL;
	pctokenlist *cur;
	pctoken *val;
	symtype type;

	printf("## ENTERING pcp_variable_definition ##\n");

	if (token->sym != idsym) return pcp_error("Expected ID.");
	tokens.token = token;
	tail = &tokens;

	NEXTTOKEN();
	while (token->sym == commasym) {
		NEXTTOKEN();

		/* update the linked list */
		if (token->sym != idsym) return pcp_error("Expected ID.");
		ADDTOKEN(tail, token);

		NEXTTOKEN();
	}

	if (token->sym != colonsym) return pcp_error("Expected ':'.");

	NEXTTOKEN();
	val = token;
	if (!sym_to_type(val->sym, &type)) return 0;	

	/* add all the id's to the symbol table */
	cur = &tokens;
	while (cur != NULL) {
		if (!pcaddsym(cur->token->val.id, type, (symval)0, cur->token->lineno)) return 0;
		tail = cur;
		cur = cur->next;
		if (tail != &tokens) free(tail);
	}

	NEXTTOKEN();
	if (token->sym != semicolonsym) return pcp_error("Expected ';'.");

	NEXTTOKEN();
	if (token->sym == idsym) return pcp_variable_definition();

	return 1;	
}

int
pcp_variable_definition_part() {
	printf("## ENTERING pcp_variable_definition_part ##\n");
	if (token->sym != varsym) return 1;
	NEXTTOKEN();

	return pcp_variable_definition();
}

int
pcp_constant_definition() {
	pctokenlist tokens = {NULL, NULL};
	pctokenlist *tail = NULL;
	pctokenlist *cur;
	pctoken *val;
	symtype type;

	printf("## ENTERING pcp_constant_definition ##\n");

	if (token->sym != idsym) return pcp_error("Expected ID.");
	tokens.token = token;
	tail = &tokens;

	NEXTTOKEN();
	while (token->sym == commasym) {
		NEXTTOKEN();

		/* update the linked list */
		if (token->sym != idsym) return pcp_error("Expected ID.");
		ADDTOKEN(tail, token);

		NEXTTOKEN();
	}

	if (token->sym != eqsym) return pcp_error("Expected '='.");

	NEXTTOKEN();
	if (!(val = pcp_const_no_id(&type))) return 0;

	/* add all the id's to the symbol table */
	cur = &tokens;
	while (cur != NULL) {
		if (!pcaddsym(cur->token->val.id, type, val->val, cur->token->lineno)) return 0;
		tail = cur;
		cur = cur->next;
		if (tail != &tokens) free(tail);
	}

	NEXTTOKEN();
	if (token->sym != semicolonsym) return pcp_error("Expected ';'.");

	NEXTTOKEN();
	if (token->sym == idsym) return pcp_constant_definition();

	return 1;
}

int
pcp_constant_definition_part() {
	printf("## ENTERING pcp_constant_definition_part ##\n");
	if (token->sym != constsym) return 1;
	NEXTTOKEN();

	return pcp_constant_definition();
}

int
pcp_block() {
	if (!pcp_constant_definition_part()) return 0;
	/*if (!pcp_type_definition_part()) return 0;*/
	if (!pcp_variable_definition_part()) return 0;
	if (!pcp_procedure_and_function_definition_part()) return 0;
	if (!pcp_statement_part()) return 0;

	return 1;
}

int
pcp_program() {
	if (token->sym != programsym) return pcp_error("Must start with 'program'.");
	NEXTTOKEN();

	/* add to our symbol table */
	if (token->sym != idsym) return pcp_error("Expected ID.");
	ADDSYM(token->val.id, programtype, token);
	NEXTTOKEN();

	if (token->sym != semicolonsym) return pcp_error("Expected ';'");
	NEXTTOKEN();

	if (!pcp_block()) return 0;

	NEXTTOKEN();
	if (token->sym != dotsym) return pcp_error("Must end program with '.'.");

	return 1;
}

int
pcp_start() {
	return pcp_program();
}

int
pcparse(FILE *ifp) {
	fp = ifp;
	nexttoken = pcgettoken(fp);
	NEXTTOKEN();

	return pcp_start();
}