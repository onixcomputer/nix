# Fork Maintenance Specification

## Purpose

Defines the `fork-maintenance` capability.

## Requirements

### Requirement: Upstream rebase target is explicit
r[nix_fork.upstream_rebase.target] The fork update process MUST record the exact upstream remote, upstream ref, upstream commit, current fork head, and fork-only commit range before replaying local history.

#### Scenario: Target recorded before rebase
r[nix_fork.upstream_rebase.target.recorded]
- GIVEN the operator has fetched upstream metadata
- WHEN the operator selects a rebase target
- THEN the update notes include the upstream remote, selected ref, selected commit, current fork head, and fork-only commit range

#### Scenario: Missing target blocks the rebase
r[nix_fork.upstream_rebase.target.missing]
- GIVEN no exact upstream commit has been recorded
- WHEN the operator is ready to rebase
- THEN the process MUST stop before rewriting history

### Requirement: Clean checkpoint protects the fork
r[nix_fork.upstream_rebase.checkpoint] The fork update process MUST require a clean worktree and a recoverable local backup ref for the pre-rebase fork head before any history rewrite.

#### Scenario: Clean checkpoint exists
r[nix_fork.upstream_rebase.checkpoint.clean]
- GIVEN the worktree is clean and remotes have been recorded
- WHEN the operator starts the rebase
- THEN a backup ref exists that points at the pre-rebase fork head

#### Scenario: Dirty worktree blocks rewrite
r[nix_fork.upstream_rebase.checkpoint.dirty]
- GIVEN the worktree has uncommitted changes
- WHEN the operator attempts to start the rebase
- THEN the process MUST stop until those changes are committed, stashed, or explicitly discarded

### Requirement: Baseline health is known before rewrite
r[nix_fork.upstream_rebase.baseline] The fork update process SHOULD run the smallest relevant pre-rebase baseline check that currently passes, or MUST record the existing blocker before changing history.

#### Scenario: Passing baseline captured
r[nix_fork.upstream_rebase.baseline.pass]
- GIVEN a focused baseline check is available
- WHEN the operator runs it before the rebase
- THEN the update notes include the command and result

#### Scenario: Baseline blocker captured
r[nix_fork.upstream_rebase.baseline.blocked]
- GIVEN the baseline check cannot run or already fails
- WHEN the operator proceeds with the rebase
- THEN the update notes include the blocker, owner, and next best verification path

### Requirement: Fork commits replay without silent loss
r[nix_fork.upstream_rebase.replay] The fork update process MUST replay fork-only commits onto the selected upstream target without silently squashing, dropping, or reordering behavior-bearing commits.

#### Scenario: Fork range is replayed
r[nix_fork.upstream_rebase.replay.range]
- GIVEN the fork-only commit range has been recorded
- WHEN the rebase completes
- THEN each behavior-bearing fork commit is present in the rebased history or has an explicit recorded retirement decision

#### Scenario: Unexpected dropped commit blocks acceptance
r[nix_fork.upstream_rebase.replay.dropped]
- GIVEN a fork-only behavior-bearing commit is absent after the rebase
- WHEN no retirement decision exists for that commit
- THEN the update MUST NOT be accepted

### Requirement: Conflict decisions preserve fork behavior by default
r[nix_fork.upstream_rebase.conflicts] The fork update process MUST document every manual conflict decision that affects fork-specific behavior, choosing preserve, upstream-replaces, or retire with rationale.

#### Scenario: Conflict decision recorded
r[nix_fork.upstream_rebase.conflicts.recorded]
- GIVEN a conflict touches local fork behavior
- WHEN the operator resolves the conflict
- THEN the conflict notes identify the behavior, selected decision, and rationale

#### Scenario: Unexplained conflict blocks acceptance
r[nix_fork.upstream_rebase.conflicts.missing]
- GIVEN a conflict touched local fork behavior
- WHEN no conflict decision note exists
- THEN the update MUST NOT be accepted

### Requirement: Metadata matches the rebased tree
r[nix_fork.upstream_rebase.metadata] The fork update process MUST review version, release-note, packaging, and lock-file metadata affected by the upstream target so checked-in metadata matches the rebased source tree.

