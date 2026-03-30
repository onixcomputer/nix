# Crash Reporting with Stack Traces

**Upstream:** Lix CL/1854
**Upstream version:** Lix 2.92 (Jan 2025)
**Tier:** 2 — debugging improvement

## Why

When Nix hits an unhandled C++ exception (a bug), it prints the exception
message and exits. No stack trace, no indication of where the crash happened,
no reporting instructions. This makes bug reports nearly useless for diagnosis.

Lix now catches unhandled exceptions and prints:
- Bug report instructions
- The exception type and message
- A rudimentary stack trace (using backtrace)
- Then aborts (generating a core dump if configured)

## What Changes

- **Exception handler**: `std::set_terminate` or equivalent catches unhandled
  exceptions before the runtime kills the process.
- **Stack trace**: Uses `backtrace(3)` / `backtrace_symbols(3)` to print
  frame addresses and symbols.
- **Abort**: Calls `abort()` after printing, to generate a core dump.

## Capabilities

### New Capabilities
- `crash-reporting`: Unhandled exceptions produce stack traces and reporting instructions

## Impact

- **Files**: `src/libmain/shared.cc` (exception handling setup), new
  `src/libutil/stacktrace.cc` or similar
- **APIs**: None
- **Dependencies**: None beyond POSIX `execinfo.h` (already available on Linux/macOS)
- **Testing**: Hard to test directly; verify the terminate handler is installed

## Merge Conflict Risk

Low. Additive code in the startup path.
