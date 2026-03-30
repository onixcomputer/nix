# Block io_uring in Linux Sandbox

**Upstream:** Lix CL/1611
**Upstream version:** Lix 2.91 (Aug 2024)
**Tier:** 1 — security fix

## Why

The `io_uring` Linux API cannot be selectively filtered by seccomp — it's all
or nothing. New operations are added regularly to `io_uring` and any of them
could bypass sandbox restrictions (filesystem access, network, etc). Blocking
the entire API in the sandbox is the safe default.

Attempts to use `io_uring` inside the sandbox will get `ENOSYS` as if the
kernel doesn't support it. Most programs fall back gracefully.

## What Changes

- **Sandbox seccomp filter**: Add `io_uring_setup`, `io_uring_enter`,
  `io_uring_register` to the seccomp deny list, returning `ENOSYS`.

## Capabilities

### Modified Capabilities
- `sandbox-security`: io_uring syscalls blocked in build sandbox

## Impact

- **Files**: `src/libstore/linux/personality.cc` or seccomp filter setup
- **APIs**: None
- **Dependencies**: None
- **Testing**: Test that a builder using io_uring gets ENOSYS

## Merge Conflict Risk

Low. The seccomp filter code is relatively stable. Adding three syscalls to
the deny list.
