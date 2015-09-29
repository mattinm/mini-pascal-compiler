PROJNAME = mini-pascal-compiler
CC = gcc
CFLAGS = -lfl
OBJ = $(PROJNAME).tab.o lex.yy.o
LEX = scanner.l
PARSE = parser.y
PARSEFLAGS = -v -d
REMOVEFILES = parser.tab.* lex.yy.* $(PROJNAME) *.s *.output *.o

all: $(PROJNAME)

debug: CFLAGS += -DDEBUG
debug: $(PROJNAME)

$(PROJNAME): $(OBJ)
	$(CC) compiler.c parser.tab.c lex.yy.c $(CFLAGS) -o $@

$(PROJNAME).tab.o: $(PARSE)
	bison $(PARSEFLAGS) $(PARSE)

lex.yy.o: $(LEX)
	flex $(LEX)

clean:
	rm -f $(REMOVEFILES)