#pragma once

#include <expected>
#include <format>
#include <nlohmann/json.hpp>

namespace fnis_aa::config {
    /// Errors that can occur while parsing an FNIS version string.
    enum class FNISVersionError {
        /// The version string is shorter than the minimum required length.
        VersionStringTooShort,

        /// The expected `V` prefix is missing.
        ExpectedVersionPrefix,

        /// The expected `.` separator is missing.
        ExpectedVersionSeparator,

        /// The major version is not a decimal number.
        ExpectedMajorNumber,

        /// The first minor version is not a decimal number.
        ExpectedMinor1Number,

        /// The second minor version is not a decimal number.
        ExpectedMinor2Number,

        /// The optional flags value is not a decimal number.
        ExpectedFlagsNumber,
    };

    [[nodiscard]]
    constexpr std::string_view fnis_version_error_to_str(const FNISVersionError& error) noexcept {
        switch (error) {
        case FNISVersionError::VersionStringTooShort:
            return "version string is too short";
        case FNISVersionError::ExpectedVersionPrefix:
            return "expected 'V' version prefix";
        case FNISVersionError::ExpectedVersionSeparator:
            return "expected '.' version separator";
        case FNISVersionError::ExpectedMajorNumber:
            return "expected major version number";
        case FNISVersionError::ExpectedMinor1Number:
            return "expected minor1 version number";
        case FNISVersionError::ExpectedMinor2Number:
            return "expected minor2 version number";
        case FNISVersionError::ExpectedFlagsNumber:
            return "expected flags value (0, 1, or 2)";
        }
        return "unknown FNIS version error";
    }

    /// Version helpers — read from g_config.version (set at load time from JSON)
    ///
    /// Format: `VXX.YY.ZZ.F`  (same as original FNIS spec. e.g., `V07.06.00.0`)
    /// - [1..2] = Major
    /// - [4..5] = Minor1
    /// - [7..8] = Minor2
    /// - [10]   = Flags (0=Release, 1=Alpha, 2=Beta, 3=invalid)
    struct FNISVersion {
        int32_t major = 0;
        int32_t minor1 = 0;
        int32_t minor2 = 0;
        int32_t flags = 3;

        constexpr bool operator==(const FNISVersion&) const noexcept = default;  // intended Rust: #[derive(PartialEq, Eq)]

        /// Parses an FNIS version string in the form `V<DD>.<DD>.<DD>.<D>`
        ///
        /// # Errors
        ///
        /// Returns `FNISVersionError` when the version string is too short, has an unexpected prefix or separator, or contains a non-numeric version component.
        [[nodiscard]]
        static constexpr std::expected<FNISVersion, FNISVersionError> from_str(std::string_view ver) noexcept {
            if (ver.size() < 11) {
                return std::unexpected(FNISVersionError::VersionStringTooShort);
            }

            if (ver[0] != 'V') {
                return std::unexpected(FNISVersionError::ExpectedVersionPrefix);
            }

            if (ver[3] != '.' || ver[6] != '.' || ver[9] != '.') {
                return std::unexpected(FNISVersionError::ExpectedVersionSeparator);
            }

            const auto parse_two_digits = [](char tens, char ones) -> std::optional<int> {
                if (tens < '0' || tens > '9' || ones < '0' || ones > '9') {
                    return std::nullopt;
                }

                return (tens - '0') * 10 + (ones - '0');
            };

            const auto major = parse_two_digits(ver[1], ver[2]);
            if (!major) {
                return std::unexpected(FNISVersionError::ExpectedMajorNumber);
            }

            const auto minor1 = parse_two_digits(ver[4], ver[5]);
            if (!minor1) {
                return std::unexpected(FNISVersionError::ExpectedMinor1Number);
            }

            const auto minor2 = parse_two_digits(ver[7], ver[8]);
            if (!minor2) {
                return std::unexpected(FNISVersionError::ExpectedMinor2Number);
            }

            if (ver[10] < '0' || ver[10] > '2') {
                return std::unexpected(FNISVersionError::ExpectedFlagsNumber);
            }

            return FNISVersion{
                .major = *major,
                .minor1 = *minor1,
                .minor2 = *minor2,
                .flags = ver[10] - '0',
            };
        }

        [[nodiscard]] static constexpr FNISVersion latest() noexcept {
            return { .major = 7, .minor1 = 6, .minor2 = 0, .flags = 0 };
        }

        [[nodiscard]] bool is_err() const {
            return flags == 3;
        }
    };

    template <typename T>
    consteval bool test_fnis_version(std::string_view input, const T& expected) {
        const auto result = FNISVersion::from_str(input);
        return result.has_value() && *result == expected;
    }
    consteval bool test_fnis_version_error(std::string_view input, FNISVersionError expected) {
        const auto result = FNISVersion::from_str(input);
        return !result.has_value() && result.error() == expected;
    }

    static_assert(test_fnis_version("V07.06.00.0", FNISVersion{ .major = 7, .minor1 = 6, .minor2 = 0, .flags = 0 }));
    static_assert(test_fnis_version("V07.06.00.1", FNISVersion{ .major = 7, .minor1 = 6, .minor2 = 0, .flags = 1 }));
    static_assert(test_fnis_version("V07.06.00.2", FNISVersion{ .major = 7, .minor1 = 6, .minor2 = 0, .flags = 2 }));

    static_assert(test_fnis_version_error("V07.06.00", FNISVersionError::VersionStringTooShort));
    static_assert(test_fnis_version_error("v07.06.00.0", FNISVersionError::ExpectedVersionPrefix));
    static_assert(test_fnis_version_error("V07-06.00.0", FNISVersionError::ExpectedVersionSeparator));
    static_assert(test_fnis_version_error("Vxx.06.00.0", FNISVersionError::ExpectedMajorNumber));
    static_assert(test_fnis_version_error("V07.xx.00.0", FNISVersionError::ExpectedMinor1Number));
    static_assert(test_fnis_version_error("V07.06.xx.0", FNISVersionError::ExpectedMinor2Number));
    static_assert(test_fnis_version_error("V07.06.00.3", FNISVersionError::ExpectedFlagsNumber));

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

        /// Rust-like debug format (derived Debug style)
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
                out += std::format("        AASet {{ mod_id: {}, group_id: {}, base: {} }}{}", s.mod_id, s.group_id, s.base, (i == set_list.size() - 1 ? "" : ",\n"));
            }
            out += "\n    ]\n}";

            return out;
        }

        static Config from_json(const nlohmann::json& j);
    };

#ifndef TEST
    // NOLINTBEGIN(cert-err58-cpp): initialize exception warn

    /// Global cache
    inline Config g_config;

    // NOLINTEND(cert-err58-cpp)

    /// new global `Config` from `Data/SKSE/Plugins/fnis_aa/config.json`
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
        g_config = Config::from_json(j);

        spdlog::set_level(g_config.log_level);
        SPDLOG_INFO("Log level initialized: {}", spdlog::level::to_string_view(g_config.log_level));
    }
#endif
}
