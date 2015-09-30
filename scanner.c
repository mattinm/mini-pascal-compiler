#include "scanner.h"
#include "compiler.h"
#include "io.h"
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_BUFF		2048

int pcscanerrors = 0;
int pcscanwarnings = 0;

char line[LINE_BUFF];
char *lineptr = line;
size_t linesize = 0;

/*
Updates the current and next characters from the FILE stream.

@param cur the current character (will be overwritten)
@param next the next character (will be overwritten)
@param fp the FILE pointer
*/
void
pcgetnextc(char *cur, char *next, FILE *fp) {
	*cur = *next;
	*next = pcgetc(fp);

	/* add the current character to our line for printing */
	if (linesize < LINE_BUFF && *cur != EOF) {
		*lineptr++ = *cur;
		++linesize;
	}
}

/*
Appends the next character to the buffer at the given location and
increments the buffer.

@param b the current location in the buffer (will be overwritten)
@param cur the current character (will be overwritten)
@param next the next character (will be overwritten)
@param fp the FILE pointer
*/
void
pcappendnext(char **b, char *cur, char *next, FILE *fp) {
	pcgetnextc(cur, next, fp);
	**b = *cur;
	(*b)++;
}

/*
Determines if the character is a terminating character (i.e. punctuation).

@param c the character to check
@return 1 if terminating; 0 otherwise
*/
int
pcistermintor(char c) {
	return (
		c==':' || c==';' || c==',' || c=='.' ||
		c=='=' || c=='>' || c=='<' || 
		c=='(' || c==')' || c=='[' || c==']' ||
		c=='+' || c=='-' || c=='*' || c=='/'
	);
}

/*
Determines if the character signifies that we are at the end of the token
(i.e. EOF, whitespace, terminator) or some other random character that isn't
a letter or a number.

@param c the character to check
@return 1 if we should return; 0 otherwise
*/
int
pcisendoftoken(char c) {
	return (
		c == EOF || isspace(c) || pcistermintor(c)/* ||
		(!isalpha(c) && !isdigit(c))*/
	);
}

/*
Determines if the character is a random character (i.e. not a end of token,
not alpha, and not a number).
*/
int
pcisrandom(char c) {
	return (!pcisendoftoken(c) && !isalpha(c) && !isdigit(c));
}

void
pcresetline() {
	/* print the line */
	*lineptr = '\0';
	printf("[%d] %s\n", pclineno, line);

	/* reset the lineptr and size */
	lineptr = line;
	linesize = 0;
}

/*
Skips whitespace.
*/
void
pcskipwhitespace(char *cur, char *next, FILE *fp) {
	int dontprint = 0;
	while (isspace(*cur)) {
		/* see if we have a new line */
		if (*cur == '\n') {
			/* don't print multiple blank lines */
			if (!dontprint) {
				pcresetline();
				dontprint = 1;
			} else {
				lineptr = line;
				linesize = 0;
				*lineptr = '\0';
			}

			/* increment our line counter */
			++pclineno;
		}

		pcgetnextc(cur, next, fp);
	}
}

/*
Pulls a keyword from the given buffer, if available.

@param b the buffer the check
@param sym the pcsym to be updated
@return 1 on success; 0 on failure (not a keyword)
*/
int
pcgetkeyword(char *b, pcsym *sym) {
	if (strcmp("div", b) == 0) *sym = idivsym;
	else if (strcmp("mod", b) == 0) *sym = modsym;

	else if (strcmp("program", b) == 0) *sym = programsym;
	else if (strcmp("procedure", b) == 0) *sym = proceduresym;
	else if (strcmp("function", b) == 0) *sym = functionsym;
	else if (strcmp("begin", b) == 0) *sym = beginsym;
	else if (strcmp("end", b) == 0) *sym = endsym;

	else if (strcmp("and", b) == 0) *sym = andsym;
	else if (strcmp("or", b) == 0) *sym = orsym;
	else if (strcmp("not", b) == 0) *sym = notsym;

	else if (strcmp("if", b) == 0) *sym = ifsym;
	else if (strcmp("else", b) == 0) *sym = elsesym;
	else if (strcmp("then", b) == 0) *sym = thensym;
	else if (strcmp("do", b) == 0) *sym = dosym;
	else if (strcmp("while", b) == 0) *sym = whilesym;

	else if (strcmp("array", b) == 0) *sym = arraysym;
	else if (strcmp("of", b) == 0) *sym = ofsym;
	else if (strcmp("char", b) == 0) *sym = charsym;
	else if (strcmp("string", b) == 0) *sym = stringsym;
	else if (strcmp("integer", b) == 0) *sym = integersym;
	else if (strcmp("real", b) == 0) *sym = realsym;
	else if (strcmp("var", b) == 0) *sym = varsym;

	else if (strcmp("chr", b) == 0) *sym = chrsym;
	else if (strcmp("ord", b) == 0) *sym = ordsym;
	else if (strcmp("read", b) == 0) *sym = readsym;
	else if (strcmp("readln", b) == 0) *sym = readlnsym;
	else if (strcmp("write", b) == 0) *sym = writesym;
	else if (strcmp("writeln", b) == 0) *sym = writelnsym;

	/* unknown keyword */
	else return 0;

	/* found a keyword; sym has been updated */
	return 1;
}

