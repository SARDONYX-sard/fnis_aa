#include "aa_registory.hh"

namespace FNIS_aa2 {

    /// Replaces `Int Function GetAAnumber(Int listType) Global`
    /// listType=0 → mod count, listType=1 → set count, listType=2 → crc
    static int32_t GetAAnumber(RE::StaticFunctionTag*, int32_t listType) {
        switch (listType) {
        case 0:
            {
                const auto count = static_cast<int32_t>(aa_registry::g_registry.mods.size());
                spdlog::debug("GetAAnumber(0) → mod count: {}", count);
                return count;
            }
        case 1:
            {
                const auto count = static_cast<int32_t>(aa_registry::g_registry.total_set_count());
                spdlog::debug("GetAAnumber(1) → set count: {}", count);
                return count;
            }
        default:
            {
                const auto crc = static_cast<int32_t>(aa_registry::g_registry.crc);
                spdlog::debug("GetAAnumber(2) → crc: {}", crc);
                return crc;
            }
        }
    }

    /// Replaces `String[] Function GetAAprefixList(Int nMods, String mod, Bool debugOutput) Global`
    static std::vector<RE::BSFixedString> GetAAprefixList(
        RE::StaticFunctionTag*,
        int32_t           nMods,
        RE::BSFixedString mod,
        bool              debugOutput) {
        spdlog::debug("GetAAprefixList(nMods={}, mod={})", nMods, mod.c_str());
        std::vector<RE::BSFixedString> result(30);
        for (auto& aa_mod : aa_registry::g_registry.mods) {
            if (aa_mod.mod_id < 30) {
                spdlog::debug("  [{}] prefix={}", aa_mod.mod_id, aa_mod.prefix);
                result[aa_mod.mod_id] = aa_mod.prefix.c_str();
            }
        }
        return result;
    }

    /// Replaces `String[] Function GetAAsetList(Int nSets, String mod, Bool debugOutput) Global`
    ///
    /// Each entry encodes PPGGBB → (mod_id * 10000) + (group_id * 100) + base
    static std::vector<RE::BSFixedString> GetAAsetList(
        RE::StaticFunctionTag*,
        int32_t           nSets,
        RE::BSFixedString mod,
        bool              debugOutput) {
        spdlog::debug("GetAAsetList(nSets={}, mod={})", nSets, mod.c_str());
        std::vector<RE::BSFixedString> result(128);
        size_t                         i = 0;
        for (auto& aa_mod : aa_registry::g_registry.mods) {
            for (auto& group : aa_mod.groups) {
                if (i >= 128) {
                    spdlog::warn("GetAAsetList: set count exceeded 128, truncating");
                    break;
                }
                const uint32_t encoded = aa_mod.mod_id * 10000 + group.group_id * 100 + group.base;
                spdlog::debug("  [{}] mod={} group={} encoded={:06}", i, aa_mod.prefix, group.name, encoded);
                result[i++] = std::to_string(encoded).c_str();
            }
        }
        return result;
    }

    // Replace `String Function get() Global`
    static RE::BSFixedString GetVersion(RE::StaticFunctionTag*) {
        return aa_registry::g_registry.fnis_version.c_str();
    }

    inline bool Register(RE::BSScript::IVirtualMachine* vm) {
        spdlog::info("Registering FNIS_aa2 native overrides");
        vm->RegisterFunction("GetAAnumber", "FNIS_aa2", GetAAnumber);
        vm->RegisterFunction("GetAAprefixList", "FNIS_aa2", GetAAprefixList);
        vm->RegisterFunction("GetAAsetList", "FNIS_aa2", GetAAsetList);

        vm->RegisterFunction("get", "FNISVersionGenerated", GetVersion);

        spdlog::info("FNIS_aa2 native overrides registered");
        return true;
    }

}  // namespace FNIS_aa
