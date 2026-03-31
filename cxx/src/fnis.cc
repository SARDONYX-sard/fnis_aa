#include "config.hh"

namespace FNIS {

    namespace {
        using namespace ::config;

        // ---------------------------------------------------------------------------
        // AAReport
        // ---------------------------------------------------------------------------

        static void AAReport_impl(
            const std::string& long_report,
            const std::string& short_report,
            int32_t            aa_debug,
            bool               is_error) {
            if (aa_debug >= 1) {
                if (!long_report.empty()) {
                    SPDLOG_WARN("LONG REPORT:{}", long_report);
                }
                if (aa_debug >= 2 && !short_report.empty()) {
                    RE::SendHUDMessage::ShowHUDMessage(short_report.c_str());
                    if (aa_debug == 3 && is_error) {
                        RE::DebugMessageBox(long_report.c_str());
                    }
                }
            }
        }

        // void FNIS.AAReport(string longReport, string shortReport, int AAdebug, bool isError)
        static void AAReport(
            RE::StaticFunctionTag*,
            RE::BSFixedString long_report,
            RE::BSFixedString short_report,
            int32_t           aa_debug,
            bool              is_error) {
            AAReport_impl(long_report.c_str(), short_report.c_str(), aa_debug, is_error);
        }

        // ---------------------------------------------------------------------------
        // set_AACondition
        // ---------------------------------------------------------------------------

        static int32_t SetAACondition(
            RE::StaticFunctionTag*,
            RE::Actor*        ac,
            RE::BSFixedString aa_type,
            RE::BSFixedString mod,
            int32_t           aa_cond,
            int32_t           aa_debug) {
            SPDLOG_DEBUG("call set_AACondition(actor={}, aa_type='{}', mod='{}', aa_cond={}, aa_debug={})",
                ac ? ac->GetName() : "(null)", aa_type.c_str(), mod.c_str(), aa_cond, aa_debug);
            if (!ac) {
                SPDLOG_WARN("set_AACondition: null actor (mod='{}'), return -1", mod.c_str());
                return -1;
            }

            const std::string actor_name =
                ac->GetBaseObject() ? ac->GetBaseObject()->GetName() : "(unknown)";

            // ERROR 1: 3D load check
            if (ac == RE::PlayerCharacter::GetSingleton()) {
                RE::PlayerCamera::GetSingleton()->ForceThirdPerson();
            } else if (!ac->Is3DLoaded()) {
                AAReport_impl(
                    std::format("FNIS AA ERROR(mod={}): expected actor '{}' to be 3D-loaded, but it was not",
                        mod.c_str(), actor_name),
                    "", aa_debug, true);
                return -1;
            }

            // ERROR 2: AA type
            int32_t i_aa = 0;
            if (std::string_view(aa_type) == "mt_loco_forward") {
                i_aa = 1;
            } else if (std::string_view(aa_type) == "mt_locomotion") {
                i_aa = 2;
            } else {
                AAReport_impl(
                    std::format("FNIS AA ERROR(mod={}): expected aa_type to be one of "
                                "['mt_loco_forward', 'mt_locomotion'], but got '{}'",
                        mod.c_str(), aa_type.c_str()),
                    "", aa_debug, true);
                return -2;
            }

            // ERROR 3: condition range
            if (aa_cond < 0 || aa_cond > 10) {
                AAReport_impl(
                    std::format("FNIS AA ERROR(mod={}): expected aa_cond in range [0, 10], but got {}", mod.c_str(), aa_cond),
                    "", aa_debug, true);
                return -3;
            }

            // ERROR 4: mod base variable
            const auto mod_var = std::format("FNISvaa_{}", mod.c_str());
            const auto aa_var = std::format("FNISvaa{}", i_aa);

            int32_t i_base = 0;
            ac->GetGraphVariableInt(mod_var, i_base);
            if (i_base <= 0) {
                AAReport_impl(
                    std::format("FNIS AA ERROR(mod={}): expected AnimVar '{}' > 0 on actor '{}', but got {} ", mod.c_str(), mod_var, actor_name, i_base),
                    "", aa_debug, true);
                return -4;
            }

            // Write
            const int32_t i_set = (aa_cond > 0) ? (i_base + aa_cond) : 0;
            ac->SetGraphVariableInt(aa_var, i_set);

            // ERROR 0: verify write succeeded
            int32_t i_result = 0;
            ac->GetGraphVariableInt(aa_var, i_result);
            if (i_set != i_result) {
                AAReport_impl(
                    std::format("FNIS AA ERROR(mod={}): expected AnimVar '{}' == {} after write, but got {} "
                                "(graph may have rejected the value)",
                        mod.c_str(), aa_var, i_set, i_result),
                    "", aa_debug, true);
                return 0;
            }

            AAReport_impl(
                std::format("FNIS AA(mod={}): set AnimVar '{}' = {} on actor '{}'",
                    mod.c_str(), aa_var, i_set, actor_name),
                std::format("{}: {} {} {}", mod.c_str(), actor_name, aa_var, i_set),
                aa_debug, false);

            return i_set;
        }

