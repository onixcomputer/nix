# Tasks: Lazy Path Inputs

## Phase 1: Implementation

- [x] Return PosixSourceAccessor for non-store path inputs
- [x] Compute lastModified by stat-walking the directory tree
- [x] Compute NAR hash fingerprint via hashPath()
- [x] Tag lazy accessors with lazyPathInput flag
- [x] Re-mount store accessor in mountInput() for lazy path inputs
- [x] Preserve fingerprint when re-mounting

## Phase 2: Testing

- [x] All 208 functional tests pass
- [x] C API flake tests pass (nix_api_load_flake_with_flags)
- [x] All unit tests pass (flake-tests-run, fetchers-tests-run, expr-tests-run)
