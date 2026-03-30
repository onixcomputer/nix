# builtins.filterAttrs

**Upstream:** DeterminateSystems/nix-src PR #291
**Upstream version:** Determinate Nix v3.14.0 (Dec 2025)
**Tier:** 1 — small, universal win

## Why

Every Nix codebase uses `lib.filterAttrs` from nixpkgs. It's implemented in Nix
as a fold + `removeAttrs`, which forces evaluation of the entire attribute set
even when most entries pass the predicate. A C++ builtin can skip the
intermediate set construction and short-circuit per-attribute.

This is one of the most called library functions in nixpkgs. Making it a builtin
gives a direct eval speedup for any expression that filters attribute sets.

## What Changes

- **builtins.filterAttrs**: New primop `filterAttrs (pred: name -> value -> bool) (set) -> set`
  Applies `pred` to each attribute; returns the subset where `pred` returns true.

## Capabilities

### New Capabilities
- `filterattrs-builtin`: Native attribute set filtering without nixpkgs dependency

## Impact

- **Files**: `src/libexpr/primops.cc` (new primop registration)
- **APIs**: Adds `builtins.filterAttrs` — no existing API changes
- **Dependencies**: None
- **Testing**: Add eval test in `tests/functional/lang/`

## Merge Conflict Risk

Low. Single primop addition. DetSys PR #291 was 14 comments, mostly bikeshedding the name and semantics (lazy vs strict predicate application). The actual diff is small.
