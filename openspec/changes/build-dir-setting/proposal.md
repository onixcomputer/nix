# build-dir Setting

**Upstream:** Lix CL/1514 (based on NixOS/nix #10303, #10312, #10883)
**Upstream version:** Lix 2.91 (Aug 2024)
**Tier:** 1 — config option

## Why

Build sandboxes use the system temporary directory by default. On machines
where `/tmp` is a small tmpfs, large builds fail with ENOSPC. Users currently
work around this by setting `TMPDIR`, but that also affects the environment
inherited by `nix-shell`, `nix shell`, and `nix run` — which is rarely desired.

`build-dir` lets you relocate build sandbox backing directories to a different
filesystem without side effects on interactive shells.

Additionally, `XDG_RUNTIME_DIR` is no longer considered when selecting the
default temp directory, since it's typically a small per-session tmpfs not
intended for large build artifacts.

## What Changes

- **New config setting**: `build-dir` — path to use as the backing directory
  for build sandboxes.
- **Default temp selection**: `XDG_RUNTIME_DIR` is no longer used as a
  candidate for the default temporary directory.

## Capabilities

### New Capabilities
- `build-dir`: Configurable build sandbox backing directory

## Impact

- **Files**: `src/libstore/build/local-derivation-goal.cc`, `src/libutil/util.cc`,
  `src/libstore/globals.hh` (setting declaration)
- **APIs**: New `build-dir` config setting
- **Dependencies**: None
- **Testing**: Functional test building a derivation with `build-dir` set to a
  custom path

## Merge Conflict Risk

Low. The setting declaration is additive, and the sandbox directory selection
is a small change in derivation goal setup.
