# Integer Overflow Is an Error

**Upstream:** Lix CL/1594, CL/1595, CL/1597, CL/1609
**Upstream version:** Lix 2.91 (Aug 2024)
**Tier:** 1 — correctness fix

## Why

Integer overflow in the Nix language previously invoked C++ signed overflow,
which is undefined behaviour. In practice it wrapped around, but that's never
intentional in Nix expressions. Lix shipped this as an error in 2.91 and
nobody's nixpkgs code broke — the three months where it was accidentally
an error before anyone noticed confirms that real-world code doesn't rely
on overflow.

Additionally, `builtins.fromJSON` of values exceeding a signed 64-bit int
should error, and `nixConfig` in flakes should reject negative values for
settings that expect non-negative integers.

## What Changes

- **Evaluator**: Arithmetic operations (`+`, `-`, `*`) check for overflow and
  throw an evaluation error instead of wrapping.
- **builtins.fromJSON**: Rejects integers outside the int64 range.
- **nixConfig**: Rejects negative values for non-negative config settings.

## Capabilities

### Modified Capabilities
- `evaluator-safety`: Integer overflow is a hard error

## Impact

- **Files**: `src/libexpr/eval.cc` (arithmetic primops), `src/libexpr/primops.cc`
  (fromJSON), config parsing
- **APIs**: No new APIs; existing arithmetic now errors on overflow
- **Dependencies**: None
- **Testing**: Eval tests for overflow in each arithmetic op, fromJSON edge cases

## Merge Conflict Risk

Low. The arithmetic operations are small, well-isolated functions. The fromJSON
change is a bounds check on the JSON parser output.
