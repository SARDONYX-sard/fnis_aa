#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

namespace aa_registry {
    void OnLoad();

    struct AAGroup {
        std::string name;
        uint32_t    group_id;
        uint32_t    base;
        uint32_t    slot_count;
    };

    struct AAMod {
        std::string          prefix;
        std::string          name;
        uint32_t             mod_id;
        std::vector<AAGroup> groups;
    };

    class AARegistry {
    public:
        std::vector<AAMod> mods;
        uint32_t           crc{ 0 };
        std::string        fnis_version{ "V07.06.00.0" };  // fallback default

        bool LoadFromJson(const std::filesystem::path& path);

        // Returns -1 if not found
        [[nodiscard]] int32_t GetModID(std::string_view prefix) const;

        // Returns 0 (vanilla) if not found
        [[nodiscard]] uint32_t GetGroupBase(uint32_t mod_id, uint32_t group_id) const;

        [[nodiscard]] size_t total_set_count() const {
            size_t n = 0;
            for (auto& m : mods) n += m.groups.size();
            return n;
        }

    private:
        // mod_id → group_id → base
        std::unordered_map<uint32_t,
            std::unordered_map<uint32_t, uint32_t>>
            lookup_;

        void BuildLookup();
    };

    inline AARegistry g_registry;
}
