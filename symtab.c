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
	"block"
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
	current->children 	= NULL;
	current->entries 	= NULL;

	val.ival = 0;

	/* this is expanded from scanner.c -> pcgetkeyword() */
	/* TODO: make a pckeywords struct array */
	
	pcaddsym("div", keywordtype, val, 0);
	pcaddsym("mod", keywordtype, val, 0);

	pcaddsym("program", keywordtype, val, 0);
	pcaddsym("procedure", keywordtype, val, 0);
	pcaddsym("function", keywordtype, val, 0);
	pcaddsym("begin", keywordtype, val, 0);
	pcaddsym("end", keywordtype, val, 0);

	pcaddsym("and", keywordtype, val, 0);
	pcaddsym("or", keywordtype, val, 0);
	pcaddsym("not", keywordtype, val, 0);

	pcaddsym("if", keywordtype, val, 0);
	pcaddsym("else", keywordtype, val, 0);
	pcaddsym("then", keywordtype, val, 0);
	pcaddsym("do", keywordtype, val, 0);
	pcaddsym("while", keywordtype, val, 0);

	pcaddsym("array", keywordtype, val, 0);
	pcaddsym("of", keywordtype, val, 0);
	pcaddsym("char", keywordtype, val, 0);
	pcaddsym("string", keywordtype, val, 0);
	pcaddsym("integer", keywordtype, val, 0);
	pcaddsym("real", keywordtype, val, 0);
	pcaddsym("var", keywordtype, val, 0);

	pcaddsym("chr", keywordtype, val, 0);
	pcaddsym("ord", keywordtype, val, 0);
	pcaddsym("read", keywordtype, val, 0);
	pcaddsym("readln", keywordtype, val, 0);
	pcaddsym("write", keywordtype, val, 0);
	pcaddsym("writeln", keywordtype, val, 0);

	return 1;
}

void pcprintsymtabnode(symtab *node, unsigned depth) {
	symtabentry *child;
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
		printf("%s%s (%s) : %d : %s\n", tabs, entry->name, symtypestr[entry->type], entry->lineno, entry->val.str);
		entry = entry->next;
	}

	/* print all children (will cascade) */
	child = node->children;
	while (child) {
		pcprintsymtabnode(child->ptr, depth+1);
		child = child->next;
	}
}

void pcprintsymtab() {
	printf("\n===== SYMBOL TABLE =====\n");
	pcprintsymtabnode(root, 0);
}

int pcaddsym(const char *name, symtype type, symval val, unsigned lineno) {
	symentry *entry;

	/* make sure we have a root */
	if (!current) return 0;

	/* make sure it doesn't yet exist in this scope */
	if (pclookupsym_internal(name, 1)) {
		pcerror("{%d} ERR: %s already exists in symbol table.\n", lineno, name);
		return 0;
	}

	/* populate our entry */
	if (!(entry = malloc(sizeof(*entry)))) return 0;
	entry->name 	= strdup(name);
	entry->type 	= type;
	entry->val 		= val;
	entry->lineno	= lineno;

	/* add to the head of the entries */
	entry->next			= current->entries;
	current->entries	= entry;
	return 1;
}

symentry *pclookupsym(const char *name) {
	return pclookupsym_internal(name, 0);
}

int pcenterscope(const char *name, symtype type, unsigned lineno) {
	// add later

	return 1;
}

int pcleavescope() {
	/* can't leave if we're top dog */
	if (current == root) return 0;

	/* go up to our parent */
	current = current->parent;
	return 1;
}