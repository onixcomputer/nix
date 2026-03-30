# Concurrent Substitution Deduplication

**Upstream:** DeterminateSystems/nix-src PR #398
**Upstream version:** Determinate Nix v3.17.2 (Mar 2026)
**Tier:** 1 — clean recent fix

## Why

When multiple builds need the same store path substituted, Nix currently
downloads it once per request. This wastes bandwidth and time, especially on
CI or multi-user daemons where many builds trigger simultaneously.

DetSys PR #398 deduplicates concurrent substitution requests so the same path
is only downloaded once, with other waiters sharing the result.

## What Changes

- **Substitution**: When a substitution is already in flight for a path,
  subsequent requests wait for the existing download instead of starting a new
  one.

## Capabilities

### Modified Capabilities
- `substitution`: Concurrent substitutions of the same path coalesce into one download

## Impact

- **Files**: `src/libstore/build/substitution-goal.cc` or equivalent
- **APIs**: No user-facing API changes
- **Dependencies**: None
- **Testing**: Add test with concurrent substitution of the same path

## Merge Conflict Risk

Low-medium. This is a recent (Mar 2026) PR against DetSys main which tracks
close to upstream 2.33.x. Should apply with minor fixups.
