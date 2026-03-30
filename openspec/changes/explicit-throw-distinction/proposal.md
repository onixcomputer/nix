# Distinguish Explicit Throws from Errors During Throw

**Upstream:** Lix CL/1511
**Upstream version:** Lix 2.91 (Aug 2024)
**Tier:** 2 — error UX

## Why

`throw "message"` shows up as "while calling the 'throw' builtin" — identical
to what you'd see if an error occurred while evaluating throw's arguments
(e.g., a type error in string interpolation). This makes it hard to tell
whether a throw was intentional or something went wrong evaluating the
throw expression itself.

Lix now distinguishes the two cases:

- Intentional: `… caused by explicit throw`
- Error during evaluation: `… while calling the 'throw' builtin` (followed by
  the actual error deeper in the trace)

## What Changes

- **Evaluator**: The `throw` builtin sets a flag or uses a distinct exception
  type to indicate an explicit throw vs an error during throw argument
  evaluation.
- **Error formatting**: Explicit throws display "caused by explicit throw"
  instead of "while calling the 'throw' builtin".

## Capabilities

### Modified Capabilities
- `error-messages`: Explicit throws have distinct trace wording

## Impact

- **Files**: `src/libexpr/primops.cc` (throw builtin), `src/libexpr/eval.cc`
  (error formatting / trace display)
- **APIs**: None
- **Dependencies**: None
- **Testing**: Eval-fail tests for explicit throw and for error-during-throw

## Merge Conflict Risk

Low. The throw builtin is a small function. The trace formatting change is
additive.
