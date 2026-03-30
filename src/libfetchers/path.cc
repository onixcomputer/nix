#include "nix/fetchers/fetchers.hh"
#include "nix/store/store-api.hh"
#include "nix/util/archive.hh"
#include "nix/fetchers/cache.hh"
#include "nix/fetchers/fetch-to-store.hh"
#include "nix/fetchers/fetch-settings.hh"
#include "nix/util/posix-source-accessor.hh"
#include "nix/util/file-system.hh"
#include "nix/util/signals.hh"

namespace nix::fetchers {

struct PathInputScheme : InputScheme
{
    std::optional<Input> inputFromURL(const Settings & settings, const ParsedURL & url, bool requireTree) const override
    {
        if (url.scheme != "path")
            return {};

        if (url.authority && url.authority->host.size())
            throw Error("path URL '%s' should not have an authority ('%s')", url, *url.authority);

        Input input{};
        input.attrs.insert_or_assign("type", "path");
        input.attrs.insert_or_assign("path", renderUrlPathEnsureLegal(url.path));

        for (auto & [name, value] : url.query)
            if (name == "rev" || name == "narHash")
                input.attrs.insert_or_assign(name, value);
            else if (name == "revCount" || name == "lastModified") {
                if (auto n = string2Int<uint64_t>(value))
                    input.attrs.insert_or_assign(name, *n);
                else
                    throw Error("path URL '%s' has invalid parameter '%s'", url, name);
            } else
                throw Error("path URL '%s' has unsupported parameter '%s'", url, name);

        return input;
    }

    std::string_view schemeName() const override
    {
        return "path";
    }

    std::string schemeDescription() const override
    {
        // TODO
        return "";
    }

    const std::map<std::string, AttributeInfo> & allowedAttrs() const override
    {
        static const std::map<std::string, AttributeInfo> attrs = {
            {
                "path",
                {},
            },
            /* Allow the user to pass in "fake" tree info
               attributes. This is useful for making a pinned tree work
               the same as the repository from which is exported (e.g.
               path:/nix/store/...-source?lastModified=1585388205&rev=b0c285...).
             */
            {
                "rev",
                {},
            },
            {
                "revCount",
                {},
            },
            {
                "lastModified",
                {},
            },
            {
                "narHash",
                {},
            },
        };
        return attrs;
    }

    std::optional<Input> inputFromAttrs(const Settings & settings, const Attrs & attrs) const override
    {
        getStrAttr(attrs, "path");

        Input input{};
        input.attrs = attrs;
        return input;
    }

    ParsedURL toURL(const Input & input) const override
    {
        auto query = attrsToQuery(input.attrs);
        query.erase("path");
        query.erase("type");
        query.erase("__final");
        return ParsedURL{
            .scheme = "path",
            .path = splitString<std::vector<std::string>>(getStrAttr(input.attrs, "path"), "/"),
            .query = query,
        };
    }

    std::optional<std::filesystem::path> getSourcePath(const Input & input) const override
    {
        return getAbsPath(input);
    }

    void putFile(
        const Input & input,
        const CanonPath & path,
        std::string_view contents,
        std::optional<std::string> commitMsg) const override
    {
        writeFile(getAbsPath(input) / path.rel(), contents);
    }

    std::optional<std::string> isRelative(const Input & input) const override
    {
        auto path = getStrAttr(input.attrs, "path");
        if (isAbsolute(path))
            return std::nullopt;
        else
            return path;
    }

    bool isLocked(const Settings & settings, const Input & input) const override
    {
        return (bool) input.getNarHash();
    }

    std::filesystem::path getAbsPath(const Input & input) const
    {
        auto path = getStrAttr(input.attrs, "path");

        if (isAbsolute(path))
            return canonPath(path);

        throw Error("cannot fetch input '%s' because it uses a relative path", input.to_string());
    }

    std::pair<ref<SourceAccessor>, Input>
    getAccessor(const Settings & settings, Store & store, const Input & _input) const override
    {
        Input input(_input);
        auto absPath = getAbsPath(input);

        // FIXME: check whether access to 'path' is allowed.

        /* If the path is already a valid store path named "source",
           use the store accessor directly (avoids redundant copy). */
        auto storePath = store.maybeParseStorePath(absPath.string());
        if (storePath) {
            store.addTempRoot(*storePath);
            if (storePath->name() == "source" && store.isValidPath(*storePath)) {
                auto accessor = store.requireStoreObjectAccessor(*storePath);
                auto info = store.queryPathInfo(*storePath);
                accessor->fingerprint =
                    fmt("path:%s", info->narHash.to_string(HashFormat::SRI, true));
                settings.getCache()->upsert(
                    makeFetchToStoreCacheKey(
                        input.getName(), *accessor->fingerprint, ContentAddressMethod::Raw::NixArchive, "/"),
                    store,
                    {},
                    *storePath);
                if (!input.getLastModified())
                    input.attrs.insert_or_assign("lastModified", uint64_t(info->registrationTime));
                return {accessor, std::move(input)};
            }
        }

        /* Return a lazy accessor that reads directly from the
           filesystem. The store copy is deferred until mountInput()
           calls fetchToStore(). This avoids copying large directory
           trees for evaluation that only touches a few files. */
        auto accessor = make_ref<PosixSourceAccessor>(std::filesystem::path(absPath), true);
        accessor->lazyPathInput = true;

        /* Compute lastModified by stat'ing the tree. Cheaper than
           a full store copy since we only stat, not read contents. */
        if (!input.getLastModified()) {
            time_t maxMtime = 0;
            struct stat rootSt;
            if (lstat(absPath.string().c_str(), &rootSt) == 0) {
                maxMtime = rootSt.st_mtime;
                if (S_ISDIR(rootSt.st_mode)) {
                    std::function<void(const std::string &)> walk;
                    walk = [&](const std::string & dir) {
                        for (auto & entry : DirectoryIterator{dir}) {
                            checkInterrupt();
                            auto p = entry.path().string();
                            struct stat st;
                            if (lstat(p.c_str(), &st) == 0) {
                                if (st.st_mtime > maxMtime)
                                    maxMtime = st.st_mtime;
                                if (S_ISDIR(st.st_mode))
                                    walk(p);
                            }
                        }
                    };
                    walk(absPath.string());
                }
            }
            input.attrs.insert_or_assign("lastModified", uint64_t(maxMtime));
        }

        accessor->setPathDisplay("«" + input.to_string() + "»");

        /* Compute a NAR hash fingerprint so fetchToStore() can cache
           the result. This reads file contents (O(n)) but avoids the
           store write — the actual copy happens lazily in
           mountInput(). */
        auto narHash = accessor->hashPath(CanonPath::root);
        accessor->fingerprint =
            fmt("path:%s", narHash.to_string(HashFormat::SRI, true));

        return {accessor, std::move(input)};
    }

    std::optional<ExperimentalFeature> experimentalFeature() const override
    {
        return Xp::Flakes;
    }
};

static auto rPathInputScheme = OnStartup([] { registerInputScheme(std::make_unique<PathInputScheme>()); });

} // namespace nix::fetchers
