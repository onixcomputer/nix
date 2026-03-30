# nix flake metadata Prints Modified Dates

**Upstream:** Lix CL/1700
**Upstream version:** Lix 2.91 (Aug 2024)
**Tier:** 1 — UX improvement

## Why

"When did I last update nixpkgs?" is a common question with no good answer.
`nix flake metadata` shows inputs but not when they were last modified.
Adding timestamps for each locked input makes the answer immediately visible.

## What Changes

- **nix flake metadata**: Print `Last modified:` date for the root flake and
  for each locked input.

## Capabilities

### Modified Capabilities
- `flake-metadata`: Shows last-modified timestamps for all inputs

## Impact

- **Files**: `src/nix/flake.cc` (metadata display)
- **APIs**: None
- **Dependencies**: None
- **Testing**: Test that `nix flake metadata` output includes date strings

## Merge Conflict Risk

Low. Adds a few lines to the metadata printer. The `lastModified` data is
already available in the lock file.
