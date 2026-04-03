# builtins.wasm Specification

## Requirements

### Requirement: WASM errors are catchable

When a WASM plugin function panics, traps, or returns a guest-originated
error (e.g. Nickel BlameError, FieldMissing, InfiniteRecursion), the
`builtins.wasm` builtin MUST throw a catchable `EvalError`, not an
uncatchable abort.

`builtins.tryEval` MUST be able to intercept WASM plugin errors the same
way it intercepts native Nix evaluation errors (e.g. `throw`, missing
attribute, type mismatch).

#### Scenario: tryEval catches WASM BlameError

- GIVEN a Nickel plugin that applies a Port contract
- WHEN `builtins.wasm` is called with `port = "not-a-number"`
- THEN `builtins.tryEval` returns `{ success = false; }`

#### Scenario: tryEval catches WASM FieldMissing

- GIVEN a Nickel expression that accesses `record.missing_field`
- WHEN `builtins.wasm` evaluates it and Nickel raises FieldMissing
- THEN `builtins.tryEval` returns `{ success = false; }`

#### Scenario: WASM error message preserved

- GIVEN a WASM plugin that panics with a descriptive message
- WHEN the error is caught by `try` in C++ or displayed to the user
- THEN the panic message, WASM backtrace, and guest error details
  SHOULD be preserved in the Nix error message

### Requirement: Successful WASM calls unchanged

Normal WASM plugin invocations that return values without error MUST
continue to work identically. This change MUST NOT affect the happy path.

#### Scenario: Normal evaluation unaffected

- GIVEN a Nickel plugin that evaluates `{ x = 1 + 1 }`
- WHEN `builtins.wasm` is called
- THEN the result is `{ x = 2; }` with no behavior change

### Requirement: Public EvalState::realisePath

`EvalState` MUST expose a public `realisePath` method that coerces a
value to a path, realises any string context, and optionally resolves
symlinks. The static free function in primops.cc MUST be removed and
all callers MUST use the method.

#### Scenario: wasm.cc uses EvalState::realisePath

- GIVEN wasm.cc needs to resolve a path value
- WHEN it calls `state.realisePath(pos, value)`
- THEN the path is coerced, context realised, and symlinks resolved
- AND no local `wasmRealisePath` function exists in wasm.cc

### Requirement: NixWasmInstancePre uses compile() method

`NixWasmInstancePre` MUST use a `compile(std::span<uint8_t>)` method
instead of a statement expression in the member initializer list.
The struct MUST store a `std::string name` instead of `SourcePath`.

### Requirement: Instance cache handles compilation failure

The WASM instance cache MUST NOT store null entries. If
`NixWasmInstancePre` construction throws, the cache entry MUST NOT
exist. Repeated access to a path that failed compilation MUST retry
compilation, not dereference a null pointer.

#### Scenario: Bad WASM file loaded twice in nix repl

- GIVEN an invalid WASM file path
- WHEN `builtins.wasm` is called with it twice
- THEN both calls throw an error describing the compilation failure
- AND nix does not crash

### Requirement: WAT attribute support

`builtins.wasm` MUST accept a `wat` attribute containing a WebAssembly
Text format string. `wat` and `path` MUST be mutually exclusive.
Exactly one MUST be provided.

#### Scenario: Inline WAT evaluation

- GIVEN a WAT string defining a function `fib`
- WHEN `builtins.wasm { wat = watString; function = "fib"; } 10`
- THEN the function is compiled from WAT, executed, and returns the
  correct result

#### Scenario: wat and path mutual exclusivity

- GIVEN both `wat` and `path` attributes
- WHEN `builtins.wasm` is called
- THEN an error is thrown stating they are mutually exclusive

#### Scenario: Neither wat nor path

- GIVEN neither `wat` nor `path` attribute
- WHEN `builtins.wasm` is called
- THEN an error is thrown stating one is required

### Requirement: Attr validation before path extraction

`builtins.wasm` MUST check for unknown attributes before extracting
or evaluating the `path`/`wat` attribute. This ensures bad input is
rejected before any WASM compilation or path coercion occurs.
