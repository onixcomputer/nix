# Restrict URL Schemes in Transfers

**Upstream:** Lix CL/2106
**Upstream version:** Lix 2.92 (Jan 2025)
**Tier:** 1 — security hardening

## Why

curl supports many URL schemes beyond HTTP — things like `dict://`, `gopher://`,
`ldap://`, `telnet://`. Allowing arbitrary schemes in `builtins.fetchurl`,
`<nix/fetchurl.nix>`, and binary cache transfers is an unnecessary attack
surface. Restricting to `http://`, `https://`, `ftp://`, `ftps://`, and
`file://` eliminates exotic protocol abuse.

`s3://` is unaffected (uses the AWS SDK, not curl). Flake inputs using
multi-protocol schemes like `git+ssh` are unaffected (they use external
utilities).

## What Changes

- **File transfer**: Only allow `http`, `https`, `ftp`, `ftps`, and `file`
  URL schemes through curl.

## Capabilities

### Modified Capabilities
- `transfer-security`: Only standard URL schemes permitted in transfers

## Impact

- **Files**: `src/libstore/filetransfer.cc`
- **APIs**: None; exotic schemes now return an error
- **Dependencies**: None
- **Testing**: Test that `http` and `file` work; test that `gopher://` is rejected

## Merge Conflict Risk

Low. A scheme whitelist check before the curl call.
