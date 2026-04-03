## Context

Our `builtins.wasm` was cherry-picked from DetSys and adapted for 2.33.3.
Since then DetSys made several improvements. We want to adopt six of them
while keeping our additions (string context functions, ThrownError fix).

## Goals / Non-Goals

**Goals:** Adopt crash fix, cleaner code structure, WAT support, public
realisePath, better attr validation order.

**Non-Goals:** Changing our string context API. Changing the ThrownError
error propagation fix.

## Decisions

### 1. Make `realisePath` a public `EvalState` method

**Choice:** Move the static `realisePath` from primops.cc to a public
method `EvalState::realisePath`. Remove the local `wasmRealisePath` from
wasm.cc.

**Rationale:** DetSys already did this. Eliminates code duplication
between primops.cc and wasm.cc. The function is useful for any primop
that needs to coerce a value to a realized path.

**Implementation:**
- Add declaration to `src/libexpr/include/nix/expr/eval.hh`
- Change `static SourcePath realisePath(EvalState & state, ...)` in
  primops.cc to `SourcePath EvalState::realisePath(...)`, dropping the
  `state` param (use `this`)
- Update all callers in primops.cc: `realisePath(state, ...)` ->
  `state.realisePath(...)`
- Remove `wasmRealisePath` from wasm.cc, replace calls with
  `state.realisePath(...)`

### 2. Refactor `NixWasmInstancePre` with `compile()` method

**Choice:** Extract the compilation logic from the member initializer's
statement expression into a `compile(std::span<uint8_t>)` method. Store
`std::string name` instead of `SourcePath wasmPath`.

**Rationale:** Statement expressions in initializer lists are a GCC
extension and hard to read. The `compile()` method is reusable for WAT
input.

**Implementation:** Match DetSys structure: default-initialized fields,
`compile()` method, two constructors (SourcePath, string_view WAT).
Update error messages to use `pre->name` instead of `wasmPath`.

### 3. `LazyMakeRef` crash fix

**Choice:** Replace `nullptr` initial value in `try_emplace_and_cvisit`
with `LazyMakeRef` that constructs the instance during emplacement.

**Rationale:** If `NixWasmInstancePre` constructor throws (bad WASM
file), the `nullptr` entry persists in the map. Second access
dereferences null -> crash. `LazyMakeRef` makes emplacement atomic:
if the constructor throws, no entry is created.

**Implementation:** Add `LazyMakeRef<T>` template struct. Change
`concurrent_flat_map<..., shared_ptr<...>>` to
`concurrent_flat_map<..., LazyMakeRef<...>>`. Simplify the lambda.

### 4. WAT support

**Choice:** Add `wat` attribute to `builtins.wasm`. Accepts inline
WebAssembly Text format string. Mutually exclusive with `path`.

**Rationale:** Useful for testing and small inline WASM functions
without needing pre-compiled .wasm files. DetSys already has this.
`wasmtime::wat2wasm` is available in our wasmtime v40.0.2
(`WASMTIME_FEATURE_WAT` is defined in conf.h).

**Implementation:**
- Add `NixWasmInstancePre(std::string_view wat)` constructor
- In `prim_wasm`: accept `wat` attr, validate mutual exclusivity
  with `path`, instantiate via string constructor
- WAT modules are not cached (no stable `SourcePath` key)
- Add test with `fib.wat` fixture

### 5. Attr validation order

**Choice:** Check for unknown attributes before extracting `path`/`wat`.

**Rationale:** Fail fast on bad input. User sees "unknown attribute
'bogus'" before any path coercion or WASM compilation happens.

**Implementation:** Move the unknown-attribute loop above path/wat
extraction in `prim_wasm`.

## Risks / Trade-offs

**[Error messages change slightly]** — Messages switch from
`wasmPath` (full SourcePath) to `pre->name` (baseName). Less detail
in error output. Accept this for consistency with DetSys.

**[WAT not cached]** — Inline WAT modules bypass the `instancesPre`
cache. Each call recompiles. Fine for testing; not for hot loops.
