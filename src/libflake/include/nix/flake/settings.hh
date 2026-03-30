#pragma once
///@file

#include <sys/types.h>
#include <string>

#include "nix/util/configuration.hh"

namespace nix {
// Forward declarations
struct EvalSettings;

} // namespace nix

namespace nix::flake {

/**
 * Tri-state for `accept-flake-config`:
 * - `AcceptConfig`: silently accept all flake config options
 * - `RejectConfig`: silently reject all flake config options
 * - `AskConfig`: prompt the user for each option (default)
 */
enum AcceptFlakeConfigMode { AcceptConfig, RejectConfig, AskConfig };

} // namespace nix::flake

namespace nix {

template<>
flake::AcceptFlakeConfigMode BaseSetting<flake::AcceptFlakeConfigMode>::parse(const std::string & str) const;
template<>
std::string BaseSetting<flake::AcceptFlakeConfigMode>::to_string() const;

} // namespace nix

namespace nix::flake {

struct Settings : public Config
{
    Settings();

    void configureEvalSettings(nix::EvalSettings & evalSettings) const;

    Setting<bool> useRegistries{
        this,
        true,
        "use-registries",
        "Whether to use flake registries to resolve flake references.",
        {},
        true,
        Xp::Flakes};

    Setting<AcceptFlakeConfigMode> acceptFlakeConfig{
        this,
        AskConfig,
        "accept-flake-config",
        R"(
          Whether to accept Nix configuration settings from a flake
          without prompting.

          - `true`: accept all flake config options silently
          - `false`: reject all flake config options silently
          - `ask` (default): prompt interactively for each option
        )",
        {},
        true,
        Xp::Flakes};

    Setting<std::string> commitLockFileSummary{
        this,
        "",
        "commit-lock-file-summary",
        R"(
          The commit summary to use when committing changed flake lock files. If
          empty, the summary is generated based on the action performed.
        )",
        {"commit-lockfile-summary"},
        true,
        Xp::Flakes};
};

} // namespace nix::flake
