# WAL Mode for SQLite Cache Databases

**Upstream:** DeterminateSystems/nix-src PR #167
**Upstream version:** Determinate Nix v3.8.5 (Aug 2025)
**Tier:** 1 — small, better concurrency

## Why

Nix uses SQLite databases for binary cache narinfo caching. The default journal
mode (DELETE) serializes all writers and blocks readers during writes. WAL
(Write-Ahead Logging) mode allows concurrent readers during writes and is
generally faster for read-heavy workloads, which is exactly the narinfo cache
access pattern.

## What Changes

- **SQLite cache**: Set `PRAGMA journal_mode=WAL` on cache databases
  (`binary-cache-v*.sqlite` and similar).

## Capabilities

### Modified Capabilities
- `cache-concurrency`: Binary cache SQLite databases use WAL for concurrent reads

## Impact

- **Files**: `src/libstore/sqlite.cc` or binary cache store implementation
- **APIs**: None
- **Dependencies**: None (SQLite already supports WAL)
- **Testing**: Existing store tests cover correctness; concurrent access tests optional

## Merge Conflict Risk

Minimal. A pragma setting on database open.
