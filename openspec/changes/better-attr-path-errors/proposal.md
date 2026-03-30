# Better Error Messages for Bad Attribute Paths

**Upstream:** Lix CL/2277, CL/2280
**Upstream version:** Lix 2.92 (Jan 2025)
**Tier:** 2 — error UX

## Why

When `nix eval -f '<nixpkgs>' lixVersions.lix_2_92` fails because `lix_2_92`
doesn't exist, Nix says so but doesn't tell you what IS available. You have to
drop into `nix repl` to find out. Lix now includes the contents of the attrset
being selected from:

```
error: attribute 'lix_2_92' in selection path 'lixVersions.lix_2_92' not found
  inside path 'lixVersions', whose contents are: { lix_2_90 = «thunk»; lix_2_91 = «thunk»; ... }
  Did you mean one of lix_2_90 or lix_2_91?
```

This saves round-trips to the REPL or debugger.

## What Changes

- **CLI attr path resolution**: When an attribute selection fails from the
  command line, include a truncated representation of the attrset contents
  in the error message.
- **Existing "Did you mean"**: Preserved alongside the new context.

## Capabilities

### Modified Capabilities
- `error-messages`: Attr path errors show available attributes

## Impact

- **Files**: `src/nix/installable-attr-path.cc` or `src/libexpr/attr-path.cc`,
  error formatting
- **APIs**: None
- **Dependencies**: None
- **Testing**: Functional tests for missing attribute with suggestion and context

## Merge Conflict Risk

Low-medium. The attr path resolution code is stable but the error formatting
may have diverged from Lix's structure.
