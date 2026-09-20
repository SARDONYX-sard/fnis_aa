#include <SKSEMenuFramework.h>

#include "alt_group_table.hh"
#include "config.hh"

namespace fnis_aa::menu {

    namespace {
        constexpr const char* SECTION_NAME = "Dyn FNIS AA Functions";

        namespace color {
            consteval ImGuiMCP::ImVec4 rgba(std::uint32_t color) noexcept {
                return ImGuiMCP::ImVec4{
                    .x = static_cast<float>((color >> 24) & 0xFF) / 255.0f,
                    .y = static_cast<float>((color >> 16) & 0xFF) / 255.0f,
                    .z = static_cast<float>((color >> 8) & 0xFF) / 255.0f,
                    .w = static_cast<float>(color & 0xFF) / 255.0f,
                };
            }

            constexpr ImGuiMCP::ImVec4 BLUE = rgba(0x61B0F0FF);
            constexpr ImGuiMCP::ImVec4 GREEN = rgba(0x9AB975FF);
            constexpr ImGuiMCP::ImVec4 ORANGE = rgba(0xD19A66FF);
            constexpr ImGuiMCP::ImVec4 PURPLE = rgba(0xC778DEFF);
            constexpr ImGuiMCP::ImVec4 RED = rgba(0xE06B75FF);
            constexpr ImGuiMCP::ImVec4 WHITE = rgba(0xFFFFFFFF);
            constexpr ImGuiMCP::ImVec4 YELLOW = rgba(0xE6BF78FF);

            constexpr ImGuiMCP::ImVec4 SUCCESS = rgba(0x4DD95AFF);
            constexpr ImGuiMCP::ImVec4 INFO = rgba(0x59A6F2FF);
            constexpr ImGuiMCP::ImVec4 WARN = rgba(0xF2BF33FF);
            constexpr ImGuiMCP::ImVec4 ERROR_ = rgba(0xF24040FF);
        }

        [[nodiscard]] const char* diagnostic_label(config::DiagnosticLevel level) noexcept {
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
            switch (level) {
            case config::DiagnosticLevel::success:
                return color::SUCCESS;
            case config::DiagnosticLevel::info:
                return color::INFO;
            case config::DiagnosticLevel::warning:
                return color::WARN;
            case config::DiagnosticLevel::error:
                return color::ERROR_;
            }

            return color::WHITE;
        }
        inline void draw_status(config::DiagnosticLevel level, const char* label, const char* message) {
            ImGuiMCP::TextColored(diagnostic_color(level), "[%s] %s", label, message);
        }

        inline void draw_status(config::DiagnosticLevel level, const char* message) {
            draw_status(level, diagnostic_label(level), message);
        }

        constexpr ImGuiMCP::ImGuiTableFlags PROPERTY_TABLE_FLAGS =
            ImGuiMCP::ImGuiTableFlags_Reorderable |
            ImGuiMCP::ImGuiTableFlags_Resizable |
            ImGuiMCP::ImGuiTableFlags_Borders |
            ImGuiMCP::ImGuiTableFlags_RowBg |
            ImGuiMCP::ImGuiTableFlags_SizingStretchProp;

        constexpr ImGuiMCP::ImGuiTableFlags DATA_TABLE_FLAGS =
            ImGuiMCP::ImGuiTableFlags_Resizable |
            ImGuiMCP::ImGuiTableFlags_Reorderable |
            ImGuiMCP::ImGuiTableFlags_Borders |
            ImGuiMCP::ImGuiTableFlags_RowBg |
            ImGuiMCP::ImGuiTableFlags_SizingStretchSame |
            ImGuiMCP::ImGuiTableFlags_ScrollY;

