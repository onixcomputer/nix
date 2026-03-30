# Flakeref Display Specification

## Purpose

Defines requirements for how flake references are displayed in user-facing output.

## MODIFIED Requirements

### Requirement: Abbreviated revisions in human output

In human-readable output (non-JSON), flake references MUST abbreviate
the revision hash to 7 characters.

JSON output MUST preserve full revision hashes unchanged.

#### Scenario: Lockfile diff shows abbreviated rev

- GIVEN a flake with a locked input at rev `e21630230c77140bc6478a21cd71e8bb73706fce`
- WHEN the lock file is updated and a diff is printed
- THEN the diff shows `rev=e216302` (7 chars) not the full hash

#### Scenario: Metadata inputs show abbreviated rev

- GIVEN a flake with locked inputs
- WHEN `nix flake metadata` is run (non-JSON)
- THEN the input tree shows abbreviated revisions

#### Scenario: JSON output preserves full rev

- GIVEN a flake with locked inputs
- WHEN `nix flake metadata --json` is run
- THEN the `revision` field contains the full 40-char hash
