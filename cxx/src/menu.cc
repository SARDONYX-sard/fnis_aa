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

        enum class DiagnosticLevel : uint8_t {
            info,
            success,
            warning,
            error,
        };

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
            case config::DiagnosticLevel::success:
                return "SUCCESS";
            case config::DiagnosticLevel::warning:
                return "WARN";

            case config::DiagnosticLevel::error:
                return "ERROR";
            }

            return "UNKNOWN";
        }

        [[nodiscard]] ImGuiMCP::ImVec4 diagnostic_color(
            config::DiagnosticLevel level) noexcept {
            switch (level) {
            case config::DiagnosticLevel::success:
                return { .x = 0.30f, .y = 0.85f, .z = 0.35f, .w = 1.0f };

            case config::DiagnosticLevel::warning:
                return { .x = 0.95f, .y = 0.75f, .z = 0.20f, .w = 1.0f };

            case config::DiagnosticLevel::error:
                return { .x = 0.95f, .y = 0.25f, .z = 0.25f, .w = 1.0f };
            }

            return { .x = 1.0f, .y = 1.0f, .z = 1.0f, .w = 1.0f };
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
            if (!ImGuiMCP::BeginTable(
                    "FnisAaModLayout",
                    4,
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
                draw_status(
                    diagnostic.level,
                    diagnostic_label(diagnostic.level),
                    diagnostic.message.c_str());
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

        void __stdcall render() {
            if (!ImGuiMCP::BeginTabBar("FnisAaTabs", ImGuiTabBarFlags_FittingPolicyResizeDown)) {
                return;
            }

            if (ImGuiMCP::BeginTabItem("Overview")) {
                draw_overview();
                ImGuiMCP::EndTabItem();
            }

            if (ImGuiMCP::BeginTabItem("Mod Layout")) {
                draw_mod_layout();
                ImGuiMCP::EndTabItem();
            }

            if (ImGuiMCP::BeginTabItem("Slot Map")) {
                draw_slot_map();
                ImGuiMCP::EndTabItem();
            }

            if (ImGuiMCP::BeginTabItem("Diagnostics")) {
                draw_diagnostics();
                ImGuiMCP::EndTabItem();
            }

            ImGuiMCP::EndTabBar();
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
    }
    // NOLINTEND(misc-use-internal-linkage)
}
