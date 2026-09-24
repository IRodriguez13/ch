#!/usr/bin/env bash
set -euo pipefail

BIN="${1:?usage: smoke.sh /path/to/ch-adds}"
case "$BIN" in
  /*) ;;
  *) BIN="$(cd "$(dirname "$BIN")" && pwd)/$(basename "$BIN")" ;;
esac
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

cd "$TMP"
git init -q
git config user.email "test@example.com"
git config user.name "Test"

cat > sample.txt <<'EOF'
line one
line two
EOF
git add sample.txt
git commit -q -m "init"

echo "line three" >> sample.txt

out="$("$BIN" sample.txt -q -0)"
[[ "$out" == "line three" ]] || { echo "FAIL: expected added line"; echo "$out"; exit 1; }

sed -i '/line two/d' sample.txt

out="$("$BIN" sample.txt --removed -q)"
[[ "$out" == $'-line two\n+line three' ]] || {
	echo "FAIL: expected removal then addition"
	echo "$out"
	exit 1
}

cat > other.txt <<'EOF'
alpha
EOF
git add other.txt
git commit -q -m "other"
echo "beta" >> other.txt

out="$("$BIN" -a -q -0)"
echo "$out" | grep -qx 'line three' || { echo "FAIL: --all missing sample.txt addition"; echo "$out"; exit 1; }
echo "$out" | grep -qx 'beta' || { echo "FAIL: --all missing other.txt addition"; echo "$out"; exit 1; }

echo "smoke OK"
