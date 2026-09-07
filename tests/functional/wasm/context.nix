let
  call =
    function: value:
    builtins.wasm {
      inherit function;
      wat = builtins.readFile ./context.wat;
    } value;
  drv = derivation {
    name = "wasm-context-metadata";
    system = builtins.currentSystem;
    builder = "/bin/sh";
  };
  drvPath = builtins.unsafeDiscardStringContext drv.drvPath;
  outputNames = [
    "out"
    "dev"
  ];
  # One opaque reference, one deep reference, and two output references.
  context = {
    ${drvPath} = {
      path = true;
      allOutputs = true;
      outputs = outputNames ++ outputNames;
    };
  };
  contextual = builtins.appendContext "payload" context;
  referenceKindsWithoutOutputs = 2;
  expectedCount = referenceKindsWithoutOutputs + builtins.length outputNames;
  roundtrip = builtins.wasm {
    path = ./string_context.wasm;
    function = "passthrough_string";
  } contextual;
  rejects = value: !(builtins.tryEval value).success;
  wrongTypes = [
    false
    1
    null
    [ ]
    { }
    ./.
  ];
in
assert call "has" "" == 0;
assert call "count" "plain" == 0;
assert call "has" contextual == 1;
assert call "count" contextual == expectedCount;
assert call "count" (builtins.appendContext contextual context) == expectedCount;
assert builtins.getContext roundtrip == builtins.getContext contextual;
assert
  call "counts" [
    "plain"
    contextual
    contextual
    "${drv}"
    contextual
    ""
  ] == [
    0
    expectedCount
    expectedCount
    1
    expectedCount
    0
  ];
assert rejects (
  call "counts" [
    contextual
    false
  ]
);
assert builtins.all (value: rejects (call "has" value)) wrongTypes;
assert builtins.all (value: rejects (call "count" value)) wrongTypes;
assert rejects (call "invalid" null);
true
