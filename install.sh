#!/usr/bin/env bash
# install.sh — build and install ch
set -euo pipefail

PREFIX="${PREFIX:-${HOME}/.local}"
SRC_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

usage() {
  cat <<EOF
Usage: ./install.sh [options]

Options:
  --prefix PATH   install prefix (default: \$HOME/.local)
  --uninstall     remove installed binary and man page
  -h, --help      show this help

Examples:
  ./install.sh
  PREFIX=/usr/local ./install.sh
  ./install.sh --uninstall
EOF
}

uninstall=false
while [[ $# -gt 0 ]]; do
  case "$1" in
    --prefix)
      PREFIX="$2"
      shift 2
      ;;
    --uninstall)
      uninstall=true
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "install.sh: unknown option: $1" >&2
      usage >&2
      exit 1
      ;;
  esac
done

cd "$SRC_DIR"

if $uninstall; then
  make PREFIX="$PREFIX" uninstall
  echo "Uninstalled from $PREFIX"
  exit 0
fi

make clean all
make PREFIX="$PREFIX" install

installed="$PREFIX/bin/ch"
echo ""
echo "Installed:"
echo "  $installed"
echo "  $PREFIX/share/man/man1/ch.1"
echo "  $PREFIX/share/bash-completion/completions/ch"
echo "  $PREFIX/share/zsh/site-functions/_ch"
echo "  $PREFIX/share/fish/vendor_completions.d/ch.fish"
echo ""
echo "Shell completions (bash / zsh / fish):"
echo "  bash: needs bash-completion; loads from \$PREFIX/share/bash-completion/completions"
echo "  zsh:  add to fpath, e.g. fpath=(\$HOME/.local/share/zsh/site-functions \$fpath)"
echo "  fish: auto-loads vendor_completions.d on startup"
echo ""
echo "Ensure \$PREFIX/bin is in PATH."

if declare -F ch >/dev/null 2>&1; then
  echo ""
  echo "WARNING: shell function 'ch' is defined (e.g. ch() { command ch-adds ... })."
  echo "Remove that alias from ~/.bashrc — the installed binary is already named 'ch'."
fi

if command -v ch >/dev/null 2>&1; then
  active="$(command -v ch)"
  if [[ "$active" != "$installed" ]]; then
    echo ""
    echo "WARNING: 'ch' in PATH is $active, not $installed."
    echo "Run 'hash -r' or open a new shell after install."
  fi
fi
