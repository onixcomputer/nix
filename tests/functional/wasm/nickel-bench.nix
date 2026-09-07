# Supply a built onix-wasm Nickel plugin. Compare the same plugin and workload
# with both Nix binaries. The result assertion also verifies name/value pairing.
{ plugin, attributeCount ? 10000 }:
let
  attrs = builtins.listToAttrs (builtins.genList (index: {
    name = "field${toString index}";
    value = index;
  }) attributeCount) // { field0 = -1; };
  result = builtins.wasm {
    path = plugin;
    function = "evalNickelWith";
  } { source = "fun args => args"; args = attrs; };
in
assert attributeCount > 0;
assert result == attrs;
builtins.length (builtins.attrNames result)
