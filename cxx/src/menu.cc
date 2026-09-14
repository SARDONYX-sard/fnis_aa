#include <SKSEMenuFramework.h>

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "alt_group_table.hh"
#include "config.hh"

namespace fnis_aa::menu {

    namespace {
        using namespace ImGuiMCP;

        constexpr std::string_view kSection = "Dyn FNIS AA Functions";

        struct ModEntry {
            int32_t     mod_id;
            std::string prefix;
        };

        struct SetEntry {
            size_t      index;
            int32_t     mod_id;
            int32_t     group_id;
            int32_t     base;
            std::string prefix;
            std::string group;
        };

        struct Snapshot {
            int32_t     crc = 0;
            int32_t     mod_count = 0;
            int32_t     set_count = 0;
            std::string version;
            std::string creature_version;

            std::vector<ModEntry> mods;
            std::vector<SetEntry> sets;
        };

        Snapshot g_snapshot;

        [[nodiscard]] std::string_view group_name(int32_t group_id) noexcept {
            for (const auto& group : ALT_GROUP_TABLE) {
                if (group.id == group_id) {
                    return group.name;
                }
            }
            return {};
        }

        [[nodiscard]] const char* diagnostic_label(
            config::DiagnosticLevel level) noexcept {
            switch (level) {
            case config::DiagnosticLevel::info:
                return "INFO";
            case config::DiagnosticLevel::success:
                return "SUCCESS";
            case config::DiagnosticLevel::warning:
                return "WARN";

            case config::DiagnosticLevel::error:
                return "ERROR";
            }

            return "UNKNOWN";
        }

        [[nodiscard]] ImGuiMCP::ImVec4 diagnostic_color(config::DiagnosticLevel level) noexcept {
            constexpr auto rgba = [](std::uint32_t color) noexcept {
                return ImGuiMCP::ImVec4{
                    .x = static_cast<float>((color >> 24) & 0xFF) / 255.0f,
                    .y = static_cast<float>((color >> 16) & 0xFF) / 255.0f,
                    .z = static_cast<float>((color >> 8) & 0xFF) / 255.0f,
                    .w = static_cast<float>(color & 0xFF) / 255.0f,
                };
            };

            constexpr ImGuiMCP::ImVec4 SUCCESS_COLOR = rgba(0x4DD95AFF);
            constexpr ImGuiMCP::ImVec4 INFO_COLOR = rgba(0x59A6F2FF);
            constexpr ImGuiMCP::ImVec4 WARN_COLOR = rgba(0xF2BF33FF);
            constexpr ImGuiMCP::ImVec4 error_COLOR = rgba(0xF24040FF);

            switch (level) {
            case config::DiagnosticLevel::success:
                return SUCCESS_COLOR;

            case config::DiagnosticLevel::info:
                return INFO_COLOR;

            case config::DiagnosticLevel::warning:
                return WARN_COLOR;

            case config::DiagnosticLevel::error:
                return error_COLOR;
            }

            return rgba(0xFFFFFFFF);
        }

        inline void draw_status(config::DiagnosticLevel level, const char* label, const char* message) {
            ImGuiMCP::TextColored(diagnostic_color(level), "[%s] %s", label, message);
        }

        inline void draw_status(config::DiagnosticLevel level, const char* message) {
            draw_status(level, diagnostic_label(level), message);
        }

        /// # Safety
        ///
        /// The strings supplied in `rows` must remain valid for the duration
        /// of this function call and must be null-terminated.
        void draw_property_table(std::initializer_list<std::pair<std::string_view, std::string_view>> rows) {
            if (!ImGuiMCP::BeginTable(
                    "PropertyTable",
                    2,
                    ImGuiTableFlags_Reorderable |
                        ImGuiTableFlags_Resizable |
                        ImGuiTableFlags_Borders |
                        ImGuiTableFlags_RowBg |
                        ImGuiTableFlags_SizingStretchProp)) {
                return;
            }

            ImGuiMCP::TableSetupColumn("[Description]", ImGuiTableColumnFlags_WidthFixed, 270.0f);
            ImGuiMCP::TableSetupColumn("[Value]", ImGuiTableColumnFlags_WidthStretch);

            ImGuiMCP::TableHeadersRow();

            for (const auto& [description, value] : rows) {
                ImGuiMCP::TableNextRow();

                ImGuiMCP::TableNextColumn();

                // NOLINTBEGIN(bugprone-suspicious-stringview-data-usage)
                ImGuiMCP::TextUnformatted(description.data());

                ImGuiMCP::TableNextColumn();

                ImGuiMCP::TextUnformatted(value.data());
                // NOLINTEND(bugprone-suspicious-stringview-data-usage)
            }

            ImGuiMCP::EndTable();
        }

