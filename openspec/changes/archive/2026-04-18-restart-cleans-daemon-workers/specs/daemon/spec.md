# Daemon Service Specification

## Purpose

Defines systemd service lifecycle requirements for `nix-daemon` on Linux
multi-user installations.

## ADDED Requirements

### Requirement: Restarting nix-daemon cleans service-owned workers

The system MUST ensure that, on Linux systemd multi-user installations,
stopping or restarting `nix-daemon.service` terminates all daemon worker
processes and delegated build cgroups that belong to the service before the
restarted unit is treated as clean.

#### Scenario: Restart during active daemon-managed build

- GIVEN an active daemon-managed build with `use-cgroups = true`
- WHEN `systemctl restart nix-daemon` is issued
- THEN the old daemon worker processes are terminated
- AND stale `nix-build-uid-*` cgroups from the old service instance do not
  remain populated
- AND the restarted service contains only the fresh daemon instance needed for
  new work

#### Scenario: Garbage collection after restart

- GIVEN `nix-daemon.service` was restarted while daemon workers previously held
  store state
- WHEN `nix-store --gc --max-freed 0` runs after the restart
- THEN garbage collection completes without waiting on stale daemon workers
  from the old service instance
