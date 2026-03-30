#include "nix/util/error.hh"
#include "nix/fetchers/fetchers.hh"
#include "nix/fetchers/cache.hh"
#include "nix/store/globals.hh"
#include "nix/store/store-api.hh"
#include "nix/util/url-parts.hh"
#include "nix/util/processes.hh"
#include "nix/fetchers/git-utils.hh"
#include "nix/util/logging.hh"
#include "nix/fetchers/fetch-settings.hh"
#include "nix/store/pathlocks.hh"
#include "nix/util/finally.hh"
#include "nix/util/file-system.hh"

#include <regex>
#include <sys/time.h>
#include <limits>
#include <chrono>

using namespace std::string_literals;

namespace nix::fetchers {

namespace {

constexpr size_t MAX_DOMAIN_NAME_LENGTH = 253;  // Full FQDN, not single label (which is 63)
constexpr size_t MAX_RADICLE_ID_LENGTH = 100;
constexpr std::string_view REFS_HEADS_PREFIX = "refs/heads/";

// Create RunOptions for rad commands with PATH lookup enabled
static RunOptions radOptions(const Strings & args)
{
    return {
        .program = "rad",
        .lookupPath = true,  // Let Nix find 'rad' in PATH (the NixOS way)
        .args = args
    };
}

// Validate node identifier to prevent command injection
bool isValidNodeIdentifier(std::string_view node)
{
    if (node.empty() || node.length() > MAX_DOMAIN_NAME_LENGTH)
        return false;
    // Node IDs can be domain names or Radicle peer IDs (alphanumeric with dots/hyphens)
    static const std::regex nodeRegex(R"(^[a-zA-Z0-9]([a-zA-Z0-9\-\.]*[a-zA-Z0-9])?$)");
    return std::regex_match(node.begin(), node.end(), nodeRegex);
}

// Parse a Radicle ID (rad:z...) from a string
std::optional<std::string> parseRadicleId(std::string_view str)
{
    if (str.empty())
        return std::nullopt;
    // Radicle IDs start with "rad:z" followed by base-58 style alphanumeric chars
    // Limit length to MAX_RADICLE_ID_LENGTH to prevent ReDoS
    std::string ridPattern = "rad:z[A-Za-z0-9]{1," + std::to_string(MAX_RADICLE_ID_LENGTH - 5) + "}";
    static const std::regex ridRegex(ridPattern);
    std::match_results<std::string_view::const_iterator> match;
    if (std::regex_search(str.begin(), str.end(), match, ridRegex)) {
        return std::string(match[0].first, match[0].second);
    }
    return std::nullopt;
}

// Validate that a string is a valid Radicle ID
bool isValidRadicleId(std::string_view rid)
{
    auto parsed = parseRadicleId(rid);
    return parsed && *parsed == rid;
}

bool isCacheFileWithinTtl(time_t now, const struct stat & st)
{
    time_t ttl = static_cast<time_t>(settings.tarballTtl);
    // Check for overflow before adding
    if (st.st_mtime > std::numeric_limits<time_t>::max() - ttl)
        return true;  // Treat overflow as "within TTL" (cache is valid)
    return st.st_mtime + ttl > now;
}

Path getCachePath(std::string_view rid, const std::optional<std::string> & node)
{
    std::string key(rid);
    if (node)
        key += "@" + *node;
    return getCacheDir() + "/radicle/" + hashString(HashAlgorithm::SHA256, key).to_string(HashFormat::Nix32, false);
}

// Get the default branch from the git repository
std::optional<std::string> getDefaultBranch(const Path & repoPath)
{
    try {
        auto [status, output] = runProgram(RunOptions{
            .program = "git",
            .lookupPath = true,
            .args = {"-C", repoPath, "symbolic-ref", "HEAD"},
        });

        if (status != 0)
            return std::nullopt;

        std::string ref = chomp(output);
        // Remove refs/heads/ prefix
        if (ref.starts_with(REFS_HEADS_PREFIX))
            ref = ref.substr(REFS_HEADS_PREFIX.length());

        // Validate ref before returning - don't trust git output
        if (ref.empty() || !isLegalRefName(ref))
            return std::nullopt;

        return ref;
    } catch (ExecError &) {
        return std::nullopt;
    }
}

// Clone a Radicle repository using rad CLI
void cloneRadicleRepo(const std::string & rid, const std::optional<std::string> & node, const Path & destDir)
{
    Strings args = {"clone", rid};

    // Add seed node if specified
    if (node) {
        args.push_back("--seed");
        args.push_back(*node);
    }

    // Set output directory (positional argument)
    args.push_back(destDir);

    auto [status, output] = runProgram(radOptions(args));
    if (status != 0) {
        throw Error("rad clone failed with exit code %d. Output: %s", status, output);
    }
}

// Fetch updates for a Radicle repository using git
bool fetchRadicleRepo(const Path & repoPath, const std::string & rid)
{
    try {
        auto [status, output] = runProgram(RunOptions{
            .program = "git",
            .lookupPath = true,
            .args = {"-C", repoPath, "fetch", "--all", "--tags"},
        });
        if (status != 0) {
            warn("failed to fetch updates for Radicle repository '%s': %s", rid, output);
            return false;
        }
        return true;
    } catch (ExecError & e) {
        warn("failed to fetch updates for Radicle repository '%s': %s", rid, e.what());
        return false;
    }
}

struct RadicleRepoInfo
{
    std::string rid;
    std::optional<std::string> node;
    Path repoPath;
};

// Template helper function to handle cache lookup, error recovery, computation, and upsert
template<typename T>
T getCachedAttribute(
    const Settings & settings,
    const std::string & cacheType,
    const Hash & rev,
    const std::string & attrName,
    std::function<T(void)> computeValue)
{
    auto cache = settings.getCache();
    Cache::Key cacheKey{cacheType, {{"rev", rev.gitRev()}}};

    if (auto res = cache->lookup(cacheKey)) {
        try {
            return getIntAttr(*res, attrName);
        } catch (Error & e) {
            warn("corrupted cache entry for '%s', recomputing: %s", cacheType, e.what());
            auto value = computeValue();
            Attrs cacheVal;
            cacheVal.insert_or_assign(attrName, value);
            cache->upsert(cacheKey, cacheVal);
            return value;
        }
    } else {
        auto value = computeValue();
        Attrs cacheVal;
        cacheVal.insert_or_assign(attrName, value);
        cache->upsert(cacheKey, cacheVal);
        return value;
    }
}

} // anonymous namespace

struct RadicleInputScheme : InputScheme
{
    std::optional<ExperimentalFeature> experimentalFeature() const override
    {
        return Xp::Radicle;
    }

