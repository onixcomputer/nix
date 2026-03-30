# Tasks: builtins.filterAttrs

## Phase 1: Port

- [x] Fetch DetSys PR #291 diff and study the primop implementation
- [x] Add `prim_filterAttrs` to `src/libexpr/primops.cc`
- [x] Register `__filterAttrs` in the primops table (exposed as `builtins.filterAttrs`)
- [x] Add eval tests: `eval-okay-filterattrs.nix` (filter by value) and `eval-okay-filterattrs-names.nix` (filter by name)

## Phase 2: Verify

- [x] `nix build .#nix-cli` — compiles cleanly
- [x] Manual test: `builtins.filterAttrs (n: v: v > 5) { a = 3; b = 6; c = 10; }` → `{ b = 6; c = 10; }`
- [x] Manual test: `builtins.filterAttrs (n: v: n == "a") { a = 3; b = 6; c = 10; }` → `{ a = 3; }`
- [x] Edge cases: empty set, all-false predicate both return `{ }`
- [x] Committed on branch `raise-fd-limit-2.33.3` (bc883d78d)
