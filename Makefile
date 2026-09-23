PREFIX ?= $(HOME)/.local
BINDIR ?= $(PREFIX)/bin
MANDIR ?= $(PREFIX)/share/man/man1
BASH_COMPLETION_DIR ?= $(PREFIX)/share/bash-completion/completions
ZSH_COMPLETION_DIR ?= $(PREFIX)/share/zsh/site-functions
FISH_COMPLETION_DIR ?= $(PREFIX)/share/fish/vendor_completions.d

CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2 -D_POSIX_C_SOURCE=200809L
LDFLAGS ?=

SRC = src/main.c src/config.c src/git.c src/patch.c src/render.c src/util.c src/version.c
OBJ = $(SRC:.c=.o)

.PHONY: all clean install uninstall test

all: ch-adds

ch-adds: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

src/%.o: src/%.c include/ch_adds.h
	$(CC) $(CFLAGS) -Iinclude -c -o $@ $<

install: ch-adds
	install -d "$(BINDIR)" "$(MANDIR)" \
		"$(BASH_COMPLETION_DIR)" "$(ZSH_COMPLETION_DIR)" "$(FISH_COMPLETION_DIR)"
	install -m 755 ch-adds "$(BINDIR)/ch-adds"
	install -m 644 man/ch-adds.1 "$(MANDIR)/ch-adds.1"
	install -m 644 completions/bash/ch-adds "$(BASH_COMPLETION_DIR)/ch-adds"
	install -m 644 completions/zsh/_ch-adds "$(ZSH_COMPLETION_DIR)/_ch-adds"
	install -m 644 completions/fish/ch-adds.fish "$(FISH_COMPLETION_DIR)/ch-adds.fish"
	-command -v mandb >/dev/null 2>&1 && mandb -q "$(MANDIR)" 2>/dev/null || true

uninstall:
	rm -f "$(BINDIR)/ch-adds" "$(MANDIR)/ch-adds.1" \
		"$(BASH_COMPLETION_DIR)/ch-adds" \
		"$(ZSH_COMPLETION_DIR)/_ch-adds" \
		"$(FISH_COMPLETION_DIR)/ch-adds.fish"

test: ch-adds
	@./tests/smoke.sh "$(CURDIR)/ch-adds"
	@./tests/completions.sh

clean:
	rm -f $(OBJ) ch-adds
