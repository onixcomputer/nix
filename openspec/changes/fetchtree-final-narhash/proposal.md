# fetchTree Treats narHash-Supplied Trees as Final

**Upstream:** DeterminateSystems/nix-src PR #297
**Upstream version:** Determinate Nix v3.15.0 (Dec 2025)
**Tier:** 2 — performance

## Why

`builtins.fetchTree` with a `narHash` attribute still computes `lastModified`,
`revCount`, and other metadata even though they're not needed for correctness.
Treating the tree as "final" when `narHash` is supplied means:

1. Unnecessary metadata attributes aren't returned unless explicitly provided
   by the caller.
2. The tree can be substituted from a binary cache (since the narHash is known).
3. For Git inputs, shallow fetch is sufficient (no need to walk history for
   revCount).

This primarily benefits `flake-compat` users, since it calls `builtins.fetchTree`
internally.

## What Changes

- **builtins.fetchTree**: When `narHash` is supplied, treat the result as final.
  Don't compute or return `lastModified`, `revCount`, etc unless they were
  explicitly passed.
- **Substitution**: Allow the tree to be fetched from a binary cache.
- **Git fetch**: Use shallow fetch when the tree is final.

## Capabilities

### Modified Capabilities
- `fetchtree-performance`: narHash-supplied trees skip metadata computation
  and allow substitution

## Impact

- **Files**: `src/libfetchers/fetchers.cc`, `src/libfetchers/git.cc`
- **APIs**: `builtins.fetchTree` may return fewer attributes when narHash is
  given
- **Dependencies**: None
- **Testing**: Test fetchTree with narHash returns minimal attributes; test
  substitution from a local binary cache

## Merge Conflict Risk

Medium. Touches the input accessor and fetch result attribution logic, which
differs between upstream and DetSys.
