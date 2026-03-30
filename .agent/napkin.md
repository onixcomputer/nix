# Napkin

## Corrections
| Date | Source | What Went Wrong | What To Do Instead |
|------|--------|----------------|-------------------|

## User Preferences
- Working on a CppNix 2.33.3 fork, cherry-picking from DetSys and Lix
- Uses openspec for change management
- Build system: meson, C++23, Nix flake

## Patterns That Work
- `expectStderr N cmd` in test pipelines: exit code must match exactly (nix-build returns 102 for BuildError, not 1)
- For functional test error grepping, capture stderr to a file: `cmd 2>"$TEST_ROOT/err" && fail || true; grepQuiet pattern "$TEST_ROOT/err"`
- `fixed.builder1.sh` requires IMPURE_VAR1/IMPURE_VAR2; use `buildCommand` in mkDerivation for simple FOD tests
- `PosixSourceAccessor::createAtRoot()` returns a SourcePath, not ref<SourceAccessor>; use `make_ref<PosixSourceAccessor>(path, true)` directly
- Template specializations for BaseSetting<T> MUST be in `namespace nix`, not in sub-namespaces like `nix::flake` — compiler rejects cross-namespace specializations
- New enum settings need: parse(), to_string(), trait (appendable=false), convertToArg(), JSON serializer (for `nix config show --json`), AND explicit template instantiation `template class BaseSetting<T>` with abstract-setting-to-json.hh included
- Follow SandboxMode pattern in globals.cc for any tri-state config settings
- Cross-layer config (libutil needs libstore setting): use global override function in libutil, wire it in libstore init (see temp-dir/setTempDirOverride pattern)
- `nix develop` shell is broken (meson patch failure) — use `nix build .#nix-cli` instead for compilation checks
- Config path resolution: use thread-local for cross-cutting concern (config dir → parsePath) rather than threading through class hierarchy. Simpler and doesn't change APIs.
- `parsePath` in configuration.cc is the central path resolver for PathSetting/OptionalPathSetting/BaseSetting<path> — all path settings flow through it
- Strings-typed settings like `secret-key-files` do NOT go through parsePath — per-token path resolution for list settings would need a dedicated PathListSetting type

## Patterns That Don't Work
- lazy-path-inputs via PosixSourceAccessor: `mountInput()` in `eval.cc` mounts the ORIGINAL accessor into storeFS. With PosixAccessor, this means storeFS delegates to the filesystem for mounted paths. When something later calls `lstat()` (throwing) on `flake.lock` through the storeFS, it hits the PosixAccessor which throws FileNotFound. The old store accessor works because the store copy has the same files but the store backend handles missing files in NAR-based paths differently. Fix requires either: (a) modifying `mountInput` to re-mount a store accessor after `fetchToStore`, or (b) making PosixAccessor's `lstat()` return a sentinel instead of throwing when mounted in storeFS. Affects C API test `nix_api_load_flake_with_flags` — flakes with inputs trigger it.
- `input-substitution-before-fetch` (DetSys #380): already implemented in 2.33.3 in `getAccessorUnchecked()` — `isFinal() && getNarHash()` → `store.ensurePath()` before `scheme->getAccessor()`

## Domain Notes
- wasmtime v40.0.2 needs rustc 1.89+; fixed by bumping nixpkgs from nixos-25.05 to nixos-unstable (rustc 1.94)
- DO NOT downgrade wasmtime — v36 has completely different C++ wrapper API (no capi(), no InstancePre, no custom stdout callbacks)
- `builtins.wasm` cherry-pick (23f3ccee2) already included the merged wasm/wasi from DetSys #359 + #370
- Commit d444c28c4 fixed compilation against 2.33.3 API (wasmRealisePath, .fun field, explicit std::vector)
- Zero wasm/wasi tests exist in the repo
- Experimental feature gate: `Xp::WasmBuiltin` / `--extra-experimental-features wasm-builtin`
- `git checkout --` may silently fail on unstaged changes; always use `git checkout HEAD --`
