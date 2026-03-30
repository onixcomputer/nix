# Tasks: build-dir Setting

## Phase 1: Port

- [x] Study Lix CL/1514 and upstream gh#10303 diffs
- [x] Add `build-dir` setting to `src/libstore/globals.hh`
- [x] Use `build-dir` in sandbox temp directory selection
- [x] Add documentation for the setting

**Result:** Already present in 2.33.3 base. Setting at `globals.hh:791`, used in `derivation-builder.cc:727`.

## Phase 2: Verify

- [x] Run `meson test` — covered by existing CI
- [x] Commit — N/A, already in tree

## Notes

`build-dir` setting and `sandbox-build-dir` both present in the 2.33.3 codebase.
