#include "symtab.h"
#include "io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

symtab *root 	= NULL;	/* our root table (keywords only) */
symtab *current	= NULL; /* our current table */

symentry *rootentry = NULL;	/* our root entry */

const char *symtypestr[numsymtypes] = {
	/* Keywords */
	"keyword",

	/* Variable types */
	"char",
	"string",
	"integer",
	"real",

	/* Block types */
	"program",
	"procedure",
	"function",
	"block",

	/* nothingness */
	"notype"
};

/*
Lookup a lexeme with the option of restricting to the current scope.

@param name the name of the lexeme
@param current_scope_only whether to restrict to current scope
@return the entry or NULL if not found
*/
symentry *pclookupsym_internal(const char *name, int current_scope_only, int *offset) {
	symentry 	*entry;
	symtab 		*tab;

	if (!current) return NULL;

	/* go through the current scope looking for the lexeme */
	entry = current->entries;
	while (entry) {
		if (strcmp(entry->name, name) == 0) {
			if (offset) *offset = entry->offset;
			return entry;
		}

		entry = entry->next;
	}

	/* check the see if we're referencing our own name */
	entry = current->block;
	if (strcmp(entry->name, name) == 0) {
		if (offset) *offset = entry->offset;
		return entry;
	}

	/* go up to the parent, if needed */
	if (current_scope_only) return NULL;

	if (offset) *offset = current->block->size;
	tab = current->parent;
	while (tab) {
		entry = tab->entries;
		while (entry) {
			if (strcmp(entry->name, name) == 0) {
				if (offset) *offset += entry->offset;
				return entry;
			}

			entry = entry->next;
		}

		/* keep going up */
		if (offset) *offset += current->block->size;
		tab = tab->parent;
	}

	/* never found */
	return NULL;
}

int pcintializesymtab() {
	symval val;

	/* create a main entry */
	if (!(rootentry = malloc(sizeof(*rootentry)))) return 0;
	rootentry->name 		= strdup("main");
	rootentry->type 		= programtype;
	rootentry->val 			= (symval)0;
	rootentry->bconst 		= 1;
	rootentry->lineno		= 0;
	rootentry->tab 			= NULL;
	rootentry->params		= NULL;
	rootentry->returntype	= notype;
	rootentry->size 		= 0;
	rootentry->offset 		= 0;

	/* create or root table and set it current */
	if (!(root = malloc(sizeof(*root)))) return 0;

	current = root;
	current->parent		= NULL;
	current->entries 	= NULL;
	current->block 		= rootentry;

	val.ival = 0;

	/* this is expanded from scanner.c -> pcgetkeyword() */
	/* TODO: make a pckeywords struct array */
	
	pcaddsym("div", keywordtype, (symval)"div", 1, 0);
	pcaddsym("mod", keywordtype, (symval)"mod", 1, 0);

	pcaddsym("program", keywordtype, (symval)"program", 1, 0);
	pcaddsym("procedure", keywordtype, (symval)"procedure", 1, 0);
	pcaddsym("function", keywordtype, (symval)"function", 1, 0);
	pcaddsym("begin", keywordtype, (symval)"begin", 1, 0);
	pcaddsym("end", keywordtype, (symval)"end", 1, 0);

	pcaddsym("and", keywordtype, (symval)"and", 1, 0);
	pcaddsym("or", keywordtype, (symval)"or", 1, 0);
	pcaddsym("not", keywordtype, (symval)"not", 1, 0);

	pcaddsym("if", keywordtype, (symval)"if", 1, 0);
	pcaddsym("else", keywordtype, (symval)"else", 1, 0);
	pcaddsym("then", keywordtype, (symval)"then", 1, 0);
	pcaddsym("do", keywordtype, (symval)"do", 1, 0);
	pcaddsym("while", keywordtype, (symval)"while", 1, 0);

	pcaddsym("array", keywordtype, (symval)"array", 1, 0);
	pcaddsym("of", keywordtype, (symval)"of", 1, 0);
	pcaddsym("char", keywordtype, (symval)"char", 1, 0);
	pcaddsym("string", keywordtype, (symval)"string", 1, 0);
	pcaddsym("integer", keywordtype, (symval)"integer", 1, 0);
	pcaddsym("real", keywordtype, (symval)"real", 1, 0);
	pcaddsym("var", keywordtype, (symval)"var", 1, 0);
	pcaddsym("const", keywordtype, (symval)"const", 1, 0);

	pcaddsym("chr", keywordtype, (symval)"chr", 1, 0);
	pcaddsym("ord", keywordtype, (symval)"ord", 1, 0);
	pcaddsym("read", keywordtype, (symval)"read", 1, 0);
	pcaddsym("readln", keywordtype, (symval)"readln", 1, 0);
	pcaddsym("write", keywordtype, (symval)"write", 1, 0);
	pcaddsym("writeln", keywordtype, (symval)"writeln", 1, 0);

	return 1;
}

