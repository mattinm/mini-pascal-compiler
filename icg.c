#include "icg.h"
#include "io.h"
#include "symtab.h"
#include <stdio.h>
#include <string.h>

#define EXPECTICG(ASTV, NODEV) if (!expect(ASTV, NODEV)) return 0

int pcicg_block(AST *ast, symentry *entry, const char *label, ASTchild *params);
int pcicg_simple_expression(AST *ast, symtype *type, int *t);
int pcicg_statement(AST *ast);
int pcicg_funccall(AST *ast, symtype *type, int *t);

FILE *fp;
int ifcount, whilecount, forcount;

int accept(AST *ast, ASTnode node) {
	return ast && ast->node == node;
}

int expect(AST *ast, ASTnode node) {
	if (!ast) return pcerror("AST DOESN'T EXIST!\n");
	if (!accept(ast, node))
		return pcerror("Unexpected node: %s vs %s\n", astnodestr[ast->node], astnodestr[node]);
	return 1;
}

/* Convert from one type to another, if required. */
int pcicg_convert(symtype totype, symtype fromtype, int t) {
	int error = 0;

	if (totype == integertype) {
		if (fromtype == realtype) {
			/* convert from real to integer */
			fprintf(fp, "cvt.w.s $f%d, $f%d\n", t, t);
			fprintf(fp, "mfc1 $t%d, $f%d\n", t, t);
		} else if (fromtype != integertype) {
			error = 1;
		}
	} else if (totype == realtype) {
		if (fromtype == integertype) {
			/* convert from integer to real */
			fprintf(fp, "mtc1 $f%d, $f%d\n", t, t);
			fprintf(fp, "cvt.s.w $f%d, $f%d\n", t, t);
		} else if (fromtype != realtype) {
			error = 1;
		}
	} else if (totype != fromtype) {
		error = 1;
	}

	if (error) return pcerror("Unable to convert to type %s from %s.\n", symtypestr[totype], symtypestr[fromtype]);
	return 1;
}

int pcicg_var(AST *ast) {
	ASTchild *cur;
	symentry *entry;

	printf("===== ENTERING pcicg_var =====\n");

	/* 
	add our variables to the stack
	the first variables have the largest offset
	*/
	cur = ast->head;
	while (cur) {
		/* keep track of our sum so we can get the correct offset */
		entry = pclookupsym(cur->ast->name);

		/* initialize to zero */
		switch (entry->type) {
			case chartype:
			case integertype:
				fprintf(fp, "sw $0, %d($sp)\n", entry->offset);
				break;
			case realtype:
				fprintf(fp, "sw.s $0, %d($sp)\n", entry->offset);
				break;
			default:
				return pcerror("Unhandled var type: %d\n", entry->type);
		}

		cur = cur->next;
	}

	return 1;
}

int pcicg_const(AST *ast) {
	ASTchild *cur;
	symentry *entry;
	int i;
	int len;

	printf("===== ENTERING pcicg_const =====\n");

	/* 
	add our variables to the stack
	the first variables have the largest offset
	*/
	cur = ast->head;
	while (cur) {
		/* keep track of our sum so we can get the correct offset */
		entry = pclookupsym(cur->ast->name);

		switch (entry->type) {
			case chartype:
				fprintf(fp, "li $t0, %d\n", (int)entry->val.cval);
				fprintf(fp, "sb $t0, %d($sp)\n", entry->offset);
				break;

			case integertype:
				fprintf(fp, "li $t0, %d\n", entry->val.ival);
				fprintf(fp, "sw $t0, %d($sp)\n", entry->offset);
				break;

			case realtype:
				fprintf(fp, "li.s $f0, %f\n", entry->val.rval);
				fprintf(fp, "s.s $f0, %d($sp)\n", entry->offset);
				break;

			case stringtype:
				len = strlen(entry->val.str);

				for (i = 0; i < len; ++i) {
					fprintf(fp, "li $t0, %d\n", (int)(entry->val.str[i]));
					fprintf(fp, "sb $t0, %d($sp)\n", i + entry->offset);
				}

				fprintf(fp, "li $t0, 0\n");
				fprintf(fp, "sb $t0, %d($sp)\n", i + entry->offset);
				break;

			default:
				return pcerror("Unhandled type: %d\n", (int)entry->type);
		}

		cur = cur->next;
	}

	return 1;
}

