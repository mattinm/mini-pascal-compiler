#ifndef IO_H
#define IO_H

#include <stdio.h>

extern unsigned pclineno;

/*
Prints out an error message to the error console.
Use just like printf.
*/
void pcerror(const char *format, ...);

/*
Gets the next character from the FILE.

@param fp the FILE pointer
@return next character in the FILE
*/
char pcgetc(FILE *fp);

/*
Puts a character back onto the FILE.
*/
void pcungetc(char c, FILE *fp);

#endif /* IO_H */