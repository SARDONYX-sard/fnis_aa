#include "config.hh"

namespace FNIS_aa2 {
    namespace {
        using namespace ::config;

        /// Replaces `Int Function GetAAnumber(Int listType) Global`
        /// listType=0 -> mod count, listType=1 -> set count, listType=2 -> crc
        int32_t GetAAnumber(RE::StaticFunctionTag*, int32_t listType) {
            switch (listType) {
            case 0:
                SPDLOG_DEBUG("GetAAnumber({}) -> Mod count: {}", listType, g_config.mod_count);
                return g_config.mod_count;
            case 1:
                SPDLOG_DEBUG("GetAAnumber({}) -> Set count: {}", listType, g_config.set_count);
                return g_config.set_count;
            default:
                SPDLOG_DEBUG("GetAAnumber({}) -> crc: {}", listType, g_config.crc);
                return g_config.crc;
            }
        }

        /// Replaces `String[] Function GetAAprefixList(Int nMods, String mod, Bool debugOutput) Global`
        /// How FNIS Templates Work
        /// - The array from `mm0` to `mm30` is the default value.
        /// - Every time an FNIS Alternate Animation mod is added, it replaces the values starting from `mm0`.
        static std::vector<RE::BSFixedString> GetAAprefixList(
            RE::StaticFunctionTag*, int32_t nMods, RE::BSFixedString mod, bool debugOutput) {
            SPDLOG_DEBUG("GetAAprefixList(nMods={}, mod={}, debugOutput={}) has been called.",
                nMods, mod.c_str(), debugOutput);
            return g_config.prefix_list;
        }

        /// Replaces `String[] Function GetAAsetList(Int nSets, String mod, Bool debugOutput) Global`
        ///
        /// Each entry encodes PPGGBB as a 6-digit decimal string:
        ///   PP = mod_id  (2 digits)
        ///   GG = group_id from kAltGroupTable (2 digits)
        ///   BB = base slot index (2 digits)
        ///
        /// IMPORTANT: The list MUST be sorted by group_id (GG) in ascending order.
        /// FNIS_aa.GetGroupBaseValue() relies on this ordering for early exit:
        ///
        ///   elseif ( Group > AAgroupID )
        ///       i = nSets   ; stop searching
        ///
        /// If the list is unsorted, any group_id lower than a previously seen entry
        /// will be silently missed and GetGroupBaseValue() returns 0 (vanilla slot),
        /// causing all animation variable sets for that group to be no-ops.
        static std::vector<RE::BSFixedString> GetAAsetList(
            RE::StaticFunctionTag*, int32_t nSets, RE::BSFixedString mod, bool debugOutput) {
            SPDLOG_DEBUG("GetAAsetList(nSets={}, mod={}, debugOutput={}) has been called.",
                nSets, mod.c_str(), debugOutput);

            std::vector<RE::BSFixedString> result(128);
            const auto&                    sets = g_config.set_list;

            const size_t count = std::min<size_t>(nSets, sets.size());
            result.reserve(count);

            for (size_t i = 0; i < count; ++i) {
                // e.g.,: mod:1, group:2, base:3 -> `010203`
                result.emplace_back(sets[i].to_encoded_string());
            }

            return result;
        }

        // Replace `String Function get() Global`
        static RE::BSFixedString GetVersion(RE::StaticFunctionTag*) {
            return g_config.version_str;
        }
    }

    bool Register(RE::BSScript::IVirtualMachine* vm) {
        SPDLOG_INFO("Registering FNIS_aa2 native.");

        vm->RegisterFunction("GetAAnumber", "FNIS_aa2", GetAAnumber);
        vm->RegisterFunction("GetAAprefixList", "FNIS_aa2", GetAAprefixList);
        vm->RegisterFunction("GetAAsetList", "FNIS_aa2", GetAAsetList);

        vm->RegisterFunction("Get", "FNISVersion", GetVersion);
        vm->RegisterFunction("Get", "FNISVersionGenerated", GetVersion);

        SPDLOG_INFO("FNIS_aa2 native overrides registered");
        return true;
    }
}  // namespace FNIS_aa