pctoken *
pcgettoken(FILE *fp) {
	char 	cur, next,		/* current and next characters in the FILE */
			*b, buf[255];	/* buffer filled while grabbing characters */
	symval	val;			/* value of the token */
	pcsym	sym;			/* sym of the token */

	/* skip whitespace */
	next = pcgetc(fp);
	pcgetnextc(&cur, &next, fp);
	pcskipwhitespace(&cur, &next, fp);

	/* end-of-file? */
	if (cur == EOF) {
		pcresetline();
		return NULL;
	}

	/* initialize our variables */
	b 	= buf;
	*b 	= '\0';
	sym = eofsym;
	val.ival = 0;

	/* check the easy stuff first, the terminators */
	if (pcistermintor(cur)) {
		switch (cur) {
			case '(':	sym = lparensym; break;
			case ')':	sym = rparensym; break;
			case '[':	sym = lbracksym; break;
			case ']':	sym = rbracksym; break;
			case ';':	sym = semicolonsym; break;
			case ',':	sym = commasym; break;
			case '.':	sym = dotsym; break;
			case ':':
				if (next == '=') {
					sym = assignsym;
					pcgetnextc(&cur, &next, fp);
				} else {
					sym = colonsym;
				}
				break;
			case '=':	sym = eqsym; break;
			case '<':
				if (next == '=') {
					sym = ltesym;
					pcgetnextc(&cur, &next, fp);
				} else if (next == '>') {
					sym = neqsym;
					pcgetnextc(&cur, &next, fp);
				} else {
					sym = ltsym;
				}
				break;
			case '>':
				if (next == '=') {
					sym = gtesym;
					pcgetnextc(&cur, &next, fp);
				} else {
					sym = gtsym;
				}
				break;
			case '+':	sym = addsym; break;
			case '-':	sym = minussym; break;
			case '*':	sym = multsym; break;
			case '/':	sym = divsym; break;
		}
	}
	/* now check for a number */
	else if (isdigit(cur)) {
		*b++ = cur;

		/* keep adding digits until the next isn't a digit */
		while (isdigit(next)) {
			pcappendnext(&b, &cur, &next, fp);
		}

		/* see if we have a dot and shift to real digit */
		if (next == '.') {
			pcappendnext(&b, &cur, &next, fp);

			/* keep adding digits until the next isn't a digit */
			while (isdigit(next)) {
				pcappendnext(&b, &cur, &next, fp);
			}

			/* see if we have a e or E and shift to scientific */
			if (next == 'e' || next == 'E') {
				pcappendnext(&b, &cur, &next, fp);

				/* check for +/- */
				if (next == '+' || next == '-') {
					pcappendnext(&b, &cur, &next, fp);
				}

				/* keep adding digits */
				while (isdigit(next)) {
					pcappendnext(&b, &cur, &next, fp);
				}

				/* if we don't have a terminal/whitespace now, ill formed real number */
				if (!pcisendoftoken(next)) {
					/* keep consuming until we do hit a space or terminator */
					while (!pcisendoftoken(next)) {
						pcappendnext(&b, &cur, &next, fp);
					}

					/* print the error */
					*b = '\0';
					pcerror("{%d} ERR: Ill formed real number: %s\n", pclineno, buf);
					++pcscanerrors;

					/* go to the next token */
					if (next == EOF) return NULL;

					pcungetc(next, fp);
					return pcgettoken(fp);
				}
			} 
			/* if we don't have a terminal / whitespace / random char, ill formed real number */
			else if (!pcisendoftoken(next)) {
					/* keep consuming until we do hit a space or terminator */
					while (!pcisendoftoken(next)) {
						pcappendnext(&b, &cur, &next, fp);
					}

					/* print the error */
					*b = '\0';
					pcerror("{%d} ERR: Ill formed real number: %s\n", pclineno, buf);
					++pcscanerrors;

					/* go to the next token */
					if (next == EOF) return NULL;

					pcungetc(next, fp);
					return pcgettoken(fp);
			}

			/* we have a legitimate real number! calculate and create our token */
			*b = '\0';
			val.rval = atof(buf);
			sym = realnosym;
		}

		/* if we don't have a end of token, ill formed integer number */
		else if (!pcisendoftoken(next)) {
			/* keep consuming until we do hit a space or terminator */
			while (!pcisendoftoken(next)) {
				pcappendnext(&b, &cur, &next, fp);
			}

			/* print the error */
			*b = '\0';
			pcerror("{%d} ERR: Ill formed integer number or id: %s\n", pclineno, buf);
			++pcscanerrors;

			/* go to the next token */
			if (next == EOF) return NULL;

			pcungetc(next, fp);
			return pcgettoken(fp);
		}

		/* we have a good integer! */
		else {
			*b = '\0';
			val.ival = atoi(buf);
			sym = integernosym;
		}
	}

	/* now check for strings and characters */
	else if (cur == '\'') {
		int scount = 0;

		/* add values to the buffer until we hit a \n or ' or EOF */
		while (next != '\n' && next != '\'' && next != EOF) {
			pcgetnextc(&cur, &next, fp);
			*b++ = cur;
			++scount;
		}

		/* if we hit a new line or EOF, then we have an ill-formed string */
		if (next == '\n' || next == EOF) {
			/* print the error */
			*b = '\0';
			pcerror("{%d} ERR: No closing ': %s\n", pclineno, buf);
			++pcscanerrors;

			/* go to the next token */
			if (next == EOF) return NULL;
			pcungetc(next, fp);
			return pcgettoken(fp);
		}

		/* warn about empty strings */
		*b = '\0';
		if (!scount) {
			pcerror("{%d} WARN: Empty string/character found.\n", pclineno);
			++pcscanwarnings;
		}

		/* prepare our character if 1 value */
		if (scount == 1) {
			sym = charvalsym;
			val.cval = *buf;
		}
		/* otherwise, it's a string */
		else {
			sym = stringvalsym;
			val.str = strdup(buf);
		}

		/* we consume another from the stream, so the tick doesn't go back in */
		pcgetnextc(&cur, &next, fp);
	}

	/* now check for keywords and id's */
	else if (isalpha(cur)) {
		*b++ = cur;

		/* consume letters and numbers */
		while (isalpha(next) || isdigit(next)) {
			pcappendnext(&b, &cur, &next, fp);
		}

		/* make sure we have an end of token */
		if (!pcisendoftoken(next)) {
			while (!pcisendoftoken(next)) {
				pcappendnext(&b, &cur, &next, fp);
			}

			/* print the error */
			*b = '\0';
			pcerror("{%d} ERR: Ill formed keyword or id: %s\n", pclineno, buf);
			++pcscanerrors;

			/* go to the next token */
			if (next == EOF) return NULL;

			pcungetc(next, fp);
			return pcgettoken(fp);
		}

		/* determine what kind of symbol we have */
		*b = '\0';
		strtolower(buf);
		if (!pcgetkeyword(buf, &sym)) {
			sym = idsym;
			val.id = strdup(buf);
		}
	}

	/* unknown character */
	else {
		pcerror("{%d} ERR: Unknown character: %c\n", pclineno, cur);
		++pcscanerrors;

		/* get the next token */
		if (next == EOF) return NULL;

		pcungetc(next, fp);
		return pcgettoken(fp);
	}

	/* unget our next value (so it's our current in next call) */
	if (next != EOF) pcungetc(next, fp);

	/* generate and return our token */
	return pcnewtoken(sym, val, pclineno);
}