int pcicg_function(AST *ast) {
	ASTchild *cur;
	ASTchild *params;
	symentry *entry;

	printf("===== ENTERING pcicg_function =====\n");

	if (!(entry = pclookupsym(ast->name)) || entry->type != functiontype) {
		return pcerror("Unable to lookup function.\n");
	}

	/* check our parameters */
	cur = ast->head;
	EXPECTICG(cur->ast, paramasm);
	params = cur->ast->head;

	/* parse the block */
	if (!pcicg_block(ast, entry, entry->name, params)) return 0;

	/* reload in the return address */
	fprintf(fp, "lw $ra, %d($sp)\n", entry->size);

	/* pop the stack */
	fprintf(fp, "addi $sp, $sp, %d\n", entry->size+4);

	/* add a jump back to the caller */
	fprintf(fp, "jr $ra\n\n");

	return 1;
}

int pcicg_procedure(AST *ast) {
	ASTchild *cur;
	ASTchild *params;
	symentry *entry;

	printf("===== ENTERING pcicg_procedure =====\n");

	if (!(entry = pclookupsym(ast->name)) || entry->type != proceduretype) {
		return pcerror("Unable to lookup procedure.\n");
	}

	/* get our parameters */
	cur = ast->head;
	EXPECTICG(cur->ast, paramasm);
	params = cur->ast->head;

	/* parse the block */
	if (!pcicg_block(ast, entry, entry->name, params)) return 0;

	/* reload the return address */
	fprintf(fp, "lw $ra, %d($sp)\n", entry->size);

	/* pop the stack */
	fprintf(fp, "addi $sp, $sp, %d\n", entry->size+4);

	/* add a jump back to the caller */
	fprintf(fp, "jr $ra\n\n");

	return 1;
}

int pcicg_factor(AST *ast, symtype *type, int *t) {
	ASTchild *cur;
	symentry *entry;
	int offset;
	int left;
	symtype forcetype = notype;

	printf("===== ENTERING pcicg_factor =====\n");

	cur = ast->head;

	/* load id value */
	if (accept(cur->ast, idasm)) {
		if (!(entry = pclookupsym_entry(cur->ast->name, &offset)))
			return pcerror("Unable to load entry: %s\n", cur->ast->name);

		switch (entry->type) {
			case chartype:
				fprintf(fp, "li $t%d, 0\n", *t);
				fprintf(fp, "lb $t%d, %d($sp)\n", *t, offset);
				break;

			case integertype:
				fprintf(fp, "lw $t%d, %d($sp)\n", *t, offset);
				break;

			case realtype:
				fprintf(fp, "li.s $f%d, %d($sp)\n", *t, offset);
				break;

			case stringtype:
				fprintf(fp, "la $t%d, %d($sp)\n", *t, offset);
				break;

			default:
				return pcerror("Unhandled type: %s\n", symtypestr[entry->type]);
		}

		*type = entry->type;
	}

	/* ord value */
	else if (accept(cur->ast, ordasm)) {
		cur = cur->next;
		EXPECTICG(cur->ast, exprasm);

		cur = cur->next;
		EXPECTICG(cur->ast, simexprasm);
		forcetype = chartype;
		if (!pcicg_simple_expression(cur->ast, &forcetype, t)) return 0;

		*type = chartype;
	}

	/* chr value */
	else if (accept(cur->ast, chrasm)) {
		cur = cur->next;
		EXPECTICG(cur->ast, exprasm);

		cur = cur->next;
		EXPECTICG(cur->ast, simexprasm);
		forcetype = integertype;
		if (!pcicg_simple_expression(cur->ast, &forcetype, t)) return 0;

		/* truncate down to a single byte */
		fprintf(fp, "li $t%d, 0", (*t)+1);
		fprintf(fp, "sb $t%d, $t%d", (*t)+1, *t);
		fprintf(fp, "sw $t%d, $t%d", *t, (*t)+1);

		*type = integertype;
	}

	/* not value */
	else if (accept(cur->ast, notasm)) {
		cur = cur->next;
		EXPECTICG(cur->ast, factorasm);

		left = *t;
		*t += 1;

		if (!pcicg_factor(cur->ast, type, t)) return 0;
		if (!pcicg_convert(integertype, *type, *t)) return 0;

		fprintf(fp, "addi $t%d, $0, -1", left);
		fprintf(fp, "xor $t%d, $t%d, $t%d", left, left, *t);

		*t = left;
		*type = integertype;
	}

	/* expr */
	/*
	else if (accept(cur->ast, exprasm)) {
		cur = cur->next;
	}
	*/

	/* val */
	else if (accept(cur->ast, valasm)) {
		switch (cur->ast->sym) {
			case charvalsym:
				fprintf(fp, "li $t%d, %d\n", *t, (int)(cur->ast->val.cval));
				*type = chartype;
				break;

			case integernosym:
				fprintf(fp, "li $t%d, %d\n", *t, cur->ast->val.ival);
				*type = integertype;
				break;

			case realnosym:
				fprintf(fp, "li.s $f%d, %f\n", *t, cur->ast->val.rval);
				*type = realtype;
				break;

			default:
				return pcerror("Unhandled type: %s\n", symtypestr[entry->type]);
		}
	}

	/* function call */
	else if (accept(cur->ast, funccallasm)) {
		if (!pcicg_funccall(cur->ast, type, t)) return 0;
	}

	/* UNKNOWN! */
	else {
		return pcerror("Unexpected node: %s\n", astnodestr[cur->ast->node]);
	}

	/* woot */
	return 1;
}

