## Phase 1: Inventory existing coverage

- [x] [serial] r[nix_fork.upstream_rebase.regression_inventory] Build a preserved-behavior coverage matrix from the archived rebase evidence.
- [x] [serial] r[nix_fork.upstream_rebase.regression_tests] Map existing config path, WASM, Radicle, lazy path-input, active-build, FOD, GC, file-transfer, temp-dir, and config-resolution tests to positive and negative coverage.

## Phase 2: Add focused gap coverage

- [x] [serial] r[nix_fork.upstream_rebase.regression_tests] Add focused functional or NixOS tests for high-risk uncovered preserved behavior.
- [x] [serial] r[nix_fork.upstream_rebase.regression_tests] Record blockers with owner, reason, and next validation path for any preserved behavior that cannot be automated in this change.

## Phase 3: Verify and close

- [x] [serial] r[nix_fork.upstream_rebase.regression_inventory] Run Cairn validation and the proposal/design/tasks gates for this change.
- [x] [serial] r[nix_fork.upstream_rebase.regression_tests] Run focused added/changed regression tests and the smallest viable package/check gate.
- [x] [serial] r[nix_fork.upstream_rebase.regression_inventory] Sync accepted fork-maintenance spec updates, archive the change, and record evidence.