        [[nodiscard]] Snapshot build_snapshot(
            const config::Config& config) {
            Snapshot snapshot;

            snapshot.crc = config.crc;
            snapshot.mod_count = config.mod_count;
            snapshot.set_count = config.set_count;
            snapshot.version = config.version_str;
            snapshot.creature_version = config.creature_version_str;

            snapshot.mods.reserve(config.prefix_list.size());
            snapshot.sets.reserve(config.set_list.size());

            for (size_t mod_id = 0; mod_id < config.prefix_list.size(); ++mod_id) {
                const auto& prefix = config.prefix_list[mod_id];

                if (prefix.empty()) {
                    continue;
                }

                snapshot.mods.push_back({
                    .mod_id = static_cast<int32_t>(mod_id),
                    .prefix = prefix.c_str(),
                });
            }

            for (size_t index = 0; index < config.set_list.size(); ++index) {
                const auto& set = config.set_list[index];
                std::string prefix;

                if (set.mod_id >= 0 && static_cast<size_t>(set.mod_id) < config.prefix_list.size()) {
                    prefix = config.prefix_list[static_cast<size_t>(set.mod_id)].c_str();
                }

                const auto group = group_name(set.group_id);

                snapshot.sets.push_back({
                    .index = index,
                    .mod_id = set.mod_id,
                    .group_id = set.group_id,
                    .base = set.base,
                    .prefix = std::move(prefix),
                    .group = group.empty() ? std::format("<unknown:{}>", set.group_id) : std::string(group),
                });
            }

            return snapshot;
        }

        void draw_log_level() {
            static constexpr std::array<const char*, 7> kLogLevels = {
                "trace",
                "debug",
                "info",
                "warn",
                "error",
                "critical",
                "off",
            };

            auto& log_level = config::g_config.log_level;

            int current = static_cast<int>(log_level);

            if (ImGuiMCP::Combo("Log Level", &current, kLogLevels.data(), static_cast<int>(kLogLevels.size()))) {
                log_level = static_cast<spdlog::level::level_enum>(current);
                spdlog::set_level(log_level);
                SPDLOG_INFO("Logger level changed to {}", spdlog::level::to_string_view(log_level));
            }

            if (ImGuiMCP::IsItemHovered()) {
                ImGuiMCP::SetTooltip(
                    "Log level is read from the 'log_level' key in config.json.\n"
                    "Defaults to 'info' when the key is not present.");
            }
        }

        void draw_overview() {
            ImGuiMCP::TextUnformatted("FNIS Alternate Animation");

            ImGuiMCP::Separator();

            const bool has_errors = std::ranges::any_of(config::g_diagnostics, [](const config::Diagnostic& diagnostic) {
                return diagnostic.level == config::DiagnosticLevel::error;
            });

            const std::string configuration = has_errors ? "Errors detected" : "Loaded";

            const std::string mods = std::to_string(g_snapshot.mod_count);
            const std::string sets = std::to_string(g_snapshot.set_count);
            const std::string crc = std::format("0x{:08X}", static_cast<uint32_t>(g_snapshot.crc));

            draw_property_table({
                { "Configuration", configuration },
                { "FNIS Version", g_snapshot.version },
                { "Creature Version", g_snapshot.creature_version },
                { "Mods", mods },
                { "Groups / Sets", sets },
                { "Layout CRC", crc },
            });

            ImGuiMCP::Spacing();
            ImGuiMCP::Separator();
            ImGuiMCP::Spacing();

            if (has_errors) {
                draw_status(config::DiagnosticLevel::error, "Configuration contains errors.");
            } else {
                draw_status(config::DiagnosticLevel::success, "Configuration is valid.");
            }

            draw_log_level();
        }

