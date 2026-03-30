# accept-flake-config = false Auto-Rejects

**Upstream:** Lix CL/1541
**Upstream version:** Lix 2.91 (Aug 2024)
**Tier:** 1 — UX fix

## Why

Setting `accept-flake-config = false` should mean "never accept flake config
options." Instead, it still prompts the user for each option. The only way to
avoid prompts was to accept everything. Lix added a third value `ask` (the
default) to preserve the prompting behavior while letting `false` actually
reject.

Also adds `N` (capital) as a response to reject ALL remaining untrusted
settings at once, rather than having to reject each individually.

## What Changes

- **accept-flake-config**: `false` auto-rejects all flake config options.
  `ask` (new default) preserves the interactive prompt behavior.
- **Interactive prompt**: `N` rejects all remaining untrusted settings.

## Capabilities

### Modified Capabilities
- `flake-config`: `accept-flake-config = false` silently rejects

## Impact

- **Files**: `src/libmain/shared.cc` or wherever flake config trust is evaluated
- **APIs**: `accept-flake-config` gains a third value `ask`
- **Dependencies**: None
- **Testing**: Test that `false` skips prompts; test that `ask` prompts

## Merge Conflict Risk

Low. Small change to the config trust evaluation logic.
