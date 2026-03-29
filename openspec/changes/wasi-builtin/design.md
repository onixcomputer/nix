# Design: Merge builtins.wasm and builtins.wasi

## Context

DetSys PRs #359 and #370 were merged upstream on Feb 20 and Feb 27, 2026.
Our cherry-pick of NixOS/nix#15380 (commit `23f3ccee2`) already included the
merged result — a unified `builtins.wasm` with WASI auto-detection. Commit
`d444c28c4` adapted the code to compile against the 2.33.3 API.

The implementation is complete. No code changes are needed. This design
documents the existing architecture and the remaining work: tests.

## Goals / Non-Goals

**Goals:**
- Verify the existing implementation matches upstream behavior
- Add functional tests for both non-WASI and WASI modes
- Document the 2.33.3 API adaptations

**Non-Goals:**
- Filesystem preopens (`preopenDir`) — not in upstream PRs, future work
- Environment variable passing — not in upstream PRs, future work
- Changes to the host function interface

## Decisions

### 1. Already Ported

**Choice:** No code changes needed. The cherry-pick already contains the
merged wasm/wasi.

**Rationale:** Comparing our `wasm.cc` line-by-line against the upstream
merge commit (`3306013`) confirms functional equivalence. Our adaptations
for 2.33.3 are:
- `wasmRealisePath()` — local helper because `realisePath` is a static
  function in `primops.cc` in 2.33.3, not a method on `EvalState`
- `.fun` field on `RegisterPrimOp` — 2.33.3 naming (later renamed to `.impl`)
- Explicit `std::vector<std::string>` for `wasiConfig.argv()` — brace-init
  ambiguity fix
- Split `getExport<Func>().call()` — template parse issue workaround

**Implementation:** N/A — already done.

### 2. WASI Detection Strategy

**Choice:** Auto-detect WASI by scanning module imports for
`wasi_snapshot_preview1`.

**Rationale:** This matches the upstream final decision (commit `9a660b9`).
Earlier iterations checked for a `_start` export or used an explicit `wasi`
boolean attribute, but import scanning is more reliable — a module could
export `_start` without being a WASI module, and requiring users to specify
`wasi = true` is redundant when the module's imports make it unambiguous.

### 3. Test Strategy

**Choice:** Add functional tests using pre-compiled `.wasm` fixtures.

**Rationale:** Building Wasm modules from Rust source requires a
`wasm32-wasi` toolchain, which is heavy for the test suite. Pre-compiled
fixtures (committed as `.wasm` files) keep tests fast and self-contained.
The upstream `nix-wasm-rust` repo has example modules we can use as reference.

Test coverage needed:
- Non-WASI: pure function call (e.g. fibonacci), type round-trips
- WASI: module with `_start` + `return_to_nix`, stdout/stderr capture
- Error cases: missing `path`, unknown attr, `function` on WASI module,
  missing `function` on non-WASI module, WASI module without `return_to_nix`
- Feature gate: error without `--extra-experimental-features wasm-builtin`

## Risks / Trade-offs

**[Fixture staleness]** Pre-compiled `.wasm` fixtures could become stale if
the host interface changes. Mitigated by keeping fixtures minimal and
documenting how they were built.

**[No upstream tests to port]** The DetSys PRs don't include functional tests
in the nix repo — their testing is in the separate `nix-wasm-rust` repo. We'll
need to write tests from scratch.
