# Relative and Tilde Paths in Configuration

**Upstream:** Lix CL/1851, CL/1863, CL/1864
**Upstream version:** Lix 2.92 (Jan 2025)
**Tier:** 1 — config improvement

## Why

Settings like `repl-overlays` and `secret-key-files` require absolute paths,
which is awkward when your nix.conf lives in a dotfiles repo. With relative
path support, `repl-overlays = repl.nix` in `~/.config/nix/nix.conf` resolves
to `~/.config/nix/repl.nix`. Tilde paths like `~/dotfiles/repl.nix` also work.

`include` directives in config files can also use tilde paths.

Only user config files support these features. `$NIX_CONFIG` does not allow
relative paths (no meaningful base directory).

## What Changes

- **Config parser**: Resolve relative paths against the directory containing
  the config file. Expand `~/` to the user's home directory.
- **include directive**: Supports tilde paths.
- **Restriction**: Only user config files (not `$NIX_CONFIG`) can use relative
  or tilde paths.

## Capabilities

### Modified Capabilities
- `config-parsing`: Relative and tilde path resolution in user config files

## Impact

- **Files**: `src/libutil/config-global.cc` or `src/libutil/configuration.cc`,
  config file parsing
- **APIs**: No new settings; existing path-valued settings gain relative resolution
- **Dependencies**: None
- **Testing**: Test relative and tilde paths in nix.conf for repl-overlays,
  secret-key-files, include

## Merge Conflict Risk

Low. Config parsing is stable. The change adds a resolution step before
existing path validation.
