# Tasks: Hash Mismatch URL Diagnostic

## Phase 1: Implementation

- [x] Add `const StringPairs & env` parameter to `checkOutputs` declaration in `derivation-check.hh`
- [x] Add `const StringPairs & env` parameter to `checkOutputs` definition in `derivation-check.cc`
- [x] Extract URL from `env` (`url` key, then first token of `urls` key)
- [x] Append URL to hash mismatch error format string when present
- [x] Update call site in `derivation-builder.cc` to pass `drv.env`

## Phase 2: Testing

- [x] Add functional test: FOD with wrong hash shows URL in error
- [x] Build and verify compilation (all 208 tests pass)
