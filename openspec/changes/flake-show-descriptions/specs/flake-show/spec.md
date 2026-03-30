# Flake Show Specification

## Purpose

Defines requirements for `nix flake show` output.

## MODIFIED Requirements

### Requirement: Package descriptions in human-readable output

When `nix flake show` prints a package entry (non-JSON mode), it MUST
append the `meta.description` string if present and non-empty.

Format: `<tree prefix>: package '<name>' - <description>`

If `meta.description` is absent or empty, the output MUST remain
unchanged: `<tree prefix>: package '<name>'`

#### Scenario: Package with meta.description

- GIVEN a flake with a package that has `meta.description = "A greeting"`
- WHEN `nix flake show` is run (non-JSON)
- THEN the output line contains `- A greeting` after the package name

#### Scenario: Package without meta.description

- GIVEN a flake with a package that has no `meta.description`
- WHEN `nix flake show` is run (non-JSON)
- THEN the output format is unchanged (no trailing dash)

#### Scenario: JSON output includes description

- GIVEN a flake with a package that has `meta.description`
- WHEN `nix flake show --json` is run
- THEN the JSON `description` field contains the description string
