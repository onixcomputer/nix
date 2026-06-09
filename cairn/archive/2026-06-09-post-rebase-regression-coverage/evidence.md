# Post-rebase regression coverage evidence

## Origin verification and cleanup

Fresh-origin verification after publishing the archived rebase evidence:

```text
Command: git clone --depth=1 --branch main git@github.com:onixcomputer/nix.git /tmp/nix-origin-verify-20260609182001
Result: passed
cloned_head=dd00fcb767fbe88d9a33ee6470e414ea3e402bf5

Command: nix eval --raw .#packages.x86_64-linux.nix.version --accept-flake-config
Result: passed, version=2.35.0

Command: nix build .#packages.x86_64-linux.nix --no-link --accept-flake-config --print-out-paths
Result: passed
Output: /nix/store/pf56mi71iax17wzswrkgpxjk2sfyb4a3-nix-2.35.0-man /nix/store/f4n7m7zpm73a4v7xd99n21vi76n84737-nix-2.35.0
```

Local backup refs removed after origin verification:

```text
deleted refs/backup/rebase-from-upstream-2026-06-09 9e9e409f5ac26ddc7bbf6e5cd0594b4b0203bc10
deleted refs/backup/rebase-from-upstream-2026-06-09-repaired-from 63a2993e827ce6ee84639309d92e13ae4799419f
backup_refs_removed
```

## Coverage inventory

| Preserved behavior | Positive coverage | Negative / boundary coverage | Status |
| --- | --- | --- | --- |
| Config-file-relative path settings and `$NIX_CONFIG` cwd-relative paths | `tests/functional/config.sh` verifies `NIX_USER_CONF_FILES` relative `diff-hook` resolves below the config directory and `$NIX_CONFIG="diff-hook = relative/path"` resolves below the caller cwd. | Same test separates the config-file path case from the env-only cwd case; include/tilde cases also cover alternate config path resolution. | Covered by existing functional test. |
| WASM primops | `tests/functional/wasm.sh` covers non-WASI, WASI, string-context round trips, inline WAT, and precompiled `.wasm` execution. | `tests/functional/wasm.sh` rejects missing path/wat, unknown attributes, invalid WASI/non-WASI function usage, no-return WASI modules, panic propagation, feature-gate failures, and path/wat mutual exclusion. | Covered by existing functional test. |
| Radicle fetcher parsing and safety | `tests/functional/fetchRadicle.sh` parses Radicle URLs, attr-based inputs, refs, authorities, and rev parameters; `tests/nixos/radicle-fetch.nix` defines a VM-level Radicle fetch scenario. | `tests/functional/fetchRadicle.sh` rejects empty RIDs, wrong schemes, invalid IDs, and command-injection-style input. | Mock/parse coverage exists. Full network fetch remains blocked: owner Onix Nix fork maintainer; reason requires a reliable Radicle service/identity fixture; next path is make `tests/nixos/radicle-fetch.nix` self-contained and add it to the relevant VM gate. |
| Lazy path inputs with Nix cache inside the input tree | Added `tests/functional/flakes/non-flake-inputs.sh` coverage that sets `XDG_CACHE_HOME` inside a non-flake path input, reads from the input, and asserts no `contents have changed` error occurs. | Added wrong-`narHash` path-input assertion that still fails with `NAR hash mismatch`, proving eager snapshotting does not bypass integrity checks. | Covered by new functional test. |
| Active build tracking / `nix ps` | Source port is present in `src/libstore/active-builds.cc`, `src/libstore/local-store-active-builds.cc`, `src/libstore/remote-store.cc`, and `src/nix/ps.cc`; no focused executable test was found. | No negative protocol/unsupported-remote test was found. | Blocked: owner Onix Nix fork maintainer; reason requires a concurrent long-running build fixture and daemon/remote-store harness; next path is add a functional or NixOS test that starts a controlled build, observes `nix ps --json`, then verifies the list empties after completion. |
| FOD diagnostics/security behavior | `tests/functional/build.sh`, `tests/functional/fixed*.sh`, and `tests/nixos/fod-symlink-overwrite/default.nix` cover FOD hash mismatch behavior, cancelled-goal reporting, and symlink overwrite prevention. | FOD failure paths assert hash-mismatch diagnostics and verify malicious symlink overwrite attempts do not corrupt host files. | Covered by existing functional/NixOS tests. |
| GC dry-run behavior | `tests/functional/gc.sh` checks `nix-collect-garbage --dry-run` reports dead paths and expected freed output. | Same test verifies dry-run reports without deleting the target before later GC behavior. | Covered by existing functional test. |
| FileTransfer/curl behavior | `tests/filetransfer-retry-backoff/default.nix` covers deterministic retry backoff behavior; `tests/nixos/s3-binary-cache-store.nix` covers forked `builtin:fetchurl` FileTransfer creation and credential pre-resolution behavior. | S3 and retry tests cover error/retry and concurrent fetch boundary behavior. | Covered by existing tests; not rerun in this focused change. |
| Temp/build-dir settings and config path resolution | `tests/functional/check.sh`, `tests/functional/nested-sandboxing.sh`, `tests/functional/nix-shell.sh`, and related sandbox tests exercise build-dir/TMPDIR behavior. | Nested sandboxing rejects invalid sandbox build-dir layouts; keep-failed/custom build-dir paths cover failure retention behavior. | Covered by existing functional tests. |

## Validation

Pre-change baseline:

```text
Command: nix build .#packages.x86_64-linux.nix-functional-tests --no-link --accept-flake-config -L
Result: passed
```

Focused test after adding lazy path-input coverage:

```text
Command: nix develop .#packages.x86_64-linux.nix-functional-tests --accept-flake-config -c bash -lc 'meson setup tests/functional "$0" >/dev/null && meson test -C "$0" --suite flakes non-flake-inputs --print-errorlogs' /tmp/nix-functional-debug-build
Result: passed
Test: flakes - nix-functional-tests:non-flake-inputs OK
```

Full functional package after change:

```text
Command: nix build .#packages.x86_64-linux.nix-functional-tests --no-link --accept-flake-config -L
Result: passed
Summary: Ok 208, Fail 0, Skipped 8
Output: /nix/store/s93x1zdgpqf665d1lr20hbzghmziqyc8-nix-functional-tests-2.35.0
```

Non-blocking infrastructure warning observed during focused test environment creation:

```text
cannot build on 'ssh-ng://root@10.10.10.1': error: failed to start SSH connection to '10.10.10.1'
```

The build recovered locally and the focused test passed.
