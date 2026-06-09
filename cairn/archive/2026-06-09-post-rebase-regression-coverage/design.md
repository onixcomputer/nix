## Context

The completed `2026-06-09-rebase-from-upstream` Cairn preserved fork behavior across a large upstream rebase to Nix `2.35.0`. Its evidence names the preserved areas and records focused manual checks for a subset of them.

Existing tests already cover some preserved behavior, including config-file-relative path settings, `$NIX_CONFIG` cwd-relative path settings, WASM positive and negative cases, and Radicle URL parsing/security cases. The remaining risk is that coverage is implicit and uneven: future rebases may not know which behaviors are protected by tests and which still depend on manual verification.

## Decisions

### 1. Treat regression inventory as fork-maintenance evidence

**Choice:** Maintain a rebase-follow-up inventory that maps each preserved fork behavior to executable tests or an explicit blocker.

**Rationale:** The rebase spec already requires validation evidence. Making the inventory explicit turns one-time evidence into a reusable maintenance artifact for the next rebase.

### 2. Prefer focused functional coverage before broad integration gates

**Choice:** Add or identify small functional/NixOS tests for each preserved behavior before relying on a full package build or larger check.

**Rationale:** Focused tests fail near the behavioral contract and are easier to run during conflict resolution than a broad build that only proves compilation.

### 3. Require positive and negative behavior coverage

**Choice:** Each preserved behavior needs a happy-path assertion and a failure/rejection/boundary assertion unless a blocker is recorded.

**Rationale:** Rebase conflicts often preserve the obvious happy path while dropping validation, error handling, path isolation, or security behavior.

## Risks / Trade-offs

- Some preserved behavior, such as active remote build tracking or Radicle network fetches, may require service or VM fixtures rather than cheap shell tests.
- Additional functional tests can increase gate runtime; this change should prefer focused coverage and avoid duplicating broad integration checks.
- If a behavior cannot be made executable immediately, the blocker must be explicit enough to keep the gap visible.