# Flake Schemas

**Upstream:** DeterminateSystems/nix-src PR #217
**Upstream version:** Determinate Nix v3.11.0+ (ongoing, 64 comments)
**Tier:** 3

## Why

Flake outputs are untyped attribute sets. `nix flake check` has hardcoded
knowledge of which output attributes are valid and how to check them. This
makes it impossible for third-party tools to define new output types or for
`nix flake show` to understand custom outputs.

Flake schemas make the output structure machine-readable. A flake can declare
a schema that describes its output types, enabling:
- Better `nix flake check` with per-output success/failure reporting (#285)
- `isLegacy` attribute to distinguish compat outputs (#381)
- Third-party output types without upstream changes

## What Changes

- **Flake schema system**: Flakes can reference a schema that describes their
  output structure.
- **nix flake check**: Reports which specific outputs failed or succeeded
  instead of stopping at the first failure.
- **isLegacy**: Schema attribute to mark legacy/compat outputs.

## Capabilities

### New Capabilities
- `flake-schemas`: Machine-readable flake output type descriptions
- `check-reporting`: Per-output pass/fail reporting in `nix flake check`

## Impact

- **Files**: `src/libflake/flake/`, `src/nix/flake.cc`, new schema resolution code
- **APIs**: New flake.nix `schema` attribute, changed `nix flake check` output
- **Dependencies**: None
- **Testing**: Functional tests for schema resolution and check reporting

## Merge Conflict Risk

High. This is a large feature (64 comments on the PR) that adds a new
concept to the flake system. The schema resolution and check reporting touch
core flake evaluation paths.
