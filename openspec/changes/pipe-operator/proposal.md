# Pipe Operator |>

**Upstream:** Lix CL/1654, RFC 148
**Upstream version:** Lix 2.91 (Aug 2024)
**Tier:** 3

## Why

Nix expressions frequently nest function calls: `f (g (h x))`. The pipe
operator lets you write this as `x |> h |> g |> f`, which reads left-to-right
in data flow order. This is RFC 148, widely requested by the community.

Lix ships it behind `--extra-experimental-features pipe-operator`. We'd do
the same — experimental flag, no commitment to stabilization yet.

## What Changes

- **Parser**: New `|>` binary operator with appropriate precedence.
- **Evaluator**: `a |> f` desugars to `f a`.
- **Experimental feature flag**: `pipe-operator` — must be explicitly enabled.

## Capabilities

### New Capabilities
- `pipe-operator`: `x |> f` syntax (experimental)

## Impact

- **Files**: `src/libexpr/parser/parser.cc` (or `.y`), `src/libexpr/eval.cc`,
  `src/libexpr/nixexpr.hh` (AST node), experimental features list
- **APIs**: New syntax behind experimental flag
- **Dependencies**: None
- **Testing**: Parser tests, eval tests, ensure flag gating works

## Merge Conflict Risk

Medium. Parser changes are always touchy, but this is a well-defined addition
(one new binary operator) and Lix's implementation is clean. The 2.33.3 parser
may have diverged in structure from the Lix version.
