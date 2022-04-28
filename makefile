.POSIX:

include config.mk

BIN = dulceti

TEST_DULCET = test_dulcet
TEST_DULCET_PARSER = test_dulcet_parser
TEST = $(TEST_DULCET) $(TEST_DULCET_PARSER)

SRC = dulceti.c dulcet.c test_dulcet.c dulcet_parser.c test_dulcet_parser.c
OBJ = $(SRC:.c=.o)
INC = dulcet.h dulcet_parser.h

all: $(BIN) $(LIB)

$(BIN): dulceti.o dulcet.o dulcet_parser.o
	$(CC) -o $@ dulceti.o dulcet.o dulcet_parser.o $(LDFLAGS)

$(TEST_DULCET): test_dulcet.o dulcet.o
	$(CC) -o $@ test_dulcet.o dulcet.o $(LDFLAGS)

$(TEST_DULCET_PARSER): test_dulcet_parser.o dulcet.o dulcet_parser.o
	$(CC) -o $@ test_dulcet_parser.o dulcet.o dulcet_parser.o $(LDFLAGS)

$(OBJ): $(INC)

.c.o:
	$(CC) -c -o $@ $< $(CPPFLAGS) $(CFLAGS)

test_dulcet.o: test_dulcet.c
	$(CC) -c -o $@ $< $(CPPFLAGS) -DZIDANE_SINGLE_THREADED $(CFLAGS)

test_dulcet_parser.o: test_dulcet_parser.c
	$(CC) -c -o $@ $< $(CPPFLAGS) -DZIDANE_SINGLE_THREADED $(CFLAGS)

test: $(TEST)
	@for t in $(TEST); do echo "./$$t"; ./$$t; done

clean:
	rm -f $(BIN) $(OBJ) $(TEST)

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/include
	install -m 775 dulceti $(DESTDIR)$(PREFIX)/bin
	install -m 664 dulcet.h $(DESTDIR)$(PREFIX)/include

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/dulceti
	rm -f $(DESTDIR)$(PREFIX)/include/dulcet.h

.PHONY: all test clean install uninstall
