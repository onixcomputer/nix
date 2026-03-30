# Fix Flake restrict-eval Path Access

**Upstream:** Lix 2.92 (security fix)
**Upstream version:** Lix 2.92 (Jan 2025)
**Tier:** 2 — security fix

## Why

Flakes and `--restrict-eval` are supposed to prevent access to arbitrary paths.
A bug present since at least Nix 2.18 allowed flakes to read impure paths as
if `--impure` was passed:

```nix
{
  inputs = {};
  outputs = {...}: {
    lol = builtins.readFile "${/etc/passwd}";
  };
}
```

This evaluated successfully without `--impure`, reading `/etc/passwd`.

While this isn't a violation of Nix's documented security model (untrusted
code runs as the invoking user), it defeats the purpose of restrict-eval for
sandboxing evaluation.

## What Changes

- **Eval restriction**: Fix the path access check so that flake evaluation
  and `--restrict-eval` correctly block access to paths outside the allowed
  set.

## Capabilities

### Modified Capabilities
- `eval-restriction`: Flakes correctly restrict path access

## Impact

- **Files**: `src/libexpr/eval.cc` or `src/libexpr/paths.cc` (path access
  checking)
- **APIs**: None; previously-bypassed restriction now enforced
- **Dependencies**: None
- **Testing**: Test that a flake cannot read `/etc/passwd` without `--impure`

## Merge Conflict Risk

Low. The fix is a correction to an existing access check.
