#include "parser.h"
#include "symtab.h"
#include "ast.h"
#include "io.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int pcp_block();
int pcp_statement_part();
int pcp_application();
int pcp_constant_definition();
int pcp_expression();

FILE *fp = NULL;
pctoken *lasttoken = NULL;
pctoken *token = NULL;
pctoken *nexttoken = NULL;

#define NEXTTOKEN() if (!pcp_next()) return 0
#define ADDTOKEN(TAIL, TOKEN) if (!(TAIL = tokenlist_add(TAIL, TOKEN))) return 0
#define EXPECT(SYM) if (!pcp_expect(SYM)) return 0
#define EXPECT2(SYM1, SYM2) if (!pcp_expect2(SYM1, SYM2)) return 0

typedef struct pctokenlist {
	pctoken *token;
	struct pctokenlist *next;
} pctokenlist;


/* Updates the tokens */
int
pcp_next() {
	/* get the next token */
	lasttoken = token;
	token = nexttoken;
	nexttoken = pcgettoken(fp);

	if (!token) {
		return pcerror("Unexpected end of tokens.\n");
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

/* Accepts a given symbol and skips the next symbol if a match is found.
 
@param sym the symbol to match against
@return 1 on success; 0 otherwise
*/
int
pcp_accept(pcsym sym) {
	if (token->sym == sym) {
		return pcp_next() != 0;
	}

	return 0;
}

/* Forces a specific symbol to be found.
 
@param sym the symbol to match against
@return 1 on success; 0 otherwise
*/
int
pcp_expect(pcsym sym) {
	if (pcp_accept(sym)) {
		return 1;
	}

	return pcerror("[%u] pcp_expect: Unexpected symbol: %s vs %s\n", token->lineno, token->val, pcsymstr[sym]);
}

/* Forces two specific symbols to be found in sequence.
 
@param sym1 the first symbol to match against
@param sym2 the second symbol to match against
@return 1 on success; 0 otherwise
*/
int
pcp_expect2(pcsym sym1, pcsym sym2) {
pcsym tmpsym = token->sym;

	if (pcp_accept(sym1)) {
		if (pcp_accept(sym2)) {
			return 1;
		}

		return pcerror("[%u] pcp_expect: Unexpected end to symbol sequence: %s %s vs %s %s\n", 
			token->lineno, pcsymstr[tmpsym], pcsymstr[token->sym], pcsymstr[sym1], pcsymstr[sym2]);
	}

	return pcerror("[%u] pcp_expect: Unexpected start to symbol sequence: %s %s vs %s %s\n", 
		token->lineno, pcsymstr[tmpsym], pcsymstr[nexttoken->sym], pcsymstr[sym1], pcsymstr[sym2]);
}

/* Converts a symbol to a symtype used to store in the symbol table.
 
@param sym the symbol to check for type
@param type return value of the type
@return 1 on success; 0 otherwise
*/
int
sym_to_type(pcsym sym, symtype *type) {
	if (sym == integersym) *type = integertype;
	else if (sym == realsym) *type = realtype;
	else if (sym == stringsym) *type = stringtype;
	else if (sym == charsym) *type = chartype;
	else return pcerror("Unknown type. Arrays and custom types not yet supported.\n");

	return 1;
}

/* Adds a pctoken to the end of our list.
 
@param tail the tail of our list
@param token the token to add
@return the new tail pointer
*/
pctokenlist *
tokenlist_add(pctokenlist *tail, pctoken *token) {
	pctokenlist *next;

	if (!(next = malloc(sizeof(*next)))) {
		pcerror("Out of memory.\n");
		return NULL;
	}
	next->token = token;
	next->next = NULL;

	tail->next = next;
	return next;
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

/* Ensures the the ord() function is called correctly.
 
@return 1 on success; 0 otherwise
*/
int
pcp_ord(AST *ast) {
	AST *astord;

	printf("## ENTERING pcp_ord ##\n");

	EXPECT(lparensym);

	astord = AST_initialize(ordasm);
	AST_addchild(ast, astord);

	if (!pcp_expression(astord)) return 0;

	EXPECT(rparensym);

	return 1;
}

/* Ensures the the chr() function is called correctly.
 
@return 1 on success; 0 otherwise
*/
int
pcp_chr(AST *ast) {
	AST *astchr;

	printf("## ENTERING pcp_chr ##\n");

	EXPECT(lparensym);

	astchr = AST_initialize(chrasm);
	AST_addchild(ast, astchr);

	if (!pcp_expression(astchr)) return 0;

	EXPECT(rparensym);

	return 1;
}

int
pcp_factor(AST *ast) {
	AST *astfactor;
	AST *astother;
	pctoken *ltoken;

	printf("## ENTERED pcp_factor ##\n");

	astfactor = AST_initialize(factorasm);
	AST_addchild(ast, astfactor);

	if (pcp_accept(notsym)) {
		astother = AST_initialize(notasm);
		AST_addchild(astfactor, astother);
		return pcp_factor(astfactor); 
	}

	if (pcp_accept(idsym)) {
		ltoken = lasttoken;
		if (pcp_accept(lparensym)) {
			return pcp_application(astfactor, ltoken);
		} if (pcp_accept(lbracksym)) {
			return pcerror("Arrays not yet supported.\n");
		} else {
			astother = AST_initialize(idasm);
			astother->name = strdup(ltoken->val.id);
			AST_addchild(astfactor, astother);
		}
		
		return 1;
	}

	if (pcp_accept(ordsym)) {
		return pcp_ord(astfactor);
	}

	if (pcp_accept(chrsym)) {
		return pcp_chr(astfactor);
	}

	if (pcp_accept(lparensym)) {
		if (!pcp_expression(astfactor)) return 0;

		pcp_expect(rparensym);

		return 1;
	}

	/* see if we have an inline 'constant' value */
	if (token->sym == integernosym || token->sym == realnosym || token->sym == stringvalsym || token->sym == charvalsym) {
		astother = AST_initialize(valasm);
		astother->sym = token->sym;

		if (token->sym == stringvalsym) {
			astother->val.str = strdup(token->val.str);
		} else {
			astother->val = token->val;
		}

		AST_addchild(astfactor, astother);
		NEXTTOKEN();
		return 1;
	}

	/* failed all our branches */
	return 0;
}

int
pcp_term(AST *ast) {
	AST *astterm;
	AST *astmult;

	printf("## ENTERED pcp_term ##\n");

	astterm = AST_initialize(termasm);
	AST_addchild(ast, astterm);

	if (!pcp_factor(astterm)) return 0;

	/* keep doing all the multiplicitive arithmetic */
	while (pcp_accept(multsym) || pcp_accept(idivsym) || pcp_accept(divsym) || pcp_accept(andsym)) {
		astmult = AST_initialize(multasm);
		astmult->sym = lasttoken->sym;
		AST_addchild(astterm, astmult);

		if (!pcp_factor(astterm)) return 0;
	}

	return 1;
}

int
pcp_simple_expression(AST *ast) {
	AST *astsimexpr;
	AST *astaddasm;

	printf("## ENTERED pcp_simple_expression ##\n");

	astsimexpr = AST_initialize(simexprasm);
	AST_addchild(ast, astsimexpr);

	if (!pcp_term(astsimexpr)) return 0;

	/* keep doing all the additional arithemetic */
	while (pcp_accept(addsym) || pcp_accept(minussym) || pcp_accept(orsym)) {
		astaddasm = AST_initialize(addasm);
		astaddasm->sym = lasttoken->sym;
		AST_addchild(astsimexpr, astaddasm);

		if (!pcp_term(astsimexpr)) return 0;
	}

	/* skip next token */
	/*NEXTTOKEN();*/
	return 1;
}

int
pcp_expression(AST *ast) {
	AST *astexpr;
	AST *astrel;

	printf("## ENTERED pcp_expression ##\n");

	astexpr = AST_initialize(exprasm);
	AST_addchild(ast, astexpr);

	if (!pcp_simple_expression(astexpr)) return 0;

	/* see if this is relational */
	if (pcp_accept(eqsym) || pcp_accept(neqsym) || pcp_accept(ltsym) || pcp_accept(ltesym) || pcp_accept(gtesym) || pcp_accept(gtsym)) {
		astrel = AST_initialize(relasm);
		astrel->sym = lasttoken->sym;
		AST_addchild(astexpr, astrel);
		return pcp_simple_expression(astexpr);
	}

	return 1;
}

/*
int
pcp_for(AST *ast) {
	AST *astfor;

	printf("## ENTERED pcp_for ##\n");

	astfor = AST_initialize(forasm);
	AST_addchild(ast, astfor);

	if (!pcp_expression(astwhile)) return 0;

	EXPECT(dosym);

	return pcp_statement_part(astwhile);
	return pcp_statement_part();
}*/

int
pcp_while(AST *ast) {
	AST *astwhile;

	printf("## ENTERED pcp_while ##\n");

	astwhile = AST_initialize(whileasm);
	AST_addchild(ast, astwhile);

	if (!pcp_expression(astwhile)) return 0;

	EXPECT(dosym);

	return pcp_statement_part(astwhile);
}

int
pcp_if(AST *ast) {
	AST *astif;

	printf("## ENTERED pcp_if ##\n");

	astif = AST_initialize(ifasm);
	AST_addchild(ast, astif);

	if (!pcp_expression(astif)) return 0;

	EXPECT(thensym);

	if (!pcp_statement_part(astif)) return 0;

	if (pcp_accept(elsesym)) {
		return pcp_statement_part(astif);
	}

	return 1;
}

int
pcp_write(AST *ast, ASTnode nodetype) {
	AST *astwrite;

	printf("## ENTERED pcp_write ##\n");
	
	EXPECT(lparensym);

	astwrite = AST_initialize(nodetype);
	AST_addchild(ast, astwrite);
	
	if (!pcp_expression(astwrite)) return 0;

	while (pcp_accept(commasym)) {
		if (!pcp_expression(astwrite)) return 0;
	}

	EXPECT(rparensym);
	return 1;
}

int
pcp_read(AST *ast) {
	AST *astread;
	AST *astcur;

	printf("## ENTERED pcp_read ##\n");
	
	EXPECT(lparensym);
	EXPECT(idsym);

	astread = AST_initialize(readasm);
	AST_addchild(ast, astread);

	astcur = AST_initialize(idasm);
	astcur->name = strdup(lasttoken->val.id);
	AST_addchild(astread, astcur);

	while (pcp_accept(commasym)) {
		EXPECT(idsym);
		astcur = AST_initialize(idasm);
		astcur->name = strdup(lasttoken->val.id);
		AST_addchild(astread, astcur);
	}

	EXPECT(rparensym);
	return 1;
}

int
pcp_application(AST *ast, pctoken *ltoken) {
	AST *astfunccall;
	symentry *entry = pclookupsym(ltoken->val.id);
	int params = 1;

	printf("## ENTERED pcp_application ##\n");

	if (!entry || entry->type != functiontype) {
		return pcerror("Undefined ID or unexpected type.\n");
	}

	astfunccall = AST_initialize(funccallasm);
	astfunccall->name = strdup(ltoken->val.id);
	AST_addchild(ast, astfunccall);

	if (!pcp_expression(astfunccall)) return 0;

	while (pcp_accept(commasym)) {
		if (!pcp_expression(astfunccall)) return 0;
		++params;
	}

	EXPECT(rparensym);
	return 1;
}

int
pcp_procedure_call(AST *ast, pctoken *ltoken) {
	AST *astproccall;
	symentry *entry = pclookupsym(ltoken->val.id);
	int params = 1;

	printf("## ENTERED pcp_procedure_call ##\n");

	if (!entry || entry->type != proceduretype) {
		return pcerror("Undefined ID or unexpected type.\n");
	}

	astproccall = AST_initialize(proccallasm);
	astproccall->name = strdup(ltoken->val.id);
	AST_addchild(ast, astproccall);

	if (!pcp_expression(astproccall)) return 0;

	while (pcp_accept(commasym)) {
		if (!pcp_expression(astproccall)) return 0;
		++params;
	}

	EXPECT(rparensym);
	return 1;
}

int
pcp_procedure_call_or_application(AST *ast, pctoken *ltoken) {
	return pcp_procedure_call(ast, ltoken) || pcp_application(ast, ltoken);
}

int
pcp_assign(AST *ast, pctoken *ltoken) {
	AST *astassign;
	AST *astlval;
	symentry *entry = pclookupsym(ltoken->val.id);

	printf("## ENTERED pcp_assign ##\n");

	if (!entry) return pcerror("Undefined ID.\n");
	if (entry->type != functiontype && entry->type != integertype && entry->type != realtype && entry->type != chartype && entry->type != stringtype)
		return pcerror("Unexpected type: %d\n", entry->type);

	astassign = AST_initialize(assignasm);
	astassign->name = strdup(ltoken->val.id);
	AST_addchild(ast, astassign);

	astlval = AST_initialize(idasm);
	astlval->name = strdup(ltoken->val.id);
	AST_addchild(astassign, astlval);

	return pcp_expression(astassign);
}

int
pcp_statement(AST *ast) {
	int success = 0;

	printf("## ENTERED pcp_statement ##\n");

	/* procedure/function call or assignment */
	if (pcp_accept(idsym)) {
		pctoken *oldtoken = lasttoken;

		if (pcp_accept(lparensym)) success = pcp_procedure_call_or_application(ast, oldtoken);
		else if (pcp_accept(assignsym)) success = pcp_assign(ast, oldtoken);
		else if (pcp_accept(lbracksym)) return pcerror("Arrays not yet supported.\n");
		else return pcerror("Unexpected symbol.\n");
	} else if (pcp_accept(readsym) || pcp_accept(readlnsym)) {
		success = pcp_read(ast);
	} else if (pcp_accept(writesym)) { 
		success = pcp_write(ast, writeasm);
	} else if (pcp_accept(writelnsym)) {
		success = pcp_write(ast, writelnasm);
	} else if (pcp_accept(ifsym)) {
		success = pcp_if(ast);
	} else if (pcp_accept(whilesym)) {
		success = pcp_while(ast);
	} /*else if (pcp_accept(forsym)) {
		success = pcp_for();
	}*/ else if (pcp_accept(beginsym)) {
		success = pcp_statement_part(ast);
	} else {
		return pcerror("Unexpected statement.\n");
	}

	if (!success) return 0;

	/*NEXTTOKEN();*/
	EXPECT(semicolonsym);

	return 1;
}

int
pcp_statement_part(AST *ast) {
	AST *aststatement;

	printf("## ENTERING pcp_statement_part ##\n");

	EXPECT(beginsym);

	aststatement = AST_initialize(statementasm);
	AST_addchild(ast, aststatement);

	/* go through all the statements until end */
	while (!pcp_accept(endsym)) {
		if (!pcp_statement(aststatement)) return 0;
	}

	return 1;
}

int
pcp_formal_parameters(AST *ast) {
	pctokenlist tokens = {NULL, NULL};
	pctokenlist *tail = NULL;
	pctokenlist *cur;
	pctoken *val;
	symtype type;
	AST *astparam;
	AST *astcur;

	printf("## ENTERING pcp_formal_parameters ##\n");

	EXPECT(idsym);
	tokens.token = lasttoken;
	tail = &tokens;

	astparam = AST_initialize(paramasm);
	AST_addchild(ast, astparam);

	while (pcp_accept(commasym)) {
		EXPECT(idsym);
		ADDTOKEN(tail, lasttoken);
	}

	EXPECT(colonsym);
	
	val = token;
	if (!sym_to_type(val->sym, &type)) return 0;
	NEXTTOKEN();	

	/* add all the id's to the symbol table */
	cur = &tokens;
	while (cur != NULL) {
		if (!pcaddparam(cur->token->val.id, type, cur->token->lineno)) return 0;

		astcur = AST_initialize(idasm);
		astcur->name = strdup(cur->token->val.id);
		AST_addchild(astparam, astcur);

		tail = cur;
		cur = cur->next;
		if (tail != &tokens) free(tail);
	}

	return 1;
}

int
pcp_function_declaration(AST *ast) {
	symtype type;
	AST *astfunc;
	symentry *entry;

	printf("## ENTERING pcp_function_declaration ##\n");

	/* enter our new scope for the function */
	EXPECT(idsym);
	if (!(entry = pcenterscope(lasttoken->val.id, functiontype, lasttoken->lineno))) return 0;

	astfunc = AST_initialize(functionasm);
	astfunc->name = strdup(lasttoken->val.id);
	AST_addchild(ast, astfunc);

	EXPECT(lparensym);
	if (!pcp_formal_parameters(astfunc)) return 0;

	EXPECT(rparensym);
	EXPECT(colonsym);

	if (!sym_to_type(token->sym, &type)) return 0;

	/* update our return type */
	entry->returntype = type;

	NEXTTOKEN();
	EXPECT(semicolonsym);

	if (!pcp_block(astfunc)) return 0;

	/* leave the function scope */
	return pcleavescope();
}

int
pcp_procedure_declaration(AST *ast) {
	AST *astproc;

	printf("## ENTERING pcp_procedure_declaration ##\n");

	/* create our new scope for the new variables */
	EXPECT(idsym);
	if (!pcenterscope(lasttoken->val.id, proceduretype, lasttoken->lineno)) return 0;

	astproc = AST_initialize(procedureasm);
	astproc->name = strdup(lasttoken->val.id);
	AST_addchild(ast, astproc);

	EXPECT(lparensym);
	if (!pcp_formal_parameters(astproc)) return 0;

	EXPECT(rparensym);
	EXPECT(semicolonsym);

	if (!pcp_block(astproc)) return 0;

	/* leave the procedure scope */
	return pcleavescope();
}

int
pcp_procedure_and_function_definition_part(AST *ast) {
	printf("## ENTERING pcp_procedure_and_function_definition_part ##\n");

	if (pcp_accept(proceduresym)) {
		pcp_procedure_declaration(ast);
	} else if (pcp_accept(functionsym)) {
		pcp_function_declaration(ast);
	} else return 1;

	/*NEXTTOKEN();*/
	EXPECT(semicolonsym);

	return pcp_procedure_and_function_definition_part(ast);
}

int
pcp_variable_definition(AST *ast) {
	pctokenlist tokens = {NULL, NULL};
	pctokenlist *tail = NULL;
	pctokenlist *cur;
	pctoken *val;
	symtype type;
	AST *astcur;

	printf("## ENTERING pcp_variable_definition ##\n");

	EXPECT(idsym);
	tokens.token = lasttoken;
	tail = &tokens;

	while (pcp_accept(commasym)) {
		/* update the linked list */
		EXPECT(idsym);
		ADDTOKEN(tail, lasttoken);
	}

	EXPECT(colonsym);

	val = token;
	if (!sym_to_type(val->sym, &type)) return 0;	
	NEXTTOKEN();

	/* add all the id's to the symbol table */
	cur = &tokens;
	while (cur != NULL) {
		if (!pcaddsym(cur->token->val.id, type, (symval)0, 0, cur->token->lineno)) return 0;

		/* add to the parse tree */
		astcur = AST_initialize(idasm);
		astcur->name = strdup(cur->token->val.id);
		AST_addchild(ast, astcur);

		tail = cur;
		cur = cur->next;
		if (tail != &tokens) free(tail);
	}

	EXPECT(semicolonsym);
	if (token->sym == idsym) return pcp_variable_definition(ast);

	return 1;	
}

int
pcp_variable_definition_part(AST *ast) {
	AST *astvar;
	printf("## ENTERING pcp_variable_definition_part ##\n");

	if (!pcp_accept(varsym)) return 1;

	astvar = AST_initialize(varasm);
	AST_addchild(ast, astvar);

	return pcp_variable_definition(astvar);
}

int
pcp_constant_definition(AST *ast) {
	pctokenlist tokens = {NULL, NULL};
	pctokenlist *tail = NULL;
	pctokenlist *cur;
	pctoken *val;
	symtype type;
	AST *astcur;

	printf("## ENTERING pcp_constant_definition ##\n");

	EXPECT(idsym);
	tokens.token = lasttoken;
	tail = &tokens;

	while (pcp_accept(commasym)) {
		/* update the linked list */
		EXPECT(idsym);
		ADDTOKEN(tail, lasttoken);
	}

	EXPECT(eqsym);

	if (!(val = pcp_const_no_id(&type))) return 0;
	NEXTTOKEN();

	/* add all the id's to the symbol table */
	cur = &tokens;
	while (cur != NULL) {
		/* add to the symbol table */
		if (!pcaddsym(cur->token->val.id, type, val->val, 1, cur->token->lineno)) return 0;

		/* add to the parse tree */
		astcur = AST_initialize(idasm);
		astcur->name = strdup(cur->token->val.id);
		AST_addchild(ast, astcur);

		/* free and go to the next */
		tail = cur;
		cur = cur->next;
		if (tail != &tokens) free(tail);
	}

	EXPECT(semicolonsym);
	if (token->sym == idsym) return pcp_constant_definition(ast);

	return 1;
}

int
pcp_constant_definition_part(AST *ast) {
	AST *astconst;

	printf("## ENTERING pcp_constant_definition_part ##\n");

	if (!pcp_accept(constsym)) return 1;

	astconst = AST_initialize(constasm);
	AST_addchild(ast, astconst);

	return pcp_constant_definition(astconst);
}

int
pcp_block(AST *ast) {
	if (!pcp_constant_definition_part(ast)) return 0;
	/*if (!pcp_type_definition_part()) return 0;*/
	if (!pcp_variable_definition_part(ast)) return 0;
	if (!pcp_procedure_and_function_definition_part(ast)) return 0;
	if (!pcp_statement_part(ast)) return 0;

	return 1;
}

int
pcp_program() {
	EXPECT(programsym);

	/* add to our symbol table */
	EXPECT(idsym);
	if (!pcaddsym(lasttoken->val.id, programtype, (symval)0, 0, lasttoken->lineno)) return 0;

	/* add to our tree */
	astroot = AST_initialize(programasm);
	astroot->name = strdup(lasttoken->val.id);

	EXPECT(semicolonsym);

	if (!pcp_block(astroot)) return 0;

	/*NEXTTOKEN();*/
	if (token->sym != dotsym) {
		return pcerror("[%u] pcp_expect: Unexpected symbol: %s vs %s\n", token->lineno, token->val, pcsymstr[dotsym]);
	}

	if (pcgettoken(fp) != NULL) {
		pcerror("Expected end-of-file, but there is still content.");
		return 0;
	}

	return 1;
}

int
pcp_start() {
	return pcp_program();
}

int
pcparse(FILE *ifp) {
	fp = ifp;
	lasttoken = token = nexttoken = pcgettoken(fp);
	NEXTTOKEN();

	return pcp_start();
}