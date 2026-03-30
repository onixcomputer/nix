# Tasks: Abbreviate Flakerefs

## Phase 1: Implementation

- [x] Add `FlakeRef::to_abbreviated_string()` declaration in `flakeref.hh`
- [x] Implement in `flakeref.cc` — truncate rev hash via `rfind`/`replace`
- [x] Use in `describe()` (lockfile.cc) for lockfile diffs
- [x] Use in error trace (flake.cc)
- [x] Use in metadata input tree listing (flake.cc)

## Phase 2: Testing

- [x] Update flakes.sh test to match abbreviated rev in metadata
- [x] Build and verify all tests pass
