#include "config.hh"

namespace FNIS_aa {
    namespace {
        using namespace ::config;

        /// use ref version function
        /// This is for Papyrus compatibility. (However, internally, I'd prefer to use references whenever possible.)
        ///
        /// - `animGroup`: `_1hmeqp`
        static bool SetAnimGroupEX_inner(
            RE::Actor*               ac,
            const RE::BSFixedString& animGroup,
            int32_t                  base,
            int32_t                  number,
            const RE::BSFixedString& mod,
            bool                     debugOutput,
            bool                     skipForce3D) {
            if (!ac || base < 0 || number < 0 || number > 9) {
                SPDLOG_WARN("SetAnimGroupEX: bad param mod={}", mod.c_str());
                return false;
            }

            if (ac == RE::PlayerCharacter::GetSingleton()) {
                if (!skipForce3D) {
                    RE::PlayerCamera::GetSingleton()->ForceThirdPerson();
                }
            } else if (!ac->Is3DLoaded()) {
                if (debugOutput)
                    spdlog::debug("SetAnimGroupEX: actor not loaded mod={}", mod.c_str());
                return false;
            }
            const int32_t value = (base > 0) ? base + number : base;

            SPDLOG_DEBUG("call SetGraphVariableInt(\"FNISaa{}\", base({}) + value({}))", animGroup.c_str(), base, number);
            SPDLOG_DEBUG("call SetGraphVariableInt(\"FNISaa_crc\", {})", g_config.crc);
            SPDLOG_DEBUG("call SetGraphVariableInt(\"FNISaa{}_crc\", {})", animGroup.c_str(), g_config.crc);

            ac->SetGraphVariableInt(std::format("FNISaa{}", animGroup.c_str()), value);
            ac->SetGraphVariableInt("FNISaa_crc", g_config.crc);
            ac->SetGraphVariableInt(std::format("FNISaa{}_crc", animGroup.c_str()), g_config.crc);
            return true;
        }

        // bool FNIS_aa.SetAnimGroup(actor, animGroup, base, number, mod, debugOutput)
        static bool SetAnimGroup(
            RE::StaticFunctionTag*,
            RE::Actor*        ac,
            RE::BSFixedString animGroup,
            int32_t           base,
            int32_t           number,
            RE::BSFixedString mod,
            bool              debugOutput) {
            SPDLOG_DEBUG("call SetAnimGroup({{ actor(FormID): {:X}, animGroup: \"{}\", base: {}, number: {}, mod: \"{}\", skipForce3D: false }})",
                ac ? ac->GetFormID() : 0, animGroup.c_str(), base, number, mod.c_str());
            return SetAnimGroupEX_inner(ac, animGroup, base, number, mod, debugOutput, false);
        }

        // bool FNIS_aa.SetAnimGroupEX(actor, animGroup, base, number, mod, debugOutput, skipForce3D)
        static bool SetAnimGroupEX(
            RE::StaticFunctionTag*,
            RE::Actor*        ac,
            RE::BSFixedString animGroup,
            int32_t           base,
            int32_t           number,
            RE::BSFixedString mod,
            bool              debugOutput,
            bool              skipForce3D) {
            SPDLOG_DEBUG("call SetAnimGroupEX {{ actor(FormID): {:X}, animGroup: \"{}\", base: {}, number: {}, mod: \"{}\", skipForce3D: {} }}",
                ac ? ac->GetFormID() : 0, animGroup.c_str(), base, number, mod.c_str(), skipForce3D);
            return SetAnimGroupEX_inner(ac, animGroup, base, number, mod, debugOutput, skipForce3D);
        }

        /// `int FNIS_aa.GetAAmodID(string myAAprefix, string mod, bool debugOutput) global`
        ///
        /// # Requirements specified by FNIS
        /// - If not present, return -1.
        static int32_t GetAAmodID(
            RE::StaticFunctionTag*,
            RE::BSFixedString prefix,
            RE::BSFixedString mod,
            bool) {
            const auto& list = g_config.prefix_list;
            for (int32_t i = 0; i < static_cast<int32_t>(list.size()); ++i) {
                if (list[i] == prefix) {
                    SPDLOG_DEBUG("GetAAmodID mod={} prefix={} id={}", mod.c_str(), prefix.c_str(), i);
                    return i;
                }
            }
            SPDLOG_WARN("GetAAmodID: prefix '{}' not found for mod '{}'", prefix.c_str(), mod.c_str());
            return -1;
        }