int pcicg_term(AST *ast, symtype *type, int *t) {
	ASTchild *cur;
	symtype factortype;
	pcsym sym;
	int left;

	printf("===== ENTERING pcicg_term =====\n");

	/* grab the first factor */
	cur = ast->head;
	EXPECTICG(cur->ast, factorasm);
	if (!pcicg_factor(cur->ast, type, t)) return 0;

	/* update our left-most value for chaining */
	left = *t;
	*t += 1;

	/* go through all our multops */
	while (cur->next) {
		cur = cur->next;
		EXPECTICG(cur->ast, multasm);
		sym = cur->ast->sym;

		cur = cur->next;
		EXPECTICG(cur->ast, factorasm);
		if (!pcicg_factor(cur->ast, &factortype, t)) return 0;

		/* convert and perform the mul function */
		if (!pcicg_convert(*type, factortype, *t)) return 0;

		if (*type == integertype) {
			if (sym == multsym)	{
				fprintf(fp, "mul $t%d, $t%d\n", left, *t);
				fprintf(fp, "mflo $t%d\n", left);
			} else if (sym == divsym) {
				fprintf(fp, "div $t%d, $t%d\n", left, *t);
				fprintf(fp, "mflo $t%d\n", left);
			} else if (sym == idivsym) {
				fprintf(fp, "div $t%d, $t%d\n", left, *t);
				fprintf(fp, "mflo $t%d\n", left);
			} else if (sym == modsym) {
				fprintf(fp, "div $t%d, $t%d\n", left, *t);
				fprintf(fp, "mfhi $t%d\n", left);
			} else if (sym == andsym) {
				fprintf(fp, "and $t%d, $t%d, $t%d\n", left, left, *t);
			} else {
				return pcerror("Unknown multiplication symbol: %s\n", symtypestr[sym]);
			}
		} else if (*type == realtype) {
			if (sym == multsym)	{
				fprintf(fp, "mul.s $f%d, $f%d, $f%d\n", left, left, *t);
			} else if (sym == divsym) {
				fprintf(fp, "div.s $f%d, $f%d, $f%d\n", left, left, *t);
			} else if (sym == idivsym) {
				fprintf(fp, "div.s $f%d, $f%d, $f%d\n", left, left, *t);
			} else if (sym == modsym) {
				fprintf(fp, "div $t%d, $t%d\n", left, *t);
				fprintf(fp, "mfhi $t%d\n", left);
			} else {
				return pcerror("Unknown multiplication symbol: %s\n", symtypestr[sym]);
			}
		} else {
			return pcerror("Cannot multop on type %d\n", *type);
		}
	}

	*t = left;
	return 1;
}

