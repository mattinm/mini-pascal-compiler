#include "ast.h"
#include "io.h"
#include <stdlib.h>

AST *astroot = NULL;

AST* AST_initialize(ASTnode node) {
AST *ast;

	if (!(ast = malloc(sizeof(*ast)))) {
		pcerror("Out of memory.\n");
		return NULL;
	}

	ast->node = node;
	ast->name = NULL;
	ast->sym = eofsym;
	ast->val.ival = 0;
	ast->head = NULL;
	ast->tail = NULL;

	return ast;
}

int AST_addchild(AST *root, AST* child) {
ASTchild *cur;

	if (!(cur = malloc(sizeof(*cur)))) {
		return pcerror("Out of memory.\n");
	}

	cur->ast = child;
	cur->next = NULL;

	/* see if we have any children yet, and initialize if we don't */
	if (!root->head) {
		root->head = cur;
		root->tail = cur;

		return 1;
	}

	/* update the tail */
	root->tail->next = cur;
	root->tail = cur;
	return 1;
}

void AST_cleanup(AST **root) {
AST *ast;
ASTchild *cur;

	ast = *root;

	/* clean up all our children, left-to-right, starting at the deepest child */
	while ((cur = ast->head)) {
		AST_cleanup(&(cur->ast));
		ast->head = cur->next;

		/* cleanup the actual child */
		cur->next = NULL;
		free(cur);
	}

	if (ast->name) free(ast->name);
	ast->name = NULL;
	ast->head = NULL;
	ast->tail = NULL;

	free(ast);
	*root = NULL;
}

const char *AST_nodestr(ASTnode node) {
	switch (node) {
		/* End-of-Tokens */
		case eofasm: return "eof";

		/* Operators */
		case addasm: return "add";
		case multasm: return "mult";

		/* Scopes */
		case programasm: return "program";
		case procedureasm: return "procedure";
		case functionasm: return "function";
		case paramasm: return "param";
		case statementasm: return "statement";
		case proccallasm: return "proccall";
		case funccallasm: return "funccall";

		/* Expressions */
		case exprasm: return "expr";
		case simexprasm: return "simexpr";
		case termasm: return "term";
		case factorasm: return "factor";

		/* Boolean operators */
		case relasm: return "rel";
		case notasm: return "not";

		/* Punctuation */
		case assignasm: return "assign";
		case dotdotasm: return "dotdot";

		/* Control flow */
		case ifasm: return "if";
		case whileasm: return "while";

		/* Variables */
		case idasm: return "id";
		case arrayasm: return "array";
		case ofasm: return "of";
		case charasm: return "char";
		case stringasm: return "string";
		case integerasm: return "integer";
		case realasm: return "real";
		case varasm: return "var";

		/* Constants */
		case valasm: return "val";
		case constasm: return "const";

		/* Built-in functions */
		case chrasm: return "chr";
		case ordasm: return "ord";
		case readasm: return "read";
		case readlnasm: return "readln";
		case writeasm: return "write";
		case writelnasm: return "writeln";

		/* Number of syms */
		case numasms: return "numasms";

		default: return "ERR";
	}

	return "ERR";
}

void AST_print_internal(AST *root, FILE *fp, int depth) {
	int i = depth;
	char str[1024];
	char *c = str;
	ASTchild *cur = root->head;

	while (i--) *c++ = '\t';

	if (root->name) {
		if (root->val.ival) snprintf(c, 1023-depth, "[%s name:%s val:Y]\n", AST_nodestr(root->node), root->name);
		else snprintf(c, 1023-depth, "[%s name:%s]\n", AST_nodestr(root->node), root->name);
	} else {
		if (root->val.ival) snprintf(c, 1023-depth, "[%s val:Y]\n", AST_nodestr(root->node));
		else snprintf(c, 1023-depth, "[%s]\n", AST_nodestr(root->node));
	}

	/* print to file */
	fprintf(fp, "%s", str);

	/* print children in order */
	while (cur) {
		AST_print_internal(cur->ast, fp, depth+1);
		cur = cur->next;
	}
}

void AST_print(AST *root, FILE *fp) {
	AST_print_internal(root, fp, 0);
}