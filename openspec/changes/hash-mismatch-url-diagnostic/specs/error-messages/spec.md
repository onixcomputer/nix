# Error Messages Specification

## Purpose

Defines requirements for diagnostic error messages in Nix.

## ADDED Requirements

### Requirement: Hash Mismatch Shows Source URL

When a fixed-output derivation fails its hash check, the error message MUST
include the likely source URL extracted from the derivation's `url` or `urls`
environment variable, if present.

#### Scenario: FOD with single url env var

- GIVEN a fixed-output derivation with `url` set in its environment
- WHEN the hash check fails
- THEN the error message includes the URL alongside the expected and actual hashes

#### Scenario: FOD with urls env var (space-separated)

- GIVEN a fixed-output derivation with `urls` set (space-separated list)
- WHEN the hash check fails
- THEN the error message includes the first URL from the list

#### Scenario: FOD with no url env var

- GIVEN a fixed-output derivation with neither `url` nor `urls` in its environment
- WHEN the hash check fails
- THEN the error message is unchanged from the current format (no URL line)