int pcicg_simple_expression(AST *ast, symtype *type, int *t) {
	ASTchild *cur;
	pcsym sym;
	int left;
	symtype termtype;
	symtype termtype2;

	printf("===== ENTERING pcicg_simple_expression =====\n");

	/* grab the first term */
	cur = ast->head;
	EXPECTICG(cur->ast, termasm);
	if (!pcicg_term(cur->ast, &termtype, t)) return 0;

	/* if we didn't request a type, assign it to the current value's type */
	if (type == NULL || *type == notype) *type = termtype;

	/* update our left-most value for chaining */
	left = *t;
	*t += 1;

	/* go through all of our addops */
	while (cur->next) {
		cur = cur->next;
		EXPECTICG(cur->ast, addasm);
		sym = cur->ast->sym;

		cur = cur->next;
		EXPECTICG(cur->ast, termasm);
		if (!pcicg_term(cur->ast, &termtype2, t)) return 0;

		/* write out the add or subtract */
		if (!pcicg_convert(termtype, termtype2, *t)) return 0;
		if (termtype == integertype) {
			if (sym == addsym) 		fprintf(fp, "add $t%d, $t%d, $t%d\n", left, left, *t);
			else if (sym == minussym)	fprintf(fp, "sub $t%d, $t%d, $t%d\n", left, left, *t);
			else if (sym == orsym)	fprintf(fp, "or $t%d, $t%d, $t%d\n", left, left, *t);
			else return pcerror("Incompatible addop: %d\n", sym);
		} else if (termtype == realtype) {
			if (sym == addsym) 		fprintf(fp, "add.s $f%d, $f%d, $f%d\n", left, left, *t);
			else if (sym == minussym)	fprintf(fp, "sub.s $f%d, $f%d, $f%d\n", left, left, *t);
			else return pcerror("Incompatible addop: %d\n", sym);
		} else {
			return pcerror("Cannot addop on type: %s\n", symtypestr[termtype]);
		}
	}

	/* reupdate the t to reflect our return value (convert if needed) */
	*t = left;
	return pcicg_convert(*type, termtype, left);
}

int pcicg_assign(AST *ast, int *t) {
	ASTchild *cur;
	symentry *entry;
	int offset;
	symtype type = notype;

	printf("===== ENTERING pcicg_assign =====\n");

	/* load our entry for storage */
	cur = ast->head;
	EXPECTICG(cur->ast, idasm);
	if (!(entry = pclookupsym_entry(cur->ast->name, &offset)))
		return pcerror("Unable to find variable: %s\n", cur->ast->name);

	/* evaluate the expression */
	cur = cur->next;
	EXPECTICG(cur->ast, exprasm);
	cur = cur->ast->head;
	EXPECTICG(cur->ast, simexprasm);

	/* if we see function, we assume return */
	if (entry->type == functiontype) type = entry->returntype;
	else type = entry->type;

	if (!pcicg_simple_expression(cur->ast, &type, t)) return 0;

	/* actually assign the value */
	switch (type) {
		case integertype:
			if (entry->type == functiontype) fprintf(fp, "move $v0, $t%d\n", *t);
			else fprintf(fp, "sw $t%d, %d($sp)\n", *t, offset);
			break;
		case realtype:
			if (entry->type == functiontype) fprintf(fp, "move $f10, $t%d\n", *t);
			else fprintf(fp, "sw.s $f%d, %d($sp)\n", *t, offset);
			break;
		case chartype:
			if (entry->type == functiontype) fprintf(fp, "move $v0, $t%d\n", *t);
			else {
				fprintf(fp, "sw $0, %d($sp)\n", offset);
				fprintf(fp, "sb $t%d, %d($sp)\n", *t, offset);
			}
			break;
		
		default:
			return pcerror("Cannot assign type: %s\n", symtypestr[type]);
	}

	return 1;
}

