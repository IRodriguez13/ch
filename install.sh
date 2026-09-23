#!/usr/bin/env bash
# install.sh — build and install ch-adds
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

echo ""
echo "Installed:"
echo "  $PREFIX/bin/ch-adds"
echo "  $PREFIX/share/man/man1/ch-adds.1"
echo "  $PREFIX/share/bash-completion/completions/ch-adds"
echo "  $PREFIX/share/zsh/site-functions/_ch-adds"
echo "  $PREFIX/share/fish/vendor_completions.d/ch-adds.fish"
echo ""
echo "Optional shell alias (~/.bashrc):"
echo "  ch() { command ch-adds \"\$@\"; }"
echo ""
echo "Shell completions (bash / zsh / fish):"
echo "  bash: needs bash-completion; loads from \$PREFIX/share/bash-completion/completions"
echo "  zsh:  add to fpath, e.g. fpath=(\$HOME/.local/share/zsh/site-functions \$fpath)"
echo "  fish: auto-loads vendor_completions.d on startup"
echo ""
echo "Ensure \$PREFIX/bin is in PATH."
