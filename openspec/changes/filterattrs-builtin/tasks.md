# Tasks: builtins.filterAttrs

## Phase 1: Port

- [ ] Fetch DetSys PR #291 diff and study the primop implementation
- [ ] Add `prim_filterAttrs` to `src/libexpr/primops.cc`
- [ ] Register `filterAttrs` in the primops table
- [ ] Add eval test `tests/functional/lang/eval-okay-filterAttrs.nix`
- [ ] Add expected output `tests/functional/lang/eval-okay-filterAttrs.exp`

## Phase 2: Verify

- [ ] Run `meson test` — all existing tests pass
- [ ] Run the new filterAttrs eval test
- [ ] Test that `builtins.filterAttrs (n: v: n != "x") { x = 1; y = 2; }` returns `{ y = 2; }`
- [ ] Commit on a new branch `filterattrs-2.33.3`
