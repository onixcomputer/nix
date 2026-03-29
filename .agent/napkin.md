# Napkin

## Corrections
| Date | Source | What Went Wrong | What To Do Instead |
|------|--------|----------------|-------------------|

## User Preferences
- Working on a CppNix 2.33.3 fork, cherry-picking from DetSys and Lix
- Uses openspec for change management
- Build system: meson, C++23, Nix flake

## Patterns That Work
- (accumulate here as you learn them)

## Patterns That Don't Work
- (accumulate here as approaches fail and why)

## Domain Notes
- wasmtime v40.0.2 needs rustc 1.89+; fixed by bumping nixpkgs from nixos-25.05 to nixos-unstable (rustc 1.94)
- DO NOT downgrade wasmtime — v36 has completely different C++ wrapper API (no capi(), no InstancePre, no custom stdout callbacks)
- `builtins.wasm` cherry-pick (23f3ccee2) already included the merged wasm/wasi from DetSys #359 + #370
- Commit d444c28c4 fixed compilation against 2.33.3 API (wasmRealisePath, .fun field, explicit std::vector)
- Zero wasm/wasi tests exist in the repo
- Experimental feature gate: `Xp::WasmBuiltin` / `--extra-experimental-features wasm-builtin`
- `git checkout --` may silently fail on unstaged changes; always use `git checkout HEAD --`
