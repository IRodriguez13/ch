# ch

Show **only added lines** (`+`) from a file diff — editor-style, without noise from context or deletions.

Not a git replacement: one job, one question — *what did I add (or remove with `-d`)?*
Single file by default; **whole repo** when you omit the path, pass `--all`, `*`, or a shell glob that expands to all files in the directory.

## Install

```bash
git clone https://github.com/IRodriguez13/ch.git
cd ch
./install.sh
```

Custom prefix:

```bash
PREFIX=/usr/local ./install.sh
```

Requires: `gcc`, `make`, `git` (for git mode).

If you still have the old `ch() { command ch-adds … }` alias in `~/.bashrc`, remove it — the binary is now `ch`.

## Usage

```bash
ch path/to/file.py                 # vs HEAD (staged + unstaged)
ch path/to/file.py develop         # vs develop...HEAD (MR style)
ch path/to/file.py --staged
ch path/to/file.py --unstaged
ch path/to/file.py -p fix.patch
ch path/to/file.py develop -q      # pipe-friendly
ch path/to/file.py --removed       # + and - lines, diff order
ch -d                              # all changed files (+ and -)
ch -d *                            # same (shell glob)
ch -a develop                      # all files vs develop...HEAD
ch Makefile src/main.c             # several explicit paths
ch --diff ~/proj-a ~/proj-b        # mirror trees (different git history OK)
ch --diff ~/mirror ~/laptop dojo/foo.py --exclude=related --exclude=.env
```

See `man ch` after install.

## Shell completions

`make install` installs completions for **bash**, **zsh**, and **fish** under
`$PREFIX/share/…`.

| Shell | Path (default `PREFIX=~/.local`) |
|-------|----------------------------------|
| bash  | `~/.local/share/bash-completion/completions/ch` |
| zsh   | `~/.local/share/zsh/site-functions/_ch` |
| fish  | `~/.local/share/fish/vendor_completions.d/ch.fish` |

**bash** — requires the `bash-completion` package; user installs usually work out of the box.

**zsh** — add to `~/.zshrc` before `compinit`:

```bash
fpath=(~/.local/share/zsh/site-functions $fpath)
autoload -Uz compinit && compinit
```

**fish** — vendor completions load automatically; restart the shell or run `fish -c ch<Tab>`.

Tab-complete: flags, file paths, git branches/tags (`HEAD`, `develop`, …), and `.patch` files.

## Example output

```text
┌─ k8s/pre/all/values.yaml
│ ref: develop...HEAD
│ +1 line(s)
└────────────────────────────────────────
     4 │ +gesvulImage: docker-registry.../gesvul:4.8.22
```

With `--removed` (same box, removals in red, additions in green):

```text
┌─ src/foo.py
│ ref: develop...HEAD
│ +1  -1 change(s)
└────────────────────────────────────────
    12 │ -old_call()
    12 │ +new_call()
```

## Build / test

```bash
make
make test
make install
make uninstall PREFIX=$HOME/.local
```

## Design

| In scope | Out of scope |
|----------|--------------|
| Single-file `+` lines | Full diffs, `--stat`, blame |
| git ref or `.patch` | commit, merge, stash |
| `--staged` / `--unstaged` | multi-repo |
| `--diff DIR1 DIR2` (via `diff -ruN`) | reimplementing diff |

## License

GPLv3+ — see [LICENSE](LICENSE).
