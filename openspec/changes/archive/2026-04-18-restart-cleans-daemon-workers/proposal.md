# Restart cleans nix-daemon workers

**Upstream:** NixOS/nix issue #10964 (no merged fix as of upstream/master `ca49695a6`)
**Upstream version:** Not yet merged upstream (observed on upstream master 2026-04-18)
**Tier:** 1 — systemd unit fix plus regression test

## Why

`nix-daemon.service` currently uses `KillMode=process`. On restart, systemd
kills only the main daemon PID and leaves per-client `nix-daemon <pid>` worker
processes and delegated build-user cgroups alive. Those stale workers can keep
store and GC lock state alive after the service is considered restarted.

In practice this leaves `systemctl restart nix-daemon` in a half-clean state:
a fresh daemon starts, but orphaned workers from the old instance can still
block `nix-store --gc` and other store operations.

## What Changes

- **systemd unit**: Change `nix-daemon.service` from `KillMode=process` to
  `KillMode=control-group` so stop/restart terminates all service cgroup
  members, not only the main daemon PID.
- **regression test**: Extend the NixOS cgroups VM test to restart the daemon
  during an active daemon-managed build, assert stale worker/build cgroups are
  gone, and verify GC no longer waits on stale locks.

## Capabilities

### Modified Capabilities
- `nix-daemon-restart`: Restarting the daemon now cleans up all service-owned
  workers and delegated build cgroups before the new daemon instance continues
- `gc-after-restart`: Garbage collection no longer waits on stale daemon
  workers left behind by a prior restart

## Impact

- **Files**: `misc/systemd/nix-daemon.service.in`, `tests/nixos/cgroups/default.nix`
- **APIs**: No user-facing API change; service stop/restart semantics change on systemd Linux
- **Dependencies**: None
- **Testing**: `nix build .#hydraJobs.tests.cgroups -L`

## Merge Conflict Risk

Low. The functional change is one unit-file line. Test coverage extends an
existing NixOS cgroups test without changing daemon protocol or evaluator
behavior.
