## Phase 1: Prepare the rebase

- [x] [serial] r[nix_fork.upstream_rebase.target] Fetch upstream metadata and record the exact upstream ref, upstream commit, current fork head, and fork-only commit range selected for replay.
- [x] [serial] r[nix_fork.upstream_rebase.checkpoint] Confirm the worktree is clean, record existing remotes, and create a recoverable local backup ref for the pre-rebase fork head.
- [x] [serial] r[nix_fork.upstream_rebase.baseline] Run the smallest relevant pre-rebase baseline check that currently passes, or record the existing blocker before changing history.

## Phase 2: Rebase and reconcile

- [ ] [serial] r[nix_fork.upstream_rebase.replay] Rebase the fork-only commits onto the selected upstream target without squashing or dropping commits unless a recorded retirement decision says otherwise.
- [ ] [serial] r[nix_fork.upstream_rebase.conflicts] Resolve conflicts with notes for every fork behavior that was preserved, replaced by upstream, or intentionally retired.
- [ ] [serial] r[nix_fork.upstream_rebase.metadata] Review and update version, release-note, packaging, and lock-file metadata so it matches the rebased source tree.

## Phase 3: Verify and close

- [ ] [serial] r[nix_fork.upstream_rebase.validation] Run focused checks for conflict-touched areas, including positive and negative behavior coverage for touched fork features.
- [ ] [serial] r[nix_fork.upstream_rebase.validation] Run the smallest viable full integration gate for this repo and record any skipped checks with owner, reason, and follow-up.
- [ ] [serial] r[nix_fork.upstream_rebase.evidence] Record the before/after commit range, conflict summary, validation evidence, and publication instructions before archiving this Cairn.
