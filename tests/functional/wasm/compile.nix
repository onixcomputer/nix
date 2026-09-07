# Many independent functions exercise module compilation without external I/O.
let
  functionCount = 1024;
  expected = 42;
  functions = builtins.genList (index: ''
    (func (export "f${toString index}") (param i32) (result i32) local.get 0)
  '') functionCount;
  wat = ''
    (module
      (memory (export "memory") 1)
      (func (export "nix_wasm_init_v1"))
      ${builtins.concatStringsSep "\n" functions})
  '';
  call =
    index:
    builtins.wasm {
      inherit wat;
      function = "f${toString index}";
    } expected;
  invalid = builtins.tryEval (
    builtins.wasm {
      wat = "(module (func (result i32) f64.const 0))";
      function = "f0";
    } expected
  );
in
assert call 0 == expected;
assert call (functionCount - 1) == expected;
assert !invalid.success;
true