        // int FNIS_aa.GetGroupBaseValue(int AAmodID, int AAgroupID, string mod, bool debugOutput)
        /// Requirements specified by FNIS
        /// - If not found or invalid value, return 0.
        static int32_t GetGroupBaseValue(
            RE::StaticFunctionTag*,
            int32_t           mod_id,
            int32_t           group_id,
            RE::BSFixedString mod,
            bool) {
            SPDLOG_DEBUG("call GetGroupBaseValue(mod_id={}, group_id={}, mod=\"{}\")", mod_id, group_id, mod.c_str());

            if (mod_id < 0 || mod_id > 29) {
                SPDLOG_WARN("Wrong range arg's mod_id: expected 0 <= value <= 29, but got {}. So return 0", mod_id);
                return 0;
            }
            if (group_id < 0 || group_id > 53) {
                SPDLOG_WARN("Wrong range arg's group_id: expected 0 <= value <= 53, but got {}. So return 0", group_id);
                return 0;
            }

            for (const auto& set : g_config.set_list) {
                if (set.group_id == group_id) {
                    if (set.mod_id == mod_id)
                        return set.base;
                } else if (set.group_id > group_id) {
                    break;
                }
            }

            SPDLOG_WARN("GetGroupBaseValue: not found mod={} mod_id={} group_id={}", mod.c_str(), mod_id, group_id);
            return 0;
        }

        // int[] FNIS_aa.GetAllGroupBaseValues(int AAmodID, string mod, bool debugOutput)
        static std::vector<int32_t> GetAllGroupBaseValues(
            RE::StaticFunctionTag*,
            int32_t           mod_id,
            RE::BSFixedString mod,
            bool              debugOutput) {
            std::vector<int32_t> result(54, 0);  // == vec![0; 54];
            SPDLOG_WARN("GetAllGroupBaseValues(mod_id={}, mod=\"{}\", debugOutput={})", mod_id, mod.c_str(), debugOutput);

            if (mod_id < 0 || mod_id > 29) {
                SPDLOG_WARN("Invalid mod_id arg: expected 0 <= value <= 29, but got {}. Return default", mod_id);
                return result;
            }

            for (const auto& set : g_config.set_list) {
                if (set.mod_id == mod_id) {
                    if (set.group_id >= 0 && set.group_id < 54) {
                        result[set.group_id] = set.base;
                        SPDLOG_DEBUG("  [Match] group={} base={}", set.group_id, set.base);
                    } else {
                        SPDLOG_WARN("GetAllGroupBaseValues: internal data error (mod=\"{}\")", mod.c_str());
                        SPDLOG_WARN("  -> Invalid group_id in set_list: expected 0 <= value < 54, but got {}", set.group_id);
                    }
                }
            }

            return result;
        }

        // int FNIS_aa.GetInstallationCRC()
        static int32_t GetInstallationCRC(RE::StaticFunctionTag*) {
            return g_config.crc;
        }

        // void FNIS_aa.GetAAsets(int nSets, int[] GroupId, int[] ModId, int[] Base, int[] Index, string mod, bool debugOutput)
        static void GetAAsets(
            RE::StaticFunctionTag*,
            int32_t                            n_sets,
            RE::BSScript::reference_array<int> group_id,  // Output
            RE::BSScript::reference_array<int> mod_id,    // Output
            RE::BSScript::reference_array<int> base,      // Output
            RE::BSScript::reference_array<int> index,     // Output
            RE::BSFixedString                  mod,
            bool) {
            SPDLOG_DEBUG("call GetAAsets(mod=\"{}\", n_sets={})", mod.c_str(), n_sets);

            const auto& set_list = g_config.set_list;
            int32_t     last_group = -1;

            auto limit = std::min<size_t>({ static_cast<size_t>(n_sets),
                set_list.size(),
                group_id.size(),
                mod_id.size(),
                base.size() });

            for (size_t i = 0; i < limit; ++i) {
                const auto& src = set_list[i];

                group_id[i] = src.group_id;
                mod_id[i] = src.mod_id;
                base[i] = src.base;

                if (src.group_id != last_group) {
                    last_group = src.group_id;
                    if (static_cast<size_t>(src.group_id) < index.size()) {
                        index[src.group_id] = (static_cast<int32_t>(i + 1));
                    }
                }
            }
        }
    }

    bool Register(RE::BSScript::IVirtualMachine* vm) {
        SPDLOG_INFO("Registering FNIS_aa native...");

        vm->RegisterFunction("SetAnimGroup", "FNIS_aa", SetAnimGroup);
        vm->RegisterFunction("SetAnimGroupEX", "FNIS_aa", SetAnimGroupEX);
        vm->RegisterFunction("GetAAmodID", "FNIS_aa", GetAAmodID);
        vm->RegisterFunction("GetGroupBaseValue", "FNIS_aa", GetGroupBaseValue);
        vm->RegisterFunction("GetAllGroupBaseValues", "FNIS_aa", GetAllGroupBaseValues);
        vm->RegisterFunction("GetInstallationCRC", "FNIS_aa", GetInstallationCRC);
        vm->RegisterFunction("GetAAsets", "FNIS_aa", GetAAsets);

        SPDLOG_INFO("FNIS_aa native overrides registered");
        return true;
    }
}
