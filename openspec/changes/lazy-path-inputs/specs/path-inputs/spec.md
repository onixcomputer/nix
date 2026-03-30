# Path Input Specification

## Purpose

Defines requirements for `path:` type flake inputs.

## MODIFIED Requirements

### Requirement: Lazy filesystem access for path inputs

Path inputs MUST return a lazy accessor that reads directly from the
filesystem instead of eagerly copying the entire directory tree to the
Nix store. The store copy MUST be deferred until `mountInput()` calls
`fetchToStore()`.

#### Scenario: Evaluation reads only needed files

- GIVEN a path input pointing at a large directory
- WHEN evaluation only accesses `flake.nix`
- THEN only `flake.nix` is read from the filesystem (not the full tree)

#### Scenario: Store copy on build

- GIVEN a lazy path input accessor
- WHEN a build requires the store path
- THEN `fetchToStore()` copies the tree to the store

#### Scenario: Already-valid store paths use store accessor

- GIVEN a path input pointing at an existing valid store path named "source"
- WHEN `getAccessor()` is called
- THEN the store accessor is returned directly (no lazy wrapper)
