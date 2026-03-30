# Tasks: Raise Open File Soft Limit

## Phase 1: Port

- [x] Fetch DetSys PR #347 diff
- [x] Add `bumpFileLimit()` in `src/libmain/shared.cc` — calls `setrlimit(RLIMIT_NOFILE)` to raise soft to hard
- [x] Called from `initNix()` after `umask(0022)`
- [x] Guarded with `#ifndef _WIN32` (matches upstream)

## Phase 2: Verify

- [x] `nix build .#nix-cli` — compiles, links `getrlimit64`/`setrlimit64` from glibc
- [x] `nix build .#nix-main` — `nm -D` confirms symbols in `libnixmain.so`
- [x] Committed on branch `raise-fd-limit-2.33.3` (21dd41e66)
