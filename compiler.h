#ifndef COMPILER_H
#define COMPILER_H

#ifdef DEBUG
# define DEBUG_PRINTF(x) printf x
#else
# define DEBUG_PRINTF(x) do {} while (0)
#endif

/**
Transforms a string to a lower-case alternative.
Assumes that the string is NUL-terminated.
*/
char *strtolower(char *s);

#endif /* COMPILER_H */