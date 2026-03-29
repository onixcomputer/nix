{ pkgs ? import <nixpkgs> {} }:

let
  fenix = import (fetchTarball "https://github.com/nix-community/fenix/archive/main.tar.gz") { inherit pkgs; };
  rust = fenix.stable.withComponents [
    "cargo"
    "rustc"
    "rust-std"
  ];
  rustWasm = fenix.stable.withComponents [
    "cargo"
    "rustc"
    "rust-std"
  ] // {
    # This doesn't work directly; we need targets
  };
in

# Simpler approach: use fenix to get a toolchain with wasm targets
let
  toolchain = fenix.combine [
    fenix.stable.cargo
    fenix.stable.rustc
    fenix.targets.wasm32-unknown-unknown.stable.rust-std
    fenix.targets.wasm32-wasip1.stable.rust-std
  ];
in
pkgs.stdenv.mkDerivation {
  name = "nix-wasm-test-fixtures";
  src = ./.;
  nativeBuildInputs = [ toolchain pkgs.lld ];
  buildPhase = ''
    export HOME=$TMPDIR
    cargo build --release --target wasm32-unknown-unknown -p pure-double
    cargo build --release --target wasm32-wasip1 -p wasi-double
    cargo build --release --target wasm32-wasip1 -p wasi-hello
    cargo build --release --target wasm32-wasip1 -p wasi-no-return
  '';
  installPhase = ''
    mkdir -p $out
    cp target/wasm32-unknown-unknown/release/pure_double.wasm $out/
    cp target/wasm32-wasip1/release/wasi_double.wasm $out/
    cp target/wasm32-wasip1/release/wasi_hello.wasm $out/
    cp target/wasm32-wasip1/release/wasi_no_return.wasm $out/
  '';
}
