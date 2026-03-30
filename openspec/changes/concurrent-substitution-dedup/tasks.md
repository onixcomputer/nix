# Tasks: Concurrent Substitution Dedup

## Phase 1: Port

- [x] Fetch DetSys PR #398 diff
- [x] Add `EnsureRead` wrapper Source to `src/libutil/include/nix/util/serialise.hh`
- [x] Remove `narRead`/`cleanup` skip logic from `LocalStore::addToStore` in `local-store.cc`
- [x] Wrap source in `EnsureRead` in `daemon.cc` (daemon addToStore call)
- [x] Wrap source in `EnsureRead` in `store-api.cc` (`addMultipleToStore`)
- [x] Update `store-api.hh` doc comment noting NAR may not be fully read

## Phase 2: Verify

- [x] `nix build .#nix-cli` — all 8 derivations build cleanly
- [x] Committed on branch `raise-fd-limit-2.33.3` (7d90473f0)

## Notes

- `export-import.cc` did not need changes — our code uses StringSource (data already captured)
- `copyStorePath` test hook (`_NIX_TEST_CONCURRENT_SUBSTITUTION`) not ported (optional, only for testing)
- The `nix-copy-ssh-ng.sh` and `binary-cache.sh` functional test changes not ported (require test infra)
