PROJNAME = mini-pascal-compiler
YYNAME = yy-mini-pascal-compiler
CC = gcc
CFLAGS = 
YYCFLAGS = -DYYCOMPILE
YYOBJ = $(YYNAME).tab.o lex.yy.o
LEX = scanner.l
PARSE = parser.y
PARSEFLAGS = -v -d
REMOVEFILES = parser.tab.* lex.yy.* $(PROJNAME) $(YYNAME) *.s *.output *.o
SOURCES = compiler.c io.c scanner.c symtab.c tokens.c
YYSOURCES = compiler.c parser.tab.c lex.yy.c

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	YYCFLAGS += -lfl
endif
ifeq ($(UNAME_S),Darwin)
	YYCFLAGS += -ll
endif

hand: $(PROJNAME)

yy: $(YYNAME)

all: $(PROJNAME) $(YYNAME)

debug: CFLAGS += -DDEBUG
debug: $(PROJNAME)

debugyy: YYCFLAGS += -DDEBUG
debugyy: $(YYNAME)

$(PROJNAME):
	$(CC) $(SOURCES) $(CFLAGS) -o $@

$(YYNAME): $(YYOBJ)
	$(CC) $(YYSOURCES) $(YYCFLAGS) -o $@

$(YYNAME).tab.o: $(PARSE)
	bison $(PARSEFLAGS) $(PARSE)

lex.yy.o: $(LEX)
	flex $(LEX)

clean:
	rm -f $(REMOVEFILES)
