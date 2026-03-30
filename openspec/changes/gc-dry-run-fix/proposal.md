# Fix nix-collect-garbage --dry-run

**Upstream:** Lix CL/1566
**Upstream version:** Lix 2.91 (Aug 2024)
**Tier:** 2

## Why

`nix-collect-garbage --dry-run` currently produces no output at all. It exits
silently without even checking which paths would be deleted. Users expect it
to show what *would* be collected, like every other `--dry-run` flag in Unix
tools.

## What Changes

- **nix-collect-garbage**: `--dry-run` now enumerates dead paths and prints
  them with a summary of count and freed space, but does not delete anything.

## Capabilities

### Modified Capabilities
- `gc-dry-run`: Actually reports what would be deleted

## Impact

- **Files**: `src/nix/nix-collect-garbage/nix-collect-garbage.cc` or GC implementation
- **APIs**: CLI output change (was empty, now informative)
- **Dependencies**: None
- **Testing**: Functional test that `--dry-run` produces output and doesn't delete

## Merge Conflict Risk

Low. The fix is in the garbage collector's dry-run code path.
