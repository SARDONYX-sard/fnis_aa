#include <catch2/catch_test_macros.hpp>

#include "config.hh"

// NOLINTBEGIN(cert-err58-cpp)
namespace {
    using json = nlohmann::json;
    using namespace fnis_aa;
    using namespace fnis_aa::config;

    static const char* JSON_STR = R"({
    "log_level": "info",
    "crc": 1520082533,
    "fnis_version": "V07.06.00.0",
    "mods": [
        {
            "prefix": "fsm",
            "name": "fsm",
            "mod_id": 0,
            "groups": [
                {
                    "name": "_mt",
                    "base": 1
                }
            ]
        },
        {
            "prefix": "fs3",
            "name": "fs3",
            "mod_id": 1,
            "groups": [
                {
                    "name": "_mt",
                    "base": 10
                },
                {
                    "name": "_mtx",
                    "base": 1
                }
            ]
        },
        {
            "prefix": "xpe",
            "name": "xpe",
            "mod_id": 2,
            "groups": [
                {
                    "name": "_1hmeqp",
                    "base": 1
                },
                {
                    "name": "_2hmeqp",
                    "base": 1
                }
            ]
        }
    ]
})";

    struct ValidFixture {
        Config                  cfg;
        std::vector<Diagnostic> diagnostics;

        ValidFixture() {
            auto result = ParsedConfig::from_json(json::parse(JSON_STR));

            cfg = std::move(result.config);
            diagnostics = std::move(result.diagnostics);

            UNSCOPED_INFO(cfg.debug_str());

            for (const auto& diagnostic : diagnostics) {
                UNSCOPED_INFO(diagnostic.message);
            }

            spdlog::info("ValidFixture:\n{}", cfg.debug_str());
        }
    };

    TEST_CASE_METHOD(ValidFixture, "crc and version") {
        CHECK(cfg.crc == 1520082533);
        CHECK(cfg.version_str == "V07.06.00.0");
        CHECK(cfg.version == FNISVersion::latest());
    }

    TEST_CASE_METHOD(ValidFixture, "mod count and set count") {
        CHECK(cfg.mod_count == 3);
        CHECK(cfg.set_count == 5);
    }

    TEST_CASE_METHOD(ValidFixture, "prefix list indexed by mod_id") {
        CHECK(cfg.prefix_list[0] == "fsm");
        CHECK(cfg.prefix_list[1] == "fs3");
        CHECK(cfg.prefix_list[2] == "xpe");
    }

    TEST_CASE_METHOD(ValidFixture, "set list sorted by group_id") {
        CHECK(cfg.set_list[0].to_encoded_string() == "001001");
        CHECK(cfg.set_list[1].to_encoded_string() == "011010");
        CHECK(cfg.set_list[2].to_encoded_string() == "011101");
        CHECK(cfg.set_list[3].to_encoded_string() == "023701");
        CHECK(cfg.set_list[4].to_encoded_string() == "023901");
    }

    TEST_CASE("missing crc defaults to 0") {
        const auto result = ParsedConfig::from_json(
            json::parse(R"({"mods":[]})"));

        CHECK(result.config.crc == 0);
    }

    TEST_CASE("missing fnis_version defaults") {
        const auto result = ParsedConfig::from_json(
            json::parse(R"({"mods":[]})"));

        CHECK(result.config.version_str == "V07.06.00.0");
    }

    TEST_CASE("unknown group is skipped") {
        const auto result = ParsedConfig::from_json(
            json::parse(R"({
            "crc": 0,
            "fnis_version": "V07.06.00.0",
            "mods": [
                {
                    "prefix": "x",
                    "name": "x",
                    "mod_id": 0,
                    "groups": [
                        {
                            "name": "_unknown_group",
                            "base": 1
                        }
                    ]
                }
            ]
        })"));

        CHECK(result.config.set_count == 0);
        CHECK(result.config.set_list.empty());
    }

}
// NOLINTEND(cert-err58-cpp)
