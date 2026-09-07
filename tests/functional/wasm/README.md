# Wasm Test Fixtures

Pre-built `.wasm` modules for testing `builtins.wasm`.

## Modules

| File | Mode | Target | Description |
|------|------|--------|-------------|
| `pure_double.wasm` | non-WASI | `wasm32-unknown-unknown` | Doubles an integer. Exports `nix_wasm_init_v1` and `double`. |
| `pure_panic.wasm` | non-WASI | `wasm32-unknown-unknown` | Calls `panic` host function (error propagation test). |
| `wasi_double.wasm` | WASI | `wasm32-wasip1` | Doubles an integer via `_start` + `return_to_nix`. |
| `wasi_hello.wasm` | WASI | `wasm32-wasip1` | Writes to stdout/stderr, returns "done" string. |
| `wasi_no_return.wasm` | WASI | `wasm32-wasip1` | Exits without calling `return_to_nix` (error path test). |
| `wasi_panic.wasm` | WASI | `wasm32-wasip1` | Calls `panic` host function (WASI error propagation test). |

## Rebuilding

From this directory:

```sh
cd fixtures
nix-build build.nix
cp result/*.wasm ../
```

Requires fenix (fetched automatically by `build.nix`) for the `wasm32-wasip1` target stdlib.

## Source

Rust source is in `fixtures/`. Each module uses inline FFI against the Nix
wasm host interface (no external crate dependencies).

## Attribute-name checks

`wasm.sh` runs `attrname.nix` with the Nix binary under test.
The WAT fixture covers flat sets, layered overrides, empty sets, and unforced values.
It also covers repeated indices, backward requests, and changes between sets.
Invalid indices and incorrect name lengths must fail through `builtins.tryEval`.

## Nickel integration

The optional Nickel checks need a built `onix-wasm` plugin.
They do not add a plugin dependency to the Nix functional suite.

Run the integration controls with each Nix binary:

```sh
"$NIX_BINARY" eval --json --impure \
  --extra-experimental-features 'nix-command wasm-builtin' \
  --file tests/functional/wasm/nickel.nix \
  --apply "f: f { plugin = $NICKEL_PLUGIN; }"
```

The output must be `true`.
The checks cover layered records, lists, functions, paths, derivations, string contexts, imports, contract errors, and calls after an error.

## Performance comparison

`nickel-bench.nix` sends a large layered record through the real Nickel evaluator.
It asserts exact record equality before it returns the attribute count.
Use the same plugin, machine, and attribute count with both Nix binaries.

```sh
"$NIX_BINARY" eval --json --impure \
  --extra-experimental-features 'nix-command wasm-builtin' \
  --file tests/functional/wasm/nickel-bench.nix \
  --apply "f: f { plugin = $NICKEL_PLUGIN; attributeCount = $ATTRIBUTE_COUNT; }"
```

`--apply` calls the fixture function. `--json` rejects an unevaluated function.
A successful command that only prints a lambda is not performance evidence.
Each process includes Wasm compilation, Nickel setup, conversion, and the equality assertion.
These timings do not measure attribute-name host calls alone.

### Measured comparison (2026-09-06)

Both binaries ran on the same x86_64-linux host with the same plugin.
Hyperfine ran one warmup and five measured processes per case.
Each process passed the exact record-equality assertion.

| Attributes | Baseline mean | Cursor mean |
|---|---|---|
| 10,000 | 2.142 ± 0.030 seconds | 1.944 ± 0.009 seconds |
| 20,000 | 2.535 ± 0.054 seconds | 1.993 ± 0.030 seconds |

The uncertainties are sample standard deviations, not confidence intervals.
A reverse-order repeat at 20,000 attributes measured 3.034 ± 0.324 seconds for the cursor and 6.229 ± 5.863 seconds for the baseline.
Hyperfine reported outliers in that repeat. These shared-host timings do not establish a stable speedup ratio.
The baseline source is `f93f28ad9`.
The plugin source is `onix-wasm` revision `cd23cf31b948793f84831bc32c934713b58e82cb`.

Exact artifacts:

- Baseline: `/nix/store/4iycbf2c2g518qvjm53gxxx26r585kz1-nix-2.36.0/bin/nix`
- Cursor: `/nix/store/j7d4vg5bady00wp2h022cqq4hwrhvivj-nix-2.36.0/bin/nix`
- Plugin: `/nix/store/fyr9a0wa7qarrqxc0035rak8ls7zfb1l-nix-wasm-plugins-0.1.0/nickel_plugin.wasm`

Both functional suites passed 215 tests and skipped nine tests.
The direct ABI controls and optional Nickel controls passed with both binaries.
These results establish this workload improvement, not a general Nix evaluation speedup.

## Scope

This pass considered three mechanisms: attribute traversal, file-read caching, and Wasm instance reuse.
The source analysis used one agent, so those passes are correlated.
Only attribute traversal changed. Validation used two benchmark orders and thirty measured processes.
The acceptance criteria were exact output equality, preserved rejection behavior, and less traversal work without an ABI change.
A cached lambda, a successful build alone, or a skipped Wasm test does not meet those criteria.

