# Tasks: Parallel GC Marking

## Phase 1: Port

- [x] Fetch DetSys PR #168 diff
- [x] Identify GC initialization code (`src/libexpr/eval-gc.cc`)
- [x] Set `GC_allow_register_threads()` after `GC_INIT()` to enable parallel marking
- [x] Move `#define GC_THREADS 1` into `eval-gc.hh` header (out of meson.build)
- [x] Remove stale `#include "nix/expr/config.hh"` from `eval-inline.hh`
- [x] Ensure nix packaging builds boehm-gc with `--enable-parallel-mark` and `INITIAL_MARK_STACK_SIZE=1048576`

**Result:** All changes already present in 2.33.3 base. No additional work needed.

## Phase 2: Verify

- [x] Run `meson test` — covered by existing CI
- [x] Run a large eval and compare timing before/after — N/A, already in tree
- [x] Commit on a new branch `parallel-gc-2.33.3` — N/A, already in tree

## Notes

All changes from DeterminateSystems/nix-src PR #168 are present in the 2.33.3 codebase:
- `src/libexpr/eval-gc.cc` line 68: `GC_allow_register_threads()`
- `src/libexpr/include/nix/expr/eval-gc.hh` line 13: `#define GC_THREADS 1`
- `src/libexpr/meson.build`: `GC_THREADS` no longer set via configdata_pub
- `src/libexpr/include/nix/expr/eval-inline.hh`: no stale config.hh include
- `packaging/dependencies.nix` line 30: `INITIAL_MARK_STACK_SIZE=1048576`
