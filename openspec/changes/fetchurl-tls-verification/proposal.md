# <nix/fetchurl.nix> TLS Verification

**Upstream:** Lix 2.92 (attributed to Eelco Dolstra, merged upstream)
**Upstream version:** Lix 2.92 (Jan 2025)
**Tier:** 2 — security fix

## Why

`<nix/fetchurl.nix>` (the builtin derivation builder `builtin:fetchurl`) did
not perform TLS certificate verification. The rationale was that the sandbox
lacked certificates and the hash was checked anyway. But:

1. The sandbox now has certificate access.
2. Impure derivations don't always check hashes.
3. Authentication data from `netrc` and URLs is exposed to MITM attackers
   without TLS verification.

This is distinct from `builtins.fetchurl` (the evaluation-time function),
which was not affected.

## What Changes

- **builtin:fetchurl**: Enable TLS certificate verification for HTTPS downloads.

## Capabilities

### Modified Capabilities
- `fetchurl-security`: builtin:fetchurl verifies TLS certificates

## Impact

- **Files**: `src/libstore/builtins/fetchurl.cc`
- **APIs**: HTTPS fetches with invalid certificates now fail
- **Dependencies**: None
- **Testing**: Test that HTTPS with a valid cert succeeds; test that an
  invalid cert fails

## Merge Conflict Risk

Low. Small change to the curl options in the fetchurl builder.
