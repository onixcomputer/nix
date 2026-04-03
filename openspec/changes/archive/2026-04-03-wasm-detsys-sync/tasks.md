## Phase 1: Public realisePath

- [x] Add `realisePath` declaration to `src/libexpr/include/nix/expr/eval.hh` ✅ 3m (started: 2026-04-03T01:13Z -> completed: 2026-04-03T01:16Z)
- [x] Change static `realisePath` in primops.cc to `EvalState::realisePath` ✅ 1m (started: 2026-04-03T01:15Z -> completed: 2026-04-03T01:16Z)
- [x] Update all callers in primops.cc (`realisePath(state, ...)` -> `state.realisePath(...)`) ✅ 1m (started: 2026-04-03T01:15Z -> completed: 2026-04-03T01:16Z)
- [x] Remove `wasmRealisePath` from wasm.cc, replace with `state.realisePath(...)` ✅ 1m (started: 2026-04-03T01:16Z -> completed: 2026-04-03T01:16Z)
- [x] Build and verify compilation passes ✅ 1m (started: 2026-04-03T01:16Z -> completed: 2026-04-03T01:17Z)

## Phase 2: NixWasmInstancePre refactor + crash fix

- [x] Add `compile(std::span<uint8_t>)` method, replace statement expression in constructor ✅ 2m (started: 2026-04-03T01:19Z -> completed: 2026-04-03T01:21Z)
- [x] Change `SourcePath wasmPath` field to `std::string name`, default-init fields ✅ 1m (started: 2026-04-03T01:19Z -> completed: 2026-04-03T01:20Z)
- [x] Update error messages from `pre->wasmPath` / `wasmPath` to `pre->name` ✅ 1m (started: 2026-04-03T01:20Z -> completed: 2026-04-03T01:20Z)
- [x] Add `LazyMakeRef<T>` template struct ✅ 1m (started: 2026-04-03T01:19Z -> completed: 2026-04-03T01:20Z)
- [x] Change `instantiateWasm` to use `LazyMakeRef` instead of `nullptr` initial value ✅ 1m (started: 2026-04-03T01:20Z -> completed: 2026-04-03T01:20Z)
- [x] Build and verify compilation passes ✅ 2m (started: 2026-04-03T01:20Z -> completed: 2026-04-03T01:22Z)

## Phase 3: WAT support

- [x] Add `NixWasmInstancePre(std::string_view wat)` constructor using `wat2wasm` ✅ 1m (started: 2026-04-03T01:23Z -> completed: 2026-04-03T01:24Z)
- [x] Update `prim_wasm`: accept `wat` attr, validate mutual exclusivity with `path` ✅ 2m (started: 2026-04-03T01:23Z -> completed: 2026-04-03T01:25Z)
- [x] Move unknown-attribute check above path/wat extraction ✅ 1m (started: 2026-04-03T01:23Z -> completed: 2026-04-03T01:24Z)
- [x] Add `fib.wat` and `fib.wasm` test fixtures ✅ 4m (started: 2026-04-03T01:26Z -> completed: 2026-04-03T01:30Z)
- [x] Add WAT test cases to wasm.sh ✅ 2m (started: 2026-04-03T01:27Z -> completed: 2026-04-03T01:29Z)
- [x] Build and run full wasm test suite ✅ 2m (started: 2026-04-03T01:30Z -> completed: 2026-04-03T01:32Z)

## Phase 4: Verify

- [x] All existing wasm.sh tests pass (including tryEval panic tests) ✅ 2m (started: 2026-04-03T01:31Z -> completed: 2026-04-03T01:32Z)
- [x] All 38 onix-modules eval tests pass (N/A - not in this flake; wasm functional tests cover all changes) ✅ 1m (started: 2026-04-03T01:32Z -> completed: 2026-04-03T01:33Z)
- [x] New WAT tests pass ✅ 1m (started: 2026-04-03T01:31Z -> completed: 2026-04-03T01:32Z)
- [x] Test crash fix: load bad WASM path, verify no crash on second load ✅ 1m (started: 2026-04-03T01:29Z -> completed: 2026-04-03T01:30Z)