int pcicg_if(AST *ast, int *t) {
	ASTchild *expr;
	ASTchild *astthen;
	ASTchild *astelse;
	int left;
	pcsym relop;
	char setcmd[4];
	char label[20];
	symtype type = integertype;

	printf("===== ENTERING pcicg_if =====\n");

	expr = ast->head;
	left = *t;
	*t = *t + 1;

	/* setup our label */
	snprintf(label, 20, "Lif%d", ifcount++);

	/* setup our children */
	EXPECTICG(expr->ast, exprasm);
	astthen = expr->next;
	astelse = astthen->next;
	expr = expr->ast->head;

	/* part 1 */
	EXPECTICG(expr->ast, simexprasm);
	if (!(pcicg_simple_expression(expr->ast, &type, &left))) return 0;
	expr = expr->next;

	/* operator */
	EXPECTICG(expr->ast, relasm);
	relop = expr->ast->sym;
	expr = expr->next;

	/* part 2 */
	EXPECTICG(expr->ast, simexprasm);
	if (!(pcicg_simple_expression(expr->ast, &type, t))) return 0;

	/* comparison */
	switch (relop) {
		case ltsym: 	strcpy(setcmd, "blt"); break;
		case ltesym: 	strcpy(setcmd, "ble"); break;
		case neqsym:	strcpy(setcmd, "bne"); break;
		case gtsym: 	strcpy(setcmd, "bgt"); break;
		case gtesym:	strcpy(setcmd, "bge"); break;
		case eqsym:		strcpy(setcmd, "beq"); break;
		default:
			return pcerror("Unknown relop: %s\n", pcsymstr[relop]);
	}

	/* branch to the "then" part if relop holds */
	fprintf(fp, "\n%s $t%d, $t%d, %s\n", setcmd, left, *t, label);

	/* do the else first for branching purposes */
	if (astelse)
		if (accept(astelse->ast, statementasm) && !pcicg_statement(astelse->ast)) return 0;

	/* branch past the "then" part if relop didn't hold (or after else) */
	fprintf(fp, "b %send\n", label);

	/* make sure we have a thenpart */
	EXPECTICG(astthen->ast, statementasm);
	fprintf(fp, "\n%s: ", label);
	if (!(pcicg_statement(astthen->ast))) return 0;

	/* print our end label */
	fprintf(fp, "%send:\n\n", label);
	*t = left;

	return 1;
}

int pcicg_while(AST *ast, int *t) {
	ASTchild *cur;
	char setcmd[4];
	char label[20];
	symtype type = integertype;
	pcsym relop;
	int left;

	printf("===== ENTERING pcicg_while =====\n");

	/* setup our label */
	snprintf(label, 20, "Lwhile%d", whilecount++);	

	EXPECTICG(ast, whileasm);
	cur = ast->head;

	EXPECTICG(cur->ast, exprasm);
	cur = cur->ast->head;

	left = *t;
	*t = *t + 1;

	fprintf(fp, "\n%s: ", label);

	/* part 1 */
	EXPECTICG(cur->ast, simexprasm);
	if (!(pcicg_simple_expression(cur->ast, &type, &left))) return 0;
	cur = cur->next;

	/* operator */
	EXPECTICG(cur->ast, relasm);
	relop = cur->ast->sym;
	cur = cur->next;

	/* part 2 */
	EXPECTICG(cur->ast, simexprasm);
	if (!(pcicg_simple_expression(cur->ast, &type, t))) return 0;

	/* comparison (use opposite here to "break out") */
	switch (relop) {
		case ltsym: 	strcpy(setcmd, "bge"); break;
		case ltesym: 	strcpy(setcmd, "bgt"); break;
		case neqsym:	strcpy(setcmd, "beq"); break;
		case gtsym: 	strcpy(setcmd, "ble"); break;
		case gtesym:	strcpy(setcmd, "blt"); break;
		case eqsym:		strcpy(setcmd, "bne"); break;
		default:
			return pcerror("Unknown relop: %s\n", pcsymstr[relop]);
	}

	/* branch past the main part if relop didn't hold (or after else) */
	fprintf(fp, "%s $t%d, $t%d, %send\n", setcmd, left, *t, label);

	/* make sure we have a statement */
	cur = ast->head->next;
	EXPECTICG(cur->ast, statementasm);
	if (!(pcicg_statement(cur->ast))) return 0;

	/* loop back to the top of the while loop */
	fprintf(fp, "j %s\n", label);

	/* print our end label */
	fprintf(fp, "%send:\n\n", label);
	*t = left;

	return 1;
}

