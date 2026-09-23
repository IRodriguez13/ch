# ch-adds

Show **only added lines** (`+`) from a file diff — editor-style, without noise from context or deletions.

Not a git replacement: one job, one file, one question — *what did I add here?*

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
```

See `man ch-adds` after install.

## Example output

```text
┌─ k8s/pre/all/values.yaml
│ ref: develop...HEAD
│ +1 line(s)
└────────────────────────────────────────
     4 │ +gesvulImage: docker-registry.../gesvul:4.8.22
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

MIT — see [LICENSE](LICENSE).
