# Merge builtins.wasm and builtins.wasi

**Upstream:** DeterminateSystems/nix-src PR #359, #370
**Upstream version:** Determinate Nix v3.16.1 (Feb 2026)
**Tier:** 2

## Why

We already have `builtins.wasm` from NixOS/nix#15380. DetSys extended this
with `builtins.wasi` (PR #359) and then merged both into a unified interface
(PR #370). WASI support lets Wasm modules access a virtual filesystem and
environment variables, making them useful for real plugins that need to read
inputs and produce outputs beyond pure computation.

## What Changes

- **builtins.wasm**: Extended to support WASI modules. The existing `builtins.wasm`
  function gains optional arguments for filesystem mappings and environment.
- **builtins.wasi**: Removed as a separate builtin; functionality merged into `builtins.wasm`.

## Capabilities

### Modified Capabilities
- `wasm-builtin`: Wasm evaluator supports WASI (filesystem, env) in addition to pure computation

## Impact

- **Files**: `src/libexpr/primops/wasm.cc`, possibly `packaging/wasmtime.nix`
- **APIs**: `builtins.wasm` gains new optional attrs; `builtins.wasi` does not exist as separate
- **Dependencies**: wasmtime (already a dependency for our wasm support)
- **Testing**: Extend existing wasm tests with WASI test cases

## Merge Conflict Risk

Medium. We already diverged from upstream's wasm implementation to port it to
2.33.3. DetSys's unified wasm/wasi is against their main branch. Will need
manual adaptation.