    std::string_view schemeName() const override
    {
        return "rad";
    }

    std::string schemeDescription() const override
    {
        return "Radicle repositories";
    }

    const std::map<std::string, AttributeInfo> & allowedAttrs() const override
    {
        static const std::map<std::string, AttributeInfo> attrs = {
            {"rid", {.required = true}},
            {"url", {.required = false}},
            {"ref", {.required = false}},
            {"rev", {.required = false}},
            {"node", {.required = false}},
            {"lastModified", {.type = "Integer", .required = false}},
            {"revCount", {.type = "Integer", .required = false}},
            {"narHash", {.required = false}},
            {"name", {.required = false}},
        };
        return attrs;
    }

    std::optional<Input> inputFromURL(const Settings & settings, const ParsedURL & url, bool requireTree) const override
    {
        if (url.scheme != "rad")
            return {};

        // Parse path segments: ["z3gqc..."] or ["z3gqc...", "ref"]
        // or with authority: ["", "z3gqc..."] or ["", "z3gqc...", "ref"]

        if (url.path.empty())
            throw BadURL("invalid Radicle URL '%s': missing repository ID", url.to_string());

        size_t pathIndex = 0;

        // Skip empty first element if we have an authority
        if (url.authority && !url.path.empty() && url.path[0].empty()) {
            pathIndex = 1;
        }

        // Get RID from first real path segment
        if (pathIndex >= url.path.size() || url.path[pathIndex].empty() || !url.path[pathIndex].starts_with("z"))
            throw BadURL("invalid Radicle URL '%s': expected format 'rad:z...' or 'rad:z.../ref'", url.to_string());

        std::string ridPart = url.path[pathIndex];
        std::string rid = "rad:" + ridPart;

        // Validate RID format
        if (!isValidRadicleId(rid))
            throw BadURL("invalid Radicle ID '%s': expected format 'rad:z...'", rid);

        Attrs attrs;
        attrs.emplace("type", "rad");
        attrs.emplace("rid", rid);

        // Get ref from second segment if present
        if (pathIndex + 1 < url.path.size()) {
            std::string ref = url.path[pathIndex + 1];
            if (!ref.empty())
                attrs.emplace("ref", ref);
        }

        // Parse query parameters
        for (auto & [name, value] : url.query) {
            if (name == "rev" || name == "ref" || name == "node")
                attrs.emplace(name, value);
        }

        // Handle node from URL authority (rad://node.example.com/z...)
        if (url.authority && !attrs.contains("node"))
            attrs.emplace("node", url.authority->host);

        return inputFromAttrs(settings, attrs);
    }

