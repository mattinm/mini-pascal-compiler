#include "compiler.h"

#ifndef YYCOMPILE
# include "tokens.h"
# include "scanner.h"
# include "io.h"
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
	while ((token = pcgettoken(fp))) {
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
	}

	if (pcscanerrors) {
		printf("\n%d ERRORS during scanning!\n", pcscanerrors);
	}
#endif /* YYCOMPILE */

	return EXIT_SUCCESS;
}