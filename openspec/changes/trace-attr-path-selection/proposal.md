# Trace Which Part of foo.bar.baz Errors

**Upstream:** Lix CL/1505, CL/1506
**Upstream version:** Lix 2.91 (Aug 2024)
**Tier:** 2 — error UX

## Why

When `linux_4_9.meta.description` fails, Nix points to the whole expression
but doesn't say which segment (`linux_4_9`, `meta`, or `description`) caused
the failure. You have to manually bisect the path. Lix now adds a trace frame:

```
… while evaluating 'pkgs.linuxKernel.kernels.linux_4_9' to select 'meta' on it
```

This pinpoints that `linux_4_9` evaluated to something that threw, and that
`meta` was the next selection that was never reached.

## What Changes

- **Evaluator**: Attribute path selection (`ExprSelect`) adds a trace frame
  identifying which attribute in the chain is being forced and which subsequent
  attribute was being selected.
- **Error positions**: The error position now points to the specific attribute
  in the chain that failed, not the beginning of the whole expression.

## Capabilities

### Modified Capabilities
- `error-traces`: Attr path selection traces identify the failing segment

## Impact

- **Files**: `src/libexpr/eval.cc` (ExprSelect evaluation), error position
  computation
- **APIs**: None
- **Dependencies**: None
- **Testing**: Eval-fail tests for multi-segment attr paths with errors at
  different positions; check trace output format

## Merge Conflict Risk

Medium. Touches core evaluation of `ExprSelect`, which is a hot path. The
change adds trace instrumentation, not logic changes.
