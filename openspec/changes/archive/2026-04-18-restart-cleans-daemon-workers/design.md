# Design: Restart cleans nix-daemon workers

## Context

`nix-daemon` accepts connections on the daemon socket and forks per-client
worker processes. On Linux with `use-cgroups`, it also creates delegated
sub-cgroups for daemon/build activity. The current systemd unit uses
`KillMode=process`, so `systemctl restart nix-daemon` terminates only the main
service PID. Existing daemon workers and build-user cgroups survive the
restart, even though systemd considers the service stopped and starts a new
main daemon.

That mismatch between service lifecycle and process lifetime is the root cause
of stale lock holders after restart.

## Goals / Non-Goals

**Goals**
- Make `systemctl stop/restart nix-daemon` clean the entire service cgroup
- Prevent stale daemon workers from surviving restart and holding store/GC locks
- Add regression coverage that exercises restart during active daemon work

**Non-Goals**
- Change daemon protocol semantics
- Add client reconnection logic
- Change non-systemd service managers
- Rework daemon worker forking, `setsid()`, or `dieWithParent`

## Decisions

### 1. Fix lifecycle at the systemd unit boundary

**Choice:** Change `KillMode=process` to `KillMode=control-group` in
`nix-daemon.service`.

**Rationale:** The bug is caused by systemd only killing `MainPID` on restart.
The correct fix is to make systemd manage the full service cgroup, including
worker processes and delegated build cgroups. This matches systemd guidance:
`KillMode=process` is explicitly discouraged because it lets processes escape
service lifecycle management.

**Alternative:** Keep `KillMode=process` and change daemon worker internals
(`dieWithParent`, `setsid()`, explicit worker tracking). Rejected because the
root cause is service manager policy, not daemon protocol, and daemon-side
changes would be more invasive and less obviously correct.

### 2. Prefer control-group over mixed

**Choice:** Use `KillMode=control-group`, not `mixed`.

**Rationale:** `control-group` sends the normal termination signal to all
service cgroup members, including daemon workers and delegated build cgroups,
then lets systemd escalate on timeout if something still refuses to exit.
This gives the entire service a clean interrupt path and avoids leaving old
workers alive after restart.

**Alternative:** `KillMode=mixed`. Rejected because it only sends SIGTERM to
`MainPID` and relies on later SIGKILL for the rest. For Nix, workers already
understand interruption; the goal is graceful whole-cgroup shutdown first, not
main-process-only shutdown.

### 3. Reproduce the failure in the existing cgroups VM test

**Choice:** Extend `tests/nixos/cgroups/default.nix` to restart the service
while a daemon-managed build is running, then assert no stale worker/build
cgroups remain and that GC can run immediately after restart.

**Rationale:** The existing test already exercises daemon/build cgroup layout.
Adding restart assertions makes the failure deterministic and ties regression
coverage directly to the lifecycle bug.

## Risks / Trade-offs

**Restart now interrupts in-flight daemon work** -> Expected and correct.
Restarting the service should stop the old service instance completely rather
than allowing hidden workers to continue holding locks.

**Service-manager-specific change** -> Acceptable. The bug is in systemd unit
semantics, so the fix belongs in the systemd unit.
