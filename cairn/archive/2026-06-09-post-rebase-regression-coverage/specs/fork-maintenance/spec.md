## ADDED Requirements

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
