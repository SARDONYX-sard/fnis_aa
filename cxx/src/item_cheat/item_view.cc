#include <SKSEMenuFramework.h>

#include "item_cheat/item_model.hh"
#include "item_cheat/item_view.hh"

#include "persistence.hh"
#include "ui.hh"

namespace item_cheat::view {
    void Render() {
        using namespace item_cheat::model;

        if (GetPlugins().empty()) {
            BuildPluginList();
            return;
        }

        // --- Plugin search + dropdown ---
        ImGuiMCP::TextUnformatted("Plugin: ");
        ImGuiMCP::SameLine();
        ImGuiMCP::InputText("##pluginSearch", filterPlugin.data(), filterPlugin.size());

        ImGuiMCP::SameLine();

        const char* pluginPreview = selectedPlugin >= 0 ? GetPlugins()[selectedPlugin].name.c_str() : "<Select>";
        if (ImGuiMCP::BeginCombo("##pluginDropdown", pluginPreview)) {
            const auto& plugins = GetPlugins();

            for (const auto& [i, plugin] : std::views::enumerate(plugins)) {
                if (!filterPlugin.empty() && plugin.name.find(filterPlugin.data()) == std::string::npos) {
                    continue;
                }

                if (ImGuiMCP::Selectable(plugin.name.c_str(), selectedPlugin == i)) {
                    selectedPlugin = i;
                    BuildItemList(plugin.file);
                    skyrim_cheat::persistence::SaveAll();
                }
            }
            ImGuiMCP::EndCombo();
        }

        // --- FormType search + dropdown ---
        ImGuiMCP::Text("Type: ");
        ImGuiMCP::SameLine();
        ImGuiMCP::InputText("##formTypeSearch", filterFormType.data(), filterFormType.size());

        ImGuiMCP::SameLine();

        const char* formTypePreview = ToFormTypeName(view::selectedFormType);
        if (ImGuiMCP::BeginCombo("##formTypeDropdown", formTypePreview)) {
            for (auto& [type, label] : std::span{ kFormTypeList }) {
                if (!filterFormType.empty() && std::string_view(label).find(filterFormType.data()) == std::string::npos) {
                    continue;
                }

                if (ImGuiMCP::Selectable(label, view::selectedFormType == type)) {
                    view::selectedFormType = type;
                    skyrim_cheat::persistence::SaveAll();
                }
            }
            ImGuiMCP::EndCombo();
        }

        // --- Form List Search ---
        ImGuiMCP::Text("Form List: ");
        ImGuiMCP::SameLine();
        ImGuiMCP::InputText("##formListSearch", filterFormList.data(), filterFormList.size());

        // -- Get all button
        ImGuiMCP::SameLine();
        if (ImGuiMCP::Button("Get all")) {
            auto player =
                RE::PlayerCharacter::GetSingleton()
                    ->As<RE::TESObjectREFR>();
            for (auto& item : GetItems()) {
                bool matches = true;
                if (view::selectedFormType != RE::FormType::None && view::selectedFormType != item.formType) {
                    matches = false;
                }
                if (!filterFormList.empty() && item.name.find(filterFormList.data()) == std::string::npos) {
                    matches = false;
                }

                if (matches) {
                    player->AddObjectToContainer(item.object, nullptr, 1, nullptr);
                }
            }
        }

        // --- Table ---
        ImGuiMCP::BeginChild("TableScrollRegion", ImGuiMCP::ImVec2{ 0, tableHeight }, true);
        if (ImGuiMCP::BeginTable("items", 2, ui::Example3::flags)) {
            ImGuiMCP::TableSetupColumn("Name");
            ImGuiMCP::TableSetupColumn("Type");
            ImGuiMCP::TableHeadersRow();

            for (auto& item : GetItems()) {
                bool matches = true;
                if (selectedFormType != RE::FormType::None && selectedFormType != item.formType) {
                    matches = false;
                }
                if (!filterFormList.empty() && item.name.find(filterFormList.data()) == std::string::npos) {
                    matches = false;
                }

                if (!matches) {
                    continue;
                }

                ImGuiMCP::TableNextRow();
                ImGuiMCP::TableSetColumnIndex(0);

                if (ImGuiMCP::Selectable(
                        item.name.c_str(),
                        GetSelectedItem() == item.object,
                        ImGuiMCP::ImGuiSelectableFlags_SpanAllColumns)) {
                    SelectItem(item.object);
                    skyrim_cheat::persistence::SaveAll();
                }

                ImGuiMCP::TableSetColumnIndex(1);
                ImGuiMCP::TextUnformatted(ToFormTypeName(item.formType));
            }

            ImGuiMCP::EndTable();
        }
        ImGuiMCP::EndChild();

        // --- Add / Amount ---
        if (GetSelectedItem()) {
            ImGuiMCP::Separator();

            if (ImGuiMCP::Button("Add")) {
                AddSelectedItem(addCount);
                skyrim_cheat::persistence::SaveAll();
            }
            ImGuiMCP::SameLine();
            if (ImGuiMCP::SliderInt("Amount", &addCount, 1, 999999)) {
                skyrim_cheat::persistence::SaveAll();
            };
        }
    }

}  // namespace item_cheat::view
