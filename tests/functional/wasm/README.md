# Wasm Test Fixtures

Pre-built `.wasm` modules for testing `builtins.wasm`.

## Modules

| File | Mode | Target | Description |
|------|------|--------|-------------|
| `pure_double.wasm` | non-WASI | `wasm32-unknown-unknown` | Doubles an integer. Exports `nix_wasm_init_v1` and `double`. |
| `wasi_double.wasm` | WASI | `wasm32-wasip1` | Doubles an integer via `_start` + `return_to_nix`. |
| `wasi_hello.wasm` | WASI | `wasm32-wasip1` | Writes to stdout/stderr, returns "done" string. |
| `wasi_no_return.wasm` | WASI | `wasm32-wasip1` | Exits without calling `return_to_nix` (error path test). |

## Rebuilding

From this directory:

```sh
cd fixtures
nix-build build.nix
cp result/*.wasm ../
```

Requires fenix (fetched automatically by `build.nix`) for the `wasm32-wasip1` target stdlib.

## Source

Rust source is in `fixtures/`. Each module uses inline FFI against the Nix
wasm host interface (no external crate dependencies).