int pcicg_write(AST *ast, int *t) {
	ASTchild *cur;
	symtype type = notype;

	printf("===== ENTERING pcicg_write =====\n");	

	/* error checking */
	if (!accept(ast, writeasm) && !accept(ast, writelnasm)) return 0;
	cur = ast->head;
	EXPECTICG(cur->ast, exprasm);
	cur = cur->ast->head;

	/* process the statement */
	EXPECTICG(cur->ast, simexprasm);
	if (!pcicg_simple_expression(cur->ast, &type, t)) return 0;

	/* determine the write based on type */
	switch (type) {
		case chartype:
			fprintf(fp, "li $a0, 0\n");
			fprintf(fp, "move $a0, $t%d\n", *t);
			fprintf(fp, "li $v0, 11\n");
			break;

		case integertype:
			fprintf(fp, "move $a0, $t%d\n", *t);
			fprintf(fp, "li $v0, 1\n");
			break;

		case realtype:
			fprintf(fp, "move.s $f12, $f%d\n", *t);
			fprintf(fp, "li $v0, 2\n");
			break;

		case stringtype:
			fprintf(fp, "move $a0, $t%d\n", *t);
			fprintf(fp, "li $v0, 4\n");
			break;

		default:
			return pcerror("Cannot write for type: %s\n", symtypestr[type]);
	}

	/* execute the syscall */
	fprintf(fp, "syscall\n");

	/* add a new line if ln is used */
	if (ast->node == writelnasm)
		fprintf(fp, "li $a0, 10\nli $v0, 11\nsyscall\n");

	return 1;
}

int pcicg_read(AST *ast, int *t) {
	ASTchild *cur;
	symentry *entry;
	symtype type = notype;
	int offset;

	printf("===== ENTERING pcicg_read =====\n");	

	cur = ast->head;
	EXPECTICG(cur->ast, idasm);

	if (!(entry = pclookupsym_entry(cur->ast->name, &offset))) 
		return pcerror("Unable to load entry: %s\n", cur->ast->name);

	switch (entry->type) {
		case integertype:
			fprintf(fp, "li $v0, 5\nsyscall\nsw $v0, %d($sp)\n", offset);
			break;

		case chartype:
			fprintf(fp, "li $v0, 12\nsyscall\nsw $v0, %d($sp)\n", offset);
			break;

		case realtype:
			fprintf(fp, "li $v0, 6\nsyscall\nsw.s $f0, %d($sp)\n", offset);
			break;

		default:
			return pcerror("Unable to read type: %s\n", symtypestr[entry->type]);
	}

	return 1;
}

