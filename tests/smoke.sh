#!/usr/bin/env bash
set -euo pipefail

BIN="${1:?usage: smoke.sh /path/to/ch}"
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

out="$("$BIN" '*' -q -0)"
echo "$out" | grep -qx 'line three' || { echo "FAIL: literal * missing sample.txt addition"; echo "$out"; exit 1; }
echo "$out" | grep -qx 'beta' || { echo "FAIL: literal * missing other.txt addition"; echo "$out"; exit 1; }

mkdir -p nested
printf 'base\n' > nested/a.txt
printf 'base\n' > nested/b.txt
git add nested
git commit -q -m "nested"
echo "nested-a" >> nested/a.txt
echo "nested-b" >> nested/b.txt

out="$("$BIN" nested -d)"
echo "$out" | grep -q '┌─ nested/a.txt' || {
	echo "FAIL: directory pathspec should header nested/a.txt"
	echo "$out"
	exit 1
}
echo "$out" | grep -q '┌─ nested/b.txt' || {
	echo "FAIL: directory pathspec should header nested/b.txt"
	echo "$out"
	exit 1
}
! echo "$out" | grep -qE '┌─ nested$' || {
	echo "FAIL: directory pathspec must not use bare directory as header"
	echo "$out"
	exit 1
}

glob_args=()
for f in *; do
	glob_args+=("$f")
done
out="$("$BIN" -d "${glob_args[@]}" -q -0)"
echo "$out" | grep -qx 'line three' || { echo "FAIL: shell glob * missing sample.txt addition"; echo "$out"; exit 1; }
echo "$out" | grep -qx 'beta' || { echo "FAIL: shell glob * missing other.txt addition"; echo "$out"; exit 1; }

MIRROR="$TMP/mirror-test"
mkdir -p "$MIRROR/a/src" "$MIRROR/b/src"
printf 'same\n' > "$MIRROR/a/src/keep.py"
printf 'same\n' > "$MIRROR/b/src/keep.py"
printf 'old\n' > "$MIRROR/a/src/changed.py"
printf 'new\n' > "$MIRROR/b/src/changed.py"

out="$("$BIN" --diff "$MIRROR/a" "$MIRROR/b" src/changed.py -q)"
[[ "$out" == $'-old\n+new' ]] || {
	echo "FAIL: --diff single file expected -old/+new"
	echo "$out"
	exit 1
}

out="$("$BIN" --diff "$MIRROR/a" "$MIRROR/b" -a -q -0)"
echo "$out" | grep -qx 'new' || {
	echo "FAIL: --diff -a missing changed.py addition"
	echo "$out"
	exit 1
}
! echo "$out" | grep -qx 'same' || {
	echo "FAIL: --diff -a should skip identical file"
	exit 1
}

echo "smoke OK"
