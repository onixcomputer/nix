# nix flake show Prints Package Descriptions

**Upstream:** Lix CL/1540
**Upstream version:** Lix 2.91 (Aug 2024)
**Tier:** 2

## Why

`nix flake show` lists packages as `package 'name'` with no indication of what
they do. You have to run `nix eval .#pkg.meta.description` separately. Lix
appends the description from `meta.description` when available:

```
├───hello: package 'hello-2.12.1' - 'A program that produces a familiar, friendly greeting'
```

Small change, big usability win for discoverability.

## What Changes

- **nix flake show**: When printing package entries, evaluate and append
  `meta.description` if it exists.

## Capabilities

### Modified Capabilities
- `flake-show`: Package listings include description text

## Impact

- **Files**: `src/nix/flake.cc` (the `show` subcommand output)
- **APIs**: Output format change (additive, non-breaking for parsers using `--json`)
- **Dependencies**: None
- **Testing**: Add functional test for `nix flake show` output format

## Merge Conflict Risk

Low. The `flake show` command formatting is localized code.
