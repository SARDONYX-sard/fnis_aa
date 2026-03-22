#include "cmd_cheat/cmd_model.hh"
#include "persistence.hh"
#include "ui.hh"

namespace cmd_cheat::view {

    namespace {
        static std::array<char, 256> inputCmd{};
        static int                   timingIndex = 0;

        constexpr std::array<const char*, 2> timingLabels = {
            "Once",
            "Every"
        };
    }

    void Render() {
        using namespace cmd_cheat::model;

        ImGuiMCP::TextUnformatted("Console Command:");
        ImGuiMCP::InputText("##cmdInput", inputCmd.data(), static_cast<int>(inputCmd.size()));

        ImGuiMCP::Combo("Event Timing", &timingIndex, timingLabels.data(), static_cast<int>(timingLabels.size()));
        if (ImGuiMCP::IsItemHovered()) {
            ImGuiMCP::BeginTooltip();
            ImGuiMCP::TextUnformatted("Once: Executes when the game loads");
            ImGuiMCP::TextUnformatted("Every: Executes every time a save is loaded");
            ImGuiMCP::EndTooltip();
        }

        if (ImGuiMCP::Button("Add Command")) {
            if (strlen(inputCmd.data()) > 0) {
                AddCommand(
                    inputCmd.data(),
                    timingIndex == 0 ? CmdTiming::OnGameLoadOnce : CmdTiming::OnEveryLoad);
                skyrim_cheat::persistence::SaveAll();
                inputCmd[0] = '\0';
            }
        }

        ImGuiMCP::Separator();

        if (ImGuiMCP::BeginTable("cmd_table", 4, ui::Example3::flags)) {
            using namespace ImGuiMCP;

            ImGuiMCP::TableSetupColumn("Enabled", ImGuiTableColumnFlags_WidthFixed, 50.0f);
            ImGuiMCP::TableSetupColumn("Command", ImGuiTableColumnFlags_WidthStretch, 220.0f);
            ImGuiMCP::TableSetupColumn("Timing", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGuiMCP::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGuiMCP::TableHeadersRow();

            auto& cmds = GetCommandsMut();

            for (size_t i = 0; i < cmds.size(); ++i) {
                auto& cmd = cmds[i];

                ImGuiMCP::TableNextRow();

                // --- Enabled Checkbox ---
                ImGuiMCP::TableSetColumnIndex(0);
                if (ImGuiMCP::Checkbox(("##enabled_" + std::to_string(i)).c_str(), &cmd.enabled)) {
                    skyrim_cheat::persistence::SaveAll();
                }

                // --- Editable Command String ---
                ImGuiMCP::TableSetColumnIndex(1);
                ImGuiMCP::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 2));
                ImGuiMCP::PushItemWidth(-1);
                static std::array<char, 256> buffer{};
                strncpy_s(buffer.data(), buffer.size(), cmd.cmd.c_str(), _TRUNCATE);
                if (ImGuiMCP::InputText(("##cmd_" + std::to_string(i)).c_str(), buffer.data(), buffer.size())) {
                    cmd.cmd = buffer.data();
                    skyrim_cheat::persistence::SaveAll();
                }
                ImGuiMCP::PopItemWidth();
                ImGuiMCP::PopStyleVar();

                // --- Editable Timing ---
                ImGuiMCP::TableSetColumnIndex(2);
                ImGuiMCP::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 2));
                ImGuiMCP::PushItemWidth(-1);
                int localTimingIndex = cmd.timing == CmdTiming::OnGameLoadOnce ? 0 : 1;
                if (ImGuiMCP::Combo(("##timing_" + std::to_string(i)).c_str(), &localTimingIndex, timingLabels.data(), static_cast<int>(timingLabels.size()))) {
                    cmd.timing = localTimingIndex == 0 ? CmdTiming::OnGameLoadOnce : CmdTiming::OnEveryLoad;
                    skyrim_cheat::persistence::SaveAll();
                }
                ImGuiMCP::PopItemWidth();
                ImGuiMCP::PopStyleVar();

                // --- Delete Button ---
                ImGuiMCP::TableSetColumnIndex(3);
                if (ImGuiMCP::SmallButton(("Delete##" + std::to_string(i)).c_str())) {
                    RemoveCommand(i);
                    skyrim_cheat::persistence::SaveAll();
                    break;
                }
            }

            ImGuiMCP::EndTable();
        }
    }
}
