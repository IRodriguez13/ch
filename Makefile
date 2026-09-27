PREFIX ?= $(HOME)/.local
BINDIR ?= $(PREFIX)/bin
MANDIR ?= $(PREFIX)/share/man/man1
BASH_COMPLETION_DIR ?= $(PREFIX)/share/bash-completion/completions
ZSH_COMPLETION_DIR ?= $(PREFIX)/share/zsh/site-functions
FISH_COMPLETION_DIR ?= $(PREFIX)/share/fish/vendor_completions.d

CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2 -D_POSIX_C_SOURCE=200809L
LDFLAGS ?=

SRC = src/main.c src/config.c src/git.c src/patch.c src/render.c src/util.c \
      src/version.c src/dirdiff.c
OBJ = $(SRC:.c=.o)

.PHONY: all clean install uninstall test

all: ch

ch: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

src/%.o: src/%.c include/ch.h
	$(CC) $(CFLAGS) -Iinclude -c -o $@ $<

install: ch
	install -d "$(BINDIR)" "$(MANDIR)" \
		"$(BASH_COMPLETION_DIR)" "$(ZSH_COMPLETION_DIR)" "$(FISH_COMPLETION_DIR)"
	install -m 755 ch "$(BINDIR)/ch"
	install -m 644 man/ch.1 "$(MANDIR)/ch.1"
	install -m 644 completions/bash/ch "$(BASH_COMPLETION_DIR)/ch"
	install -m 644 completions/zsh/_ch "$(ZSH_COMPLETION_DIR)/_ch"
	install -m 644 completions/fish/ch.fish "$(FISH_COMPLETION_DIR)/ch.fish"
	-command -v mandb >/dev/null 2>&1 && mandb -q "$(MANDIR)" 2>/dev/null || true

uninstall:
	rm -f "$(BINDIR)/ch" "$(MANDIR)/ch.1" \
		"$(BASH_COMPLETION_DIR)/ch" \
		"$(ZSH_COMPLETION_DIR)/_ch" \
		"$(FISH_COMPLETION_DIR)/ch.fish"
	rm -f "$(BINDIR)/ch-adds" "$(MANDIR)/ch-adds.1" \
		"$(BASH_COMPLETION_DIR)/ch-adds" \
		"$(ZSH_COMPLETION_DIR)/_ch-adds" \
		"$(FISH_COMPLETION_DIR)/ch-adds.fish"

test: ch
	@./tests/smoke.sh "$(CURDIR)/ch"
	@./tests/completions.sh

clean:
	rm -f $(OBJ) ch ch-adds
