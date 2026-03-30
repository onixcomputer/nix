# Parallel Evaluation

**Upstream:** DeterminateSystems/nix-src PRs #119–#206 (many)
**Upstream version:** Determinate Nix v3.6.0+ (ongoing since mid-2025)
**Tier:** 3 — largest feature, highest impact

## Why

Nix evaluation is single-threaded. On a large nixpkgs eval, one core pegs at
100% while the rest sit idle. DetSys's parallel eval lets the evaluator use
multiple threads for thunk forcing, `builtins.parallel` for explicit
parallelism, and async path operations. The result is 2–8x eval speedup on
multi-core machines for real-world expressions.

This is DetSys's most significant technical contribution and the hardest to
port. It touches nearly every part of the evaluator.

## What Changes

- **Thread-safe evaluator**: Symbol table, position table, value allocation,
  and environment access are all thread-safe.
- **builtins.parallel**: New builtin that evaluates a list of thunks in
  parallel across worker threads.
- **Async path writing**: Store path registration happens asynchronously.
- **Concurrent data structures**: `boost::concurrent_flat_map` replaces
  `std::map` in hot paths.
- **Interruptible thunk waits**: Threads waiting on thunks being forced by
  another thread can be interrupted by Ctrl-C.
- **Parallel GC marking**: (Covered separately but part of the perf story.)

## Capabilities

### New Capabilities
- `parallel-eval`: Multi-threaded Nix evaluation
- `builtins-parallel`: Explicit parallel thunk evaluation
- `async-path-write`: Asynchronous store path registration

### Modified Capabilities
- `evaluator-safety`: All evaluator data structures are thread-safe
- `interrupt-handling`: Ctrl-C interrupts threads waiting on thunks

## Impact

- **Files**: Nearly everything in `src/libexpr/`, `src/libstore/build/`,
  `src/libutil/` (thread-safe containers)
- **APIs**: New `builtins.parallel`; existing APIs unchanged
- **Dependencies**: Boost concurrent containers
- **Testing**: Need thread-safety tests, parallel eval correctness tests,
  and benchmarks

## Merge Conflict Risk

Very high. This is 50+ PRs of incremental work touching core evaluator
internals. Cannot be cherry-picked as individual commits — needs to be
understood as a coherent body of work and adapted to our 2.33.3 base.
Recommend treating this as a long-running branch with periodic rebases.

## Approach

1. Start with thread-safety prerequisites (symbol table, pos table, allocator)
2. Add `boost::concurrent_flat_map` infrastructure
3. Port async path writing
4. Port `builtins.parallel`
5. Enable multi-threaded thunk forcing
6. Performance tuning and testing
