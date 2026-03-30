# Tasks: nix flake show Prints Package Descriptions

## Phase 1: Implementation

- [x] Hoist `meta.description` evaluation above JSON/non-JSON branch
- [x] Wrap in try/catch for broken meta attrs
- [x] Append description to non-JSON package output when present

## Phase 2: Testing

- [x] Add functional test: non-JSON output includes description
- [x] Add functional test: JSON output still includes description
- [x] Build and verify all tests pass
