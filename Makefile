PREFIX ?= $(HOME)/.local
BINDIR ?= $(PREFIX)/bin
MANDIR ?= $(PREFIX)/share/man/man1

CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2 -D_POSIX_C_SOURCE=200809L
LDFLAGS ?=

SRC = src/main.c src/config.c src/git.c src/patch.c src/render.c src/util.c
OBJ = $(SRC:.c=.o)

.PHONY: all clean install uninstall test

all: ch-adds

ch-adds: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

src/%.o: src/%.c include/ch_adds.h
	$(CC) $(CFLAGS) -Iinclude -c -o $@ $<

install: ch-adds
	install -d "$(BINDIR)" "$(MANDIR)"
	install -m 755 ch-adds "$(BINDIR)/ch-adds"
	install -m 644 man/ch-adds.1 "$(MANDIR)/ch-adds.1"
	-command -v mandb >/dev/null 2>&1 && mandb -q "$(MANDIR)" 2>/dev/null || true

uninstall:
	rm -f "$(BINDIR)/ch-adds" "$(MANDIR)/ch-adds.1"

test: ch-adds
	@./tests/smoke.sh "$(CURDIR)/ch-adds"

clean:
	rm -f $(OBJ) ch-adds
