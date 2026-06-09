## Why

This fork currently tracks Nix `2.33.3` plus Onix-specific and cherry-picked feature work. The upstream remote is configured as `https://github.com/NixOS/nix.git`, but the fork needs a reviewed, repeatable update path before rebasing `main` onto a newer upstream baseline.

Without a Cairn-backed rebase plan, upstream drift can silently drop fork-only behavior, lose provenance for cherry-picks, or leave version and lock-file metadata inconsistent with the rebased source tree.

## What Changes

- Define the operator contract for selecting an upstream rebase target and recording the before/after commit range.
- Require a clean checkpoint and recoverable backup ref before any history rewrite.
- Rebase fork commits onto the selected upstream target while preserving or explicitly retiring fork-specific behavior.
- Require conflict-resolution notes, version/lock metadata review, and validation evidence before the updated fork is accepted.

## Impact

- **Files**: Primarily Git history plus conflict-touched source, tests, docs, packaging metadata, `.version`, and `flake.lock` as needed by the selected upstream target.
- **Testing**: Baseline checks before the rebase when feasible, followed by targeted tests for conflict-touched areas, positive and negative behavior checks for fork features touched by the rebase, and the smallest viable full build/check gate before archive.
