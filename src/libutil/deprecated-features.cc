#include "nix/util/deprecated-features.hh"
#include "nix/util/fmt.hh"
#include "nix/util/strings.hh"
#include "nix/util/util.hh"

#include <nlohmann/json.hpp>

namespace nix {

struct DeprecatedFeatureDetails
{
    DeprecatedFeature tag;
    std::string_view name;
    std::string_view description;
};

constexpr std::array<DeprecatedFeatureDetails, 4> deprecatedFeatureDetails = {{
    {
        .tag = Dp::UrlLiterals,
        .name = "url-literals",
        .description = R"(
            Unquoted URL literals like `https://example.com` in Nix expressions.
            Use quoted strings instead: `"https://example.com"`.
        )",
    },
    {
        .tag = Dp::RecSetOverrides,
        .name = "rec-set-overrides",
        .description = R"(
            The `__overrides` attribute in recursive attribute sets.
            This feature has been unused for over a decade.
        )",
    },
    {
        .tag = Dp::AncientLet,
        .name = "ancient-let",
        .description = R"(
            The `let { body = ...; }` syntax. Use `let ... in ...` instead.
        )",
    },
    {
        .tag = Dp::ShadowInternalSymbols,
        .name = "shadow-internal-symbols",
        .description = R"(
            Shadowing internal arithmetic symbols (`__sub`, `__mul`, `__div`,
            `__lessThan`) in `with` expressions or let bindings.
        )",
    },
}};

static_assert(
    deprecatedFeatureDetails.size() == static_cast<size_t>(Dp::ShadowInternalSymbols) + 1,
    "Update deprecatedFeatureDetails when adding a deprecated feature");

const std::optional<DeprecatedFeature> parseDeprecatedFeature(const std::string_view & name)
{
    for (auto & [tag, n, _] : deprecatedFeatureDetails)
        if (n == name)
            return tag;
    return std::nullopt;
}

std::string_view showDeprecatedFeature(const DeprecatedFeature feature)
{
    assert(static_cast<size_t>(feature) < deprecatedFeatureDetails.size());
    return deprecatedFeatureDetails[static_cast<size_t>(feature)].name;
}

nlohmann::json documentDeprecatedFeatures()
{
    StringMap result;
    for (auto & [tag, name, description] : deprecatedFeatureDetails)
        result[std::string{name}] = trim(std::string{description});
    return result;
}

std::ostream & operator<<(std::ostream & str, const DeprecatedFeature & feature)
{
    return str << showDeprecatedFeature(feature);
}

std::set<DeprecatedFeature> parseDeprecatedFeatures(const StringSet & raw)
{
    std::set<DeprecatedFeature> result;
    for (auto & s : raw) {
        if (auto f = parseDeprecatedFeature(s))
            result.insert(*f);
        else
            warn("unknown deprecated feature '%s'", s);
    }
    return result;
}

void to_json(nlohmann::json & j, const DeprecatedFeature & feature)
{
    j = showDeprecatedFeature(feature);
}

void from_json(const nlohmann::json & j, DeprecatedFeature & feature)
{
    if (auto parsed = parseDeprecatedFeature(j.get<std::string>()))
        feature = *parsed;
    else
        throw Error("unknown deprecated feature '%s'", j.get<std::string>());
}

UsedDeprecatedFeature::UsedDeprecatedFeature(DeprecatedFeature feature, std::string msg)
    : Error(msg)
    , feature(feature)
{
}

} // namespace nix
