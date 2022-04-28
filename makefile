.POSIX:

include config.mk

BIN = dulceti
TEST = test_dulcet

SRC = dulceti.c dulcet.c test_dulcet.c
OBJ = $(SRC:.c=.o)
INC = dulcet.h

all: $(BIN) $(LIB)

$(BIN): dulceti.o dulcet.o
	$(CC) -o $@ dulceti.o dulcet.o $(LDFLAGS)


$(TEST): test_dulcet.o dulcet.o
	$(CC) -o $@ test_dulcet.o dulcet.o $(LDFLAGS)

$(OBJ): $(INC)

.c.o:
	$(CC) -c -o $@ $< $(CPPFLAGS) $(CFLAGS)

test_dulcet.o: test_dulcet.c
	$(CC) -c -o $@ $< $(CPPFLAGS) -DZIDANE_SINGLE_THREADED $(CFLAGS)

test: $(TEST)
	./$<

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
