CC = gcc
CFLAGS = -pedantic -Werror -Ofast -g
LIBS = -lm
SRC = Branch.Grp1.c
BIN = Branch.Grp1

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

.PHONY: clean

clean:
	rm -f $(BIN)

