{ lib, ... }:
let
  vmCores = 2;
  vmMemoryMiB = 2048;
  firstCpu = 0;
  fixtures = ../functional/wasm;
in
{
  name = "wasm";

  nodes.machine = {
    virtualisation.cores = vmCores;
    virtualisation.memorySize = vmMemoryMiB;
    users.users.evaluator.isNormalUser = true;
    nix.settings = {
      experimental-features = [
        "nix-command"
        "wasm-builtin"
      ];
      substituters = lib.mkForce [ ];
    };
  };

  testScript = ''
    machine.wait_for_unit("multi-user.target")
    eval_command = "nix eval --json --impure --file"
    user_command = "su - evaluator -c"

    with subtest("unprivileged compilation and host calls"):
        for fixture in ["compile.nix", "attrname.nix", "context.nix"]:
            command = f"{eval_command} ${fixtures}/{fixture}"
            output = machine.succeed(f"{user_command} '{command}'")
            assert output.strip() == "true", output

    with subtest("context feature admission"):
        command = eval_command + " ${fixtures}/context-dynamic.nix --apply 'r: r.denied'"
        assert machine.succeed(command).strip() == "true"
        command = eval_command + " ${fixtures}/context-dynamic.nix --extra-experimental-features dynamic-derivations --apply 'r: r.allowed'"
        assert machine.succeed(command).strip() == "true"

    with subtest("single-CPU compilation"):
        command = "taskset -c ${toString firstCpu} " + eval_command + " ${fixtures}/compile.nix"
        output = machine.succeed(f"{user_command} '{command}'")
        assert output.strip() == "true", output

    with subtest("WASI output and return value"):
        expression = 'builtins.wasm { path = ${fixtures}/wasi_hello.wasm; } 0'
        command = f'nix eval --json --impure --expr "{expression}"'
        output = machine.succeed(f"{user_command} '{command}' 2>&1")
        assert "hello from stdout" in output, output
        assert "hello from stderr" in output, output
        assert '"done"' in output, output

    with subtest("experimental feature remains required"):
        command = "nix eval --experimental-features nix-command --impure --file ${fixtures}/compile.nix"
        output = machine.fail(f"{user_command} '{command}' 2>&1")
        assert "attribute 'wasm' missing" in output, output
  '';
}
