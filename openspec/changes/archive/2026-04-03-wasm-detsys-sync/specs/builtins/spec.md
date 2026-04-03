# builtins.wasm DetSys Sync

## MODIFIED Requirements

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

## ADDED Requirements

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
