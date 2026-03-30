# build-dir Setting

**Upstream:** Lix CL/1514, upstream gh#10303
**Upstream version:** Lix 2.91 (Aug 2024)
**Tier:** 1 — solves real ops pain

## Why

Many systems mount `/tmp` as tmpfs (RAM-backed). Large builds (e.g. Linux
kernel, Chromium) fill tmpfs and fail. Users must manually set `TMPDIR` which
also affects `nix-shell` and `nix run` environments in confusing ways.

The `build-dir` setting lets you point sandbox build directories at a different
filesystem without any side effects on interactive shells.

As a bonus, `XDG_RUNTIME_DIR` is no longer considered when selecting the default
temp directory, since it's not meant for large data.

## What Changes

- **New setting**: `build-dir` — path to use as the backing directory for build
  sandboxes. Defaults to system temp.
- **Default temp**: Stop considering `XDG_RUNTIME_DIR` for build temp selection.

## Capabilities

### New Capabilities
- `build-dir`: Configurable build sandbox backing directory

## Impact

- **Files**: `src/libstore/globals.hh` (setting), `src/libstore/build/local-derivation-goal.cc` (usage)
- **APIs**: New `nix.conf` setting `build-dir`
- **Dependencies**: None
- **Testing**: Functional test that builds with `build-dir` set to a custom path

## Merge Conflict Risk

Low. Lix implemented this against CppNix 2.18-era code but the build directory
logic hasn't changed much. The setting registration and one use site in the
sandbox setup code.
