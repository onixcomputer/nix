## Why

The upstream rebase preserved several fork-specific behaviors, but the acceptance evidence is a snapshot. Future upstream rebases need executable regression coverage that proves those preserved behaviors still work and fail safely.

Without a dedicated follow-up, the fork can drift back into relying on one-off manual smoke checks for config path semantics, lazy path-input snapshots, WASM primops, Radicle fetcher parsing, active-build reporting, and local FOD/GC/file-transfer behavior.

## What Changes

- Add a fork-maintenance requirement that preserved fork behaviors from a rebase have an explicit regression inventory.
- Add a requirement that each preserved behavior has either executable positive and negative tests or a recorded blocker with owner, reason, and next validation path.
- Audit existing tests against the preserved-behavior list from the completed upstream rebase.
- Add focused gap-filling functional tests for high-risk uncovered behavior before the change is archived.

## Impact

- **Files**: `cairn/specs/fork-maintenance/spec.md`, this change package, and focused functional tests under `tests/functional/` or `tests/nixos/` as gaps require.
- **Testing**: Cairn validation/gates plus focused functional tests for added coverage and the smallest viable Nix package/check gate after test changes.