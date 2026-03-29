# Tasks: Merge builtins.wasm and builtins.wasi

## Phase 1: Verify Existing Implementation

- [x] Diff our `wasm.cc` against upstream merge commit `3306013` — 4 localized 2.33.3 adaptations, functionally equivalent ✅ 13m (started: 2026-03-29T19:04Z -> completed: 2026-03-29T19:17Z)
- [x] Diff our `doc/manual/source/protocols/wasm.md` against upstream — identical, zero diff ✅ 1m (started: 2026-03-29T19:09Z -> completed: 2026-03-29T19:09Z)
- [x] Fix build: bump nixpkgs input from nixos-25.05 (rustc 1.86) to nixos-unstable (rustc 1.94) so wasmtime 40.0.2 compiles ✅ 7m (started: 2026-03-29T19:28Z -> completed: 2026-03-29T19:35Z)
- [x] Build nix-expr with wasm support enabled — compiles clean ✅ 1m (started: 2026-03-29T19:33Z -> completed: 2026-03-29T19:34Z)

## Phase 2: Test Fixtures

- [x] Build minimal non-WASI `.wasm` module (`pure_double.wasm`, 247 B) — inline FFI, `wasm32-unknown-unknown` ✅ 3m (started: 2026-03-29T19:36Z -> completed: 2026-03-29T19:42Z)
- [x] Build minimal WASI `.wasm` module (`wasi_double.wasm`, 43 KB) — `wasm32-wasip1`, uses `return_to_nix` ✅ 1m (started: 2026-03-29T19:39Z -> completed: 2026-03-29T19:42Z)
- [x] Build WASI module with stdout/stderr (`wasi_hello.wasm`, 53 KB) — prints then returns string ✅ 1m (started: 2026-03-29T19:39Z -> completed: 2026-03-29T19:42Z)
- [x] Commit fixtures to `tests/functional/wasm/` with README and `fixtures/build.nix` for reproducible rebuilds ✅ 2m (started: 2026-03-29T19:42Z -> completed: 2026-03-29T19:43Z)

## Phase 3: Functional Tests

- [x] Add `tests/functional/wasm.sh` test script with auto-skip if wasm-builtin unavailable ✅ 2m (started: 2026-03-29T19:42Z -> completed: 2026-03-29T19:44Z)
- [x] Test: non-WASI pure function call succeeds and returns correct result ✅
- [x] Test: WASI module call succeeds and returns correct result via `return_to_nix` ✅
- [x] Test: WASI stdout/stderr captured as warnings ✅
- [x] Test: error on missing `path` attribute ✅
- [x] Test: error on unknown attribute in config ✅
- [x] Test: error on `function` attribute with WASI module ✅
- [x] Test: error on missing `function` attribute with non-WASI module ✅
- [x] Test: error when WASI module finishes without calling `return_to_nix` ✅ 2m (started: 2026-03-29T19:45Z -> completed: 2026-03-29T19:46Z)
- [x] Test: error without `--extra-experimental-features wasm-builtin` ✅
- [x] Wire `wasm.sh` into `tests/functional/meson.build` ✅
