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

    void OnLoad() {
        std::filesystem::path dir = "Data/SKSE/Plugins/fnis_aa";
        const auto            config_path = dir / "config.json";

        if (g_registry.LoadFromJson(config_path)) {
            spdlog::info("json loaded.");
        } else {
            spdlog::error("Failed to load json from: {}", config_path.string());
        }
    }

    bool AARegistry::LoadFromJson(const std::filesystem::path& path) {
        std::ifstream f(path);
        if (!f.is_open()) {
            spdlog::warn("Failed to open: {}", path.string());
            return false;
        }

        auto j = nlohmann::json::parse(f, nullptr, false);
        if (j.is_discarded()) {
            spdlog::error("Failed to parse JSON: {}", path.string());
            return false;
        }

        crc = require_field<uint32_t>(j, "crc", 0u, "<root>");
        fnis_version = require_field<std::string>(j, "fnis_version", "V07.06.00.0", "<root>");

        if (!j.contains("mods") || !j["mods"].is_array()) {
            spdlog::warn("<root> missing or invalid 'mods' array");
            return false;
        }

        mods.clear();
        for (auto& jmod : j["mods"]) {
            AAMod mod;
            mod.prefix = require_field<std::string>(jmod, "prefix", std::string{}, "<mod>");
            mod.name = require_field<std::string>(jmod, "name", std::string{}, "<mod>");
            mod.mod_id = require_field<uint32_t>(jmod, "mod_id", 0u, mod.name);

            const auto ctx = std::format("mod '{}'", mod.name);
            if (!jmod.contains("groups") || !jmod["groups"].is_array()) {
                spdlog::warn("[{}] missing or invalid 'groups' array", ctx);
                mods.push_back(std::move(mod));
                continue;
            }

            for (auto& jg : jmod["groups"]) {
                AAGroup g{
                    .name = require_field<std::string>(jg, "name", std::string{}, ctx),
                    .base = require_field<uint32_t>(jg, "base", 0u, std::format("{} group '{}'", ctx, g.name)),
                };
                mod.groups.push_back(std::move(g));
            }

            mods.push_back(std::move(mod));
        }

        spdlog::info("Loaded {} mod(s) from: {}", mods.size(), path.string());
        return true;
    }

}
