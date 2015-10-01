/*
compiler.h is the main entry point of the program and is responsible
for handling user input and running the other portions of the compiler,
such as the scanner and parser.

@author Marshall Mattingly
*/

#ifndef COMPILER_H
#define COMPILER_H

/* MACRO for debug printing */
#ifdef DEBUG
# define DEBUG_PRINTF(x) printf x
#else
# define DEBUG_PRINTF(x) do {} while (0)
#endif

/*
Transforms a string to a lower-case alternative.
Assumes that the string is NUL-terminated.

@param s the string to turn to lowercase
@return pointer to the front of s
*/
char *strtolower(char *s);

#endif /* COMPILER_H */