# Raise Open File Soft Limit

**Upstream:** DeterminateSystems/nix-src PR #347
**Upstream version:** Determinate Nix v3.16.0 (Feb 2026)
**Tier:** 1 — one-liner fix

## Why

Large builds and evaluations (especially with many flake inputs or parallel
fetches) hit the default soft limit of 1024 open files. macOS is worse at 256.
Users get cryptic "too many open files" errors. The fix is trivial: raise the
soft limit to the hard limit at startup.

## What Changes

- **Startup**: Call `setrlimit(RLIMIT_NOFILE, ...)` raising soft to hard limit
  early in `main()`.

## Capabilities

### Modified Capabilities
- `startup`: Process initialization now maximizes available file descriptors

## Impact

- **Files**: `src/nix/main.cc` or `src/libutil/current-process.cc`
- **APIs**: None
- **Dependencies**: None
- **Testing**: Existing tests suffice; can add a functional test that opens many files

## Merge Conflict Risk

Minimal. Isolated change in process initialization.
