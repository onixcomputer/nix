## Context

The repository is a Git checkout on `main` with `origin`/`onix` pointing at `git@github.com:onixcomputer/nix.git` and `upstream` pointing at `https://github.com/NixOS/nix.git`. The working tree was clean when this Cairn was created.

The fork carries upstream Nix plus local/cherry-picked work tracked in the existing lifecycle notes. Rebasing must therefore be treated as a controlled history rewrite, not a blind fast-forward or merge.

## Decisions

### 1. Record an explicit upstream target

**Choice:** The rebase operator must name and record the exact upstream ref and commit used as the new base before replaying fork commits.

**Rationale:** A named target makes the update auditable and prevents ambiguity between upstream `master`, release-maintenance branches, tags, and transient remote-tracking state.

### 2. Create a recoverable checkpoint before rewriting history

**Choice:** The operator must start from a clean worktree and create a local backup ref for the pre-rebase fork head before running the rebase.

**Rationale:** Rebases are destructive history edits. A backup ref makes abort/retry and post-rebase comparison boring and deterministic.

### 3. Preserve fork behavior by default

**Choice:** During conflict resolution, fork-specific behavior is preserved unless the operator records an explicit retirement or upstream replacement decision.

**Rationale:** The fork intentionally contains local and cherry-picked capabilities. Silent drops are harder to detect than deliberate, reviewed retirements.

### 4. Validate by touched area and then by integration gate

**Choice:** Conflict-touched subsystems get focused checks first, including positive and negative cases for changed behavior, followed by the smallest viable full repository gate.

**Rationale:** Focused checks isolate conflict mistakes quickly; the final integration gate catches cross-cutting build, doc, packaging, and lock-file drift.

## Risks / Trade-offs

- Upstream may have build-system, lock-file, or test-suite changes that require more than mechanical conflict resolution.
- Long-lived fork commits can conflict semantically even when Git applies them cleanly.
- Full Nix checks may be expensive; record any skipped check with owner, reason, and follow-up instead of treating absence as success.
- Do not push rewritten history until the operator explicitly approves the updated branch publication path.
