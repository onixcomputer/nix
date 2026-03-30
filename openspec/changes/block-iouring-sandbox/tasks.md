# Tasks: Block io_uring in Sandbox

## Phase 1: Port

- [x] Study Lix CL/1611 diff
- [x] Locate seccomp filter setup in `src/libstore/unix/build/linux-derivation-builder.cc`
- [x] Add `io_uring_setup`, `io_uring_enter`, `io_uring_register` deny rules returning `ENOSYS`
- [x] Adapted for CppNix default-allow filter (Lix uses default-deny/allowlist)

## Phase 2: Verify

- [x] `nix build .#nix-cli` — compiles cleanly
- [x] `nix build .#nix-store` — seccomp rule error string present in `.rodata`
- [x] Committed on branch `raise-fd-limit-2.33.3` (253157f5b)

## Notes

CppNix 2.33.3 uses `SCMP_ACT_ALLOW` (default-allow) seccomp filter, while Lix
uses a default-deny allowlist. So instead of removing allow entries (Lix approach),
we add explicit `SCMP_ACT_ERRNO(ENOSYS)` deny rules for the three io_uring syscalls.
