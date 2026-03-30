# Lazy Path Inputs

**Upstream:** DeterminateSystems/nix-src PR #312
**Upstream version:** Determinate Nix v3.15.1 (Jan 2026)
**Tier:** 2

## Why

When a flake uses `path:` inputs (common during local development), Nix eagerly
copies the entire directory tree into the store before evaluation begins. For
large repos this is slow and wasteful — evaluation might only need a few files.

Lazy path inputs defer the store copy, only materializing files when the
evaluator actually accesses them. This speeds up `nix build`, `nix develop`,
and `nix flake check` on local repos.

## What Changes

- **Path input fetcher**: Returns a lazy accessor that reads files on demand
  instead of copying the whole tree upfront.

## Capabilities

### Modified Capabilities
- `path-inputs`: Path-type flake inputs are fetched lazily

## Impact

- **Files**: `src/libfetchers/path.cc`, possibly `src/libfetchers/input-accessor.cc`
- **APIs**: No user-facing changes; transparent speedup
- **Dependencies**: None
- **Testing**: Existing flake tests cover correctness; add timing test for large path inputs

## Merge Conflict Risk

Medium. Touches the fetcher/accessor abstraction which has evolved between versions.
May need adaptation to 2.33.3 accessor API.
