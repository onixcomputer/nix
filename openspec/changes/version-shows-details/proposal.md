# nix --version Shows Installation Details

**Upstream:** Lix CL/2365
**Upstream version:** Lix 2.92 (Jan 2025)
**Tier:** 2 — UX improvement

## Why

`nix --version` shows only the version string. To see system type, features,
config file paths, and store directory you need `nix --verbose --version` —
which nobody remembers. `nix-env --version` has always shown this by default
due to verbosity quirks.

## What Changes

- **nix --version**: Prints full details by default:
  - System type and additional system types
  - Enabled features (gc, signed-caches, etc)
  - System and user configuration file paths
  - Store and state directories

## Capabilities

### Modified Capabilities
- `cli-version`: `nix --version` shows full installation details

## Impact

- **Files**: `src/nix/main.cc` (version output)
- **APIs**: None
- **Dependencies**: None
- **Testing**: Test that `nix --version` includes system type and config paths

## Merge Conflict Risk

Low. Small change to the version printing code.
