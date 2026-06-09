# Rebase from upstream evidence

## Phase 1 preparation

- Date: 2026-06-09
- Repository: `/home/brittonr/git/nix`
- Current branch: `main`
- Upstream remote: `upstream`
- Selected upstream ref: `upstream/master`
- Selected upstream commit: `e471eb71fa5182bc7420dbcc3eaec75c7290e940`
- Pre-rebase fork head at initial checkpoint: `9e9e409f5ac26ddc7bbf6e5cd0594b4b0203bc10`
- Initial backup ref: `refs/backup/rebase-from-upstream-2026-06-09`
- Initial backup commit: `9e9e409f5ac26ddc7bbf6e5cd0594b4b0203bc10`
- Merge base with selected target: `63e0b5d081fed582ac6a0a66f402dc525953524b`
- Raw fork-only commit count from merge base: `22390`
- Raw upstream-ahead commit count from pre-rebase head: `19616`

## Remotes recorded before rebase

```text
onix	git@github.com:onixcomputer/nix.git (fetch)
onix	git@github.com:onixcomputer/nix.git (push)
origin	git@github.com:onixcomputer/nix.git (fetch)
origin	git@github.com:onixcomputer/nix.git (push)
upstream	https://github.com/NixOS/nix.git (fetch)
upstream	https://github.com/NixOS/nix.git (push)
```

## Target range analysis

The raw merge-base range is large because this fork history shares an old base with NixOS/nix, but `git cherry` shows most commits are patch-equivalent to upstream. Candidate patch-equivalence counts before rebase:

```text
2.33.3 patch_unique_plus=49 patch_equiv_minus=12924
2.33.6 patch_unique_plus=49 patch_equiv_minus=12924
upstream/2.33-maintenance patch_unique_plus=49 patch_equiv_minus=12924
2.34.7 patch_unique_plus=58 patch_equiv_minus=12915
upstream/latest-release patch_unique_plus=58 patch_equiv_minus=12915
upstream/master patch_unique_plus=59 patch_equiv_minus=12914
```

`upstream/master` remains the selected target because the user asked to update the fork from upstream, the configured upstream default branch is `master`, and the patch-unique replay set is small enough to attempt with a backup ref in place.

## Pre-rebase baseline

Commands:

```text
nix flake metadata --no-write-lock-file --accept-flake-config
nix eval --raw .#packages.x86_64-linux.nix.version --accept-flake-config
```

Result: passed.

Evidence summary:

```text
Revision: 9e9e409f5ac26ddc7bbf6e5cd0594b4b0203bc10
Revisions: 22391
Fingerprint: 4a005e4ad262cb5825b4832a90a660033238f8d240481665bc5321e3f8caf537
Version eval: 2.33.3
```

Full local baseline log: `/tmp/nix-pre-rebase-baseline.txt`.

## Phase 2 replay and reconciliation

- Rebase target: `upstream/master` at `e471eb71fa5182bc7420dbcc3eaec75c7290e940`.
- Backup ref retained: `refs/backup/rebase-from-upstream-2026-06-09` -> `9e9e409f5ac26ddc7bbf6e5cd0594b4b0203bc10`.
- Rebased checkpoint before post-rebase fixes: `50db9f8372ef0926ada1e110c74729b7b4815275` (`record upstream rebase checkpoint evidence`).
- Branch divergence after replay: `main...origin/main [ahead 19670, behind 18220]`.
- Version after replay: `2.35.0`.

Conflict and porting decisions:

