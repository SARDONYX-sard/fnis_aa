#include "aa_registory.hh"

namespace aa_registry {
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
        if (!f.is_open())
            return false;

        auto j = nlohmann::json::parse(f, nullptr, false);
        if (j.is_discarded())
            return false;

        crc = j.value("crc", 0u);
        fnis_version = j.value("fnis_version", "V07.06.00.0");
        mods.clear();

        for (auto& jmod : j["mods"]) {
            AAMod mod;
            mod.prefix = jmod["prefix"];
            mod.name = jmod["name"];
            mod.mod_id = jmod["mod_id"];

            for (auto& jg : jmod["groups"]) {
                AAGroup g;
                g.name = jg["name"];
                g.group_id = jg["group_id"];
                g.base = jg["base"];
                g.slot_count = jg["slot_count"];
                mod.groups.push_back(std::move(g));
            }
            mods.push_back(std::move(mod));
        }

        BuildLookup();
        return true;
    }

    void AARegistry::BuildLookup() {
        lookup_.clear();
        for (auto& mod : mods)
            for (auto& g : mod.groups)
                lookup_[mod.mod_id][g.group_id] = g.base;
    }

    int32_t AARegistry::GetModID(std::string_view prefix) const {
        for (auto& mod : mods)
            if (mod.prefix == prefix)
                return static_cast<int32_t>(mod.mod_id);
        return -1;
    }

    uint32_t AARegistry::GetGroupBase(uint32_t mod_id, uint32_t group_id) const {
        auto it = lookup_.find(mod_id);
        if (it == lookup_.end())
            return 0;
        auto it2 = it->second.find(group_id);
        return (it2 != it->second.end()) ? it2->second : 0;
    }

}
