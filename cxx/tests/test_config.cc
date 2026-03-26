#include "config.hh"

#include <catch2/catch_test_macros.hpp>

using json = nlohmann::json;
using namespace ::config;

namespace {
    static const char* kValidJson = R"({
    "log_level": "info",
    "crc": 1520082533,
    "fnis_version": "V07.06.00.0",
    "mods": [
        { "prefix": "fsm", "name": "fsm", "mod_id": 0,
          "groups": [{ "name": "_mt", "base": 1 }] },
        { "prefix": "fs3", "name": "fs3", "mod_id": 1,
          "groups": [{ "name": "_mt", "base": 10 }, { "name": "_mtx", "base": 1 }] },
        { "prefix": "xpe", "name": "xpe", "mod_id": 2,
          "groups": [{ "name": "_1hmeqp", "base": 1 }, { "name": "_2hmeqp", "base": 1 }] }
    ]
})";

    struct ValidFixture {
        ParsedConfig cfg;
        ValidFixture() : cfg(parse_config(json::parse(kValidJson))) {
            UNSCOPED_INFO(cfg.debug_str());
            spdlog::info("ValidFixture:\n{}", cfg.debug_str());
        }
    };

    TEST_CASE_METHOD(ValidFixture, "crc and version") {
        CHECK(cfg.crc == 1520082533);
        CHECK(cfg.version == "V07.06.00.0");
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
        // group_id ascending: _mt(10), _mt(10), _mtx(11), _1hmeqp(37), _2hmeqp(39)
        CHECK(cfg.set_list[0].to_encoded_string() == "001001");  // fsm _mt  base=1
        CHECK(cfg.set_list[1].to_encoded_string() == "011010");  // fs3 _mt  base=10
        CHECK(cfg.set_list[2].to_encoded_string() == "011101");  // fs3 _mtx base=1
        CHECK(cfg.set_list[3].to_encoded_string() == "023701");  // xpe _1hmeqp base=1
        CHECK(cfg.set_list[4].to_encoded_string() == "023901");  // xpe _2hmeqp base=1
    }

    TEST_CASE("missing crc defaults to 0") {
        const auto cfg = parse_config(json::parse(R"({"mods":[]})"));
        CHECK(cfg.crc == 0);
    }

    TEST_CASE("missing fnis_version defaults") {
        const auto cfg = parse_config(json::parse(R"({"mods":[]})"));
        CHECK(cfg.version == "V07.06.00.0");
    }

    TEST_CASE("unknown group is skipped") {
        const auto cfg = parse_config(json::parse(R"({
        "crc": 0, "fnis_version": "V07.06.00.0",
        "mods": [{ "prefix": "x", "name": "x", "mod_id": 0,
                   "groups": [{ "name": "_unknown_group", "base": 1 }] }]
    })"));
        CHECK(cfg.set_count == 0);
    }
}
