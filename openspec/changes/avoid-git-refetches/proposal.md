# Avoid Unnecessary Git Refetches

**Upstream:** DeterminateSystems/nix-src PR #270
**Upstream version:** Determinate Nix v3.13.2 (Nov 2025)
**Tier:** 2 — performance

## Why

When `nix flake update` fetches a Git input non-shallow (to compute revCount),
a subsequent eval may do a redundant shallow refetch because the repo is
already present but was fetched differently. The fix is to reuse the repo
from the first fetch when it already contains the needed revision.

## What Changes

- **Git fetcher**: Check if the locally cached repo already contains the target
  revision before initiating a new fetch. Reuse existing clones regardless of
  how they were originally fetched (shallow vs full).

## Capabilities

### Modified Capabilities
- `git-fetch-performance`: Reuse existing repo when revision is already available

## Impact

- **Files**: `src/libfetchers/git.cc` (fetch decision logic)
- **APIs**: None
- **Dependencies**: None
- **Testing**: Test that a full fetch followed by eval doesn't trigger a second fetch

## Merge Conflict Risk

Low-medium. The git fetcher caching logic is somewhat complex but the change
is a check-before-fetch guard.
