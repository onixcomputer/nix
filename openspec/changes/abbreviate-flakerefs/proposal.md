# Abbreviate Flakerefs in Error Messages and Lockfile Diffs

**Upstream:** DeterminateSystems/nix-src PR #243, #264
**Upstream version:** Determinate Nix v3.12.0 (Oct 2025)
**Tier:** 2

## Why

Error messages and `nix flake metadata` output include full flake references
like `github:NixOS/nixpkgs/e21630230c77140bc6478a21cd71e8bb73706fce`. These
are noisy and hard to scan. DetSys abbreviates them to the shortest unambiguous
form (e.g. `nixpkgs/e216302`) in user-facing output while keeping full refs in
machine-readable (`--json`) output.

## What Changes

- **Error messages**: Flake references in error traces use abbreviated form.
- **nix flake metadata**: Lockfile diffs and input listings use abbreviated refs.
- **JSON output**: Unchanged (full refs preserved).

## Capabilities

### Modified Capabilities
- `error-messages`: Flake references abbreviated for readability
- `flake-metadata`: Shorter refs in human output

## Impact

- **Files**: `src/libflake/flake/flakeref.cc`, `src/nix/flake.cc`
- **APIs**: Human output format changes; JSON unchanged
- **Dependencies**: None
- **Testing**: Update expected output in flake functional tests

## Merge Conflict Risk

Low-medium. Two separate PRs but both touch flakeref formatting. Port together.
