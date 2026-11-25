CC ?= cc
CFLAGS ?= -O2
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
MANDIR ?= $(PREFIX)/share/man/man1
CURSES_LIBS ?= -lncurses

all: randomwire

randomwire: randomwire.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -std=c11 -Wall -Wextra -Wpedantic $(LDFLAGS) -o $@ randomwire.c $(CURSES_LIBS) -lm

test: randomwire
	./randomwire --test

install: randomwire
	mkdir -p "$(DESTDIR)$(BINDIR)" "$(DESTDIR)$(MANDIR)"
	install -m 755 randomwire "$(DESTDIR)$(BINDIR)/randomwire"
	install -m 644 randomwire.1 "$(DESTDIR)$(MANDIR)/randomwire.1"

uninstall:
	rm -f "$(DESTDIR)$(BINDIR)/randomwire" "$(DESTDIR)$(MANDIR)/randomwire.1"

clean:
	rm -f randomwire

.PHONY: all test install uninstall clean
