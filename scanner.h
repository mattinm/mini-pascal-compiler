/*
scanner.h is responsible for tokenizing our input for Pascal.

@author Marshall Mattingly
*/

#ifndef SCANNER_H
#define SCANNER_H

#include "tokens.h"
#include <stdio.h>

extern unsigned pclineno;
extern int pcscanerrors;
extern int pcscanwarnings;

/*
Gets the next token from the stream, or NULL if consumed.

@param fp the FILE pointer
@return a malloc'd next token, or NULL if consumed
*/
pctoken *pcgettoken(FILE *fp);

#endif /* SCANNER_H */