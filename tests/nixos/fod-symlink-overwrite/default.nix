# Test that a malicious fixed-output derivation cannot overwrite arbitrary
# host files via symlinks placed inside the build chroot.
#
# The vulnerability (GHSA-g3g9-5vj6-r3gj): during FOD output registration,
# the Nix daemon would copy the output using paths inside the build chroot.
# A builder could create a symlink at <output>.tmp pointing to an arbitrary
# host file. When the daemon ran copyFile(actualPath, actualPath + ".tmp"),
# std::filesystem::copy_file would follow that symlink and overwrite the
# target with the builder's output content.
#
# The fix moves the temporary copy into a daemon-owned directory in the
# store (via createTempDirInStore), outside the chroot entirely.

{ config, ... }:

let
  pkgs = config.nodes.machine.nixpkgs.pkgs;
in
{
  name = "fod-symlink-overwrite";

  nodes.machine =
    {
      config,
      lib,
      pkgs,
      ...
    }:
    {
      virtualisation.writableStore = true;
      nix.settings.substituters = lib.mkForce [ ];
      virtualisation.additionalPaths = [
        pkgs.busybox-sandbox-shell
      ];
    };

  testScript =
    { nodes }:
    ''
      start_all()

      # Create a canary file that the attacker tries to overwrite.
      machine.succeed("echo 'original-canary-content' > /tmp/canary")
      machine.succeed("chmod 644 /tmp/canary")

      # Build a FOD whose builder tries the symlink attack:
      #   1. Write legitimate output to $out
      #   2. Create a symlink at $out.tmp -> /tmp/canary
      #
      # Before the fix, the daemon would do:
      #   copyFile(chrootRoot + $out, chrootRoot + $out + ".tmp")
      # which follows the symlink and overwrites /tmp/canary.
      machine.succeed(r"""
        nix-build -E '
          builtins.derivation {
            name = "symlink-attacker";
            system = builtins.currentSystem;
            outputHashMode = "flat";
            outputHashAlgo = "sha256";
            outputHash = builtins.hashString "sha256" "attacker-controlled-content\n";
            builder = "${pkgs.busybox-sandbox-shell}/bin/sh";
            args = [ "-c" "echo attacker-controlled-content > $out; ln -sf /tmp/canary $out.tmp || true" ];
          }' 2>&1
      """.strip())

      # The canary file must still contain its original content.
      # If the vulnerability were present, it would contain "attacker-controlled-content".
      canary = machine.succeed("cat /tmp/canary").strip()
      assert canary == "original-canary-content", \
          f"Canary file was overwritten! Expected 'original-canary-content', got '{canary}'"

      machine.log("Canary file intact — symlink attack was blocked")

      # Also verify the build output is correct
      machine.succeed(r"""
        test "$(cat ./result)" = "attacker-controlled-content"
      """.strip())
    '';
}
