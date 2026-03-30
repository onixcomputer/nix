# Enable Parallel Marking in Boehm GC

**Upstream:** DeterminateSystems/nix-src PR #168
**Upstream version:** Determinate Nix v3.8.5 (Aug 2025)
**Tier:** 1 — small change, free eval performance

## Why

The Nix evaluator uses Boehm GC for memory management. Boehm supports parallel
mark phases (`GC_MARKERS`) but Nix doesn't enable it. On multi-core machines
the GC pause time is a meaningful fraction of eval time. Enabling parallel
marking is a configuration flag — no code changes to the evaluator needed.

## What Changes

- **GC init**: Set `GC_MARKERS` environment variable or call
  `GC_set_markers_count()` during initialization to use multiple threads for
  the mark phase.

## Capabilities

### Modified Capabilities
- `gc-performance`: GC mark phase uses multiple cores

## Impact

- **Files**: `src/libexpr/eval.cc` or GC initialization code
- **APIs**: None
- **Dependencies**: Requires Boehm GC built with `--enable-parallel-mark` (already the case in nixpkgs)
- **Testing**: Run eval benchmarks before/after; no functional test changes needed

## Merge Conflict Risk

Minimal. A few lines in GC setup code.