        /// NOTE: Papyrus uses two distinct globals: FNISVersion and FNISVersionGenerated.
        /// `bool FNIS.IsGenerated()`
        static bool IsGenerated(RE::StaticFunctionTag*) {
            SPDLOG_DEBUG("call IsGenerated() -> true");
            return true;  // By json. Always return true.
        }

        // string FNIS.VersionToString(bool abCreature = false)
        static RE::BSFixedString VersionToString(RE::StaticFunctionTag*, bool creature) {
            SPDLOG_DEBUG("call VersionToString(creature={})", creature);
            return creature ? g_config.creature_version_str : g_config.version_str;
        }

        // ---------------------------------------------------------------------------
        // VersionCompare
        // ---------------------------------------------------------------------------

        // int FNIS.VersionCompare(int iCompMajor, int iCompMinor1, int iCompMinor2, bool abCreature)
        //   0  = match
        //   1  = installed is newer
        //  -1  = installed is older / not present
        static int32_t VersionCompare(
            RE::StaticFunctionTag*,
            int32_t comp_major,
            int32_t comp_minor1,
            int32_t comp_minor2,
            bool    creature) {
            SPDLOG_DEBUG("call VersionCompare(comp_major={}, comp_minor1={}, comp_minor2={}, creature={})",
                comp_major, comp_minor1, comp_minor2, creature);
            const auto& v = creature ? g_config.creature_version : g_config.version;
            if (v.flags == 3)
                return -1;  // not installed

            if (v.major != comp_major)
                return v.major > comp_major ? 1 : -1;
            if (v.minor1 != comp_minor1)
                return v.minor1 > comp_minor1 ? 1 : -1;
            if (v.minor2 != comp_minor2)
                return v.minor2 > comp_minor2 ? 1 : -1;
            return 0;
        }

        // ---------------------------------------------------------------------------
        // GetMajor / GetMinor1 / GetMinor2 / GetFlags / IsRelease
        // ---------------------------------------------------------------------------

        /// int FNIS.GetMajor(bool abCreature = false)
        static int32_t GetMajor(RE::StaticFunctionTag*, bool creature) {
            SPDLOG_DEBUG("call GetMajor(creature={})", creature);
            return creature ? g_config.creature_version.major : g_config.version.major;
        }

        /// int FNIS.GetMinor1(bool abCreature = false)
        static int32_t GetMinor1(RE::StaticFunctionTag*, bool creature) {
            SPDLOG_DEBUG("call GetMinor1(creature={})", creature);
            return creature ? g_config.creature_version.minor1 : g_config.version.minor1;
        }

        /// int FNIS.GetMinor2(bool abCreature = false)
        static int32_t GetMinor2(RE::StaticFunctionTag*, bool creature) {
            SPDLOG_DEBUG("call GetMinor2(creature={})", creature);
            return creature ? g_config.creature_version.minor2 : g_config.version.minor2;
        }

        inline static int32_t GetFlagsInner(bool creature) {
            return creature ? g_config.creature_version.flags : g_config.version.flags;
        }

        /// int FNIS.GetFlags(bool abCreature = false)
        static int32_t GetFlags(RE::StaticFunctionTag*, bool creature) {
            SPDLOG_DEBUG("call GetFlags(creature={})", creature);
            return GetFlagsInner(creature);
        }

        /// bool FNIS.IsRelease(bool abCreature = false)
        static bool IsRelease(RE::StaticFunctionTag*, bool creature) {
            SPDLOG_DEBUG("call IsRelease(creature={})", creature);
            return GetFlagsInner(creature) == 0;
        }
    }  // anonymous namespace

    // ---------------------------------------------------------------------------
    // Registration
    // ---------------------------------------------------------------------------

    bool Register(RE::BSScript::IVirtualMachine* vm) {
        SPDLOG_INFO("Registering FNIS native.");

        vm->RegisterFunction("set_AACondition", "FNIS", SetAACondition);
        vm->RegisterFunction("AAReport", "FNIS", AAReport);
        vm->RegisterFunction("IsGenerated", "FNIS", IsGenerated);
        vm->RegisterFunction("VersionToString", "FNIS", VersionToString);
        vm->RegisterFunction("VersionCompare", "FNIS", VersionCompare);
        vm->RegisterFunction("GetMajor", "FNIS", GetMajor);
        vm->RegisterFunction("GetMinor1", "FNIS", GetMinor1);
        vm->RegisterFunction("GetMinor2", "FNIS", GetMinor2);
        vm->RegisterFunction("GetFlags", "FNIS", GetFlags);
        vm->RegisterFunction("IsRelease", "FNIS", IsRelease);

        SPDLOG_INFO("FNIS native overrides registered");
        return true;
    }

}  // namespace FNIS
