#ifndef AST_H
#define AST_H

#include "tokens.h"
#include <stdio.h>

typedef enum {
/* End-of-Tokens */
	eofasm = 0,

	/* Operators */
	addasm,
	multasm,

	/* Scopes */
	programasm,
	procedureasm,
	functionasm,
	paramasm,
	statementasm,
	proccallasm,
	funccallasm,

	/* Expressions */
	exprasm,
	simexprasm,
	termasm,
	factorasm,

	/* Boolean operators */
	relasm,
	notasm,

	/* Punctuation */
	assignasm,
	dotdotasm,

	/* Control flow */
	ifasm,
	whileasm,

	/* Variables */
	idasm,
	arrayasm,
	ofasm,
	charasm,
	stringasm,
	integerasm,
	realasm,
	varasm,

	/* Constants */
	valasm,
	constasm,

	/* Built-in functions */
	chrasm,
	ordasm,
	readasm,
	readlnasm,
	writeasm,
	writelnasm,

	/* Number of syms */
	numasms
} ASTnode;

struct ASTchild;
typedef struct AST {
	ASTnode 			node;	/* node type */
	char 				*name;	/* name in the symbol table */
	pcsym				sym;	/* symbol */
	symval				val;	/* value */
	struct ASTchild		*head;	/* left-most child */
	struct ASTchild 	*tail;	/* right-most child */
} AST;

typedef struct ASTchild {
	AST*			ast;	/* value of the child */
	struct ASTchild *next;	/* next child, left-to-right */
} ASTchild;

/* Our global AST */
extern AST *astroot;

/* Initializes an AST for use.

@param node the type of AST
@return memory allocated AST
*/
AST* AST_initialize(ASTnode node);

/* Adds a child to the AST, in left-to-right order.

@param child the AST to add
@return 1 on success; 0 otherwise
*/
int AST_addchild(AST *root, AST *child);

/* Cleans up the memory for a given AST.

@param root the AST to cleanup
*/
void AST_cleanup(AST **root);

/* Print an AST tree to the given file.

@param fp the file pointer
*/
void AST_print(AST *root, FILE *fp);

#endif /* AST_H */