# Design: Fix nix-collect-garbage --dry-run

## Context

`nix-collect-garbage --dry-run` skips the GC entirely (`if (!dryRun)`) and
exits silently. The modern `nix store gc --dry-run` works correctly because
it uses `gcReturnDead` instead of `gcDeleteDead`.

## Decisions

### 1. Use gcReturnDead for dry-run

**Choice:** Run `collectGarbage` with `gcReturnDead` action when `--dry-run`
is specified. Print each dead path and a summary.
**Rationale:** The GC already supports `gcReturnDead` — it enumerates dead
paths without deleting. `nix store gc` uses this exact pattern. Aligning
`nix-collect-garbage` is a one-block change.

### 2. Output format

**Choice:** One store path per line, then a summary:
`N store paths would be deleted, X.XX MiB would be freed`
**Rationale:** Matches Lix CL/1566 format. "would be" distinguishes from
the non-dry-run `deleted`/`freed` wording.