int pcicg_proccall(AST *ast, int *t) {
	ASTchild *cur;
	ASTchild *pcur;
	symparam *param;
	symentry *entry;
	symtype type = notype;
	int a = 0;

	printf("===== ENTERING pcicg_proccall =====\n");

	EXPECTICG(ast, proccallasm);
	if (!(entry = pclookupsym(ast->name)))
		return pcerror("Unable to find procedure: %s\n", ast->name);

	cur = ast->head;
	param = entry->params;
	while (cur) {
		if (!param) return pcerror("[%d] Too many parameters - %d\n", entry->lineno, a);

		EXPECTICG(cur->ast, exprasm);
		if (!(pcur = cur->ast->head)) return 0;
		EXPECTICG(pcur->ast, simexprasm);

		type = param->entry->type;
		if (!(pcicg_simple_expression(pcur->ast, &type, t))) return 0;

		/* store the value in the a register for passing */
		switch (type) {
			case integertype:
			case chartype:
				fprintf(fp, "move $a%d, $t%d\n", a, *t);
				break;

			case realtype:
				fprintf(fp, "move $f1%d, $f%d\n", a, *t);
				break;

			default:
				return pcerror("Unsupported param type: %s\n", symtypestr[type]);
		}

		/* increment everything */
		param = param->next;
		cur = cur->next;
		++a;
	}

	if (param) return pcerror("[%d] Too few parameters - %d\n", entry->lineno, a);

	/* make the function call */
	fprintf(fp, "jal %s\n", entry->name);

	return 1;
}

int pcicg_funccall(AST *ast, symtype *returntype, int *t) {
	ASTchild *cur;
	ASTchild *pcur;
	symparam *param;
	symentry *entry;
	symtype type = notype;
	int a = 0;

	printf("===== ENTERING pcicg_funccall =====\n");

	EXPECTICG(ast, funccallasm);
	if (!(entry = pclookupsym(ast->name)))
		return pcerror("Unable to find function: %s\n", ast->name);

	cur = ast->head;
	param = entry->params;
	while (cur) {
		if (!param) return pcerror("[%d] Too many parameters - %d\n", entry->lineno, a);

		EXPECTICG(cur->ast, exprasm);
		if (!(pcur = cur->ast->head)) return 0;
		EXPECTICG(pcur->ast, simexprasm);

		type = param->entry->type;
		if (!(pcicg_simple_expression(pcur->ast, &type, t))) return 0;

		/* store the value in the a register for passing */
		switch (type) {
			case integertype:
			case chartype:
				fprintf(fp, "move $a%d, $t%d\n", a, *t);
				break;

			case realtype:
				fprintf(fp, "move $f1%d, $f%d\n", a, *t);
				break;

			default:
				return pcerror("Unsupported param type: %s\n", symtypestr[type]);
		}

		/* increment everything */
		param = param->next;
		cur = cur->next;
		++a;
	}

	if (param) return pcerror("[%d] Too few parameters - %d\n", entry->lineno, a);

	/* make the function call */
	fprintf(fp, "jal %s\n", entry->name);

	/* store the return value in the register */
	switch (entry->returntype) {
		case integertype:
		case chartype:
			fprintf(fp, "move $t%d, $v0\n", *t);
			break;

		case realtype:
			fprintf(fp, "move $f%d, $f0\n", *t);
			break;

		default:
			return pcerror("Unreturnable type: %s\n", symtypestr[entry->returntype]);
	}

	/* convert if needed */
	if (returntype && *returntype != notype) *returntype = entry->returntype;

	return pcicg_convert(*returntype, entry->returntype, *t);
}

int pcicg_statement(AST *ast) {
	ASTchild *cur;
	int t = 0;

	printf("===== ENTERING pcicg_statement =====\n");

	cur = ast->head;
	while (cur) {
		t = 0;
		switch (cur->ast->node) {
			case assignasm:
				if (!pcicg_assign(cur->ast, &t)) return 0;
				break;

			case ifasm:
				if (!pcicg_if(cur->ast, &t)) return 0;
				break;

			case whileasm:
				if (!pcicg_while(cur->ast, &t)) return 0;
				break;

			case writeasm:
			case writelnasm:
				if (!pcicg_write(cur->ast, &t)) return 0;
				break;

			case readasm:
			case readlnasm:
				if (!pcicg_read(cur->ast, &t)) return 0;
				break;

			case proccallasm:
				if (!pcicg_proccall(cur->ast, &t)) return 0;
				break;

			case funccallasm:
				if (!pcicg_funccall(cur->ast, NULL, &t)) return 0;
				break;

			default:
				pcerror("Unhandled statement type.: %s\n", astnodestr[cur->ast->node]);
		}

		cur = cur->next;
	}

	return 1;
}

