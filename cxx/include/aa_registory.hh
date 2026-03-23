#pragma once
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace aa_registry {
    void OnLoad();

    struct AAGroup {
        std::string name;
        uint32_t    base;
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

        [[nodiscard]] size_t total_set_count() const {
            size_t n = 0;
            for (auto& m : mods) n += m.groups.size();
            return n;
        }
    };

    inline AARegistry g_registry;
}
