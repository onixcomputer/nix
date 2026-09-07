# Measure context-presence probes during real Nickel input conversion.
{
  plugin,
  valueCount ? 10000,
  contextCount ? 256,
}:
let
  drv = derivation {
    name = "nickel-context-bench";
    system = builtins.currentSystem;
    builder = "/bin/sh";
  };
  drvPath = builtins.unsafeDiscardStringContext drv.drvPath;
  context = {
    ${drvPath}.outputs = builtins.genList (index: "output${toString index}") contextCount;
  };
  contextual = builtins.appendContext "payload" context;
  expectedContext = builtins.getContext contextual;
  items = builtins.genList (_: contextual) valueCount;
  result =
    builtins.wasm
      {
        path = plugin;
        function = "evalNickelWith";
      }
      {
        source = "fun args => args";
        args = { inherit items; };
      };
in
assert valueCount > 0 && contextCount > 0;
assert result.items == items;
assert builtins.getContext (builtins.head result.items) == expectedContext;
assert builtins.getContext (builtins.elemAt result.items (valueCount - 1)) == expectedContext;
builtins.length result.items
