---
synopsis: "Radicle repository fetching support for flakes"
prs: []
issues: []
---

Nix now supports fetching Radicle repositories as flake inputs via the new `rad:` URL scheme. This feature is gated behind the experimental `radicle` feature flag.

Radicle is a peer-to-peer code collaboration protocol. With this support, you can use Radicle repositories directly in your flake inputs:

```nix
{
  inputs = {
    # Simple Radicle ID
    example.url = "rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5";

    # With a specific branch
    example-main.url = "rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5/main";

    # With explicit seed node
    example-node.url = "rad://seed.example.com/z3gqcJUoA1n9HaHKufZs5FCSGazv5";
  };
}
```

The fetcher uses the `rad` CLI tool for cloning and updating repositories, and requires it to be available in your PATH. Authentication is handled through Radicle's native authentication system.

To enable this feature:

```bash
nix --extra-experimental-features radicle flake update
```

Or add to your `nix.conf`:

```
experimental-features = radicle flakes nix-command
```
