#include "io.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

unsigned pclineno = 1;		/* current line number */

void
pcerror(const char *format, ...) {
	va_list args;

	/* va print to the error console */
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}

char
pcgetc(FILE *fp) {
	return fgetc(fp);
}

void
pcungetc(char c, FILE *fp) {
	ungetc(c, fp);
}