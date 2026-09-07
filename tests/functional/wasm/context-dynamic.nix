# Output names can encode nested references. Metadata probes must retain the
# parser's dynamic-derivations admission check, even for an existing context.
let
  drv = derivation {
    name = "wasm-context-dynamic";
    system = builtins.currentSystem;
    builder = "/bin/sh";
  };
  drvPath = builtins.unsafeDiscardStringContext drv.drvPath;
  contextual = builtins.appendContext "payload" {
    ${drvPath}.outputs = [ "out!nested" ];
  };
  call =
    function:
    builtins.wasm {
      inherit function;
      wat = builtins.readFile ./context.wat;
    } contextual;
in
{
  denied =
    assert !(builtins.tryEval (call "has")).success;
    assert !(builtins.tryEval (call "count")).success;
    true;
  allowed =
    assert call "has" == 1;
    assert call "count" == 1;
    true;
}
