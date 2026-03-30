# Hash Mismatch Diagnostics Include URL

**Upstream:** Lix CL/1536
**Upstream version:** Lix 2.91 (Aug 2024)
**Tier:** 2

## Why

When a fixed-output derivation fails its hash check, the error message shows
the expected and actual hashes but not which URL was involved. When you have
multiple derivations with the same placeholder hash (`AAAA...`), tracking down
which one failed requires manual detective work.

Lix guesses the URL from the `url` or `urls` environment variables in the
derivation and includes it in the error. Yes, it's a layering violation.
The diagnostic improvement is worth it.

## What Changes

- **Hash mismatch error**: When a FOD hash check fails, extract `url`/`urls`
  from the derivation environment and include it in the error message.

## Capabilities

### Modified Capabilities
- `error-messages`: Hash mismatch errors show the likely source URL

## Impact

- **Files**: `src/libstore/build/local-derivation-goal.cc` (hash check error path)
- **APIs**: None
- **Dependencies**: None
- **Testing**: Add functional test with a FOD that has wrong hash

## Merge Conflict Risk

Low. Touches the error formatting path in one place.