    std::optional<Input> inputFromAttrs(const Settings & settings, const Attrs & attrs) const override
    {
        if (getStrAttr(attrs, "type") != "rad")
            return {};

        // Validate RID
        auto rid = getStrAttr(attrs, "rid");
        if (!isValidRadicleId(rid))
            throw BadURL("invalid Radicle ID '%s': expected format 'rad:z...'", rid);

        // Validate ref if provided
        if (auto ref = maybeGetStrAttr(attrs, "ref"); ref && !isLegalRefName(*ref))
            throw BadURL("invalid Git branch/tag name '%s'", *ref);

        // Validate node if provided (prevent command injection)
        if (auto node = maybeGetStrAttr(attrs, "node"); node && !isValidNodeIdentifier(*node))
            throw BadURL("invalid node identifier '%s': must be a valid domain name or node ID", *node);

        Input input{};
        input.attrs = attrs;
        return input;
    }

    ParsedURL toURL(const Input & input) const override
    {
        auto rid = getStrAttr(input.attrs, "rid");
        auto ref = input.getRef();

        ParsedURL url;
        url.scheme = "rad";

        // Build path as separate segments
        // Remove "rad:" prefix from RID for the path
        std::string ridPart = rid.substr(4); // Skip "rad:"

        // Check if we have an authority (node)
        auto node = maybeGetStrAttr(input.attrs, "node");

        if (node) {
            // With authority, path must start with empty element
            url.authority = ParsedURL::Authority{.host = *node};
            url.path = {"", ridPart};
        } else {
            // Without authority, path starts directly with RID
            url.path = {ridPart};
        }

        // Add ref as separate path segment
        if (ref)
            url.path.push_back(*ref);

        if (auto rev = input.getRev())
            url.query.insert_or_assign("rev", rev->gitRev());

        return url;
    }

    Input applyOverrides(const Input & input, std::optional<std::string> ref, std::optional<Hash> rev) const override
    {
        auto res(input);
        if (rev) {
            if (rev->algo != HashAlgorithm::SHA1)
                throw Error("Radicle repositories require SHA-1 hashes, got %s", printHashAlgo(rev->algo));
            res.attrs.insert_or_assign("rev", rev->gitRev());
        }
        if (ref) {
            if (!isLegalRefName(*ref))
                throw BadURL("invalid Git branch/tag name '%s'", *ref);
            res.attrs.insert_or_assign("ref", *ref);
        }
        return res;
    }

    void clone(const Settings & settings, Store & store, const Input & input, const std::filesystem::path & destDir)
        const override
    {
        auto rid = getStrAttr(input.attrs, "rid");
        auto node = maybeGetStrAttr(input.attrs, "node");

        if (input.getRev())
            throw UnimplementedError("cloning a specific Radicle revision is not implemented");

        cloneRadicleRepo(rid, node, destDir.string());

        // Checkout specific ref if requested
        if (auto ref = input.getRef()) {
            auto [status, output] = runProgram(RunOptions{
                .program = "git",
                .lookupPath = true,
                .args = {"-C", destDir.string(), "checkout", *ref},
            });
            if (status != 0)
                throw Error("failed to checkout ref '%s': %s", *ref, output);
        }
    }

    std::optional<std::string> getFingerprint(Store & store, const Input & input) const override
    {
        if (auto rev = input.getRev())
            return rev->gitRev();
        return std::nullopt;
    }

    bool isLocked(const Settings & settings, const Input & input) const override
    {
        return input.getRev().has_value();
    }

    bool isDirect(const Input & input) const override
    {
        return true;
    }

    std::optional<std::filesystem::path> getSourcePath(const Input & input) const override
    {
        // Radicle repositories are always remote (cached locally but not user workdirs)
        return std::nullopt;
    }

    void putFile(
        const Input & input,
        const CanonPath & path,
        std::string_view contents,
        std::optional<std::string> commitMsg) const override
    {
        throw UnimplementedError("putFile is not supported for Radicle repositories");
    }

private:
    RadicleRepoInfo getRepoInfo(const Input & input) const
    {
        // Validate hash algorithm if rev is specified
        if (auto rev = input.getRev()) {
            if (rev->algo != HashAlgorithm::SHA1)
                throw Error(
                    "Hash '%s' is not supported by Radicle. Only SHA-1 is supported.",
                    rev->to_string(HashFormat::Base16, true));
        }

        RadicleRepoInfo info;
        info.rid = getStrAttr(input.attrs, "rid");
        info.node = maybeGetStrAttr(input.attrs, "node");

        // Get cache path
        info.repoPath = getCachePath(info.rid, info.node);

        return info;
    }

