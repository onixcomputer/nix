# Reproducibility Checks Report All Differing Outputs

**Upstream:** Lix CL/2069
**Upstream version:** Lix 2.92 (Jan 2025)
**Tier:** 1 — bug fix

## Why

`nix-build --check` reruns a build to verify reproducibility. For multi-output
derivations, only the first differing output is reported and kept for
comparison. The rest are silently discarded, forcing users to re-run multiple
times to find all non-deterministic outputs.

## What Changes

- **Build result handling**: When `--check` finds mismatches, keep and report
  ALL differing outputs, not just the first one.

## Capabilities

### Modified Capabilities
- `reproducibility-check`: Reports all differing outputs in one run

## Impact

- **Files**: `src/libstore/build/local-derivation-goal.cc` (output comparison logic)
- **APIs**: None
- **Dependencies**: None
- **Testing**: Test `--check` on a multi-output derivation where multiple outputs differ

## Merge Conflict Risk

Low. The change is in the output comparison loop — continue iterating instead
of breaking on first mismatch.
