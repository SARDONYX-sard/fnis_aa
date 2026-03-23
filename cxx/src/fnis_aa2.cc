#include "aa_registory.hh"

#include "./alt_group_table.hh"

namespace FNIS_aa2 {
    namespace {
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
        /// How FNIS Templates Work
        /// - The array from `mm0` to `mm30` is the default value.
        /// - Every time an FNIS Alternate Animation mod is added, it replaces the values starting from `mm0`.
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
            RE::StaticFunctionTag*,
            int32_t           nSets,
            RE::BSFixedString mod,
            bool              debugOutput) {
            spdlog::debug("GetAAsetList(nSets={}, mod={})", nSets, mod.c_str());

            // Collect all entries first, then sort by group_id
            struct Entry {
                uint32_t    group_id;
                uint32_t    encoded;
                std::string mod_name;
                std::string group_name;
            };
            std::vector<Entry> entries;
            // The Papyrus String[] type has a fixed maximum length, declared as String[128]
            // in FNIS_aa2.pex. The actual number of populated entries is nSets, passed in
            // by the caller. Entries beyond nSets are ignored by all consumers.
            // In practice nSets = total number of (mod, group) registrations across all mods,
            // which is well below 128 in real-world setups (e.g. 3 mods → 19 entries).
            //
            // default: `010100`..=`010227`
            entries.reserve(128);

            for (auto& aa_mod : aa_registry::g_registry.mods) {
                for (auto& group : aa_mod.groups) {
                    const auto* info = GetAltGroup(group.name);
                    if (!info) {
                        spdlog::warn("GetAAsetList: unknown group '{}', skipping", group.name);
                        continue;
                    }
                    entries.push_back({
                        .group_id = info->id,
                        .encoded = aa_mod.mod_id * 10000 + info->id * 100 + group.base,
                        .mod_name = aa_mod.prefix,
                        .group_name = group.name,
                    });
                }
            }

            std::ranges::sort(entries, {}, &Entry::group_id);

            std::vector<RE::BSFixedString> result(128);
            for (std::size_t i = 0; i < entries.size() && i < 128; ++i) {
                auto& e = entries[i];
                spdlog::debug("  [{}] mod={} group={} id={} encoded={:06}",
                    i, e.mod_name, e.group_name, e.group_id, e.encoded);
                result[i] = std::to_string(e.encoded).c_str();
            }
            return result;
        }

        // Replace `String Function get() Global`
        static RE::BSFixedString GetVersion(RE::StaticFunctionTag*) {
            return aa_registry::g_registry.fnis_version.c_str();
        }

    }

    bool Register(RE::BSScript::IVirtualMachine* vm) {
        spdlog::info("Registering FNIS_aa2 native overrides");
        vm->RegisterFunction("GetAAnumber", "FNIS_aa2", GetAAnumber);
        vm->RegisterFunction("GetAAprefixList", "FNIS_aa2", GetAAprefixList);
        vm->RegisterFunction("GetAAsetList", "FNIS_aa2", GetAAsetList);

        vm->RegisterFunction("get", "FNISVersionGenerated", GetVersion);

        spdlog::info("FNIS_aa2 native overrides registered");
        return true;
    }

}  // namespace FNIS_aa
