{ nixpkgs, ... }:

{
  name = "cgroups";

  nodes = {
    host =
      { config, pkgs, ... }:
      {
        virtualisation.additionalPaths = [ pkgs.stdenvNoCC ];
        nix.extraOptions = ''
          extra-experimental-features = nix-command auto-allocate-uids cgroups
          extra-system-features = uid-range
        '';
        nix.settings.use-cgroups = true;
        nix.nixPath = [ "nixpkgs=${nixpkgs}" ];
      };
  };

  testScript =
    { nodes }:
    ''
      start_all()

      host.wait_for_unit("multi-user.target")

      # Start build in background
      host.execute("NIX_REMOTE=daemon nix build --auto-allocate-uids --file ${./hang.nix} >&2 &")
      service = "/sys/fs/cgroup/system.slice/nix-daemon.service"

      # Wait for cgroups to be created
      host.succeed(f"until [ -e {service}/nix-daemon ]; do sleep 1; done", timeout=30)
      host.succeed(f"until [ -e {service}/nix-build-uid-* ]; do sleep 1; done", timeout=30)

      # Check that there aren't processes where there shouldn't be, and that there are where there should be
      host.succeed(f'[ -z "$(cat {service}/cgroup.procs)" ]')
      host.succeed(f'[ -n "$(cat {service}/nix-daemon/cgroup.procs)" ]')
      host.succeed(f'[ -n "$(cat {service}/nix-build-uid-*/cgroup.procs)" ]')

      daemon_pids = host.succeed(f"cat {service}/nix-daemon/cgroup.procs").split()

      # Restarting the service must not leave daemon workers or build-user
      # cgroups alive. Stale workers can keep store locks around and block GC.
      host.succeed("systemctl restart nix-daemon")
      host.wait_until_succeeds(f'[ -z "$(cat {service}/cgroup.procs)" ]', timeout=30)
      host.wait_until_succeeds(f'[ -n "$(cat {service}/nix-daemon/cgroup.procs)" ]', timeout=30)
      for pid in daemon_pids:
          host.wait_until_succeeds(f'! kill -0 {pid} 2>/dev/null', timeout=30)
      host.wait_until_succeeds(
          f'for f in {service}/nix-build-uid-*/cgroup.procs; do [ ! -e "$f" ] || [ -z "$(cat "$f")" ]; done',
          timeout=30,
      )
      host.succeed("timeout 10 nix-store --gc --max-freed 0")
    '';

}
