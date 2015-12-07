/*
symtab.h is responsible for storing our tokens so that they can be accessed later.
Things to focus on are names, values, and scope. Initially, the symtable is only
filled with keywords.

@author Marshall Mattingly
*/

#ifndef SYMTAB_H
#define SYMTAB_H

#include "tokens.h"

/*
Symtype holds information about the type of an entry in the symbol table.
*/
typedef enum symtype {
	/* Keywords */
	keywordtype = 0,

	/* Variable types */
	chartype,
	stringtype,
	integertype,
	realtype,

	/* Block types */
	programtype,
	proceduretype,
	functiontype,
	blocktype,

	/* total count of types */
	notype,
	numsymtypes
} symtype;

/* Parameters for functions and procedures. */
struct symentry;
typedef struct symparam {
	struct symentry *entry;
	struct symparam *next;
} symparam;

/*
Array of string representation for each symtype.
KEEP UP TO DATE WITH symtype enum.
*/
extern const char *symtypestr[numsymtypes];

/*
Symentry holds links for our table.
*/
struct symtab;
typedef struct symentry {
	const char 	*name;	/* name of the lexeme */
	symtype 	type;	/* type of the lexeme */
	symval		val;	/* value of the lexeme */
	unsigned	lineno;	/* line the lexeme was declared on */
	int 		bconst;	/* whether or not it's constant */

	struct symtab 	*tab;		/* symbol table for this entry (procedures and functions) */
	symtype 		returntype; /* return type for functions */
	struct symparam *params;	/* paramaters */

	/* link to the next entry */
	struct symentry *next;
} symentry;

struct symtab {
	struct symtab 	*parent;	/* parent symtab */
	symentry 		*block;		/* the block entry that starts this */
	symentry 		*entries;	/* linked list of entries */
};

typedef struct symtab symtab;

/*
Initializes the symbol table with keywords.

@return 1 on success; 0 otherwise
*/
int pcintializesymtab();

/*
Prints the symbol table.
*/
void pcprintsymtab();

/*
Cleans up the symbol table.
*/
void pccleanupsymtab();

/*
Adds a value to the symbol table.

@param name the name of the lexeme
@param type the type of the lexeme
@param val the value of the lexeme
@param bconst 1 if constant, 0 otherwise
@param lineno the line the lexeme is declared on
@return 1 on success; 0 otherwise
*/
symentry *pcaddsym(const char *name, symtype type, symval val, int bconst, unsigned lineno);

/*
Adds a variable to the symbol table as a parameter.

@param name the name of the lexeme
@param type the type of the lexeme
@param lineno the line the lexeme is declared on
@return 1 on success; 0 otherwise
*/
symentry *pcaddparam(const char *name, symtype type, unsigned lineno);

/*
Lookup a symbol from the current table.

@param name the name of the lexeme
@return the entry or NULL if not found
*/
symentry *pclookupsym(const char *name);

/*
Enters a new scope (creating a new symbol table and entry into 
the current symbol table.

@param name the name of the lexeme
@param type the type of the lexeme
@param lineno the line the lexeme is declared on
@return 1 on success; 0 otherwise
*/
symentry *pcenterscope(const char *name, symtype type, unsigned lineno);

/*
Leaves the current scope, returning to the parent scope.

@return 1 on success; 0 otherwise
*/
int pcleavescope();


#endif /* SYMTAB_H */