#include "symtab.h"
#include "io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

symtab *root 	= NULL;	/* our root table (keywords only) */
symtab *current	= NULL; /* our current table */

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
symentry *pclookupsym_internal(const char *name, int current_scope_only) {
	symentry 	*entry;
	symtab 		*tab;

	if (!current) return NULL;

	/* go through the current scope looking for the lexeme */
	entry = current->entries;
	while (entry) {
		if (strcmp(entry->name, name) == 0) {
			return entry;
		}

		entry = entry->next;
	}

	/* go up to the parent, if needed */
	if (current_scope_only) return NULL;
	tab = current->parent;
	while (tab) {
		entry = tab->entries;
		while (entry) {
			if (strcmp(entry->name, name) == 0) {
				return entry;
			}

			entry = entry->next;
		}

		/* keep going up */
		tab = tab->parent;
	}

	/* never found */
	return NULL;
}

int pcintializesymtab() {
	symval val;

	/* create or root table and set it current */
	if (!(root = malloc(sizeof(*root)))) return 0;

	current = root;
	current->parent		= NULL;
	current->entries 	= NULL;

	val.ival = 0;

	/* this is expanded from scanner.c -> pcgetkeyword() */
	/* TODO: make a pckeywords struct array */
	
	pcaddsym("div", keywordtype, (symval)"div", 0);
	pcaddsym("mod", keywordtype, (symval)"mod", 0);

	pcaddsym("program", keywordtype, (symval)"program", 0);
	pcaddsym("procedure", keywordtype, (symval)"procedure", 0);
	pcaddsym("function", keywordtype, (symval)"function", 0);
	pcaddsym("begin", keywordtype, (symval)"begin", 0);
	pcaddsym("end", keywordtype, (symval)"end", 0);

	pcaddsym("and", keywordtype, (symval)"and", 0);
	pcaddsym("or", keywordtype, (symval)"or", 0);
	pcaddsym("not", keywordtype, (symval)"not", 0);

	pcaddsym("if", keywordtype, (symval)"if", 0);
	pcaddsym("else", keywordtype, (symval)"else", 0);
	pcaddsym("then", keywordtype, (symval)"then", 0);
	pcaddsym("do", keywordtype, (symval)"do", 0);
	pcaddsym("while", keywordtype, (symval)"while", 0);

	pcaddsym("array", keywordtype, (symval)"array", 0);
	pcaddsym("of", keywordtype, (symval)"of", 0);
	pcaddsym("char", keywordtype, (symval)"char", 0);
	pcaddsym("string", keywordtype, (symval)"string", 0);
	pcaddsym("integer", keywordtype, (symval)"integer", 0);
	pcaddsym("real", keywordtype, (symval)"real", 0);
	pcaddsym("var", keywordtype, (symval)"var", 0);
	pcaddsym("const", keywordtype, (symval)"const", 0);

	pcaddsym("chr", keywordtype, (symval)"chr", 0);
	pcaddsym("ord", keywordtype, (symval)"ord", 0);
	pcaddsym("read", keywordtype, (symval)"read", 0);
	pcaddsym("readln", keywordtype, (symval)"readln", 0);
	pcaddsym("write", keywordtype, (symval)"write", 0);
	pcaddsym("writeln", keywordtype, (symval)"writeln", 0);

	return 1;
}

void pcprintsymtabnode(symtab *node, unsigned depth) {
	symentry *entry;
	char tabs[20], *c;
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
		if (entry->type == integertype) {
			printf("%s%s (%s) : %d : %d\n", tabs, entry->name, symtypestr[entry->type], entry->lineno, entry->val.ival);
		} else if (entry->type == realtype) {
			printf("%s%s (%s) : %d : %f\n", tabs, entry->name, symtypestr[entry->type], entry->lineno, entry->val.rval);
		} else if (entry->type == chartype) {
			printf("%s%s (%s) : %d : %c\n", tabs, entry->name, symtypestr[entry->type], entry->lineno, entry->val.cval);
		} else {
			printf("%s%s (%s) : %d : %s\n", tabs, entry->name, symtypestr[entry->type], entry->lineno, entry->val.str);
		}

		/* print the symbol table for the child, if it exists */
		if (entry->tab) pcprintsymtabnode(entry->tab, depth+1);

		/* go to the next entry */
		entry = entry->next;
	}
}

void pcprintsymtab() {
	printf("\n===== SYMBOL TABLE =====\n");
	pcprintsymtabnode(root, 0);
}

symentry *pcaddsym(const char *name, symtype type, symval val, unsigned lineno) {
	symentry *entry;

	/* make sure we have a root */
	if (!current) return NULL;

	/* make sure it doesn't yet exist in this scope */
	if (pclookupsym_internal(name, 1)) {
		pcerror("{%d} ERR: %s already exists in symbol table.\n", lineno, name);
		return NULL;
	}

	/* populate our entry */
	if (!(entry = malloc(sizeof(*entry)))) return 0;
	entry->name 		= strdup(name);
	entry->type 		= type;
	entry->val 			= val;
	entry->lineno		= lineno;
	entry->tab 			= NULL;
	entry->returntype 	= notype;

	/* add to the head of the entries */
	entry->next			= current->entries;
	current->entries	= entry;
	return entry;
}

symentry *pclookupsym(const char *name) {
	return pclookupsym_internal(name, 0);
}

symentry *pcenterscope(const char *name, symtype type, unsigned lineno) {
	symentry *entry;
	if (!(entry = pcaddsym(name, type, (symval)0, lineno))) return NULL;

	/* create our table and make it the current, while updating it's parent */
	if (!(entry->tab = malloc(sizeof(*(entry->tab))))) return NULL;
	entry->tab->parent	= current;
	entry->tab->entries = NULL;
	current = entry->tab;

	return entry;
}

int pcleavescope() {
	/* can't leave if we're top dog */
	if (current == root) return 0;

	/* go up to our parent */
	current = current->parent;
	return 1;
}