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

pctoken *pcgettoken(FILE *fp);

#endif /* SCANNER_H */