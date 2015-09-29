#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern FILE *yyin;

void usage(const char *progname) {
	printf("Usage: %s filename\n  filename\tPascal file to compile\n", progname);
}

char *strtolower(char *s) {
char *c = s;
	for ( ; *c; ++c) {
		*c = tolower(*c);
	}

	return s;
}

int main(int argc, char **argv) {
char *filename;
int token;

	// read the filename from command line
	if (argc > 1) {
		FILE *fp;
		filename = argv[1];

		// open our file
		fp = fopen(filename, "r");
		if (!fp) {
			printf("Unable to open file: %s.\n", filename);
			return EXIT_FAILURE;
		}

		yyin = fp;
	} else {
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	printf("Reading file: %s\n\n", filename);

	/** just run the lexer for now, skipping the scanner */
	while ((token = yylex())) {
		printf("%d\n", token);
	}

	return EXIT_SUCCESS;
}