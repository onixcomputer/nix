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
