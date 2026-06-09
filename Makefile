CC      ?= cc
CFLAGS  ?= -std=c99 -Wall -Wextra -Wpedantic -Wshadow \
           -Wstrict-prototypes -Wmissing-prototypes \
           -Wold-style-definition -fno-common -O2
LDFLAGS ?=
LIBS    ?=

LIBS_NCURSES   := $(shell pkg-config --libs ncurses 2>/dev/null || echo -lncurses)
CFLAGS_NCURSES := $(shell pkg-config --cflags ncurses 2>/dev/null)
CFLAGS += $(CFLAGS_NCURSES)
LIBS   += $(LIBS_NCURSES)

PREFIX  ?= /usr/local
DESTDIR ?=

SRC = src/yeti.c
BIN = yeti

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LIBS)

clean:
	rm -f $(BIN)

install: $(BIN)
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 $(BIN) $(DESTDIR)$(PREFIX)/bin/$(BIN)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(BIN)

size:
	@wc -c $(SRC)

init-hooks:
	install -d .git/hooks
	ln -sf ../../scripts/commit-msg .git/hooks/commit-msg
	chmod +x .git/hooks/commit-msg

.PHONY: all clean install uninstall size init-hooks
