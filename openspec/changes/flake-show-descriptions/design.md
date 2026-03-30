# Design: nix flake show Prints Package Descriptions

## Context

`showDerivation` lambda in `CmdFlakeShow::run` already evaluates
`meta.description` for JSON output but discards it in the human-readable
branch.

## Decisions

### 1. Hoist description evaluation

**Choice:** Move `meta.description` evaluation before the JSON/non-JSON
branch so both paths can use it.
**Rationale:** Avoids duplicating the evaluation logic.

### 2. Catch EvalError on meta access

**Choice:** Wrap `meta.description` evaluation in `try/catch (EvalError &)`.
**Rationale:** Some packages have broken `meta` (e.g. nixpkgs
legacyPackages). Silently falling back to no description is better than
crashing.

### 3. Format: `- <description>` (no quotes)

**Choice:** Append ` - <description>` without quoting the description.
**Rationale:** Matches Lix CL/1540 format. Quotes around the description
would add visual noise for little benefit.
