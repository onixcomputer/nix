# REPL Tab-Completion for :colon Commands

**Upstream:** Lix CL/1367
**Upstream version:** Lix 2.91 (Aug 2024)
**Tier:** 2 — UX improvement

## Why

`nix repl` supports `:b`, `:edit`, `:doc`, `:log`, and other colon commands,
but pressing TAB doesn't complete them. You have to remember the exact command
names. Tab-completing partial input like `:ed` → `:edit` is standard REPL
behavior.

## What Changes

- **REPL completion**: When the input line starts with `:`, complete against
  the known colon commands instead of Nix expressions.

## Capabilities

### Modified Capabilities
- `repl-completion`: Tab-completes `:colon` commands

## Impact

- **Files**: `src/libcmd/repl.cc` (completion handler)
- **APIs**: None
- **Dependencies**: None
- **Testing**: REPL interaction test verifying `:ed<TAB>` completes to `:edit`

## Merge Conflict Risk

Low. The completion handler is a self-contained function. Adding a command
prefix check before the expression completer.
