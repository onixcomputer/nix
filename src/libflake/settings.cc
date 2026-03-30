#include <vector>

#include "nix/flake/settings.hh"
#include "nix/flake/flake-primops.hh"
#include "nix/expr/eval-settings.hh"
#include "nix/expr/eval.hh"
#include "nix/util/config-impl.hh"
#include "nix/util/args.hh"

namespace nix {

template<>
flake::AcceptFlakeConfigMode BaseSetting<flake::AcceptFlakeConfigMode>::parse(const std::string & str) const
{
    using namespace flake;
    if (str == "true")
        return AcceptConfig;
    else if (str == "false")
        return RejectConfig;
    else if (str == "ask")
        return AskConfig;
    else
        throw UsageError("option '%s' has invalid value '%s' (expected 'true', 'false', or 'ask')", name, str);
}

template<>
struct BaseSetting<flake::AcceptFlakeConfigMode>::trait
{
    static constexpr bool appendable = false;
};

template<>
std::string BaseSetting<flake::AcceptFlakeConfigMode>::to_string() const
{
    using namespace flake;
    if (value == AcceptConfig)
        return "true";
    else if (value == RejectConfig)
        return "false";
    else if (value == AskConfig)
        return "ask";
    else
        abort();
}

template<>
void BaseSetting<flake::AcceptFlakeConfigMode>::convertToArg(Args & args, const std::string & category)
{
    using namespace flake;
    args.addFlag({
        .longName = name,
        .description = "Accept all configuration options from flakes.",
        .category = category,
        .handler = {[this]() { override(AcceptConfig); }},
    });
    args.addFlag({
        .longName = "no-" + name,
        .description = "Reject all configuration options from flakes.",
        .category = category,
        .handler = {[this]() { override(RejectConfig); }},
    });
}

} // namespace nix

#include "nix/util/abstract-setting-to-json.hh"
template class nix::BaseSetting<nix::flake::AcceptFlakeConfigMode>;

/* JSON serialisation for the enum so that `nix config show --json` works. */
namespace nlohmann {
template<>
struct adl_serializer<nix::flake::AcceptFlakeConfigMode>
{
    static void to_json(json & j, const nix::flake::AcceptFlakeConfigMode & v)
    {
        using namespace nix::flake;
        j = v == AcceptConfig ? "true" : v == RejectConfig ? "false" : "ask";
    }
    static void from_json(const json & j, nix::flake::AcceptFlakeConfigMode & v)
    {
        using namespace nix::flake;
        auto s = j.get<std::string>();
        v = s == "true" ? AcceptConfig : s == "false" ? RejectConfig : AskConfig;
    }
};
} // namespace nlohmann

namespace nix::flake {

Settings::Settings() {}

void Settings::configureEvalSettings(nix::EvalSettings & evalSettings) const
{
    evalSettings.extraPrimOps.emplace_back(primops::getFlake(*this));
    evalSettings.extraPrimOps.emplace_back(primops::parseFlakeRef);
    evalSettings.extraPrimOps.emplace_back(primops::flakeRefToString);
}

} // namespace nix::flake