    std::pair<ref<SourceAccessor>, Input> getAccessorFromCommit(
        ref<Store> store,
        const RadicleRepoInfo & repoInfo,
        Input && input,
        const Settings & settings) const
    {
        auto rid = repoInfo.rid;
        auto repoPath = repoInfo.repoPath;

        // Ensure parent cache directory exists
        auto cacheDir = dirOf(repoPath);
        createDirs(cacheDir);

        // Check if repository exists (with lock to prevent race conditions)
        bool needsClone;
        {
            PathLocks cacheDirLock({cacheDir});
            needsClone = !pathExists(repoPath);
            // If we need to clone, create a marker to prevent concurrent clones
            if (needsClone) {
                createDirs(repoPath);
            }
        }

        // Clone or update repository (outside lock to reduce contention)
        if (needsClone) {
            Activity act(*logger, lvlTalkative, actUnknown, fmt("cloning Radicle repository '%s'", rid));

            // Use AutoDelete to clean up on failure
            AutoDelete cleanup(repoPath, true);
            cloneRadicleRepo(rid, repoInfo.node, repoPath);
            cleanup.cancel();  // Success, don't delete
        } else {
            // Check if cache is stale
            struct stat st;
            Path fetchHeadFile = repoPath + "/.git/FETCH_HEAD";
            time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            bool shouldFetch = false;

            if (stat(fetchHeadFile.c_str(), &st) == 0) {
                // FETCH_HEAD exists, check if it's stale
                shouldFetch = !isCacheFileWithinTtl(now, st);
            } else {
                // FETCH_HEAD missing, assume stale (fresh clone or corrupted cache)
                shouldFetch = true;
            }

            if (shouldFetch) {
                Activity act(*logger, lvlTalkative, actUnknown, fmt("fetching updates for Radicle repository '%s'", rid));
                bool fetchSuccess = fetchRadicleRepo(repoPath, rid);
                // Only update FETCH_HEAD timestamp on successful fetch
                if (fetchSuccess) {
                    try {
                        setWriteTime(fetchHeadFile, now, now);
                    } catch (Error & e) {
                        warn("failed to update mtime on '%s': %s", fetchHeadFile, e.info().msg);
                    }
                }
            }
        }

        // Open the git repository
        auto repo = GitRepo::openRepo(repoPath);

        // Determine which ref to use
        std::string ref;
        if (auto inputRef = input.getRef()) {
            ref = *inputRef;
        } else {
            // Use default branch
            if (auto defaultBranch = getDefaultBranch(repoPath)) {
                ref = *defaultBranch;
            } else {
                warn("could not determine default branch for Radicle repository '%s', falling back to 'main'", rid);
                ref = "main";
            }
        }

        input.attrs.insert_or_assign("ref", ref);

        // Resolve the ref to a commit hash
        Hash rev{HashAlgorithm::SHA1};
        if (auto inputRev = input.getRev()) {
            rev = *inputRev;
        } else {
            rev = repo->resolveRef(ref);
            input.attrs.insert_or_assign("rev", rev.gitRev());
        }

        // Get lastModified
        if (!input.attrs.contains("lastModified")) {
            auto lastModified = getCachedAttribute<uint64_t>(
                settings,
                "radLastModified",
                rev,
                "lastModified",
                [&]() { return repo->getLastModified(rev); }
            );
            input.attrs.insert_or_assign("lastModified", lastModified);
        }

        // Get revCount
        if (!input.attrs.contains("revCount")) {
            auto revCount = getCachedAttribute<uint64_t>(
                settings,
                "radRevCount",
                rev,
                "revCount",
                [&]() { return repo->getRevCount(rev); }
            );
            input.attrs.insert_or_assign("revCount", revCount);
        }

        // Create SourceAccessor for the specific revision
        auto accessor = repo->getAccessor(rev, {}, "«" + input.to_string() + "»");

        return std::make_pair(accessor, std::move(input));
    }

public:
    std::pair<ref<SourceAccessor>, Input>
    getAccessor(const Settings & settings, Store & store, const Input & _input) const override
    {
        Input input(_input);
        auto repoInfo = getRepoInfo(input);

        return getAccessorFromCommit(ref<Store>(store.shared_from_this()), repoInfo, std::move(input), settings);
    }
};

static auto rRadicleInputScheme = OnStartup([] { registerInputScheme(std::make_unique<RadicleInputScheme>()); });

} // namespace nix::fetchers
