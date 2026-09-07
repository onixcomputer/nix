# Exercise the attribute-name ABI without forcing attribute values.
let
  copy = attrs: index: length:
    builtins.wasm {
      function = "copy";
      wat = ''
        (module
          (import "env" "copy_attrname" (func $copy (param i32 i32 i32 i32)))
          (import "env" "make_string" (func $string (param i32 i32) (result i32)))
          (memory (export "memory") 1)
          (func (export "nix_wasm_init_v1"))
          (func (export "copy") (param $attrs i32) (result i32)
            (call $copy (local.get $attrs) (i32.const ${toString index})
              (i32.const 0) (i32.const ${toString length}))
            (call $string (i32.const 0) (i32.const ${toString length}))))
      '';
    } attrs;
  roundtrip = attrs:
    builtins.wasm {
      function = "roundtrip";
      wat = builtins.readFile ./attrname.wat;
    } attrs;
  flat = { alpha = 1; bravo = 2; delta = 3; };
  layered = flat // { bravo = 4; echo = 5; };
  interleaved = builtins.wasm {
    function = "interleave";
    wat = builtins.readFile ./attrname.wat;
  } { left = flat; right = { hotel = 6; india = 7; julia = 8; }; };
  rejects = value: !(builtins.tryEval value).success;
in
assert interleaved == flat;
assert roundtrip flat == flat;
assert roundtrip layered == layered;
assert roundtrip {} == {};
assert roundtrip { "" = 0; "é" = 1; } == { "" = 0; "é" = 1; };
assert (roundtrip { alpha = throw "attribute value was forced"; }) ? alpha;
assert rejects (copy flat 3 5);
assert rejects (copy flat (-1) 5);
assert rejects (copy false 0 0);
assert rejects (copy {} 0 0);
assert rejects (copy { alpha = 1; } 0 1);
true
