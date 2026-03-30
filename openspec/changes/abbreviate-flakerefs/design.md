# Design: Abbreviate Flakerefs

## Context

Full 40-char revision hashes in error messages and lockfile diffs are noisy.
`nix flake metadata --json` already provides full hashes for machine use.

## Decisions

### 1. Add FlakeRef::to_abbreviated_string()

**Choice:** New method on `FlakeRef` that calls `to_string()` then
truncates the rev hash portion to N characters (default 7).
**Rationale:** Works for all input types (github, git, mercurial) without
touching each scheme's `toURL()`. The rev always appears as a 40-char hex
string in the URL, so `rfind` + `replace` is safe.

### 2. Abbreviation sites

**Choice:** Use abbreviated form in:
- `describe()` in lockfile.cc (lockfile diffs)
- Error traces in flake.cc
- Input tree listing in `nix flake metadata`

Keep full form in:
- `Locked URL:` line (users may copy-paste it)
- `Revision:` line (explicitly for this purpose)
- All JSON output
- Lock file serialization

### 3. Default length: 7

**Choice:** 7 characters, matching git's default short hash.
**Rationale:** Standard convention. Collisions are negligible for
single-project use.