        void draw_mod_layout() {
            if (!ImGuiMCP::BeginTable("FnisAaModLayout", 4,
                    ImGuiTableFlags_Resizable |
                        ImGuiTableFlags_Reorderable |
                        ImGuiTableFlags_Borders |
                        ImGuiTableFlags_RowBg |
                        ImGuiTableFlags_SizingStretchSame |
                        ImGuiTableFlags_ScrollY)) {
                return;
            }

            ImGuiMCP::TableSetupColumn("Mod ID", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGuiMCP::TableSetupColumn("Prefix", ImGuiTableColumnFlags_WidthFixed, 140.0f);
            ImGuiMCP::TableSetupColumn("Entries", ImGuiTableColumnFlags_WidthFixed, 140.0f);
            ImGuiMCP::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthStretch, 100.0f);

            ImGuiMCP::TableHeadersRow();

            for (const auto& mod : g_snapshot.mods) {
                size_t entry_count = 0;

                for (const auto& set : g_snapshot.sets) {
                    if (set.mod_id == mod.mod_id) {
                        ++entry_count;
                    }
                }

                ImGuiMCP::TableNextRow();

                ImGuiMCP::TableNextColumn();
                ImGuiMCP::Text("%d", mod.mod_id);

                ImGuiMCP::TableNextColumn();
                ImGuiMCP::Text("%s", mod.prefix.c_str());

                ImGuiMCP::TableNextColumn();
                ImGuiMCP::Text("%zu", entry_count);

                ImGuiMCP::TableNextColumn();

                if (entry_count != 0) {
                    draw_status(config::DiagnosticLevel::success, "OK");
                } else {
                    draw_status(config::DiagnosticLevel::warning, "No groups");
                }
            }

            ImGuiMCP::EndTable();
        }

