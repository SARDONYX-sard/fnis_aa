#include "alt_group_table.hh"

#include "ascii.hh"
#include "config.hh"

namespace config {
    namespace {
        // Helper to get a required field with a warn fallback
        template <typename T>
        T require_field(const nlohmann::json& j, std::string_view key, T default_val,
            std::string_view context) {
            if (!j.contains(key)) {
                SPDLOG_WARN("[{}] missing required field '{}', defaulting to '{}'",
                    context, key, default_val);
                return default_val;
            }
            return j[key].get<T>();
        }

        /// Validates the log level string and applies it to spdlog.
        spdlog::level::level_enum validate_and_apply_log_level(std::string_view a_level_str) {
            spdlog::level::level_enum target_level = spdlog::level::info;
            bool                      is_valid = false;

            // Allocation-free comparison using our bitwise helper
            if (iequals_ascii(a_level_str, "trace")) {
                target_level = spdlog::level::trace;
                is_valid = true;
            } else if (iequals_ascii(a_level_str, "debug")) {
                target_level = spdlog::level::debug;
                is_valid = true;
            } else if (iequals_ascii(a_level_str, "info")) {
                target_level = spdlog::level::info;
                is_valid = true;
            } else if (iequals_ascii(a_level_str, "warn")) {
                target_level = spdlog::level::warn;
                is_valid = true;
            } else if (iequals_ascii(a_level_str, "error")) {
                target_level = spdlog::level::err;
                is_valid = true;
            } else if (iequals_ascii(a_level_str, "critical")) {
                target_level = spdlog::level::critical;
                is_valid = true;
            } else if (iequals_ascii(a_level_str, "off")) {
                target_level = spdlog::level::off;
                is_valid = true;
            }

            if (!is_valid) {
                SPDLOG_WARN("Invalid log_level: \"{}\". Expected trace/debug/info/warn/err/critical/off.", a_level_str);
                SPDLOG_WARN("  -> Falling back to default: \"trace\"");
                target_level = spdlog::level::trace;
            }

            return target_level;
        }
    }

    ParsedConfig parse_config(const nlohmann::json& j) {
        ParsedConfig r;
        {
            auto log_level_raw = j.value<std::string_view>("log_level", "trace");
            r.log_level = validate_and_apply_log_level(log_level_raw);
        }

        r.set_list.reserve(128);
        r.prefix_list.resize(30);

        r.crc = static_cast<int32_t>(require_field<uint32_t>(j, "crc", 0u, "<root>"));

        r.version_str = require_field<std::string>(j, "fnis_version", "V07.06.00.0", "<root>");
        r.version = FNISVersion::from_str(r.version_str);

        r.creature_version_str = require_field<std::string>(j, "fnis_creature_version", r.version_str, "<root>");
        r.creature_version = FNISVersion::from_str(r.creature_version_str);

        if (!j.contains("mods") || !j["mods"].is_array()) {
            SPDLOG_WARN("parse_registry: missing or invalid 'mods' array");
            return r;
        }

        for (const auto& jmod : j["mods"]) {
            const auto mod_id = require_field<uint32_t>(jmod, "mod_id", 0u, "<mod>");
            const auto prefix = require_field<std::string>(jmod, "prefix", {}, "<mod>");
            const auto name = require_field<std::string>(jmod, "name", {}, "<mod>");
            const auto ctx = std::format("mod '{}'", name);

            r.mod_count++;
            if (mod_id < 30) {
                r.prefix_list[mod_id] = prefix;
            } else {
                SPDLOG_WARN("It appears that the original FNIS specification does not support more than 30 mod prefixes.");
                SPDLOG_INFO("Expanding prefix_list for extended mod_id: {}, prefix: {}", mod_id, prefix);
                r.prefix_list.emplace_back(prefix);
            }

            if (!jmod.contains("groups") || !jmod["groups"].is_array()) {
                SPDLOG_WARN("[{}] missing or invalid 'groups' array", ctx);
                continue;
            }

            for (const auto& jg : jmod["groups"]) {
                const auto  gname = require_field<std::string>(jg, "name", {}, ctx);
                const auto  base_val = require_field<uint32_t>(jg, "base", 0u, ctx);
                const auto* info = GetAltGroup(gname);

                if (info) {
                    r.set_list.push_back({ .mod_id = static_cast<int32_t>(mod_id),
                        .group_id = info->id,
                        .base = static_cast<int32_t>(base_val) });
                    r.set_count++;
                }
            }
        }

        std::ranges::sort(r.set_list, [](const auto& a, const auto& b) {
            if (a.group_id != b.group_id)
                return a.group_id < b.group_id;
            return a.mod_id < b.mod_id;
        });

        SPDLOG_DEBUG("{}", r.debug_str());
        return r;
    }
}
