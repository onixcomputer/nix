# Language Feature Deprecation Framework

**Upstream:** Lix CL/1785, CL/1736, CL/1735, CL/1744, CL/2206
**Upstream version:** Lix 2.92 (Jan 2025)
**Tier:** 3

## Why

Nix has accumulated language features that are obsolete, confusing, or actively
harmful but can't be removed without a deprecation path. Lix introduced a
deprecation system mirroring the experimental features system:

- Deprecations are enabled by default (features are deprecated out of the box)
- Users opt out with `--extra-deprecated-features` for backwards compatibility
- Soft deprecations warn; hard deprecations error

Initial deprecations:
- `url-literals`: Unquoted URLs like `https://example.com` (use strings instead)
- `rec-set-overrides`: `__overrides` in rec sets (unused for a decade)
- `ancient-let`: `let { body = ...; }` syntax (use `let ... in` instead)
- `shadow-internal-symbols`: Shadowing `__sub`, `__mul`, `__div`, `__lessThan`

## What Changes

- **Deprecation system**: New `--extra-deprecated-features` flag and config option
  parallel to `--extra-experimental-features`.
- **Parser/evaluator**: Deprecated features emit warnings or errors depending on
  deprecation severity.
- **Initial deprecations**: url-literals, rec-set-overrides, ancient-let,
  shadow-internal-symbols.

## Capabilities

### New Capabilities
- `deprecation-framework`: Structured deprecation of language features with opt-out
- `deprecated-url-literals`: URL literals deprecated
- `deprecated-overrides`: `__overrides` deprecated
- `deprecated-ancient-let`: Old `let` syntax deprecated
- `deprecated-shadow-internals`: Shadowing arithmetic builtins deprecated

## Impact

- **Files**: `src/libexpr/parser/`, `src/libexpr/eval.cc`, `src/libutil/experimental-features.cc`
  (or new parallel file), config system
- **APIs**: New CLI flag, new config option
- **Dependencies**: None
- **Testing**: Tests for each deprecated feature with and without opt-out

## Merge Conflict Risk

Medium-high. The framework itself is self-contained but touches the parser
and evaluator at multiple points. Each individual deprecation is small but
the framework plumbing is the real work.
