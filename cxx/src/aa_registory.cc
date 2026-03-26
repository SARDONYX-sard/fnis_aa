#include "alt_group_table.hh"

#include "aa_registory.hh"

namespace aa_registry {
    namespace {
        // Helper to get a required field with a warn fallback
        template <typename T>
        T require_field(const nlohmann::json& j, std::string_view key, T default_val,
            std::string_view context) {
            if (!j.contains(key)) {
                spdlog::warn("[{}] missing required field '{}', defaulting to '{}'",
                    context, key, default_val);
                return default_val;
            }
            return j[key].get<T>();
        }
    }

    ParsedConfig parse_config(const nlohmann::json& j) {
        ParsedConfig r;

        r.crc = static_cast<int32_t>(require_field<uint32_t>(j, "crc", 0u, "<root>"));
        r.version = require_field<std::string>(j, "fnis_version", "V07.06.00.0", "<root>");

        if (!j.contains("mods") || !j["mods"].is_array()) {
            SPDLOG_WARN("parse_registry: missing or invalid 'mods' array");
            return r;
        }

        struct Entry {
            uint32_t group_id, encoded;
        };
        std::vector<Entry> set_entries;
        set_entries.reserve(128);

        for (const auto& jmod : j["mods"]) {
            const auto mod_id = require_field<uint32_t>(jmod, "mod_id", 0u, "<mod>");
            const auto prefix = require_field<std::string>(jmod, "prefix", {}, "<mod>");
            const auto name = require_field<std::string>(jmod, "name", {}, "<mod>");
            const auto ctx = std::format("mod '{}'", name);

            r.mod_count++;
            if (mod_id < 30) {
                r.prefix_list[mod_id] = prefix;
            }

            if (!jmod.contains("groups") || !jmod["groups"].is_array()) {
                SPDLOG_WARN("[{}] missing or invalid 'groups' array", ctx);
                continue;
            }
            for (const auto& jg : jmod["groups"]) {
                const auto  gname = require_field<std::string>(jg, "name", {}, ctx);
                const auto  base = require_field<uint32_t>(jg, "base", 0u,
                     std::format("{} group '{}'", ctx, gname));
                const auto* info = GetAltGroup(gname);
                if (!info) {
                    SPDLOG_WARN("[{}] unknown group '{}', skipping", ctx, gname);
                    continue;
                }
                set_entries.push_back({
                    .group_id = info->id,
                    .encoded = mod_id * 10000 + info->id * 100 + base,
                });
                r.set_count++;
            }
        }

        std::ranges::sort(set_entries, {}, &Entry::group_id);
        for (std::size_t i = 0; i < set_entries.size() && i < 128; ++i) {
            r.set_list[i] = std::format("{:06}", set_entries[i].encoded);
        }

        return r;
    }
}
