#include "alt_group_table.hh"

#include "config.hh"

namespace fnis_aa::config {
    namespace {
        /// Checks if two strings are equal, ignoring ASCII case. non heap alloc
        /// - https://doc.rust-lang.org/std/primitive.str.html#method.eq_ignore_ascii_case
        [[nodiscard]] constexpr bool eq_ignore_ascii_case(std::string_view a, std::string_view b) noexcept {
            if (a.size() != b.size())
                return false;

            for (std::size_t i = 0; i < a.size(); ++i) {
                auto ca = static_cast<unsigned char>(a[i]);
                auto cb = static_cast<unsigned char>(b[i]);

                if (ca != cb) {
                    // If characters don't match, try to normalize both to lowercase.
                    // Only 'A'-'Z' (0x41-0x5A) are affected by '| 0x20'.
                    auto la = (ca >= 'A' && ca <= 'Z') ? (ca | 0x20) : ca;
                    auto lb = (cb >= 'A' && cb <= 'Z') ? (cb | 0x20) : cb;

                    if (la != lb)
                        return false;
                }
            }
            return true;
        }

        // --- Compile-time Tests ---
        static_assert(eq_ignore_ascii_case("trace", "TRACE"), "Failed: Basic case insensitivity");
        static_assert(eq_ignore_ascii_case("debug", "debug"), "Failed: Exact match");
        static_assert(!eq_ignore_ascii_case("info", "warn"), "Failed: Different strings");
        static_assert(!eq_ignore_ascii_case("err", "error"), "Failed: Length mismatch");
        static_assert(eq_ignore_ascii_case("Critical", "CRITICAL"), "Failed: Mixed case");

        // Boundary check for non-alpha characters (Ensures bit 0x20 doesn't cause false positives)
        static_assert(!eq_ignore_ascii_case("@", "`"), "Failed: Symbol boundary @ vs `");
        static_assert(!eq_ignore_ascii_case("[", "{"), "Failed: Symbol boundary [ vs {");
        static_assert(eq_ignore_ascii_case("log_1", "LOG_1"), "Failed: Numbers and underscores");

        /// Validates the log level string and applies it to spdlog.
        spdlog::level::level_enum spdlog_level_from_str(std::string_view a_level_str) {
            spdlog::level::level_enum target_level = spdlog::level::info;
            bool                      is_valid = false;

            // Allocation-free comparison using our bitwise helper
            if (eq_ignore_ascii_case(a_level_str, "trace")) {
                target_level = spdlog::level::trace;
                is_valid = true;
            } else if (eq_ignore_ascii_case(a_level_str, "debug")) {
                target_level = spdlog::level::debug;
                is_valid = true;
            } else if (eq_ignore_ascii_case(a_level_str, "info")) {
                target_level = spdlog::level::info;
                is_valid = true;
            } else if (eq_ignore_ascii_case(a_level_str, "warn")) {
                target_level = spdlog::level::warn;
                is_valid = true;
            } else if (eq_ignore_ascii_case(a_level_str, "error")) {
                target_level = spdlog::level::err;
                is_valid = true;
            } else if (eq_ignore_ascii_case(a_level_str, "critical")) {
                target_level = spdlog::level::critical;
                is_valid = true;
            } else if (eq_ignore_ascii_case(a_level_str, "off")) {
                target_level = spdlog::level::off;
                is_valid = true;
            }

            if (!is_valid) {
                SPDLOG_WARN("Invalid log_level: \"{}\". Expected trace/debug/info/warn/err/critical/off.", a_level_str);
                SPDLOG_WARN("  -> Falling back to default: \"info\"");
                target_level = spdlog::level::info;
            }

            return target_level;
        }

        // Helper to get a required field with a warn fallback
        template <typename T>
        T require_field(const nlohmann::json& j, std::string_view key, T default_val, std::string_view context) {
            if (!j.contains(key)) {
                SPDLOG_WARN("[{}] missing required field '{}', defaulting to '{}'", context, key, default_val);
                return default_val;
            }
            return j[key].get<T>();
        }

        // Helper to get an optional field with an info-level fallback.
        template <typename T>
        T optional_field(const nlohmann::json& j, std::string_view key, T default_val, std::string_view context) {
            if (!j.contains(key)) {
                SPDLOG_INFO("[{}] optional field '{}' is missing, defaulting to '{}'", context, key, default_val);
                return default_val;
            }
            return j[key].get<T>();
        }

        template <typename T>
        void assign_or_log_error(T& output, const std::expected<T, FNISVersionError>& result, std::string_view name, std::string_view input) {
            if (result.has_value()) {
                output = result.value();
            } else {
                SPDLOG_ERROR("FNIS: failed to parse {}('{}'). {}", name, input, fnis_version_error_to_str(result.error()));
            }
        }
    }

    Config Config::from_json(const nlohmann::json& j) {
        Config r;

        r.set_list.reserve(128);
        r.prefix_list.resize(30);

        r.log_level = spdlog_level_from_str(optional_field<std::string_view>(j, "log_level", "info", "<root>")),
        r.crc = static_cast<int32_t>(require_field<uint32_t>(j, "crc", 0u, "<root>"));

        r.version_str = require_field<std::string>(j, "fnis_version", "V07.06.00.0", "<root>");
        r.creature_version_str = require_field<std::string>(j, "fnis_creature_version", r.version_str, "<root>");

        {
            const auto version_result = FNISVersion::from_str(r.version_str);
            assign_or_log_error(r.version, version_result, "version", r.version_str);
        }
        {
            const auto creature_version_result = FNISVersion::from_str(r.creature_version_str);
            assign_or_log_error(r.creature_version, creature_version_result, "creature version", r.creature_version_str);
        }

        if (!j.contains("mods") || !j["mods"].is_array()) {
            SPDLOG_WARN("parse_registry: missing or invalid 'mods' array");
            return r;
        }

        for (const auto& mod_json : j["mods"]) {
            const auto mod_id = require_field<uint32_t>(mod_json, "mod_id", 0u, "<mod>");
            const auto prefix = require_field<std::string>(mod_json, "prefix", {}, "<mod>");
            const auto name = require_field<std::string>(mod_json, "name", {}, "<mod>");
            const auto ctx = std::format("mod '{}'", name);

            r.mod_count++;
            if (mod_id < 30) {
                r.prefix_list[mod_id] = prefix;
            } else {
                SPDLOG_WARN("It appears that the original FNIS specification does not support more than 30 mod prefixes.");
                SPDLOG_INFO("Expanding prefix_list for extended mod_id: {}, prefix: {}", mod_id, prefix);
                r.prefix_list.emplace_back(prefix);
            }

            if (!mod_json.contains("groups") || !mod_json["groups"].is_array()) {
                SPDLOG_WARN("[{}] missing or invalid 'groups' array", ctx);
                continue;
            }

            for (const auto& jg : mod_json["groups"]) {
                const auto gname = require_field<std::string>(jg, "name", {}, ctx);
                const auto base_val = require_field<uint32_t>(jg, "base", 0u, ctx);
                const auto info = GetAltGroup(gname);

                if (info) {
                    const auto& group = info->get();

                    r.set_list.push_back({ .mod_id = static_cast<int32_t>(mod_id),
                        .group_id = group.id,
                        .base = static_cast<int32_t>(base_val) });
                    r.set_count++;
                }
            }
        }

        std::ranges::sort(r.set_list, [](const auto& a, const auto& b) {
            if (a.group_id != b.group_id) {
                return a.group_id < b.group_id;
            }
            return a.mod_id < b.mod_id;
        });

        SPDLOG_DEBUG("{}", r.debug_str());
        return r;
    }
}
