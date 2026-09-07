# Optional integration controls. Supply the same built onix-wasm plugin used
# by nickel-bench.nix. The functional suite does not fetch external plugins.
{ plugin }:
let
  eval = source: builtins.wasm { path = plugin; function = "evalNickel"; } source;
  apply = source: args:
    builtins.wasm { path = plugin; function = "evalNickelWith"; } { inherit source args; };
  identity = apply "fun args => args";
  drv = derivation { name = "nickel-context"; system = builtins.currentSystem; builder = "/bin/sh"; };
  contextual = "${drv}";
  input = {
    data = { alpha = 1; bravo = 2; } // { bravo = 3; charlie = null; };
    items = [ true false "text" ];
    path = ./.;
    function = x: x;
    package = drv;
    string = contextual;
  };
  result = identity input;
  rejected = builtins.tryEval (eval "1 | String");
  imported = builtins.wasm { path = plugin; function = "evalNickelFile"; } ./nickel.ncl;
in
assert result.data == input.data;
assert result.items == input.items;
assert result.path == input.path;
assert result.function "kept" == "kept";
assert result.package.drvPath == drv.drvPath;
assert builtins.getContext result.string == builtins.getContext contextual;
assert imported == { value = 1; };
assert !rejected.success;
assert eval "{ value = 1 }" == { value = 1; };
assert identity { value = 2; } == { value = 2; };
true
