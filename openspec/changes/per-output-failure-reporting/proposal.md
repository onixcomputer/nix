# Per-Output Failure Reporting in nix build / nix flake check

**Upstream:** DeterminateSystems/nix-src PRs #281, #285
**Upstream version:** Determinate Nix v3.14.0 (Dec 2025)
**Tier:** 2 — CI UX

## Why

When multiple outputs fail in `nix build` or `nix flake check`, the error
messages don't clearly associate failures with specific flake outputs. In CI,
you see a wall of derivation paths with no indication of which flake attribute
they came from. DetSys added per-output reporting:

```
❌ checks.aarch64-darwin.badFeatures
  error: Cannot build '/nix/store/...drv'.
  Reason: required system or feature not available

❓ checks.aarch64-darwin.twoFakeHashes (cancelled)
```

Each output gets ❌ (failed), ❓ (cancelled), or implicit success.

## What Changes

- **nix build**: Failed outputs report which flake installable they came from.
- **nix flake check**: Summary at the end shows per-output status with the
  specific error for each failure.
- **Cancelled outputs**: Outputs not attempted due to `--keep-going` limits
  are shown as cancelled.

## Capabilities

### New Capabilities
- `per-output-reporting`: Build failures attributed to specific flake outputs

## Impact

- **Files**: `src/nix/build.cc`, `src/nix/flake.cc` (check subcommand),
  `src/libcmd/installable-flake.cc` (output tracking)
- **APIs**: Changed CLI output format
- **Dependencies**: None
- **Testing**: Functional tests with multiple failing outputs, verify summary format

## Merge Conflict Risk

Medium. Touches the build result aggregation in `nix build` and the check
loop in `nix flake check`. These are high-traffic areas but the change is
mostly additive (wrapping existing error output with output attribution).