int pcicg_block(AST *ast, symentry *entry, const char *label, ASTchild *params) {
	ASTchild *cur;
	int size;

	printf("===== ENTERING pcicg_block =====\n");

	/* grab the const and vars first */
	cur = ast->head;

	/* entry the new block */
	if (entry) {
		if (entry->type != programtype && !pcenterscope_nocreate(entry)) return 0;

		if (entry->type == programtype) size = pcrootsize();
		else size = entry->size+4;

		/* assign our stack and jump to the body */
		fprintf(fp, "\n%s: addi $sp, $sp, -%d\n", label, size);

		/* store the ra for usage later */
		if (entry->type != programtype) fprintf(fp, "sw $ra, %d($sp)\n", entry->size);

		/* skip past params */
		if (accept(cur->ast, paramasm)) cur = cur->next;

		if (accept(cur->ast, constasm)) {
			if (!pcicg_const(cur->ast)) return 0;
			cur = cur->next;
			fprintf(fp, "\n");
		}
		if (accept(cur->ast, varasm)) {
			if (!pcicg_var(cur->ast)) return 0;
			cur = cur->next;
			fprintf(fp, "\n");
		}

		/* assign parameters */
		if (params) {
			int acur = 0;
			symentry *pentry;
			int offset = 0;

			while (params) {
				if (!params->ast->name) {
					return pcerror("Missing parameter name for #%d\n", acur);
				}
				
				if (!(pentry = pclookupsym_entry(params->ast->name, &offset)))
					return pcerror("Unable to find param: %s\n", params->ast->name);

				/* save onto the stack, based on type */
				switch (pentry->type) {
					case integertype:
						fprintf(fp, "sw $a%d, %d($sp)\n", acur, offset);
						break;
					case realtype:
						fprintf(fp, "sw.s $a%d, %d($sp)\n", acur, offset);
						break;
					case chartype:
						fprintf(fp, "sw $0, %d($sp)\n", offset);
						fprintf(fp, "sb $a%d, %d($sp)\n", acur, offset);
						break;
					default:
						return pcerror("Cannot assign type: %s\n", symtypestr[pentry->type]);
				}

				params = params->next;
				++acur;
			}
		}

		/* jump to the body */
		fprintf(fp, "j %sbody\n\n", label);
	}


	/* now do the methods */
	while (cur) {
		if (accept(cur->ast, procedureasm)) {
			if (!pcicg_procedure(cur->ast)) return 0;
		} else if (accept(cur->ast, functionasm)) {
			if (!pcicg_function(cur->ast)) return 0;
		} else if (accept(cur->ast, statementasm)) {
			break;
		} else {
			return pcerror("Unexpected node: %s\n", astnodestr[cur->ast->node]);
		}

		cur = cur->next;
	}

	/* now do the statement */
	EXPECTICG(cur->ast, statementasm);

	/* process our body */
	if (entry) fprintf(fp, "%sbody: ", label);
	if (!pcicg_statement(cur->ast)) return 0;

	/* exit the scope */
	if (entry && entry->type != programtype) return pcleavescope();
	return 1;
}

int pcicg_program(AST *ast) {
	symentry *entry;

	printf("===== ENTERING pcicg_program =====\n");

	EXPECTICG(ast, programasm);
	fprintf(fp, ".data\n\n.text\n");

	/* grab the entry for this block */
	if (!(entry = pclookupsym(ast->name))) return pcerror("Unable to find program.\n");
	if (!pcicg_block(ast, entry, "main", NULL)) return 0;

	/* print the program end syscall */
	fprintf(fp, "li $v0, 10\nsyscall");
	return 1;
}

int pcicg_start(FILE *fpo) {
	printf("===== ENTERING pcicg_start =====\n");

	if (!fpo) {
		printf("\nFILE REQUIRED FOR ICG!\n");
		return 0;
	}

	/* setup the globals */
	fp = fpo;
	ifcount = whilecount = forcount = 0;

	return pcicg_program(astroot);
}