#pragma once

#include <vector>

#include <nlohmann/json.hpp>
namespace aa_registry {

    struct ParsedConfig {
        int32_t crc{ 0 };
        int32_t mod_count{ 0 };
        int32_t set_count{ 0 };

#ifdef TEST
        std::string              version{ "V07.06.00.0" };
        std::vector<std::string> prefix_list = std::vector<std::string>(30);
        std::vector<std::string> set_list = std::vector<std::string>(128);

        /// Rust like debug format
        [[nodiscard]] inline std::string debug_str() const {
            std::string out = std::format(
                "ParsedConfig {{\n"
                "  crc: {},\n"
                "  mod_count: {},\n"
                "  set_count: {},\n"
                "  version: \"{}\",\n"
                "  prefix_list: [",
                crc, mod_count, set_count, version);
            for (const auto& s : prefix_list)
                out += std::format("\"{}\", ", s);
            out += "],\n  set_list: [";
            for (const auto& s : set_list)
                out += std::format("\"{}\", ", s);
            out += "]\n}";
            return out;
        }

#else
        RE::BSFixedString              version{ "V07.06.00.0" };
        std::vector<RE::BSFixedString> prefix_list = std::vector<RE::BSFixedString>(30);  // capacity: 30
        std::vector<RE::BSFixedString> set_list = std::vector<RE::BSFixedString>(128);    // capacity: 128
#endif
    };
    ParsedConfig parse_config(const nlohmann::json& j);

#ifndef TEST
    inline ParsedConfig g_config;  // Global cache;

    inline void OnLoaded() {
        const char*   CONFIG_PATH = "Data/SKSE/Plugins/fnis_aa/config.json";
        std::ifstream f{ CONFIG_PATH };
        if (!f.is_open()) {
            SPDLOG_ERROR("Failed to open config.json. path={}", CONFIG_PATH);
            return;
        }
        auto j = nlohmann::json::parse(f, nullptr, false);
        if (j.is_discarded()) {
            SPDLOG_ERROR("Failed to parse config.json. path={}", CONFIG_PATH);
            return;
        }
        g_config = parse_config(j);
    }
#endif
}
