#include "compiler.h"

#ifndef YYCOMPILE
# include "tokens.h"
# include "scanner.h"
# include "parser.h"
# include "symtab.h"
# include "io.h"
# include "ast.h"
# include "icg.h"
#endif /* YYCOMPILE */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef YYCOMPILE
	extern FILE *yyin;
	extern int yylex(void);
#else
	extern int pcscanerrors;
#endif /* YYCOMPILE */

void
usage(const char *progname) {
	printf("Usage: %s filename\n  filename\tPascal file to compile\n", progname);
}

char *
strtolower(char *s) {
	char *c = s;
	for ( ; *c; ++c) {
		*c = tolower(*c);
	}

	return s;
}

int 
main(int argc, char **argv) {
	FILE *fp;
	char *filename;
#ifdef YYCOMPILE
	int token;
#else
	pctoken *token;
	pctoken *nexttoken;
#endif /* YYCOMPILE */

	/* read the filename from command line */
	if (argc > 1) {
		filename = argv[1];

		/* open our file */
		fp = fopen(filename, "r");
		if (!fp) {
			printf("Unable to open file: %s.\n", filename);
			return EXIT_FAILURE;
		}

		#ifdef YYCOMPILE
			yyin = fp;
		#endif
	} else {
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	printf("Reading file: %s\n\n", filename);

	/** just run the lexer for now, skipping the scanner */
#ifdef YYCOMPILE
	while ((token = yylex())) {
		//printf("%d\n", token);
	}
#else
	/* initialize our symbol table */
	pcintializesymtab();

	if (pcparse(fp)) {
		printf("\nPARSING COMPLETED SUCCESSFULLY!!!!!\n");
	} else {
		printf("\nERRORS PARSING!!!!!\n");
		pcscanerrors = 1;
	}

	/* spit out errors */
	if (pcscanerrors) {
		printf("\n%d ERRORS during scanning!\n", pcscanerrors);
	}

	/* save our AST tree */
	FILE *astfp;
	if ((astfp = fopen("astfp.txt", "w"))) {
		AST_print(astroot, astfp);
		printf("\nSaved astfp.txt\n");
	}

	/* save our output */
	FILE *output;
	if (!(output = fopen("output.s", "w"))) {
		printf("\nUNABLE TO SAVE\n");
		pccleanupsymtab();
		return EXIT_FAILURE;
	}

	/* print our symbol table */
	pcprintsymtab();

	pcicg_start(output);
	AST_cleanup(&astroot);

	/* print our symbol table */
	pccleanupsymtab();
#endif /* YYCOMPILE */

	return EXIT_SUCCESS;
}