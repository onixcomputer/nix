#!/usr/bin/env bash

source common.sh

# Isolate the home for this test.
# Other tests (e.g. flake registry tests) could be writing to $HOME in parallel.
export HOME=$TEST_ROOT/userhome

# Test that using XDG_CONFIG_HOME works
# Assert the config folder didn't exist initially.
[ ! -e "$HOME/.config" ]
# Without XDG_CONFIG_HOME, creates $HOME/.config
unset XDG_CONFIG_HOME
# Run against the nix registry to create the config dir
# (Tip: this relies on removing non-existent entries being a no-op!)
nix registry remove userhome-without-xdg
# Verifies it created it
[ -e "$HOME/.config" ]
# Remove the directory it created
rm -rf "$HOME/.config"
# Run the same test, but with XDG_CONFIG_HOME
export XDG_CONFIG_HOME=$TEST_ROOT/confighome
# Assert the XDG_CONFIG_HOME/nix path does not exist yet.
[ ! -e "$TEST_ROOT/confighome/nix" ]
nix registry remove userhome-with-xdg
# Verifies the confighome path has been created
[ -e "$TEST_ROOT/confighome/nix" ]
# Assert the .config folder hasn't been created.
[ ! -e "$HOME/.config" ]

TODO_NixOS # Very specific test setup not compatible with the NixOS test environment?

# Test that files are loaded from XDG by default
export XDG_CONFIG_HOME=$TEST_ROOT/confighome
export XDG_CONFIG_DIRS=$TEST_ROOT/dir1:$TEST_ROOT/dir2
files=$(nix-build --verbose --version | grep "User config" | cut -d ':' -f2- | xargs)
[[ $files == "$TEST_ROOT/confighome/nix/nix.conf:$TEST_ROOT/dir1/nix/nix.conf:$TEST_ROOT/dir2/nix/nix.conf" ]]

# Test that setting NIX_USER_CONF_FILES overrides all the default user config files
export NIX_USER_CONF_FILES=$TEST_ROOT/file1.conf:$TEST_ROOT/file2.conf
files=$(nix-build --verbose --version | grep "User config" | cut -d ':' -f2- | xargs)
[[ $files == "$TEST_ROOT/file1.conf:$TEST_ROOT/file2.conf" ]]

# Test that it's possible to load the config from a custom location
here=$(readlink -f "$(dirname "${BASH_SOURCE[0]}")")
export NIX_USER_CONF_FILES=$here/config/nix-with-substituters.conf
var=$(nix config show | grep '^substituters =' | cut -d '=' -f 2 | xargs)
[[ $var == https://example.com ]]

# Test that we can include a file.
export NIX_USER_CONF_FILES=$here/config/nix-with-include.conf
var=$(nix config show | grep '^allowed-uris =' | cut -d '=' -f 2 | xargs)
[[ $var == https://github.com/NixOS/nix ]]

# Test that we can !include a file.
export NIX_USER_CONF_FILES=$here/config/nix-with-bang-include.conf
var=$(nix config show | grep '^experimental-features =' | cut -d '=' -f 2 | xargs)
[[ $var == nix-command ]]

# Test that it's possible to load config from the environment
prev=$(nix config show | grep '^cores' | cut -d '=' -f 2 | xargs)
export NIX_CONFIG="cores = 4242"$'\n'"experimental-features = nix-command flakes"
exp_cores=$(nix config show | grep '^cores' | cut -d '=' -f 2 | xargs)
exp_features=$(nix config show | grep '^experimental-features' | cut -d '=' -f 2 | xargs)
[[ $prev != "$exp_cores" ]]
[[ $exp_cores == "4242" ]]
# flakes implies fetch-tree
[[ $exp_features == "fetch-tree flakes nix-command" ]]

# Test that it's possible to retrieve a single setting's value
val=$(nix config show | grep '^warn-dirty' | cut -d '=' -f  2 | xargs)
val2=$(nix config show warn-dirty)
[[ $val == "$val2" ]]

# Test unit prefixes.
[[ $(nix config show --min-free 64K min-free) = 65536 ]]
[[ $(nix config show --min-free 1M min-free) = 1048576 ]]
[[ $(nix config show --min-free 2G min-free) = 2147483648 ]]

# Test that relative paths in config files resolve against the config file dir.
export NIX_USER_CONF_FILES=$here/config/nix-with-relative-path.conf
var=$(nix config show diff-hook)
# The relative path "hooks/my-diff-hook" should resolve to an absolute path
# rooted in the config directory.
[[ $var == */config/hooks/my-diff-hook ]]

# Test that tilde paths in include directives expand to $HOME.
mkdir -p "$HOME"
echo "allowed-uris = https://tilde-test.example.com" > "$HOME/extra.conf"
export NIX_USER_CONF_FILES=$here/config/nix-with-tilde-include.conf
var=$(nix config show | grep '^allowed-uris =' | cut -d '=' -f 2 | xargs)
[[ $var == https://tilde-test.example.com ]]

# Test that $NIX_CONFIG resolves relative paths against cwd (no config file dir).
export NIX_USER_CONF_FILES=
var=$(NIX_CONFIG="diff-hook = relative/path" nix config show diff-hook)
[[ $var == "$(pwd)/relative/path" ]]
