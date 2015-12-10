/*
This file generates intermediary code (platform independent) form the parse tree
and symbol table created via the scanner.
*/

#ifndef ICG_H
#define ICG_H

#include "ast.h"

int pcicg_start(FILE *fpo);

#endif /* ICG_H */