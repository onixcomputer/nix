# Skip revCount/lastModified Computation When Already Specified

**Upstream:** DeterminateSystems/nix-src PR #269
**Upstream version:** Determinate Nix v3.13.2 (Nov 2025)
**Tier:** 2 — performance

## Why

When a lock file already specifies `revCount` and `lastModified` for a Git
input, Nix still recomputes them by walking the git log. On large repos this
is slow. These values are non-security-critical metadata — an incorrect value
doesn't affect build reproducibility (unlike `narHash`). Trusting the lock
file values saves significant time.

## What Changes

- **Git fetcher**: Skip `revCount` / `lastModified` computation when the
  values are already present in the locked input attributes.

## Capabilities

### Modified Capabilities
- `git-fetch-performance`: Skip redundant revCount/lastModified computation

## Impact

- **Files**: `src/libfetchers/git.cc` (post-fetch attribute computation)
- **APIs**: None
- **Dependencies**: None
- **Testing**: Verify that a locked input with known revCount doesn't trigger
  git log traversal

## Merge Conflict Risk

Low. Conditional check before the computation functions.
