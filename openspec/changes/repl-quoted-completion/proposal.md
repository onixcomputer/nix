# REPL Tab-Completion with Proper Quoting

**Upstream:** Lix CL/1783
**Upstream version:** Lix 2.92 (Jan 2025)
**Tier:** 2 — UX fix

## Why

When tab-completing attribute names that require quoting (e.g., names containing
`@`, spaces, or dots), the REPL suggests them without quotes, producing invalid
syntax. `"hello@example.com"` completes as `hello@example.com` which fails to
parse.

## What Changes

- **REPL completion**: Attribute names that require quoting are completed with
  surrounding quotes.

## Capabilities

### Modified Capabilities
- `repl-completion`: Quoted attribute names complete with quotes

## Impact

- **Files**: `src/libcmd/repl.cc` (completion handler)
- **APIs**: None
- **Dependencies**: None
- **Testing**: REPL test with an attrset containing a key like `"foo@bar"`

## Merge Conflict Risk

Low. Small addition to the completion formatting logic.