void pcprintsymtabnode(symtab *node, unsigned depth) {
	symentry *entry;
	char tabs[21], *c;
	int i;

	if (!node) return;

	/* setup our tabs */
	c = tabs;
	for (i = 0; (i < depth && i < 20); ++i) {
		*c++ = '\t';
	}
	*c = '\0';

	/* print self first */
	entry = node->entries;
	while (entry) {
		switch (entry->type) {
			case integertype:
				printf("%s%s (%s @ %d // %d) : %d : %d :\n", tabs, entry->name, symtypestr[entry->type], entry->offset, entry->size, entry->lineno, entry->val.ival);
				break;
			case realtype:
				printf("%s%s (%s @ %d // %d) : %d : %f\n", tabs, entry->name, symtypestr[entry->type], entry->offset, entry->size, entry->lineno, entry->val.rval);
				break;
			case chartype:
				printf("%s%s (%s @ %d // %d) : %d : %c\n", tabs, entry->name, symtypestr[entry->type], entry->offset, entry->size, entry->lineno, entry->val.cval);
				break;
			case stringtype:
				printf("%s%s (%s @ %d // %d) : %d : %s\n", tabs, entry->name, symtypestr[entry->type], entry->offset, entry->size, entry->lineno, entry->val.str);
				break;
			default:
				printf("%s%s (%s @ %d // %d) : %d : NULL\n", tabs, entry->name, symtypestr[entry->type], entry->offset, entry->size, entry->lineno);
		}

		/* print the symbol table for the child, if it exists */
		if (entry->tab) pcprintsymtabnode(entry->tab, depth+1);

		/* go to the next entry */
		entry = entry->next;
	}
}

void pcprintsymtab() {
	printf("\n===== SYMBOL TABLE @ %d =====\n", root->block->size);
	pcprintsymtabnode(root, 0);
}

void pccleanupsymtabnode(symtab **tab) {
	symentry *cur;

	cur = (*tab)->entries;
	while (cur) {
		/* cleanup children fist */
		if (cur->tab) {
			pccleanupsymtabnode(&(cur->tab));
			cur->tab = NULL;
		}

		/* cleanup name */
		free((void*)(cur->name)); cur->name = NULL;

		/* next */
		cur = cur->next;
	}

	/* destroy our symtab */
	free(*tab);
	(*tab) = NULL;
}

void pccleanupsymtab() {
	pccleanupsymtabnode(&root);
}

symentry *pcaddsym(const char *name, symtype type, symval val, int bconst, unsigned lineno) {
	symentry *entry;

	/* make sure we have a root */
	if (!current) {
		pcerror("{%d} ERR: No symbol table defined.\n");
		return NULL;
	}

	/* make sure it doesn't yet exist in this scope */
	if (pclookupsym_internal(name, 1, NULL)) {
		pcerror("{%d} ERR: %s already exists in symbol table.\n", lineno, name);
		return NULL;
	}

	/* populate our entry */
	if (!(entry = malloc(sizeof(*entry)))) return 0;
	entry->name 		= strdup(name);
	entry->type 		= type;
	entry->val 			= val;
	entry->bconst 		= bconst;
	entry->lineno		= lineno;
	entry->tab 			= NULL;
	entry->params		= NULL;
	entry->returntype 	= notype;

	/* stack memory information */
	switch (type) {
		case stringtype:
			entry->size 	= strlen(val.str) + 1;

			/* make sure we're padded to 4 */
			if (entry->size % 4) {
				entry->size = entry->size + (4 - (entry->size % 4));
			}

			/* update our offset */
			entry->offset = current->block->size;
			break;

		case blocktype:
		case functiontype:
		case proceduretype:
		case programtype:
		case keywordtype:
			/* block types have no size or offset */
			entry->size = 0;
			entry->offset = 0;
			break;

		default:
			/* update our offset */
			entry->size = 4;
			entry->offset = current->block->size;
	}

	/* update our current stack size */
	if (type != keywordtype) current->block->size += entry->size;

	/* add to the head of the entries */
	entry->next			= current->entries;
	current->entries	= entry;
	return entry;
}

symentry *pcaddparam(const char *name, symtype type, unsigned lineno) {
	symentry *entry;
	symentry *func;
	symparam *param;
	symparam *cur;

	/* make sure we're in a function/procedure */
	if (!(func = current->block) || (func->type != proceduretype && func->type != functiontype)) {
		pcerror("{%d} ERR: Unable to determine function/procedure.\n", lineno);
		return NULL;
	}

	/* add this to the symbol table */
	if (!(entry = pcaddsym(name, type, (symval)0, 0, lineno))) return NULL;

	/* update the params with the new param */
	param = malloc(sizeof(*param));
	param->entry = entry;
	param->next = NULL;

	/* add to the root if there isn't one here yet */
	if (!(func->params)) {
		func->params = param;
		return entry;
	}

	/* add to the tail */
	cur = func->params;
	while (cur->next) cur = cur->next;
	cur->next = param;

	return entry;
}

symentry *pclookupsym_entry(const char *name, int *offset) {
	*offset = 0;
	return pclookupsym_internal(name, 0, offset);
}

symentry *pclookupsym(const char *name) {
	return pclookupsym_internal(name, 0, NULL);
}

int pcenterscope_nocreate(symentry *entry) {
	if (!entry || !entry->tab) return pcerror("Unable to enter scope!\n");

	current = entry->tab;
	return 1;
}

symentry *pcenterscope(const char *name, symtype type, unsigned lineno) {
	symentry *entry;

	if (!(entry = pcaddsym(name, type, (symval)0, 0, lineno))) return NULL;

	/* create our table and make it the current, while updating it's parent */
	if (!(entry->tab = malloc(sizeof(*(entry->tab))))) return NULL;
	entry->tab->parent	= current;
	entry->tab->entries = NULL;
	entry->tab->block = entry;

	if (!pcenterscope_nocreate(entry)) return 0;
	return entry;
}

int pcleavescope() {
	/* can't leave if we're top dog */
	if (current == root) return 0;

	/* go up to our parent */
	current = current->parent;
	return 1;
}

int pcrootsize() {
	if (root) return root->block->size;
	return 0;
}