#### Scenario: Metadata reviewed
r[nix_fork.upstream_rebase.metadata.reviewed]
- GIVEN the rebase has completed
- WHEN the operator reviews repository metadata
- THEN changed version, release-note, packaging, and lock-file files are either updated or recorded as intentionally unchanged

#### Scenario: Stale metadata blocks acceptance
r[nix_fork.upstream_rebase.metadata.stale]
- GIVEN checked-in metadata still describes the old upstream base
- WHEN the operator prepares to accept the rebase
- THEN the update MUST remain blocked until the metadata is corrected or explicitly justified

### Requirement: Rebased fork has validation evidence
r[nix_fork.upstream_rebase.validation] The fork update process MUST collect validation evidence for conflict-touched areas and SHOULD include both positive and negative behavior checks for touched fork features.

#### Scenario: Focused validation covers touched behavior
r[nix_fork.upstream_rebase.validation.focused]
- GIVEN conflicts or upstream changes touched a fork feature
- WHEN validation is run
- THEN the evidence includes focused checks for the touched behavior and records positive and negative coverage where applicable

#### Scenario: Integration gate result recorded
r[nix_fork.upstream_rebase.validation.integration]
- GIVEN focused checks have passed or documented blockers exist
- WHEN the operator runs the final integration gate
- THEN the update notes include the command, result, and any skipped checks with owner, reason, and follow-up

### Requirement: Acceptance evidence is complete before publication
r[nix_fork.upstream_rebase.evidence] The fork update process MUST record before/after commits, upstream target, conflict summary, validation evidence, and publication instructions before the rebased branch is published or the Cairn is archived.

#### Scenario: Evidence bundle is complete
r[nix_fork.upstream_rebase.evidence.complete]
- GIVEN the rebase and validation are complete
- WHEN the operator prepares to publish or archive
- THEN the evidence identifies the pre-rebase head, rebased head, upstream target, conflict decisions, validation results, and publication instructions

#### Scenario: Publication without evidence is blocked
r[nix_fork.upstream_rebase.evidence.missing]
- GIVEN acceptance evidence is incomplete
- WHEN the operator prepares to publish the rebased branch
- THEN publication MUST wait until the evidence is completed

### Requirement: Preserved fork behavior has a regression inventory
r[nix_fork.upstream_rebase.regression_inventory] The fork update process MUST maintain an inventory that maps every preserved fork behavior from a completed upstream rebase to executable regression coverage or an explicit blocker.

#### Scenario: Preserved behavior is mapped to coverage
r[nix_fork.upstream_rebase.regression_inventory.mapped]
- GIVEN a completed upstream rebase records preserved fork behavior
- WHEN the follow-up hardening work audits regression coverage
- THEN each preserved behavior is mapped to one or more focused tests or to a recorded blocker with owner, reason, and next validation path

#### Scenario: Unmapped preserved behavior blocks archive
r[nix_fork.upstream_rebase.regression_inventory.missing]
- GIVEN a preserved fork behavior has no mapped test and no recorded blocker
- WHEN the follow-up hardening work is ready to archive
- THEN archive MUST remain blocked until the mapping exists

### Requirement: Preserved fork behavior has positive and negative tests
r[nix_fork.upstream_rebase.regression_tests] Preserved fork behavior SHOULD have executable positive and negative regression tests, and MUST record a blocker when either side cannot be automated in the current change.

#### Scenario: Positive and negative coverage is executable
r[nix_fork.upstream_rebase.regression_tests.both]
- GIVEN a preserved fork behavior can be exercised by the repository test harness
- WHEN regression coverage is added or audited
- THEN the evidence identifies both the positive behavior assertion and the negative, boundary, or rejection assertion

#### Scenario: Missing negative coverage is recorded
r[nix_fork.upstream_rebase.regression_tests.negative_blocked]
- GIVEN only happy-path coverage can be automated for a preserved behavior
- WHEN the follow-up hardening work records its evidence
- THEN the evidence records why negative coverage is blocked, who owns the follow-up, and what next validation path should be used
