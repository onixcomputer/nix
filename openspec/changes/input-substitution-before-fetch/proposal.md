# Try Substitution Before Fetching Inputs

**Upstream:** DeterminateSystems/nix-src PR #380
**Upstream version:** Determinate Nix v3.17.0 (Mar 2026)
**Tier:** 2

## Why

When resolving a locked flake input, Nix fetches from the original source (e.g.
GitHub) even when the exact NAR hash is already available in a binary cache.
Checking the cache first avoids network round-trips to upstream sources and is
faster when you have a well-populated cache.

## What Changes

- **Input resolution**: `Input::getAccessor()` checks binary cache substituters
  for the locked NAR hash before attempting to fetch from the original source.

## Capabilities

### Modified Capabilities
- `input-fetch`: Locked inputs check substituters before fetching from source

## Impact

- **Files**: `src/libfetchers/fetchers.cc` or `src/libfetchers/input-accessor.cc`
- **APIs**: No user-facing changes
- **Dependencies**: None
- **Testing**: Test with a locked input that exists in a local binary cache

## Merge Conflict Risk

Low-medium. The input accessor interface is the main concern. The logic itself
is straightforward: try substitute, fall back to fetch.
