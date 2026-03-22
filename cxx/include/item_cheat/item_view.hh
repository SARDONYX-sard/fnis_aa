#pragma once

#include <SKSEMenuFramework.h>

namespace item_cheat::view {
    inline static RE::FormType selectedFormType = RE::FormType::None;

    static constexpr std::pair<RE::FormType, const char*> kFormTypeList[] = {
        { RE::FormType::None, "All" },
        { RE::FormType::Weapon, "Weapon" },
        { RE::FormType::Armor, "Armor" },
        { RE::FormType::Ammo, "Ammo" },
        { RE::FormType::Ingredient, "Ingredient" },
        { RE::FormType::AlchemyItem, "Alchemy" },
        { RE::FormType::Book, "Book" },
        { RE::FormType::Scroll, "Scroll" },
        { RE::FormType::KeyMaster, "Key" },
        { RE::FormType::Misc, "Misc" },

        { RE::FormType::NPC, "NPC" },
    };

    // Unlike the CommonLibSSE-NG implementation, set `None` to `All`.
    inline const char* FormTypeToString(RE::FormType type) {
        for (auto& [t, name] : kFormTypeList) {
            if (t == type) {
                return name;
            }
        }
        return "All";
    }
    inline RE::FormType StringToFormType(const std::string& str) {
        for (auto& [t, name] : kFormTypeList) {
            if (str == name) {
                return t;
            }
        }
        return RE::FormType::None;
    }

    constexpr const char* ToFormTypeName(RE::FormType type) {
        for (auto& [t, name] : kFormTypeList) {
            if (t == type) {
                return name;
            }
        }
        return "Unknown";
    }

    void Render();

    // --- UI Filters / state ---
    inline std::array<char, 100> filterPlugin{};
    inline std::array<char, 50>  filterFormType{};
    inline std::array<char, 256> filterFormList{};

    inline std::atomic_int64_t selectedPlugin{ -1 };
    inline int                 addCount{ 1 };
    inline float               tableHeight{ 400.0f };
}