        void draw_slot_map() {
            ImGuiMCP::TextUnformatted("Configured FNIS AA entries.");

            ImGuiMCP::Spacing();

            if (!ImGuiMCP::BeginTable(
                    "FnisAaSlotMap",
                    6,
                    ImGuiTableFlags_Resizable |
                        ImGuiTableFlags_Reorderable |
                        ImGuiTableFlags_Borders |
                        ImGuiTableFlags_RowBg |
                        ImGuiTableFlags_SizingStretchSame |
                        ImGuiTableFlags_ScrollY)) {
                return;
            }

            ImGuiMCP::TableSetupColumn("Entry", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGuiMCP::TableSetupColumn("Mod ID", ImGuiTableColumnFlags_WidthFixed, 110.0f);
            ImGuiMCP::TableSetupColumn("Prefix", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGuiMCP::TableSetupColumn("Group ID", ImGuiTableColumnFlags_WidthFixed, 140.0f);
            ImGuiMCP::TableSetupColumn("Group", ImGuiTableColumnFlags_WidthStretch);
            ImGuiMCP::TableSetupColumn("Base", ImGuiTableColumnFlags_WidthFixed, 80.0f);

            ImGuiMCP::TableHeadersRow();

            for (const auto& set : g_snapshot.sets) {
                ImGuiMCP::TableNextRow();

                ImGuiMCP::TableNextColumn();
                ImGuiMCP::Text("%zu", set.index);

                ImGuiMCP::TableNextColumn();
                ImGuiMCP::Text("%d", set.mod_id);

                ImGuiMCP::TableNextColumn();
                ImGuiMCP::Text("%s", set.prefix.empty() ? "<missing>" : set.prefix.c_str());

                ImGuiMCP::TableNextColumn();
                ImGuiMCP::Text("%d", set.group_id);

                ImGuiMCP::TableNextColumn();
                ImGuiMCP::Text("%s", set.group.c_str());

                ImGuiMCP::TableNextColumn();
                ImGuiMCP::Text("%d", set.base);
            }

            ImGuiMCP::EndTable();

            ImGuiMCP::Spacing();
        }

        void draw_diagnostics() {
            ImGuiMCP::TextUnformatted("Diagnostics");
            ImGuiMCP::Separator();

            const bool has_errors = std::ranges::any_of(config::g_diagnostics, [](const config::Diagnostic& diagnostic) {
                return diagnostic.level == config::DiagnosticLevel::error;
            });

            if (has_errors) {
                draw_status(config::DiagnosticLevel::error, "Configuration contains errors.");
            } else if (config::g_diagnostics.empty()) {
                draw_status(config::DiagnosticLevel::success, "No configuration diagnostics.");
            } else {
                draw_status(config::DiagnosticLevel::success, "No configuration errors detected.");
            }

            ImGuiMCP::Spacing();

            for (const auto& diagnostic : config::g_diagnostics) {
                draw_status(diagnostic.level, diagnostic_label(diagnostic.level), diagnostic.message.c_str());
            }

            ImGuiMCP::Spacing();
            ImGuiMCP::Separator();
            ImGuiMCP::Spacing();

            ImGuiMCP::TextUnformatted("Configuration Statistics");

            const std::string configured_mods = std::to_string(g_snapshot.mod_count);
            const std::string configured_sets = std::to_string(g_snapshot.set_count);
            const std::string snapshot_mods = std::to_string(g_snapshot.mods.size());
            const std::string snapshot_sets = std::to_string(g_snapshot.sets.size());
            const std::string crc = std::format("0x{:08X}", static_cast<uint32_t>(g_snapshot.crc));

            draw_property_table({
                { "Configured Mods", configured_mods },
                { "Configured Sets", configured_sets },
                { "Snapshot Mods", snapshot_mods },
                { "Snapshot Sets", snapshot_sets },
                { "Layout CRC", crc },
            });
        }

        enum class FFITestState {
            idle,
            pending,
            success,
            error
        };

        struct FFITestResult {
            FFITestState state = FFITestState::idle;
            std::string  result;
            std::string  error;
        };

        inline std::vector<FFITestResult> g_ffi_results;
        inline std::mutex                 g_ffi_mutex;
        inline std::atomic_uint32_t       g_ffi_result_count{ 0 };

        [[nodiscard]]
        inline std::size_t register_ffi_test() {
            const auto index =
                g_ffi_result_count.fetch_add(1, std::memory_order_relaxed);

            std::scoped_lock lock(g_ffi_mutex);

            if (index >= g_ffi_results.size()) {
                g_ffi_results.resize(index + 1);
            }

            return index;
        }

        inline void set_ffi_pending(std::size_t index) {
            std::scoped_lock lock(g_ffi_mutex);

            if (index >= g_ffi_results.size()) {
                return;
            }

            auto& result = g_ffi_results[index];

            result.state = FFITestState::pending;
            result.result.clear();
            result.error.clear();
        }

        inline void set_ffi_success(std::size_t index, std::string result) {
            std::scoped_lock lock(g_ffi_mutex);

            if (index >= g_ffi_results.size()) {
                return;
            }

            auto& test = g_ffi_results[index];

            test.state = FFITestState::success;
            test.result = std::move(result);
            test.error.clear();
        }

        inline void set_ffi_error(
            std::size_t index,
            std::string error) {
            std::scoped_lock lock(g_ffi_mutex);

            if (index >= g_ffi_results.size()) {
                return;
            }

            auto& result = g_ffi_results[index];

            result.state = FFITestState::error;
            result.result.clear();
            result.error = std::move(error);
        }

        std::string format_ffi_variable(const RE::BSScript::Variable& value) {
            auto type_info = value.GetType();

            switch (type_info.GetRawType()) {
            case RE::BSScript::TypeInfo::RawType::kNone:
                return "None";

            case RE::BSScript::TypeInfo::RawType::kBool:
                return value.GetBool() ? "true" : "false";

            case RE::BSScript::TypeInfo::RawType::kInt:
                return std::to_string(value.GetSInt());

            case RE::BSScript::TypeInfo::RawType::kFloat:
                return std::format("{}", value.GetFloat());

            case RE::BSScript::TypeInfo::RawType::kString:
                return std::format("\"{}\"", value.GetString());

            case RE::BSScript::TypeInfo::RawType::kNoneArray:
            case RE::BSScript::TypeInfo::RawType::kObjectArray:
            case RE::BSScript::TypeInfo::RawType::kStringArray:
            case RE::BSScript::TypeInfo::RawType::kIntArray:
            case RE::BSScript::TypeInfo::RawType::kFloatArray:
            case RE::BSScript::TypeInfo::RawType::kBoolArray:
                {
                    auto array = value.GetArray();

                    if (!array) {
                        return "[]";
                    }

                    std::string result = "[";

                    const auto count = std::min<RE::BSScript::Array::size_type>(array->size(), 3);

                    for (RE::BSScript::Array::size_type i = 0; i < count; ++i) {
                        if (i != 0) {
                            result += ", ";
                        }

                        result += format_ffi_variable((*array)[i]);
                    }

                    if (array->size() > 10) {
                        if (count != 0) {
                            result += ", ";
                        }

                        result += "...";
                    }

                    result += "]";

                    return result;
                }

            default:
                return std::format("<unsupported type>: \"{}\"", type_info.TypeAsString());
            }
        }

        class FFITestCallback final : public RE::BSScript::IStackCallbackFunctor {
        public:
            explicit FFITestCallback(std::size_t result_index) : _result_index(result_index) {}

            void operator()(RE::BSScript::Variable a_result) override {
                try {
                    set_ffi_success(_result_index, format_ffi_variable(a_result));
                } catch (...) {
                    set_ffi_error(_result_index, "Failed to format Papyrus return value");
                }
            }

            bool CanSave() const override { return false; }
            void SetObject(const RE::BSTSmartPointer<RE::BSScript::Object>&) override {}

        private:
            std::size_t _result_index;
        };

        template <class... Args>
        void call_ffi_function(std::size_t result_index, std::string_view script_name, std::string_view function_name, Args&&... args) {
            set_ffi_pending(result_index);

            const auto skyrim_vm = RE::SkyrimVM::GetSingleton();
            if (!skyrim_vm || !skyrim_vm->GetVMRuntimeData().impl) {
                set_ffi_error(result_index, "SkyrimVM is unavailable");
                return;
            }

            auto callback = RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor>(new FFITestCallback(result_index));

            auto fn_args = RE::MakeFunctionArguments(std::forward<Args>(args)...);

            skyrim_vm->GetVMRuntimeData().impl->DispatchStaticCall(
                RE::BSFixedString(script_name),
                RE::BSFixedString(function_name),
                fn_args,
                callback);
        }

        namespace AtomOneDark {
            constexpr ImVec4 blue{ .x = 0.38f, .y = 0.69f, .z = 0.94f, .w = 1.0f };
            // constexpr ImVec4 cyan{ .x = 0.33f, .y = 0.76f, .z = 0.82f, .w = 1.0f };
            constexpr ImVec4 green{ .x = 0.60f, .y = 0.73f, .z = 0.47f, .w = 1.0f };
            constexpr ImVec4 orange{ .x = 0.820f, .y = 0.604f, .z = 0.400f, .w = 1.0f };
            constexpr ImVec4 purple{ .x = 0.78f, .y = 0.47f, .z = 0.87f, .w = 1.0f };
            constexpr ImVec4 red{ .x = 0.88f, .y = 0.42f, .z = 0.46f, .w = 1.0f };
            constexpr ImVec4 yellow{ .x = 0.90f, .y = 0.75f, .z = 0.47f, .w = 1.0f };
        }

        inline void color_text(const char* parenthesis, const ImGuiMCP::ImVec4& color) {
            ImGuiMCP::PushStyleColor(ImGuiCol_Text, color);
            ImGuiMCP::TextUnformatted(parenthesis);
            ImGuiMCP::PopStyleColor();
        }

        inline void draw_ffi_result(std::size_t result_index) {
            FFITestResult result;

            {
                std::scoped_lock lock(g_ffi_mutex);

                if (result_index >= g_ffi_results.size()) {
                    return;
                }

                result = g_ffi_results[result_index];
            }

            ImGuiMCP::TextUnformatted("-> ");
            ImGuiMCP::SameLine();
            color_text("Return", AtomOneDark::purple);
            ImGuiMCP::SameLine();
            ImGuiMCP::TextUnformatted(":");
            ImGuiMCP::SameLine();

            switch (result.state) {
            case FFITestState::idle:
                ImGuiMCP::TextUnformatted("");
                break;

            case FFITestState::pending:
                ImGuiMCP::TextUnformatted("...");
                break;

            case FFITestState::success:
                color_text(result.result.c_str(), AtomOneDark::green);
                break;

            case FFITestState::error:
                color_text(std::format("<error: {}>", result.error).c_str(), AtomOneDark::red);
                break;
            }
        }

        struct FFIIntArgument {
            const char* label;
            int32_t*    value;
        };

        struct FFIStringArgument {
            const char* label;
            char*       value;
            std::size_t size;
        };

        struct FFIBoolArgument {
            const char* label;
            bool*       value;
        };

        inline void draw_ffi_argument(const FFIIntArgument& arg) {
            ImGuiMCP::SetNextItemWidth(80.0f);
            ImGuiMCP::PushStyleColor(ImGuiCol_Text, AtomOneDark::orange);
            ImGuiMCP::InputInt(std::format("##{}", arg.label).c_str(), reinterpret_cast<int*>(arg.value), 0, 0);
            ImGuiMCP::PopStyleColor();
        }

        inline void draw_ffi_argument(const FFIStringArgument& arg) {
            ImGuiMCP::SetNextItemWidth(160.0f);
            ImGuiMCP::PushStyleColor(ImGuiCol_Text, AtomOneDark::green);

            ImGuiMCP::InputText(std::format("##{}", arg.label).c_str(), arg.value, arg.size);
            ImGuiMCP::PopStyleColor();
        }

        inline void draw_ffi_argument(const FFIBoolArgument& arg) {
            ImGuiMCP::PushStyleColor(ImGuiCol_Text, AtomOneDark::orange);
            ImGuiMCP::Checkbox(std::format("##{}", arg.label).c_str(), arg.value);
            ImGuiMCP::PopStyleColor();
        }

        template <class Arg>
        void draw_ffi_argument_separator(bool& first, const Arg& arg) {
            if (!first) {
                ImGuiMCP::SameLine();
                ImGuiMCP::TextUnformatted(", ");
                ImGuiMCP::SameLine();
            }

            first = false;
            draw_ffi_argument(arg);
        }

        template <class... Args>
        void draw_ffi_arguments(const Args&... args) {
            bool first = true;
            (draw_ffi_argument_separator(first, args), ...);
        }

        /// For int32_t, bool
        template <class Arg>
        auto get_ffi_argument_value(const Arg& arg) {
            return *arg.value;
        }

        inline RE::BSFixedString get_ffi_argument_value(const FFIStringArgument& arg) {
            return { arg.value };
        }

        template <class... Args>
        void draw_ffi_call(std::size_t test_id, const char* signature, const char* function_name, const char* script_name, const Args&... args) {
            ImGuiMCP::TextUnformatted(signature);

            // format: fn()
            color_text(function_name, AtomOneDark::blue);
            ImGuiMCP::SameLine();
            color_text("(", AtomOneDark::yellow);
            ImGuiMCP::SameLine();

            draw_ffi_arguments(args...);

            ImGuiMCP::SameLine();
            color_text(")", AtomOneDark::yellow);
            ImGuiMCP::SameLine();

            if (ImGuiMCP::Button(std::format("Call##{}", test_id).c_str())) {
                call_ffi_function(test_id, script_name, function_name, get_ffi_argument_value(args)...);
            }

            draw_ffi_result(test_id);
            ImGuiMCP::NewLine();
        }

        void draw_ffi_call_tests() {
            // Skip functions(need Actor, Return void)
            //
            // - FNIS_aa.SetAnimGroup(Actor ac, string animGroup, int base, int number, string mod, bool debugOutput) -> bool
            // - FNIS_aa.SetAnimGroupEX(Actor ac, string animGroup, int base, int number, string mod, bool debugOutput, bool skipForce3D) -> bool
            // - FNIS_aa.GetAAsets(int nSets, int[] GroupId, int[] ModId, int[] Base, int[] Index, string mod, bool debugOutput) -> void

            // - FNIS.set_AACondition(Actor ac, string aaType, string mod, int aaCond, int aaDebug) -> int
            // - FNIS.AAReport(string longReport, string shortReport, int AAdebug, bool isError) -> void

            // ---------------------------------------------------------------------------------------------------------
            // FNIS_aa2.pex

            {
                static const auto id = register_ffi_test();
                static int32_t    list_type = 0;

                draw_ffi_call(id, "int FNIS_aa2.GetAAnumber(int aaNumber)", "GetAAnumber", "FNIS_aa2",
                    FFIIntArgument{
                        .label = "##FNIS_aa2_GetAAnumber_listType",
                        .value = &list_type,
                    });
            }

            {
                static const auto            id = register_ffi_test();
                static int32_t               n_mods = 0;
                static std::array<char, 256> mod{};
                static bool                  debug_output = false;

                draw_ffi_call(
                    id,
                    "string[] FNIS_aa2.GetAAprefixList(int nMods, string mod, bool debugOutput)",
                    "GetAAprefixList",
                    "FNIS_aa2",
                    FFIIntArgument{
                        .label = "##FNIS_aa2_GetAAprefixList_nMods",
                        .value = &n_mods,
                    },
                    FFIStringArgument{
                        .label = "##FNIS_aa2_GetAAprefixList_mod",
                        .value = mod.data(),
                        .size = mod.size(),
                    },
                    FFIBoolArgument{
                        .label = "##FNIS_aa2_GetAAprefixList_debugOutput",
                        .value = &debug_output,
                    });
            }

            // ---------------------------------------------------------------------------------------------------------
            // FNIS_aa.pex

            {
                static const auto            id = register_ffi_test();
                static std::array<char, 256> aa_prefix{};
                static std::array<char, 256> mod{};
                static bool                  debug_output = false;

                draw_ffi_call(id,
                    "int FNIS_aa.GetAAmodID(string myAAprefix, string mod, bool debugOutput)",
                    "GetAAmodID",
                    "FNIS_aa",
                    FFIStringArgument{
                        .label = "##FNIS_aa_GetAAmodID_aaPrefix",
                        .value = aa_prefix.data(),
                        .size = aa_prefix.size(),
                    },
                    FFIStringArgument{
                        .label = "##FNIS_aa_GetAAmodID_mod",
                        .value = mod.data(),
                        .size = mod.size(),
                    },
                    FFIBoolArgument{
                        .label = "##FNIS_aa_GetAAmodID_debugOutput",
                        .value = &debug_output,
                    });
            }

            {
                static const auto            id = register_ffi_test();
                static int32_t               aa_mod_id = 0;
                static int32_t               aa_group_id = 0;
                static std::array<char, 256> mod{};
                static bool                  debug_output = false;

                draw_ffi_call(id,
                    "int FNIS_aa.GetGroupBaseValue(int AAmodID, int AAgroupID, string mod, bool debugOutput)",
                    "GetGroupBaseValue",
                    "FNIS_aa",
                    FFIIntArgument{
                        .label = "##FNIS_aa_GetGroupBaseValue_modId",
                        .value = &aa_mod_id,
                    },
                    FFIIntArgument{
                        .label = "##FNIS_aa_GetGroupBaseValue_groupId",
                        .value = &aa_group_id,
                    },
                    FFIStringArgument{
                        .label = "##FNIS_aa_GetGroupBaseValue_mod",
                        .value = mod.data(),
                        .size = mod.size(),
                    },
                    FFIBoolArgument{
                        .label = "##FNIS_aa_GetGroupBaseValue_debugOutput",
                        .value = &debug_output,
                    });
            }

            {
                static const auto            id = register_ffi_test();
                static int32_t               aa_mod_id = 0;
                static std::array<char, 256> mod{};
                static bool                  debug_output = false;

                draw_ffi_call(id,
                    "int[] FNIS_aa.GetAllGroupBaseValues(int AAmodID, string mod, bool debugOutput)",
                    "GetAllGroupBaseValues",
                    "FNIS_aa",
                    FFIIntArgument{
                        .label = "##FNIS_aa_GetAllGroupBaseValues_modId",
                        .value = &aa_mod_id,
                    },
                    FFIStringArgument{
                        .label = "##FNIS_aa_GetAllGroupBaseValues_mod",
                        .value = mod.data(),
                        .size = mod.size(),
                    },
                    FFIBoolArgument{
                        .label = "##FNIS_aa_GetAllGroupBaseValues_debugOutput",
                        .value = &debug_output,
                    });
            }

            {
                static const auto id = register_ffi_test();
                draw_ffi_call(id, "int FNIS_aa.GetInstallationCRC()", "GetInstallationCRC", "FNIS_aa");
            }

            {
                static const auto id = register_ffi_test();
                draw_ffi_call(id, "bool FNIS.IsGenerated()", "IsGenerated", "FNIS");
            }

            {
                static const auto id = register_ffi_test();
                static bool       creature = false;

                draw_ffi_call(id, "string FNIS.VersionToString(bool abCreature)", "VersionToString", "FNIS",
                    FFIBoolArgument{
                        .label = "##FNIS_VersionToString_creature",
                        .value = &creature,
                    });
            }

            {
                static const auto id = register_ffi_test();
                static int32_t    major = 0;
                static int32_t    minor1 = 0;
                static int32_t    minor2 = 0;
                static bool       creature = false;

                draw_ffi_call(id, "int FNIS.VersionCompare(int iCompMajor, int iCompMinor1, int iCompMinor2, bool abCreature)", "VersionCompare", "FNIS",
                    FFIIntArgument{
                        .label = "##FNIS_VersionCompare_major",
                        .value = &major,
                    },
                    FFIIntArgument{
                        .label = "##FNIS_VersionCompare_minor1",
                        .value = &minor1,
                    },
                    FFIIntArgument{
                        .label = "##FNIS_VersionCompare_minor2",
                        .value = &minor2,
                    },
                    FFIBoolArgument{
                        .label = "##FNIS_VersionCompare_creature",
                        .value = &creature,
                    });
            }

            {
                static const auto id = register_ffi_test();
                static bool       creature = false;

                draw_ffi_call(id, "int FNIS.GetMajor(bool abCreature)", "GetMajor", "FNIS",
                    FFIBoolArgument{
                        .label = "##FNIS_GetMajor_creature",
                        .value = &creature,
                    });
            }

            {
                static const auto id = register_ffi_test();
                static bool       creature = false;

                draw_ffi_call(id, "int FNIS.GetMinor1(bool abCreature)", "GetMinor1", "FNIS",
                    FFIBoolArgument{
                        .label = "##FNIS_GetMinor1_creature",
                        .value = &creature,
                    });
            }

            {
                static const auto id = register_ffi_test();
                static bool       creature = false;

                draw_ffi_call(id, "int FNIS.GetMinor2(bool abCreature)", "GetMinor2", "FNIS",
                    FFIBoolArgument{
                        .label = "##FNIS_GetMinor2_creature",
                        .value = &creature,
                    });
            }

            {
                static const auto id = register_ffi_test();
                static bool       creature = false;

                draw_ffi_call(id, "int FNIS.GetFlags(bool abCreature)", "GetFlags", "FNIS",
                    FFIBoolArgument{
                        .label = "##FNIS_GetFlags_creature",
                        .value = &creature,
                    });
            }

            {
                static const auto id = register_ffi_test();
                static bool       creature = false;

                draw_ffi_call(id, "int FNIS.IsRelease(bool abCreature)", "IsRelease", "FNIS",
                    FFIBoolArgument{
                        .label = "##FNIS_IsRelease_creature",
                        .value = &creature,
                    });
            }
        }

        void __stdcall render() {
            if (!ImGuiMCP::BeginTabBar("FnisAaTabs", ImGuiTabBarFlags_FittingPolicyResizeDown)) {
                return;
            }

            auto draw_tab = []<class F>(const char* label, F&& draw) {
                if (ImGuiMCP::BeginTabItem(label)) {
                    std::forward<F>(draw)();
                    ImGuiMCP::EndTabItem();
                }
            };

            draw_tab("Overview", draw_overview);
            draw_tab("Mod Layout", draw_mod_layout);
            draw_tab("Slot Map", draw_slot_map);
            draw_tab("Diagnostics", draw_diagnostics);

            ImGuiMCP::EndTabBar();
        }

        void __stdcall render_debug_section() {
            draw_ffi_call_tests();
        }
    }

    // NOLINTBEGIN(misc-use-internal-linkage)
    void UpdateSnapshot() {
        if (!SKSEMenuFramework::IsInstalled()) {
            return;
        }

        g_snapshot = build_snapshot(config::g_config);
    }

    void Register() {
        if (!SKSEMenuFramework::IsInstalled()) {
            return;
        }

        SKSEMenuFramework::SetSection(std::string(kSection));
        SKSEMenuFramework::AddSectionItem("Status", render);
        SKSEMenuFramework::AddSectionItem("Debug", render_debug_section);
    }
    // NOLINTEND(misc-use-internal-linkage)
}
