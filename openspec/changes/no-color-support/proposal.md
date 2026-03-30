# NO_COLOR / CLICOLOR_FORCE Support

**Upstream:** Lix CL/1699, CL/1702
**Upstream version:** Lix 2.91 (Aug 2024)
**Tier:** 1 — standards compliance

## Why

Nix only partially respects `NO_COLOR`. Commands like `nix search` and
`nix flake metadata` still emit ANSI escapes even when `NO_COLOR` is set.
The `CLICOLOR_FORCE`/`FORCE_COLOR` convention for forcing color in pipes
is not supported at all.

Following the rules from https://bixense.com/clicolors/:
1. `NO_COLOR` or `NOCOLOR` set → disable color
2. `CLICOLOR_FORCE` or `FORCE_COLOR` set → enable color
3. Output is a tty and `TERM != "dumb"` → enable color
4. Otherwise → disable color

## What Changes

- **Color detection**: Centralize color decision using the precedence above.
  Apply to all output paths including `nix search`, `nix flake metadata`,
  and structured log formatters.

## Capabilities

### Modified Capabilities
- `color-output`: Follows NO_COLOR/CLICOLOR_FORCE standards

## Impact

- **Files**: `src/libutil/terminal.cc` or `src/libutil/logging.cc` (color detection),
  plus any commands that do their own ANSI emission
- **APIs**: None
- **Dependencies**: None
- **Testing**: Test `NO_COLOR=1 nix search` produces no ANSI escapes

## Merge Conflict Risk

Low. The color detection logic is centralized; the fix is making all callers
use it.
