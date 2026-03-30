# Tasks: Fix nix-collect-garbage --dry-run

## Phase 1: Implementation

- [x] Change `nix-collect-garbage` to run GC with `gcReturnDead` when `--dry-run`
- [x] Print dead paths and summary in dry-run mode

## Phase 2: Testing

- [x] Add functional test: `--dry-run` lists dead paths and shows "would be freed"
- [x] Add functional test: `--dry-run` does not delete paths
- [x] Build and verify all tests pass
