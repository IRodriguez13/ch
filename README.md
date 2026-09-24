# ch-adds

Show **only added lines** (`+`) from a file diff — editor-style, without noise from context or deletions.

Not a git replacement: one job, one question — *what did I add (or remove with `-d`)?*
Single file by default; **whole repo** when you omit the path or pass `--all`.

## Install

```bash
git clone https://github.com/IRodriguez13/ch-adds.git
cd ch-adds
./install.sh
```

Custom prefix:

```bash
PREFIX=/usr/local ./install.sh
```

Requires: `gcc`, `make`, `git` (for git mode).

## Shell alias (optional)

```bash
# ~/.bashrc
ch() { command ch-adds "$@"; }
```

## Usage

```bash
ch-adds path/to/file.py                 # vs HEAD (staged + unstaged)
ch-adds path/to/file.py develop         # vs develop...HEAD (MR style)
ch-adds path/to/file.py --staged
ch-adds path/to/file.py --unstaged
ch-adds path/to/file.py -p fix.patch
ch-adds path/to/file.py develop -q      # pipe-friendly
ch-adds path/to/file.py --removed       # + and - lines, diff order
ch -d                                   # all changed files (+ and -)
ch-adds -a develop                      # all files vs develop...HEAD
ch-adds Makefile src/main.c             # several explicit paths
```

See `man ch-adds` after install.

## Shell completions

`make install` installs completions for **bash**, **zsh**, and **fish** under
`$PREFIX/share/…`. The `ch` alias is covered too (`complete … ch` / `#compdef ch` /
`complete --wraps ch-adds -c ch`).

| Shell | Path (default `PREFIX=~/.local`) |
|-------|----------------------------------|
| bash  | `~/.local/share/bash-completion/completions/ch-adds` |
| zsh   | `~/.local/share/zsh/site-functions/_ch-adds` |
| fish  | `~/.local/share/fish/vendor_completions.d/ch-adds.fish` |

**bash** — requires the `bash-completion` package; user installs usually work out of the box.

**zsh** — add to `~/.zshrc` before `compinit`:

```bash
fpath=(~/.local/share/zsh/site-functions $fpath)
autoload -Uz compinit && compinit
```

**fish** — vendor completions load automatically; restart the shell or run `fish -c ch-adds<Tab>`.

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

## License

GPLv3+ — see [LICENSE](LICENSE).
