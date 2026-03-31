#pragma once

#include <format>
#include <nlohmann/json.hpp>

namespace config {
    /// ---------------------------------------------------------------------------
    /// Version helpers — read from g_config.version (set at load time from JSON)
    ///
    /// Format: "VXX.YY.ZZF"  (same as original FNIS spec)
    ///   [1..2] = Major
    ///   [4..5] = Minor1
    ///   [7..8] = Minor2
    ///   [10]   = Flags (0=Release, 1=Alpha, 2=Beta, 3=invalid)
    /// ---------------------------------------------------------------------------
    struct FNISVersion {
        int32_t major = 0;
        int32_t minor1 = 0;
        int32_t minor2 = 0;
        int32_t flags = 3;

        static FNISVersion from_str(const std::string& ver) {
            if (ver.size() < 10)
                return {};
            FNISVersion v;
            try {
                v.major = std::stoi(ver.substr(1, 2));
                v.minor1 = std::stoi(ver.substr(4, 2));
                v.minor2 = std::stoi(ver.substr(7, 2));
                v.flags = (ver.size() > 10) ? std::stoi(ver.substr(10, 1)) : 0;
            } catch (...) {
                SPDLOG_WARN("FNIS: failed to parse version string '{}'", ver);
            }
            return v;
        }

        bool is_err() const {
            return flags == 3;
        }
    };

    struct AASet {
        int32_t mod_id{ 0 };
        int32_t group_id{ 0 };
        int32_t base{ 0 };

        [[nodiscard]] int32_t encode() const {
            return mod_id * 10000 + group_id * 100 + base;
        }

        /// To `PPGGBB`(e.g.,: mod:1, group:2, base:3 -> "010203")
        ///
        /// If each field >99, then UB.
        [[nodiscard]] std::string to_encoded_string() const {
            return std::format("{:02}{:02}{:02}", mod_id, group_id, base);
        }
    };

    struct ParsedConfig {
        spdlog::level::level_enum log_level{};
        int32_t                   crc{ 0 };
        int32_t                   mod_count{ 0 };
        int32_t                   set_count{ 0 };
        std::string               version_str{ "V07.06.00.0" };
        std::string               creature_version_str{ "V07.06.00.0" };
        FNISVersion               version;
        FNISVersion               creature_version;

        std::vector<AASet> set_list;  // capacity: 128
#ifdef TEST
        std::vector<std::string> prefix_list;  // capacity: 30
#else
        std::vector<RE::BSFixedString> prefix_list;  // capacity: 30
#endif

        /// Rust-like debug format (derived Debug style)
        [[nodiscard]] inline std::string debug_str() const {
            std::string out = std::format(
                "ParsedConfig {{\n"
                "    log_level: {},\n"
                "    crc: {},\n"
                "    mod_count: {},\n"
                "    set_count: {},\n"
                "    version: \"{}\",\n"
                "    prefix_list: [",
                spdlog::level::to_string_view(log_level),
                crc, mod_count, set_count, version_str);

            for (size_t i = 0; i < prefix_list.size(); ++i) {
                if (!prefix_list[i].empty()) {
                    out += std::format("\"{}\"{}", prefix_list[i].c_str(), (i == mod_count - 1 ? "" : ", "));
                }
            }

            out += "],\n    set_list: [\n";
            for (size_t i = 0; i < set_list.size(); ++i) {
                const auto& s = set_list[i];
                out += std::format(
                    "        AASet {{ mod_id: {}, group_id: {}, base: {} }}{}",
                    s.mod_id, s.group_id, s.base,
                    (i == set_list.size() - 1 ? "" : ",\n"));
            }
            out += "\n    ]\n}";
            return out;
        }
    };
    ParsedConfig parse_config(const nlohmann::json& j);

#ifndef TEST
    inline ParsedConfig g_config;  // Global cache

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

        spdlog::set_level(g_config.log_level);
        SPDLOG_INFO("Logger level initialized: {}", spdlog::level::to_string_view(g_config.log_level));
    }
#endif
}
