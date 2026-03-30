# Build Provenance

**Upstream:** DeterminateSystems/nix-src PRs #321, #340, #342, #343, #344, #354, #356, #374
**Upstream version:** Determinate Nix v3.15.0–v3.17.0 (Dec 2025–Mar 2026)
**Tier:** 3 — multi-PR feature area

## Why

Supply chain security is a growing concern. Nix builds are reproducible but
there's no built-in way to answer "who built this, on what machine, from what
source, and can I verify that?" DetSys added a provenance system that:

1. Records build provenance metadata (builder host, timestamp, source info,
   meta attributes) during builds.
2. Stores provenance in the NAR info disk cache.
3. Provides `nix provenance show` to inspect provenance of store paths.
4. Provides `nix provenance verify` to cryptographically verify provenance.
5. Records provenance for unlocked inputs and impure evaluations.

This is a multi-PR feature spanning ~8 PRs over 3 months of development.

## What Changes

- **Build system**: Records provenance metadata during build and substitution.
- **NAR info cache**: Extended schema to store provenance alongside path info.
- **New CLI commands**: `nix provenance show`, `nix provenance verify`.
- **JSON log**: Build results include provenance in structured log output.

## Capabilities

### New Capabilities
- `provenance-record`: Builds record who/where/when/what provenance
- `provenance-show`: CLI command to inspect provenance of store paths
- `provenance-verify`: CLI command to verify provenance signatures
- `provenance-cache`: Provenance stored in NAR info disk cache

## Impact

- **Files**: New `src/nix/provenance.cc`, `src/libstore/build-provenance.{cc,hh}`,
  narinfo cache schema changes, JSON logger changes
- **APIs**: New CLI subcommands, extended NAR info format
- **Dependencies**: None beyond existing crypto libs
- **Testing**: Functional tests for provenance recording, display, and verification

## Merge Conflict Risk

High. This is 8 interconnected PRs. The NAR info cache schema changes and
JSON logger extensions need careful porting. The CLI commands themselves are
relatively self-contained. Recommend porting in phases:
1. Provenance data structures and recording
2. Cache storage
3. `nix provenance show`
4. `nix provenance verify`
