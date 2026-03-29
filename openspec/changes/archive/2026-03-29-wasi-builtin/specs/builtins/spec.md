# builtins.wasm Specification

## Purpose

Defines the behavior of `builtins.wasm`, the unified Nix builtin for calling
WebAssembly modules. Covers both non-WASI (pure computation) and WASI
(system-interface) modes through a single entry point with automatic mode
detection.

## Requirements

### Requirement: Unified Entry Point

The system MUST provide a single `builtins.wasm` function that handles both
pure Wasm and WASI modules. There MUST NOT be a separate `builtins.wasi`
builtin.

`builtins.wasm` MUST accept two arguments:
1. A configuration attribute set
2. The value to pass to the Wasm function

#### Scenario: Non-WASI invocation
- GIVEN a Wasm module that does not import from `wasi_snapshot_preview1`
- WHEN `builtins.wasm { path = ./module.wasm; function = "my_func"; } arg` is evaluated
- THEN the named function is called with the argument and the result is returned

#### Scenario: WASI invocation
- GIVEN a Wasm module that imports from `wasi_snapshot_preview1`
- WHEN `builtins.wasm { path = ./module.wasm; } arg` is evaluated
- THEN the module's `_start` entry point runs and the result from `return_to_nix` is returned

### Requirement: Configuration Attributes

The configuration attribute set MUST support:
- `path` (required): Path to the Wasm module file
- `function` (conditional): Name of the Wasm export to call

The system MUST reject unknown attributes with an error.

#### Scenario: Missing path attribute
- GIVEN a config attrset without `path`
- WHEN `builtins.wasm { function = "f"; } 1` is evaluated
- THEN an error is raised mentioning the missing `path` attribute

#### Scenario: Unknown attribute
- GIVEN a config attrset with `path` and an unknown attribute `foo`
- WHEN `builtins.wasm { path = ./m.wasm; foo = 1; } 1` is evaluated
- THEN an error is raised mentioning the unknown attribute `foo`

### Requirement: WASI Auto-Detection

The system MUST auto-detect WASI mode by inspecting the compiled module's
imports. If any import has module name `wasi_snapshot_preview1`, the module
is treated as WASI.

#### Scenario: Module with wasi_snapshot_preview1 import
- GIVEN a Wasm module that imports `fd_write` from `wasi_snapshot_preview1`
- WHEN loaded by `builtins.wasm`
- THEN WASI mode is activated automatically

#### Scenario: Module without WASI imports
- GIVEN a Wasm module that only imports from `env`
- WHEN loaded by `builtins.wasm`
- THEN non-WASI mode is used

### Requirement: Function Attribute Rules

For non-WASI modules, the `function` attribute MUST be provided. For WASI
modules, the `function` attribute MUST NOT be provided.

#### Scenario: Non-WASI module without function attribute
- GIVEN a non-WASI module
- WHEN `builtins.wasm { path = ./pure.wasm; } 1` is evaluated
- THEN an error is raised about the missing `function` attribute

#### Scenario: WASI module with function attribute
- GIVEN a WASI module
- WHEN `builtins.wasm { path = ./wasi.wasm; function = "f"; } 1` is evaluated
- THEN an error is raised stating `function` is not allowed for WASI modules

### Requirement: Non-WASI Calling Convention

For non-WASI modules, the system MUST:
1. Call the module's `nix_wasm_init_v1()` export once per instantiation
2. Call the function named by the `function` attribute with a single `ValueId` argument
3. Expect a single `i32` return value representing the result `ValueId`

#### Scenario: Non-WASI function returns wrong number of values
- GIVEN a non-WASI module whose function returns zero values
- WHEN the function is called
- THEN an error is raised stating the function did not return exactly one value

### Requirement: WASI Calling Convention

For WASI modules, the system MUST:
1. Configure a WASI environment (stdout/stderr capture, argv)
2. Set `argv[0]` to `"wasi"` and `argv[1]` to the decimal string of the input `ValueId`
3. Run the `_start` export
4. Collect the result from the `return_to_nix` host function call

#### Scenario: WASI module finishes without calling return_to_nix
- GIVEN a WASI module whose `_start` returns without calling `return_to_nix`
- WHEN executed
- THEN an error is raised stating the function finished without returning a value

### Requirement: WASI I/O Capture

Standard output and standard error from WASI modules MUST be captured and
emitted as Nix warnings, split by newline. Partial lines (no trailing newline)
MUST be flushed when the module exits.

#### Scenario: WASI module writes to stdout
- GIVEN a WASI module that writes "hello\nworld\n" to stdout
- WHEN executed
- THEN two Nix warnings are emitted: "hello" and "world"

### Requirement: Module Caching

Compiled modules MUST be cached by source path. Loading the same `.wasm` file
multiple times MUST reuse the compiled `InstancePre` rather than recompiling.

### Requirement: Experimental Feature Gate

`builtins.wasm` MUST be gated behind the `wasm-builtin` experimental feature.
Attempting to use it without `--extra-experimental-features wasm-builtin`
MUST produce an error.

### Requirement: Host Function Interface

The system MUST provide host functions imported from the `env` module covering:
- Type inspection: `get_type`
- Value construction: `make_int`, `make_float`, `make_string`, `make_bool`, `make_null`, `make_list`, `make_attrset`, `make_path`
- Value extraction: `get_int`, `get_float`, `get_bool`, `copy_string`, `copy_path`, `copy_list`, `copy_attrset`, `copy_attrname`, `get_attr`
- Function calls: `call_function`, `make_app`
- File I/O: `read_file`
- Diagnostics: `panic`, `warn`
- WASI-only: `return_to_nix`
