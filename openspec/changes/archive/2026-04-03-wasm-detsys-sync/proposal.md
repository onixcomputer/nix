## Why

Our `builtins.wasm` implementation has diverged from DeterminateSystems/nix-src.
Several of their changes are improvements worth adopting: a crash fix for the
WASM instance cache, cleaner code structure, WAT support, and making
`realisePath` a public `EvalState` method so wasm.cc doesn't need a local copy.

Cherry-picking these selectively keeps us close to upstream while preserving
our additions (string context functions, ThrownError fix).

## What Changes

- **Public `realisePath`**: Move the static `realisePath` from primops.cc to a
  public `EvalState` method. Drop the local `wasmRealisePath` copy from wasm.cc.
- **`NixWasmInstancePre` refactor**: Replace the statement expression in the
  member initializer with a `compile()` method. Cleaner, and required for WAT
  support.
- **`LazyMakeRef` crash fix**: Fix null pointer crash when loading an invalid
  WASM file twice in `nix repl`. If `NixWasmInstancePre` constructor throws,
  the cache entry is left as nullptr; second access dereferences it.
- **WAT support**: Add `wat` attribute for inline WebAssembly Text format
  strings. `builtins.wasm { wat = "..."; function = "f"; } arg`.
- **Attr validation order**: Check for unknown attributes before extracting
  path/wat, so bad input fails faster.

## Capabilities

### New Capabilities
- `wasm-wat-input`: `builtins.wasm` accepts `wat` attribute with inline
  WebAssembly Text format source as an alternative to `path`.

### Modified Capabilities
- `wasm-realisePath`: `EvalState::realisePath` becomes a public method.
- `wasm-instance-cache`: Instance cache no longer crashes on repeated
  access after compilation failure.

## Impact

- **Files**: `src/libexpr/primops/wasm.cc`, `src/libexpr/include/nix/expr/eval.hh`,
  `src/libexpr/eval.cc` (or wherever realisePath lives), `src/libexpr/primops/primops.cc`
- **APIs**: `EvalState::realisePath` becomes public. `builtins.wasm` gains `wat`
  attribute.
- **Dependencies**: None new. `wat2wasm()` is already in wasmtime.
- **Testing**: Add WAT test to wasm.sh. Existing tests must keep passing.
  Test the crash fix (load bad wasm path twice).
