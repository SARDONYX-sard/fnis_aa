#include "config.hh"
#include "alt_group_table.hh"

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

        // Boundary check for non-alpha characters.
        static_assert(!eq_ignore_ascii_case("@", "`"), "Failed: Symbol boundary @ vs `");
        static_assert(!eq_ignore_ascii_case("[", "{"), "Failed: Symbol boundary [ vs {");
        static_assert(eq_ignore_ascii_case("log_1", "LOG_1"), "Failed: Numbers and underscores");

        using Diagnostics = std::vector<Diagnostic>;

        void add_warning(Diagnostics& diagnostics, std::string message) {
            diagnostics.push_back({
                .level = DiagnosticLevel::warning,
                .message = std::move(message),
            });
        }

        void add_error(Diagnostics& diagnostics, std::string message) {
            diagnostics.push_back({
                .level = DiagnosticLevel::error,
                .message = std::move(message),
            });
        }

        /// Validates the log level string and applies it to spdlog.
        [[nodiscard]]
        spdlog::level::level_enum spdlog_level_from_str(std::string_view level_str, Diagnostics& diagnostics) {
            spdlog::level::level_enum target_level = spdlog::level::info;
            bool                      is_valid = false;

            if (eq_ignore_ascii_case(level_str, "trace")) {
                target_level = spdlog::level::trace;
                is_valid = true;
            } else if (eq_ignore_ascii_case(level_str, "debug")) {
                target_level = spdlog::level::debug;
                is_valid = true;
            } else if (eq_ignore_ascii_case(level_str, "info")) {
                target_level = spdlog::level::info;
                is_valid = true;
            } else if (eq_ignore_ascii_case(level_str, "warn")) {
                target_level = spdlog::level::warn;
                is_valid = true;
            } else if (eq_ignore_ascii_case(level_str, "error")) {
                target_level = spdlog::level::err;
                is_valid = true;
            } else if (eq_ignore_ascii_case(level_str, "critical")) {
                target_level = spdlog::level::critical;
                is_valid = true;
            } else if (eq_ignore_ascii_case(level_str, "off")) {
                target_level = spdlog::level::off;
                is_valid = true;
            }

            if (!is_valid) {
                add_warning(
                    diagnostics,
                    std::format("Invalid log_level: \"{}\". "
                                "Expected trace/debug/info/warn/error/critical/off. "
                                "Falling back to default: \"info\".",
                        level_str));

                target_level = spdlog::level::info;
                return target_level;
            }

            // For some reason, when I call `set_level` via `g_config` after `from_json`, the level isn't applied, so write it here.
            spdlog::set_level(target_level);
            SPDLOG_INFO("Log level initialized: {}", spdlog::level::to_string_view(target_level));

            return target_level;
        }

        /// Returns a required field or its fallback value.
        ///
        /// A missing field and a field with an invalid JSON type are reported
        /// as diagnostics instead of escaping from config parsing.
        template <typename T>
        T require_field(const nlohmann::json& j, std::string_view key, T default_val, std::string_view context, Diagnostics& diagnostics) {
            if (!j.contains(key)) {
                add_warning(diagnostics,
                    std::format(
                        "[{}] missing required field '{}', defaulting to '{}'",
                        context,
                        key,
                        default_val));

                return default_val;
            }

            try {
                return j.at(key).get<T>();
            } catch (const nlohmann::json::exception& e) {
                add_error(diagnostics,
                    std::format(
                        "[{}] invalid field '{}': {}. "
                        "Defaulting to '{}'.",
                        context,
                        key,
                        e.what(),
                        default_val));

                return default_val;
            }
        }

        /// Returns an optional field or its fallback value.
        ///
        /// A missing optional field is not a diagnostic because using the
        /// default value is expected behavior.
        template <typename T>
        T optional_field(const nlohmann::json& j, std::string_view key, T default_val, std::string_view context, Diagnostics& diagnostics) {
            if (!j.contains(key)) {
                return default_val;
            }

            try {
                return j.at(key).get<T>();
            } catch (const nlohmann::json::exception& e) {
                add_error(diagnostics,
                    std::format(
                        "[{}] invalid optional field '{}': {}. "
                        "Defaulting to '{}'.",
                        context, key, e.what(), default_val));

                return default_val;
            }
        }

        template <typename T>
        void assign_or_report_error(T& output, const std::expected<T, FNISVersionError>& result, std::string_view name, std::string_view input, Diagnostics& diagnostics) {
            if (result.has_value()) {
                output = result.value();
            } else {
                add_error(diagnostics, std::format("FNIS: failed to parse {}('{}'). {}", name, input, result.error().to_static_str()));
            }
        }
    }

    ParsedConfig ParsedConfig::from_json(const nlohmann::json& j) {
        ParsedConfig result;

        auto& r = result.config;
        auto& diagnostics = result.diagnostics;

        r.set_list.reserve(128);
        r.prefix_list.resize(30);

        r.log_level = spdlog_level_from_str(optional_field<std::string_view>(j, "log_level", "info", "<root>", diagnostics), diagnostics);
        r.crc = static_cast<int32_t>(require_field<uint32_t>(j, "crc", 0u, "<root>", diagnostics));

        r.version_str = require_field<std::string>(j, "fnis_version", "V07.06.00.0", "<root>", diagnostics);
        r.creature_version_str = require_field<std::string>(j, "fnis_creature_version", r.version_str, "<root>", diagnostics);

        {
            const auto version_result = FNISVersion::from_str(r.version_str);
            assign_or_report_error(r.version, version_result, "version", r.version_str, diagnostics);
        }

        {
            const auto creature_version_result = FNISVersion::from_str(r.creature_version_str);
            assign_or_report_error(r.creature_version, creature_version_result, "creature version", r.creature_version_str, diagnostics);
        }

        if (!j.contains("mods")) {
            add_warning(diagnostics, "parse_registry: missing 'mods' array.");
            return result;
        }

        if (!j["mods"].is_array()) {
            add_error(diagnostics, "parse_registry: 'mods' must be an array.");
            return result;
        }

        for (const auto& mod_json : j["mods"]) {
            if (!mod_json.is_object()) {
                add_error(diagnostics, "parse_registry: mod entry must be an object.");
                continue;
            }

            const auto mod_id = require_field<uint32_t>(mod_json, "mod_id", 0u, "<mod>", diagnostics);
            const auto prefix = require_field<std::string>(mod_json, "prefix", {}, "<mod>", diagnostics);

            const auto name = require_field<std::string>(mod_json, "name", {}, "<mod>", diagnostics);
            const auto ctx = std::format("mod '{}'", name);

            r.mod_count += 1;

            if (mod_id < 30) {
                r.prefix_list[mod_id] = prefix;
            } else {
                add_warning(diagnostics,
                    std::format(
                        "The original FNIS specification does not support "
                        "more than 30 mod prefixes. "
                        "Expanding prefix_list for mod_id {} with prefix '{}'.",
                        mod_id, prefix));

                r.prefix_list.emplace_back(prefix);
            }

            if (!mod_json.contains("groups")) {
                add_warning(diagnostics, std::format("[{}] missing 'groups' array.", ctx));
                continue;
            }

            if (!mod_json["groups"].is_array()) {
                add_error(diagnostics, std::format("[{}] 'groups' must be an array.", ctx));
                continue;
            }

            for (const auto& jg : mod_json["groups"]) {
                if (!jg.is_object()) {
                    add_error(diagnostics, std::format("[{}] group entry must be an object.", ctx));
                    continue;
                }

                const auto group_name = require_field<std::string>(jg, "name", {}, ctx, diagnostics);
                const auto base_val = require_field<uint32_t>(jg, "base", 0u, ctx, diagnostics);
                const auto info = GetAltGroup(group_name);

                if (info) {
                    const auto& group = info->get();
                    r.set_list.push_back({
                        .mod_id = static_cast<int32_t>(mod_id),
                        .group_id = group.id,
                        .base = static_cast<int32_t>(base_val),
                    });

                    r.set_count += 1;
                } else {
                    add_warning(diagnostics, std::format("[{}] unknown animation group '{}'.", ctx, group_name));
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

        return result;
    }
}