        /// # Safety
        ///
        /// The strings supplied in `rows` must remain valid for the duration
        /// of this function call and must be null-terminated.
        void draw_property_table(std::initializer_list<std::pair<std::string_view, std::string_view>> rows) {
            if (!ImGuiMCP::BeginTable("PropertyTable", 2, PROPERTY_TABLE_FLAGS)) {
                return;
            }
            ImGuiMCP::TableSetupColumn("[Description]", ImGuiMCP::ImGuiTableColumnFlags_WidthFixed, 270.0f);
            ImGuiMCP::TableSetupColumn("[Value]", ImGuiMCP::ImGuiTableColumnFlags_WidthStretch);

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

        void draw_overview(const config::Config& config, const std::vector<config::Diagnostic>& diagnostics) {
            static std::string PLUGIN_TITLE = std::format("{} v{}", SECTION_NAME, SKSE::GetPluginVersion().string("."));
            ImGuiMCP::TextUnformatted(PLUGIN_TITLE.c_str());
            ImGuiMCP::Separator();

            const bool has_errors = std::ranges::any_of(diagnostics, [](const config::Diagnostic& diagnostic) {
                return diagnostic.level == config::DiagnosticLevel::error;
            });

            const std::string config_status = has_errors ? "Errors detected" : "Loaded";
            const std::string mods = std::to_string(config.mod_count);
            const std::string sets = std::to_string(config.set_count);
            const std::string crc = std::format("{}(0x{:08X})", config.crc, config.crc);
            draw_property_table({
                { "Config", config_status },
                { "FNIS Version", config.version_str },
                { "Creature Version", config.creature_version_str },
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

            ImGuiMCP::Spacing();
            ImGuiMCP::SameLine();
        }

        void draw_mod_layout(const config::Config& config) {
            if (!ImGuiMCP::BeginTable("FnisAaModLayout", 4, DATA_TABLE_FLAGS)) {
                return;
            }
            ImGuiMCP::TableSetupColumn("Mod ID", ImGuiMCP::ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGuiMCP::TableSetupColumn("Prefix", ImGuiMCP::ImGuiTableColumnFlags_WidthFixed, 140.0f);
            ImGuiMCP::TableSetupColumn("Entries", ImGuiMCP::ImGuiTableColumnFlags_WidthFixed, 140.0f);
            ImGuiMCP::TableSetupColumn("Status", ImGuiMCP::ImGuiTableColumnFlags_WidthStretch, 100.0f);

            ImGuiMCP::TableHeadersRow();

            for (size_t mod_id = 0; mod_id < config.prefix_list.size(); ++mod_id) {
                const auto& prefix = config.prefix_list[mod_id];

                if (prefix.empty()) {
                    continue;
                }

                size_t entry_count = 0;
                for (const auto& set : config.set_list) {
                    if (set.mod_id == static_cast<int32_t>(mod_id)) {
                        ++entry_count;
                    }
                }

                ImGuiMCP::TableNextRow();

                ImGuiMCP::TableNextColumn();
                ImGuiMCP::Text("%d", mod_id);

                ImGuiMCP::TableNextColumn();
                ImGuiMCP::Text("%s", prefix.c_str());
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

        void draw_slot_map(const config::Config& config) {
            ImGuiMCP::TextUnformatted("Configured FNIS AA entries.");

            ImGuiMCP::Spacing();
            if (!ImGuiMCP::BeginTable("FnisAaSlotMap", 6, DATA_TABLE_FLAGS)) {
                return;
            }
            ImGuiMCP::TableSetupColumn("Entry", ImGuiMCP::ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGuiMCP::TableSetupColumn("Mod ID", ImGuiMCP::ImGuiTableColumnFlags_WidthFixed, 110.0f);
            ImGuiMCP::TableSetupColumn("Prefix", ImGuiMCP::ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGuiMCP::TableSetupColumn("Group ID", ImGuiMCP::ImGuiTableColumnFlags_WidthFixed, 140.0f);
            ImGuiMCP::TableSetupColumn("Group", ImGuiMCP::ImGuiTableColumnFlags_WidthStretch);
            ImGuiMCP::TableSetupColumn("Base", ImGuiMCP::ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGuiMCP::TableHeadersRow();

            for (size_t index = 0; index < config.set_list.size(); ++index) {
                const auto&      set = config.set_list[index];
                std::string_view prefix;

                if (set.mod_id >= 0 && static_cast<size_t>(set.mod_id) < config.prefix_list.size()) {
                    prefix = config.prefix_list[static_cast<size_t>(set.mod_id)].c_str();
                }

                ImGuiMCP::TableNextRow();

                ImGuiMCP::TableNextColumn();
                ImGuiMCP::Text("%zu", index);

                ImGuiMCP::TableNextColumn();
                ImGuiMCP::Text("%d", set.mod_id);

                ImGuiMCP::TableNextColumn();
                ImGuiMCP::Text("%s", prefix.empty() ? "<missing>" : prefix.data());
                ImGuiMCP::TableNextColumn();
                ImGuiMCP::Text("%d", set.group_id);

                ImGuiMCP::TableNextColumn();
                const auto group_name = fnis_aa::group_name(set.group_id);
                if (group_name) {
                    // Safety: `group_name` null terminated.
                    // NOLINTBEGIN(bugprone-suspicious-stringview-data-usage)
                    ImGuiMCP::Text("%s", group_name->data());
                    // NOLINTEND(bugprone-suspicious-stringview-data-usage)
                } else {
                    const auto unknown = std::format("<unknown:{}>", set.group_id);
                    ImGuiMCP::Text("%s", unknown.c_str());
                }

                ImGuiMCP::TableNextColumn();
                ImGuiMCP::Text("%d", set.base);
            }

            ImGuiMCP::EndTable();

            ImGuiMCP::Spacing();
        }

        void draw_diagnostics(const config::Config& config, const std::vector<config::Diagnostic>& diagnostics) {
            ImGuiMCP::TextUnformatted("Diagnostics");
            ImGuiMCP::Separator();
            const bool has_errors = std::ranges::any_of(diagnostics, [](const config::Diagnostic& diagnostic) {
                return diagnostic.level == config::DiagnosticLevel::error;
            });
            if (has_errors) {
                draw_status(config::DiagnosticLevel::error, "Configuration contains errors.");
            } else if (diagnostics.empty()) {
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
            const std::string configured_mods = std::to_string(config.mod_count);
            const std::string configured_sets = std::to_string(config.set_count);
            const std::string actual_mods = std::to_string(std::ranges::count_if(config.prefix_list, [](const auto& prefix) { return !prefix.empty(); }));
            const std::string actual_sets = std::to_string(config.set_list.size());
            const std::string crc = std::format("0x{:08X}", static_cast<uint32_t>(config.crc));
            draw_property_table({
                { "Configured Mods", configured_mods },
                { "Configured Sets", configured_sets },
                { "Actual Mods", actual_mods },
                { "Actual Sets", actual_sets },
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

                    constexpr auto MAX_DISPLAY_COUNT = 10;
                    const auto     count = std::min<RE::BSScript::Array::size_type>(array->size(), MAX_DISPLAY_COUNT);

                    for (RE::BSScript::Array::size_type i = 0; i < count; ++i) {
                        if (i != 0) {
                            result += ", ";
                        }

                        result += format_ffi_variable((*array)[i]);
                    }

                    if (array->size() > MAX_DISPLAY_COUNT) {
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

        // ---------------------------------------------------------------------
        // FFI test registration DSL
        // ---------------------------------------------------------------------

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

        template <typename T>
        struct FFIArgumentType;

        template <>
        struct FFIArgumentType<FFIIntArgument> {
            static constexpr const char* papyrus_name = "int";
        };

        template <>
        struct FFIArgumentType<FFIStringArgument> {
            static constexpr const char* papyrus_name = "string";
        };

        template <>
        struct FFIArgumentType<FFIBoolArgument> {
            static constexpr const char* papyrus_name = "bool";
        };

        template <typename T>
        struct FFIReturnType;

        template <>
        struct FFIReturnType<int32_t> {
            static constexpr const char* papyrus_name = "int";

            static RE::BSScript::TypeInfo type_info() {
                return RE::BSScript::TypeInfo(RE::BSScript::TypeInfo::RawType::kInt);
            }
        };

        template <>
        struct FFIReturnType<bool> {
            static constexpr const char* papyrus_name = "bool";

            static RE::BSScript::TypeInfo type_info() {
                return RE::BSScript::TypeInfo(RE::BSScript::TypeInfo::RawType::kBool);
            }
        };

        template <>
        struct FFIReturnType<std::string> {
            static constexpr const char* papyrus_name = "string";

            static RE::BSScript::TypeInfo type_info() {
                return RE::BSScript::TypeInfo(RE::BSScript::TypeInfo::RawType::kString);
            }
        };

        template <>
        struct FFIReturnType<std::vector<int32_t>> {
            static constexpr const char* papyrus_name = "int[]";

            static RE::BSScript::TypeInfo type_info() {
                return RE::BSScript::TypeInfo(RE::BSScript::TypeInfo::RawType::kIntArray);
            }
        };

        template <>
        struct FFIReturnType<std::vector<std::string>> {
            static constexpr const char* papyrus_name = "string[]";

            static RE::BSScript::TypeInfo type_info() {
                return RE::BSScript::TypeInfo(RE::BSScript::TypeInfo::RawType::kStringArray);
            }
        };

        template <typename T>
        struct FFIReturn {
            using type = T;
        };

        template <typename T>
        inline constexpr FFIReturn<T> ret{};

        template <typename... Args>
        struct FFIArgs {
            std::tuple<Args...> values;
        };

        template <typename Ret>
        class FFITestCallback final : public RE::BSScript::IStackCallbackFunctor {
        public:
            explicit FFITestCallback(std::size_t result_index) : _result_index(result_index) {}

            void operator()(RE::BSScript::Variable a_result) override {
                const auto actual_type = a_result.GetType();
                const auto expected_type = FFIReturnType<Ret>::type_info();

                if (actual_type != expected_type) {
                    set_ffi_error(_result_index,
                        std::format(
                            "Papyrus return type mismatch: expected {}, got {}",
                            FFIReturnType<Ret>::papyrus_name,
                            actual_type.TypeAsString()));
                    return;
                }

                set_ffi_success(_result_index, format_ffi_variable(a_result));
            }

            bool CanSave() const override { return false; }
            void SetObject(const RE::BSTSmartPointer<RE::BSScript::Object>&) override {}

        private:
            std::size_t _result_index;
        };

        template <typename Ret, class... Args>
        void call_ffi_function(std::size_t result_index, std::string_view script_name, std::string_view fn_name, Args&&... args) {
            set_ffi_pending(result_index);

            const auto skyrim_vm = RE::SkyrimVM::GetSingleton();
            if (!skyrim_vm || !skyrim_vm->GetVMRuntimeData().impl) {
                set_ffi_error(result_index, "SkyrimVM is unavailable");
                return;
            }

            auto fn_args = RE::MakeFunctionArguments(std::forward<Args>(args)...);
            // NOTE: Need up cast to compile err.
            auto callback = RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor>(new FFITestCallback<Ret>(result_index));
            skyrim_vm->GetVMRuntimeData().impl->DispatchStaticCall(script_name, fn_name, fn_args, callback);
        }

        inline void color_text(const char* parenthesis, const ImGuiMCP::ImVec4& color) {
            ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, color);
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
            color_text("Return", color::PURPLE);
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
                color_text(result.result.c_str(), color::GREEN);
                break;

            case FFITestState::error:
                color_text(std::format("<error: {}>", result.error).c_str(), color::RED);
                break;
            }
        }

        template <typename... Args>
        FFIArgs<Args...> make_ffi_args(Args&&... args) {
            return {
                .values = std::tuple{ std::forward<Args>(args)... }
            };
        }

        inline FFIIntArgument make_ffi_argument(const char* name, int32_t& value) {
            return {
                .label = name,
                .value = &value,
            };
        }

        template <std::size_t N>
        inline FFIStringArgument make_ffi_argument(const char* name, std::array<char, N>& value) {
            return {
                .label = name,
                .value = value.data(),
                .size = value.size(),
            };
        }

        inline FFIBoolArgument make_ffi_argument(const char* name, bool& value) {
            return {
                .label = name,
                .value = &value,
            };
        }

#define FNIS_FFI_MAKE_ARGUMENT(value) make_ffi_argument(#value, value)

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define FNIS_FFI_ARGS_1(a) make_ffi_args(FNIS_FFI_MAKE_ARGUMENT(a))

#define FNIS_FFI_ARGS_2(a, b)      \
    make_ffi_args(                 \
        FNIS_FFI_MAKE_ARGUMENT(a), \
        FNIS_FFI_MAKE_ARGUMENT(b))

#define FNIS_FFI_ARGS_3(a, b, c)   \
    make_ffi_args(                 \
        FNIS_FFI_MAKE_ARGUMENT(a), \
        FNIS_FFI_MAKE_ARGUMENT(b), \
        FNIS_FFI_MAKE_ARGUMENT(c))

#define FNIS_FFI_ARGS_4(a, b, c, d) \
    make_ffi_args(                  \
        FNIS_FFI_MAKE_ARGUMENT(a),  \
        FNIS_FFI_MAKE_ARGUMENT(b),  \
        FNIS_FFI_MAKE_ARGUMENT(c),  \
        FNIS_FFI_MAKE_ARGUMENT(d))

#define FNIS_FFI_GET_ARGS_MACRO(_1, _2, _3, _4, NAME, ...) NAME

#define args(...)            \
    FNIS_FFI_GET_ARGS_MACRO( \
        __VA_ARGS__,         \
        FNIS_FFI_ARGS_4,     \
        FNIS_FFI_ARGS_3,     \
        FNIS_FFI_ARGS_2,     \
        FNIS_FFI_ARGS_1)(__VA_ARGS__)
        // NOLINTEND(cppcoreguidelines-macro-usage)

        inline auto args0() { return make_ffi_args(); }

        template <class Arg>
        void draw_ffi_signature_separator(bool& first, const Arg& arg) {
            if (!first) {
                ImGuiMCP::SameLine();
                ImGuiMCP::TextUnformatted(", ");
            }
            first = false;

            using Argument = std::remove_cvref_t<Arg>;
            ImGuiMCP::SameLine();
            color_text(FFIArgumentType<Argument>::papyrus_name, color::YELLOW);
            ImGuiMCP::SameLine();
            ImGuiMCP::TextUnformatted(arg.label);
        }

        template <class... Args>
        void draw_ffi_signature_arguments(const Args&... args) {
            bool first = true;
            (draw_ffi_signature_separator(first, args), ...);
        }

        void draw_ffi_function_doc(const char* fn_name) {
            const std::string_view name{ fn_name };

            if (name == "GetAAnumber") {
                ImGuiMCP::SetTooltip(
                    "Returns an FNIS AA count.\n"
                    "\n"
                    "Examples:\n"
                    "  GetAAnumber(0) -> mod count\n"
                    "  GetAAnumber(1) -> set count\n"
                    "  GetAAnumber(2) -> CRC");
            } else if (name == "GetAAprefixList") {
                ImGuiMCP::SetTooltip(
                    "Returns the configured FNIS AA mod prefix list.\n"
                    "\n"
                    "nMods is kept for Papyrus compatibility and does not "
                    "limit the returned list in this implementation.\n"
                    "mod and debugOutput are also compatibility parameters.");
            } else if (name == "GetAAsetList") {
                ImGuiMCP::SetTooltip(
                    "Returns FNIS AA set entries encoded as 6-digit decimal strings.\n"
                    "\n"
                    "Encoding: PPGGBB\n"
                    "  PP = mod_id (2 digits)\n"
                    "  GG = group_id (2 digits)\n"
                    "  BB = base slot (2 digits)\n"
                    "\n"
                    "Example:\n"
                    "  mod_id=1, group_id=2, base=3 -> \"010203\"\n"
                    "\n"
                    "The entries must remain sorted by group_id because GetGroupBaseValue() relies on that ordering.");
            } else if (name == "GetAAmodID") {
                ImGuiMCP::SetTooltip(
                    "Returns the zero-based FNIS AA mod ID for a prefix.\n"
                    "\n"
                    "Returns -1 when the prefix is not registered.\n"
                    "\n"
                    "Example:\n"
                    "  [\"aaa\", \"bbb\", \"abc\"]\n"
                    "  GetAAmodID(\"abc\") -> 2");
            } else if (name == "GetGroupBaseValue") {
                ImGuiMCP::SetTooltip(
                    "Returns the base slot assigned to an FNIS AA group.\n"
                    "\n"
                    "mod_id: 0..29\n"
                    "group_id: 0..53\n"
                    "\n"
                    "Returns 0 when the arguments are out of range or the "
                    "mod/group pair is not configured.\n"
                    "\n"
                    "The group_id is the FNIS alternate-animation group ID, "
                    "not the set-list index.");
            } else if (name == "GetAllGroupBaseValues") {
                ImGuiMCP::SetTooltip(
                    "Returns all base slots for one FNIS AA mod.\n"
                    "\n"
                    "The returned array always contains 54 elements.\n"
                    "Index = group_id.\n"
                    "Value = base slot.\n"
                    "Unconfigured groups contain 0.\n"
                    "\n"
                    "mod_id must be in the range 0..29.");
            } else if (name == "GetInstallationCRC") {
                ImGuiMCP::SetTooltip(
                    "Returns the FNIS AA installation/layout CRC.\n"
                    "\n"
                    "This is the CRC stored in g_config and is also written "
                    "to the actor graph variables by SetAnimGroup().");
            } else if (name == "IsGenerated") {
                ImGuiMCP::SetTooltip(
                    "Reports whether FNIS behavior generation is available.\n"
                    "\n"
                    "This implementation always returns true because the "
                    "required FNIS data is provided by the generated JSON.");
            } else if (name == "VersionToString") {
                ImGuiMCP::SetTooltip(
                    "Returns the configured FNIS version as a string.\n"
                    "\n"
                    "The FNIS version format is V<DD>.<DD>.<DD>.<D>.\n"
                    "\n"
                    "The version consists of:\n"
                    "  major  = major version\n"
                    "  minor1 = first minor version\n"
                    "  minor2 = second minor version\n"
                    "  flags  = release state\n"
                    "\n"
                    "Flags:\n"
                    "  0 = release\n"
                    "  1 = alpha\n"
                    "  2 = beta\n"
                    "  3 = invalid or unavailable version\n"
                    "\n"
                    "abCreature selects the normal or creature FNIS version.");
            } else if (name == "VersionCompare") {
                ImGuiMCP::SetTooltip(
                    "Compares the configured FNIS version with the specified version.\n"
                    "\n"
                    "Returns:\n"
                    "   1: Newer than the specified version.\n"
                    "   0: Match\n"
                    "  -1: Older than the specified version.");
            } else if (name == "GetMajor") {
                ImGuiMCP::SetTooltip(
                    "Returns the major component of the configured FNIS version.\n"
                    "\n"
                    "abCreature selects the normal or creature FNIS version.");
            } else if (name == "GetMinor1") {
                ImGuiMCP::SetTooltip(
                    "Returns the first minor component of the configured FNIS version.\n"
                    "\n"
                    "abCreature selects the normal or creature version.");
            } else if (name == "GetMinor2") {
                ImGuiMCP::SetTooltip(
                    "Returns the second minor component of the configured FNIS version.\n"
                    "\n"
                    "abCreature selects the normal or creature version.");
            } else if (name == "GetFlags") {
                ImGuiMCP::SetTooltip(
                    "Returns the release-state flags of the configured FNIS version.\n"
                    "\n"
                    "  0 = release\n"
                    "  1 = alpha\n"
                    "  2 = beta\n"
                    "  3 = invalid or unavailable version\n"
                    "\n"
                    "abCreature selects the normal or creature FNIS version.");
            } else if (name == "IsRelease") {
                ImGuiMCP::SetTooltip(
                    "Returns true when the selected FNIS version has no flags.\n"
                    "\n"
                    "Equivalent to:\n"
                    "  GetFlags(abCreature) == 0");
            } else if (name == "SetAnimGroup" || name == "SetAnimGroupEX") {
                ImGuiMCP::SetTooltip(
                    "Sets the FNIS animation-group graph variable on an actor.\n"
                    "\n"
                    "base > 0: value = base + number\n"
                    "base <= 0: value = base\n"
                    "\n"
                    "The value is written to FNISaa<animGroup> and the installation CRC is written to FNISaa_crc and FNISaa<animGroup>_crc.\n"
                    "\n"
                    "number must be in the range 0..9.\n"
                    "\n"
                    "SetAnimGroupEX additionally accepts skipForce3D. "
                    "When false, the player is forced into third person.");
            }
        }

        template <typename Ret, class... Args>
        void draw_ffi_signature(const char* script_name, const char* fn_name, const FFIArgs<Args...>& arguments, FFIReturn<Ret>) {
            color_text(FFIReturnType<Ret>::papyrus_name, color::YELLOW);

            ImGuiMCP::SameLine();
            ImGuiMCP::TextUnformatted(script_name);
            ImGuiMCP::SameLine(0.0f);
            ImGuiMCP::TextUnformatted(".");
            ImGuiMCP::SameLine(0.0f);
            color_text(fn_name, color::BLUE);
            if (ImGuiMCP::IsItemHovered()) {
                draw_ffi_function_doc(fn_name);
            }
            ImGuiMCP::SameLine(0.0f);
            color_text("(", color::BLUE);

            std::apply([&](const auto&... args) { draw_ffi_signature_arguments(args...); }, arguments.values);

            ImGuiMCP::SameLine(0.0f);
            color_text(")", color::BLUE);
        }

        inline void draw_ffi_argument(std::size_t test_id, const FFIIntArgument& arg) {
            ImGuiMCP::SetNextItemWidth(80.0f);
            ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, color::ORANGE);
            ImGuiMCP::InputInt(std::format("##FNIS_FFI_TEST_{}_{}", test_id, arg.label).c_str(), reinterpret_cast<int*>(arg.value), 0, 0);
            ImGuiMCP::PopStyleColor();
        }

        inline void draw_ffi_argument(std::size_t test_id, const FFIStringArgument& arg) {
            ImGuiMCP::SetNextItemWidth(160.0f);
            ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, color::GREEN);
            ImGuiMCP::InputText(std::format("##FNIS_FFI_TEST_{}_{}", test_id, arg.label).c_str(), arg.value, arg.size);
            ImGuiMCP::PopStyleColor();
        }

        inline void draw_ffi_argument(std::size_t test_id, const FFIBoolArgument& arg) {
            ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, color::ORANGE);
            ImGuiMCP::Checkbox(std::format("##FNIS_FFI_TEST_{}_{}", test_id, arg.label).c_str(), arg.value);
            ImGuiMCP::PopStyleColor();
        }

        template <class Arg>
        void draw_ffi_argument_separator(std::size_t test_id, bool& first, const Arg& arg) {
            if (!first) {
                ImGuiMCP::SameLine();
                ImGuiMCP::TextUnformatted(", ");
            }
            first = false;

            ImGuiMCP::SameLine();
            draw_ffi_argument(test_id, arg);
        }

        template <class... Args>
        void draw_ffi_arguments(std::size_t test_id, const Args&... args) {
            bool first = true;
            (draw_ffi_argument_separator(test_id, first, args), ...);
        }

        /// For int32_t, bool
        template <class Arg>
        auto get_ffi_argument_value(const Arg& arg) {
            return *arg.value;
        }

        inline RE::BSFixedString get_ffi_argument_value(const FFIStringArgument& arg) {
            return { arg.value };
        }

        template <typename Ret, class... Args>
        void draw_ffi_call_line(std::size_t test_id, const char* fn_name, const char* script_name, const FFIArgs<Args...>& arguments) {
            color_text(fn_name, color::BLUE);

            ImGuiMCP::SameLine();
            color_text("(", color::YELLOW);

            std::apply([&](const auto&... args) { draw_ffi_arguments(test_id, args...); }, arguments.values);

            ImGuiMCP::SameLine();
            color_text(")", color::YELLOW);
            ImGuiMCP::SameLine();

            if (ImGuiMCP::Button(std::format("Call##{}", test_id).c_str())) {
                std::apply(
                    [&](const auto&... args) {
                        call_ffi_function<Ret>(test_id, script_name, fn_name, get_ffi_argument_value(args)...);
                    },
                    arguments.values);
            }
        }

        template <class Ret, class... Args>
        void register_ffi_test_impl(std::size_t test_id, const char* fn_name, const char* script_name, const FFIArgs<Args...>& arguments, FFIReturn<Ret>) {
            draw_ffi_signature(script_name, fn_name, arguments, ret<Ret>);
            draw_ffi_call_line<Ret>(test_id, fn_name, script_name, arguments);
            draw_ffi_result(test_id);
            ImGuiMCP::NewLine();
        }

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define REGISTER_TEST_FN(fn_name, script_name, arguments, return_type)            \
    {                                                                             \
        static const auto id = register_ffi_test();                               \
        register_ffi_test_impl(id, fn_name, script_name, arguments, return_type); \
    }
        // NOLINTEND(cppcoreguidelines-macro-usage)

        void draw_ffi_call_tests() {
            // -----------------------------------------------------------------
            // FNIS_aa2.pex
            // -----------------------------------------------------------------

            {
                static int32_t aaNumber = 0;

                REGISTER_TEST_FN("GetAAnumber", "FNIS_aa2", args(aaNumber), ret<int32_t>);
            }

            {
                static int32_t               nMods = 0;
                static std::array<char, 256> mod{};
                static bool                  debugOutput = false;

                REGISTER_TEST_FN("GetAAprefixList", "FNIS_aa2", args(nMods, mod, debugOutput), ret<std::vector<std::string>>);
            }

            {
                static int32_t               nSets = 0;
                static std::array<char, 256> mod{};
                static bool                  debugOutput = false;

                REGISTER_TEST_FN("GetAAsetList", "FNIS_aa2", args(nSets, mod, debugOutput), ret<std::vector<std::string>>);
            }

            // -----------------------------------------------------------------
            // FNIS_aa.pex
            // -----------------------------------------------------------------

            // Skip reason: Need Actor
            // - FNIS_aa.SetAnimGroup(Actor ac, string animGroup, int base, int number, string mod, bool debugOutput) -> bool
            // - FNIS_aa.SetAnimGroupEX(Actor ac, string animGroup, int base, int number, string mod, bool debugOutput, bool skipForce3D) -> bool

            {
                static std::array<char, 256> myAAprefix{};
                static std::array<char, 256> mod{};
                static bool                  debugOutput = false;

                REGISTER_TEST_FN("GetAAmodID", "FNIS_aa", args(myAAprefix, mod, debugOutput), ret<int32_t>);
            }

            {
                static int32_t               AAmodID = 0;
                static int32_t               AAgroupID = 0;
                static std::array<char, 256> mod{};
                static bool                  debugOutput = false;

                REGISTER_TEST_FN("GetGroupBaseValue", "FNIS_aa", args(AAmodID, AAgroupID, mod, debugOutput), ret<int32_t>);
            }

            {
                static int32_t               AAmodID = 0;
                static std::array<char, 256> mod{};
                static bool                  debugOutput = false;

                REGISTER_TEST_FN("GetAllGroupBaseValues", "FNIS_aa", args(AAmodID, mod, debugOutput), ret<std::vector<int32_t>>);
            }

            {
                REGISTER_TEST_FN("GetInstallationCRC", "FNIS_aa", args0(), ret<int32_t>);
            }

            // Skip Reason: output args `GroupId, ModId, Base`
            // - FNIS_aa.GetAAsets(int nSets, int[] GroupId, int[] ModId, int[] Base, int[] Index, string mod, bool debugOutput) -> void

            // -----------------------------------------------------------------
            // FNIS.pex
            // -----------------------------------------------------------------

            // Skip reason: Need Actor
            // - FNIS.set_AACondition(Actor ac, string aaType, string mod, int aaCond, int aaDebug) -> int

            // Skip reason: void
            // - FNIS.AAReport(string longReport, string shortReport, int AAdebug, bool isError) -> void

            {
                REGISTER_TEST_FN("IsGenerated", "FNIS", args0(), ret<bool>);
            }

            {
                static bool abCreature = false;

                REGISTER_TEST_FN("VersionToString", "FNIS", args(abCreature), ret<std::string>);
            }

            {
                static int32_t iCompMajor = 0;
                static int32_t iCompMinor1 = 0;
                static int32_t iCompMinor2 = 0;
                static bool    abCreature = false;

                REGISTER_TEST_FN("VersionCompare", "FNIS", args(iCompMajor, iCompMinor1, iCompMinor2, abCreature), ret<int32_t>);
            }

            {
                static bool abCreature = false;

                REGISTER_TEST_FN("GetMajor", "FNIS", args(abCreature), ret<int32_t>);
            }

            {
                static bool abCreature = false;

                REGISTER_TEST_FN("GetMinor1", "FNIS", args(abCreature), ret<int32_t>);
            }

            {
                static bool abCreature = false;

                REGISTER_TEST_FN("GetMinor2", "FNIS", args(abCreature), ret<int32_t>);
            }

            {
                static bool abCreature = false;

                REGISTER_TEST_FN("GetFlags", "FNIS", args(abCreature), ret<int32_t>);
            }

            {
                static bool abCreature = false;

                REGISTER_TEST_FN("IsRelease", "FNIS", args(abCreature), ret<bool>);
            }
        }

        void draw_log_level(config::Config& config) {
            static constexpr std::array<const char*, 7> LOG_LEVELS = { "trace", "debug", "info", "warn", "error", "critical", "off" };

            int current = static_cast<int>(config.log_level);
            if (ImGuiMCP::Combo("Log Level", &current, LOG_LEVELS.data(), static_cast<int>(LOG_LEVELS.size()))) {
                config.log_level = static_cast<spdlog::level::level_enum>(current);
                spdlog::set_level(config.log_level);
                SPDLOG_INFO("Logger level changed to {}", spdlog::level::to_string_view(config.log_level));
            }
            if (ImGuiMCP::IsItemHovered()) {
                ImGuiMCP::SetTooltip("Change log level temporary.");
            }
        }

        void draw_reload_config() {
            if (ImGuiMCP::Button("Reload config")) {
                config::LoadGlobalConfig();
            }

            if (ImGuiMCP::IsItemHovered()) {
                ImGuiMCP::SetTooltip("Reload from Data/SKSE/Plugins/fnis_aa/config.json");
            }
        }

        void __stdcall render() {
            if (!ImGuiMCP::BeginTabBar("FnisAaTabs", ImGuiMCP::ImGuiTabBarFlags_FittingPolicyResizeDown)) {
                return;
            }

            auto draw_tab = []<class F>(const char* label, F&& draw) {
                if (ImGuiMCP::BeginTabItem(label)) {
                    if constexpr (std::is_invocable_v<F, decltype(config::g_config), decltype(config::g_diagnostics)>) {
                        std::forward<F>(draw)(config::g_config, config::g_diagnostics);
                    } else {
                        std::forward<F>(draw)(config::g_config);
                    }

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
            draw_log_level(config::g_config);
            ImGuiMCP::SameLine();
            draw_reload_config();
            ImGuiMCP::Separator();

            ImGuiMCP::Text("Papyrus ffi call tests");
            ImGuiMCP::Spacing();
            draw_ffi_call_tests();
        }
    }

    // NOLINTBEGIN(misc-use-internal-linkage)
    void Register() {
        if (!SKSEMenuFramework::IsInstalled()) {
            return;
        }

        SKSEMenuFramework::SetSection(SECTION_NAME);
        SKSEMenuFramework::AddSectionItem("Status", render);
        SKSEMenuFramework::AddSectionItem("Debug", render_debug_section);
    }
    // NOLINTEND(misc-use-internal-linkage)
}
