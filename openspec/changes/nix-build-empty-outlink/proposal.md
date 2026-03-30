# nix-build -o "" Behaves as --no-out-link

**Upstream:** Lix CL/2103
**Upstream version:** Lix 2.92 (Jan 2025)
**Tier:** 1 — bug fix

## Why

`nix-build --out-link ""` currently either errors with "current working directory
already exists" (for default output) or creates symlinks starting with `-`
(e.g. `-doc`) for other outputs — a footgun for terminal commands. `nix build`
already treats an empty out-link as no-out-link. `nix-build` should match.

## What Changes

- **nix-build**: Treat `--out-link ""` the same as `--no-out-link`.

## Capabilities

### Modified Capabilities
- `cli-compat`: `nix-build -o ""` consistent with `nix build` behavior

## Impact

- **Files**: `src/nix-build/nix-build.cc`
- **APIs**: None
- **Dependencies**: None
- **Testing**: Test `nix-build -o ""` doesn't create symlinks

## Merge Conflict Risk

Minimal. One conditional check.
