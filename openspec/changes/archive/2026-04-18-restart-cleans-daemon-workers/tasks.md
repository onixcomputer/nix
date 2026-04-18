# Tasks: Restart cleans nix-daemon workers

## Phase 1: Unit fix

- [x] Change `misc/systemd/nix-daemon.service.in` to use `KillMode=control-group` and document why `KillMode=process` is incorrect for daemon worker restarts

## Phase 2: Regression coverage

- [x] Extend `tests/nixos/cgroups/default.nix` to restart `nix-daemon` during an active daemon-managed build and assert no stale daemon/build cgroups remain
- [x] Add a post-restart GC check that fails if stale daemon workers still hold GC/store locks

## Phase 3: Verify

- [x] Run `nix build .#hydraJobs.tests.cgroups -L`
