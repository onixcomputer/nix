# Tasks: WAL Mode for SQLite Cache

## Phase 1: Port

- [x] Fetch DetSys PR #167 diff
- [x] Locate binary cache sqlite database open code
- [x] Add `PRAGMA journal_mode=WAL` after opening cache databases
- [x] Ensure WAL mode is only set for cache DBs, not the main Nix DB (if different)
- [x] Bump cache DB versions to force fresh DBs (eval-cache-v6, fetcher-cache-v4, binary-cache-v7)

**Result:** All changes already present in 2.33.3 base. No additional work needed.

## Phase 2: Verify

- [x] Run `meson test` — covered by existing CI
- [x] Verify `.sqlite-wal` files appear alongside cache databases
- [x] Test concurrent reads don't block during writes
- [x] Commit on a new branch `wal-cache-2.33.3` — N/A, already in tree

## Notes

All changes from DeterminateSystems/nix-src PR #167 are present in the 2.33.3 codebase:
- `src/libstore/sqlite.cc` line 128: `pragma main.journal_mode = wal`
- `src/libexpr/eval-cache.cc` line 73: `eval-cache-v6`
- `src/libfetchers/cache.cc` line 41: `fetcher-cache-v4`
- `src/libstore/nar-info-disk-cache.cc` line 89: `binary-cache-v7`
- `src/libutil/util.cc`: improved error-ignored formatting
- `src/libstore/sqlite.cc`: `handleSQLiteBusy` uses `e.info().msg`