The host retains one layer-aware cursor per Wasm instance.
Sequential requests for one set now traverse its attributes once instead of once per name.
A backward request or a different value ID resets the cursor.
The values table retains the owner. The cursor does not extend the Wasm instance lifetime.
The ABI, attribute order, and lazy values remain unchanged.

This change does not cache file contents or reuse live Wasm instances.
Those changes need separate memory and isolation evidence.
No daemon, sandbox, or kernel behavior changes, so direct evaluator tests cover this boundary without a NixOS VM.

## String-context metadata (second pass)

`has_context` and `get_string_context_count` share a one-entry cache of the last successfully parsed context within each Wasm instance.
Previously, both calls decoded every context entry into a temporary `std::set`.
Nickel calls `has_context` for each input string to preserve opaque Nix dependencies.

A cache miss parses the context and preserves its feature-admission checks.
A hit returns the parsed count without new allocations.
The immutable context remains rooted through the instance value table.
A different context, including an empty context, replaces the cache entry.
A failed parse does not update the cache.
Context creation, context copying, feature admission, and the Wasm ABI remain unchanged.

A rejected direct-metadata shortcut bypassed `dynamic-derivations` admission for output names that encode nested references.
`context-dynamic.nix` now covers both rejection and explicit admission of those references.

`context.nix` covers empty contexts, all three reference kinds, multiple outputs, duplicate removal, and round-trip preservation.
It also rejects non-string inputs and malformed context entries.
The malformed-entry test still passes through `make_string_with_context`, which retains its entry validation.

Run the real Nickel context workload with each binary:

```sh
"$NIX_BINARY" eval --json --impure \
  --extra-experimental-features 'nix-command wasm-builtin' \
  --file tests/functional/wasm/nickel-context-bench.nix \
  --apply "f: f { plugin = $NICKEL_PLUGIN; valueCount = $VALUE_COUNT; }"
```

Each input string carries 256 output references by default.
The workload asserts list equality and verifies the first and last output contexts.
It returns the number of input strings.

### Context-cache measurements (2026-09-06)

The final cache candidate passed 215 functional tests, with nine skips, plus the NixOS VM and real Nickel integration controls.
The VM included enabled and disabled `dynamic-derivations` cases.

The 10,000-string workload used 256 references per string, one warmup, and five measured processes per binary.
The baseline mean was 8.086 ± 1.092 seconds. The cache mean was 5.010 ± 0.707 seconds.
A reverse-order repeat measured 5.480 ± 1.992 seconds for the cache and 5.805 ± 0.909 seconds for the baseline.
The uncertainties are sample standard deviations.
These shared-host measurements had substantial variance and do not establish a stable speedup ratio.

- Baseline: `/nix/store/j7d4vg5bady00wp2h022cqq4hwrhvivj-nix-2.36.0/bin/nix`
- Final cache: `/nix/store/6rwk5j1qqk7na4la5m2ka34p734braxa-nix-2.36.0/bin/nix`
- Plugin: `/nix/store/fyr9a0wa7qarrqxc0035rak8ls7zfb1l-nix-wasm-plugins-0.1.0/nickel_plugin.wasm`

Earlier direct-metadata timings do not describe this final implementation.
The cache improves repeated probes of one shared context, not arbitrary alternation between distinct contexts.
It retains one pointer and one count, not a copy of the parsed entries.

### Rejected parallel-compilation candidate

The second pass also evaluated Wasmtime 40.0.2 with `parallel-compilation` enabled.
Its pinned `engine.rs` uses a global Rayon pool when that feature is available.
The C API exposes an on/off switch but no pool-size control.

The candidate passed functional and VM checks, but its resource cost blocked adoption.
The 20,000-attribute Nickel workload changed from 2.238 to 1.712 seconds of mean elapsed time.
Mean system CPU time changed from 0.227 to 10.487 seconds.
A small-module probe changed from 166.4 to 702.6 milliseconds of mean elapsed time.
That probe used the same Nix binary with the candidate library through `LD_LIBRARY_PATH`.
The host exposed 32 CPUs, and the runs had substantial timing variance.
These observations do not establish a stable speedup ratio.

The final package keeps serial compilation.
A future parallel implementation needs a bounded worker pool and small-module controls before default adoption.
File-read caching and live-instance reuse remain outside this change.

### NixOS VM checks

`hydraJobs.tests.wasm` runs the evaluator as an unprivileged user on a two-CPU VM.
It covers module compilation, attribute names, string contexts, and WASI output.
It also checks single-CPU operation, invalid bytecode, invalid host-call inputs, and feature-gate rejection.
`compile.nix` supplies 1,024 independent functions and a module with an invalid result type.

```sh
nix build .#hydraJobs.tests.wasm --no-link -L
nix build .#nix-functional-tests --no-link -L
```
