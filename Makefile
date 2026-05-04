PREFIX ?= /usr
CFLAGS += -std=c99 -Wextra

.PHONY: all clean

all: mines

mines: mines.c
	cc $(CFLAGS) mines.c -o mines

install: mines
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -p mines $(DESTDIR)$(PREFIX)/bin/mines

clean:
	rm -f mines
