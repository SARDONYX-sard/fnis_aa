#pragma once

#include <format>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "fnis_version.hh"

namespace fnis_aa::config {

    enum class DiagnosticLevel : uint8_t {
        success,
        warning,
        error,
    };

    struct Diagnostic {
        DiagnosticLevel level;
        std::string     message;
    };

    struct AASet {
        int32_t mod_id{ 0 };
        int32_t group_id{ 0 };
        int32_t base{ 0 };

        [[nodiscard]] int32_t encode() const {
            return mod_id * 10000 + group_id * 100 + base;
        }

        /// To `PPGGBB` (e.g., mod:1, group:2, base:3 -> "010203").
        ///
        /// If each field is greater than 99, the result is not representable
        /// by the intended encoding format.
        [[nodiscard]] std::string to_encoded_string() const {
            return std::format("{:02}{:02}{:02}", mod_id, group_id, base);
        }
    };

    struct Config {
        spdlog::level::level_enum log_level{};
        int32_t                   crc{ 0 };
        int32_t                   mod_count{ 0 };
        int32_t                   set_count{ 0 };
        std::string               version_str{ "V07.06.00.0" };
        std::string               creature_version_str{ "V07.06.00.0" };
        FNISVersion               version{};
        FNISVersion               creature_version{};

        std::vector<AASet> set_list;  // capacity: 128
#ifdef TEST
        std::vector<std::string> prefix_list;  // capacity: 30
#else
        std::vector<RE::BSFixedString> prefix_list;  // capacity: 30
#endif

        [[nodiscard]] inline constexpr std::string_view get_version_str(bool creature) noexcept {
            return creature ? this->creature_version_str : this->version_str;
        }

        [[nodiscard]] inline constexpr const FNISVersion& get_version(bool creature) noexcept {
            return creature ? this->creature_version : this->version;
        }

        /// Rust-like debug format (derived Debug style).
        [[nodiscard]] inline std::string debug_str() const {
            std::string out = std::format(
                "ParsedConfig {{\n"
                "    log_level: {},\n"
                "    crc: {}(0x{:08X}),\n"
                "    mod_count: {},\n"
                "    set_count: {},\n"
                "    version_str: \"{}\",\n"
                "    creature_version_str: \"{}\",\n"
                "    version: FNISVersion {{ major: {}, minor1: {}, minor2: {}, flags: {} }},\n"
                "    creature_version: FNISVersion {{ major: {}, minor1: {}, minor2: {}, flags: {} }},\n",
                spdlog::level::to_string_view(log_level),
                crc, crc,
                mod_count,
                set_count,
                version_str,
                creature_version_str,
                version.major,
                version.minor1,
                version.minor2,
                version.flags,
                creature_version.major,
                creature_version.minor1,
                creature_version.minor2,
                creature_version.flags);

            out += "    prefix_list: [\n";

            for (size_t i = 0; i < set_list.size(); ++i) {
                const auto& s = set_list[i];

                out += std::format(
                    "        AASet {{ mod_id: {}, group_id: {}, base: {} }}{}",
                    s.mod_id,
                    s.group_id,
                    s.base,
                    (i == set_list.size() - 1 ? "" : ",\n"));
            }

            out += "\n    ]\n}";

            return out;
        }
    };

    struct ParsedConfig {
        Config                  config;
        std::vector<Diagnostic> diagnostics;

        [[nodiscard]] bool has_errors() const noexcept {
            return std::ranges::any_of(
                diagnostics,
                [](const Diagnostic& diagnostic) {
                    return diagnostic.level == DiagnosticLevel::error;
                });
        }

        [[nodiscard]] bool has_warnings() const noexcept {
            return std::ranges::any_of(
                diagnostics,
                [](const Diagnostic& diagnostic) {
                    return diagnostic.level == DiagnosticLevel::warning;
                });
        }

        [[nodiscard]] static ParsedConfig from_json(const nlohmann::json& j);
    };

#ifndef TEST

    // NOLINTBEGIN(cert-err58-cpp): initialize exception warn

    /// Global cache.
    inline Config g_config;

    /// Diagnostics produced while loading the global configuration.
    inline std::vector<Diagnostic> g_diagnostics;

    // NOLINTEND(cert-err58-cpp)

    /// Loads `Data/SKSE/Plugins/fnis_aa/config.json`.
    inline void NewGlobalConfig() {
        constexpr const char* CONFIG_PATH = "Data/SKSE/Plugins/fnis_aa/config.json";

        std::ifstream f{ CONFIG_PATH };

        if (!f.is_open()) {
            SPDLOG_ERROR("Failed to open config.json. path={}", CONFIG_PATH);
            g_diagnostics.push_back({
                .level = DiagnosticLevel::error,
                .message = std::format("Failed to open config.json. path={}", CONFIG_PATH),
            });

            return;
        }

        auto j = nlohmann::json::parse(f, nullptr, false);
        if (j.is_discarded()) {
            SPDLOG_ERROR("Failed to parse config.json. path={}", CONFIG_PATH);

            g_diagnostics.push_back({
                .level = DiagnosticLevel::error,
                .message = std::format("Failed to parse config.json. path={}", CONFIG_PATH),
            });

            return;
        }

        auto result = ParsedConfig::from_json(j);
        g_config = std::move(result.config);
        g_diagnostics = std::move(result.diagnostics);

        spdlog::set_level(g_config.log_level);
        SPDLOG_INFO("Log level initialized: {}", spdlog::level::to_string_view(g_config.log_level));
    }

#endif
};
