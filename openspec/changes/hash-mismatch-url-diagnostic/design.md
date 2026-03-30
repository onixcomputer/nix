# Design: Hash Mismatch URL Diagnostic

## Context

`checkOutputs` in `derivation-check.cc` throws `BuildError` on hash mismatch
but only shows the derivation path and hashes. The derivation's environment
often contains `url` or `urls` which would identify the source.

## Goals / Non-Goals

**Goals:** Show the source URL in fixed-output hash mismatch errors.
**Non-Goals:** Restructured error types, changes to JSON error output.

## Decisions

### 1. Pass derivation env to checkOutputs

**Choice:** Add `const StringPairs & env` parameter to `checkOutputs`.
**Rationale:** The env is already available at the call site
(`drv.env` in `derivation-builder.cc`). Passing it is the minimum change.
**Alternative:** Extract URL at call site and pass `std::optional<std::string>`.
Rejected because `checkOutputs` already knows it's looking at a CAFixed
output — it should own the URL extraction logic.

### 2. URL extraction: try `url` then `urls`

**Choice:** Check `env["url"]` first, then `env["urls"]` (taking the first
space-separated token).
**Rationale:** Matches Lix CL/1536 behavior. `fetchurl` sets `url`;
multi-mirror fetchers set `urls`.

### 3. Conditional URL line in error message

**Choice:** Append `\n  url:       <url>` only when a URL is found.
**Rationale:** No-URL FODs (e.g. `fetchGit`) shouldn't get a spurious
empty line.

## Risks / Trade-offs

**Layering violation** — `checkOutputs` peeks into env vars that are
semantically the builder's business. Acceptable: the diagnostic value
outweighs the purity concern, and Lix ships this.