- **Preserve** WASM primops; ported stale primop registration fields from `.fun` to upstream `.impl`.
- **Preserve** Radicle fetcher; ported to upstream source accessor, `GitRepo`, process, cache, and `std::filesystem::path` APIs.
- **Preserve** lazy path inputs; ported path fetcher and `EvalState::mountInput()` to upstream source-accessor/store APIs, and added an eager snapshot fallback when Nix's own cache directory is inside the path input so Nix cache writes cannot mutate the tree between dry-run hashing and forced copy.
- **Preserve** config-file-relative path settings; restored `$NIX_CONFIG` relative path semantics to resolve against cwd while keeping config-file settings relative to the config file directory.
- **Preserve** active build tracking / `nix ps`; ported local/remote active-build query code to upstream builder, platform, and worker-protocol feature APIs.
- **Preserve** fork FOD diagnostics/security fixes, GC dry-run behavior, curl/filetransfer behavior, temp-dir setting behavior, and config path resolution behavior where upstream did not replace them.
- **Retire** obsolete `src/nix/realisation.cc` Meson source entry; upstream uses the current build trace source path.
- **Upstream replaces** historical fork version metadata; `.version` remains upstream `2.35.0`.

Post-rebase source fixes were applied in:

```text
src/libexpr/paths.cc
src/libexpr/primops.cc
src/libexpr/primops/wasm.cc
src/libfetchers/path.cc
src/libfetchers/radicle.cc
src/libflake/config.cc
src/libstore/active-builds.cc
src/libstore/include/nix/store/active-builds.hh
src/libstore/local-store-active-builds.cc
src/libstore/remote-store.cc
src/libutil/configuration.cc
src/nix/meson.build
```

## Phase 3 validation

Version/output check:

```text
Command: nix eval --raw .#packages.x86_64-linux.nix.version --accept-flake-config
Result: passed, version=2.35.0

Command: nix build .#packages.x86_64-linux.nix --no-link --accept-flake-config --print-out-paths
Result: passed
Output: /nix/store/pf56mi71iax17wzswrkgpxjk2sfyb4a3-nix-2.35.0-man /nix/store/f4n7m7zpm73a4v7xd99n21vi76n84737-nix-2.35.0
```

Focused post-fix checks:

```text
Command: final built nix binary with NIX_CONFIG="diff-hook = relative/path" from a temp cwd
Result: passed, diff-hook resolved to <temp>/home/relative/path

Command: final built nix binary: nix run --no-write-lock-file . -- myarg1 myarg2
Result: passed, output "ARGS: myarg1 myarg2"

Command: final built nix binary: nix run --no-write-lock-file -- . myarg1 myarg2
Result: passed, output "ARGS: myarg1 myarg2"
```

Full local log: pueue task `115`.

Integration gate:

```text
Command: nix build .#packages.x86_64-linux.nix --no-link --accept-flake-config -L
Result: passed
Log: /tmp/nix-post-rebase-build-retry11.txt
Functional tests: Ok 208, Fail 0, Skipped 8
Built output: /nix/store/f4n7m7zpm73a4v7xd99n21vi76n84737-nix-2.35.0
```

Non-blocking infrastructure warning observed during the build:

```text
cannot build on 'ssh-ng://root@10.10.10.1': error: failed to start SSH connection to '10.10.10.1'
```

The local build recovered by building locally and completed successfully.

Static checks:

```text
Command: git diff --check
Result: passed

Command: git grep -n -E '^(<<<<<<<|=======|>>>>>>>)' -- . ':!*.lock'
Result: passed with no conflict markers
```

Cairn validation:

```text
Command: nix run path:$PWD#cairn -- validate --root /home/brittonr/git/nix
Working directory: /home/brittonr/git/cairn
Result: passed
Log: /tmp/nix-cairn-validation-final2.txt
Summary: valid=true, changes=1, specs_validated=1, issues=[]
```

## Publication instructions

1. Commit the post-rebase source fixes and this Cairn evidence on `main`.
2. Before publishing, review `git status --short --branch --untracked-files=no`; expected divergence is `main...origin/main [ahead 19670, behind 18220]` before the final evidence commit.
3. Decide whether to restore or discard the backed-up untracked-file collision directory `/tmp/nix-rebase-untracked-2026-06-09`; do not publish unrelated `.agent/`, `.pi/`, or `openspec/` files.
4. Keep backup ref `refs/backup/rebase-from-upstream-2026-06-09` until the pushed branch is verified by another checkout.
5. Publish only after explicit operator approval. Because this is a rebased history, the safe push form is `git push --force-with-lease origin main` after confirming the expected remote tip and backup ref.
