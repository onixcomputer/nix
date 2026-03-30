# temp-dir Setting

**Upstream:** Lix CL/2103 (based on NixOS/nix #7731, #8995)
**Upstream version:** Lix 2.92 (Jan 2025)
**Tier:** 1 — config option

## Why

Nix uses `TMPDIR` for various temporary files (download staging, evaluation
scratch, etc). Users who want to relocate these currently set `TMPDIR` globally,
but that leaks into `nix-shell`/`nix shell`/`nix run` environments.

A dedicated `temp-dir` config setting lets Nix use a custom temp location
without affecting the environment inherited by interactive commands.

On macOS, `TMPDIR` points to a per-session `/var/folders/` directory. Nix
previously unset `TMPDIR` for interactive shells when it pointed there, which
was confusing. This change also stops doing that.

## What Changes

- **New config setting**: `temp-dir` — directory for Nix's own temporary files.
- **macOS**: Stop unsetting `TMPDIR` in interactive shells when it points to
  per-session `/var/folders/`.

## Capabilities

### New Capabilities
- `temp-dir`: Configurable temporary directory for Nix internals

## Impact

- **Files**: `src/libutil/util.cc` (temp dir selection), `src/libstore/globals.hh`,
  possibly `src/nix/develop.cc` (shell environment)
- **APIs**: New `temp-dir` config setting
- **Dependencies**: None
- **Testing**: Test that temp files land in the configured directory

## Merge Conflict Risk

Low. Pairs naturally with `build-dir` but doesn't depend on it.
