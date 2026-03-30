# Garbage Collection Specification

## Purpose

Defines requirements for the Nix garbage collector CLI.

## MODIFIED Requirements

### Requirement: nix-collect-garbage --dry-run

When invoked with `--dry-run`, `nix-collect-garbage` MUST enumerate dead
store paths and print each path followed by a summary line showing the
count and total size that would be freed. It MUST NOT delete any paths.
