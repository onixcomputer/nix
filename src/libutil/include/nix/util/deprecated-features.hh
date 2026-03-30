#pragma once
///@file

#include "nix/util/error.hh"
#include "nix/util/types.hh"
#include "nix/util/json-non-null.hh"

#include <nlohmann/json_fwd.hpp>

namespace nix {

/**
 * The list of deprecated language features.
 *
 * Deprecated features emit warnings by default. Users can suppress
 * warnings for specific features with `--extra-deprecated-features`.
 */
enum struct DeprecatedFeature {
    UrlLiterals,
    RecSetOverrides,
    AncientLet,
    ShadowInternalSymbols,
};

using Dp = DeprecatedFeature;

const std::optional<DeprecatedFeature> parseDeprecatedFeature(const std::string_view & name);

std::string_view showDeprecatedFeature(const DeprecatedFeature);

nlohmann::json documentDeprecatedFeatures();

std::ostream & operator<<(std::ostream & str, const DeprecatedFeature & feature);

std::set<DeprecatedFeature> parseDeprecatedFeatures(const StringSet &);

/**
 * A deprecated feature was used but warnings are not suppressed.
 */
class UsedDeprecatedFeature : public Error
{
public:
    DeprecatedFeature feature;
    UsedDeprecatedFeature(DeprecatedFeature feature, std::string msg);
};

template<>
struct json_avoids_null<DeprecatedFeature> : std::true_type
{};

void to_json(nlohmann::json &, const DeprecatedFeature &);
void from_json(const nlohmann::json &, DeprecatedFeature &);

} // namespace nix
