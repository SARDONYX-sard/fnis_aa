#include "persistence.hh"
#include "cmd_cheat/cmd_model.hh"
#include "item_cheat/item_model.hh"
#include "item_cheat/item_view.hh"
#include "toml11/parser.hpp"
#include <toml.hpp>

namespace skyrim_cheat::persistence {
    using namespace item_cheat::model;
    using namespace cmd_cheat::model;
    using namespace item_cheat::view;

    // Return `PathBuf`
    std::filesystem::path GetDataFile() {
        std::filesystem::path path = "Data/SKSE/Plugins";
        const auto            pluginName = std::string{ SKSE::GetPluginName() };
        std::string           filename = pluginName + ".toml";
        path /= filename;
        return path;
    }

    void SaveAll() {
        toml::table tbl;

        // --- Cmd ---
        toml::array cmd_list{};
        for (const auto& c : GetCommands()) {
            if (!c.cmd.empty()) {
                toml::table cmd_entry;
                cmd_entry["cmd"] = c.cmd;
                cmd_entry["timing"] = (c.timing == CmdTiming::OnEveryLoad ? "every" : "once");
                cmd_entry["enabled"] = c.enabled;
                cmd_list.emplace_back(cmd_entry);
            }
        }
        tbl["cmds"]["list"] = cmd_list;

        // --- item UI ---
        tbl["item_ui"]["addCount"] = addCount;
        tbl["item_ui"]["tableHeight"] = tableHeight;
        tbl["item_ui"]["selectedFormType"] = std::string(item_cheat::view::FormTypeToString(selectedFormType));

        // --- item UI ---
        tbl["item_ui"]["addCount"] = addCount;
        tbl["item_ui"]["tableHeight"] = tableHeight;

        // --- item plugins ---
        for (const auto& [idx, p] : std::views::enumerate(GetPlugins())) {
            if (!p.name.empty() && selectedPlugin == static_cast<int64_t>(idx)) {
                toml::table plugin_entry;
                plugin_entry["name"] = p.name;
                tbl["items"]["selected_plugin"] = plugin_entry;
            }
        }

        std::ofstream ofs(GetDataFile());
        if (ofs.is_open()) {
            toml::value v{ std::move(tbl) };
            ofs << toml::format(v);
        }
    }

    void LoadAll() {
        if (!std::filesystem::exists(GetDataFile()))
            return;

        toml::value v;
        try {
            v = toml::parse(GetDataFile().string());
        } catch (toml::syntax_error& e) {
            spdlog::warn("[skyrim_cheat] TOML parse error: {}", e.what());
            return;
        }

        // --- cmds ---
        g_cmds.clear();
        if (v.contains("cmds") && v["cmds"].contains("list")) {
            for (auto& item : v["cmds"]["list"].as_array()) {
                std::string cmd = item["cmd"].as_string();
                std::string timing = item["timing"].as_string();
                CmdTiming   t = (timing == "once" ? CmdTiming::OnGameLoadOnce : CmdTiming::OnEveryLoad);
                bool        enabled = true;
                if (item.contains("enabled")) {
                    enabled = item["enabled"].as_boolean();
                }
                g_cmds.emplace_back(CmdEntry{ .cmd = cmd, .timing = t, .enabled = enabled });
            }
        }

        // --- item UI ---
        if (v.contains("item_ui")) {
            auto& ui = v["item_ui"];

            if (ui.contains("addCount")) {
                addCount = static_cast<int>(ui["addCount"].as_integer());
            }

            if (ui.contains("tableHeight")) {
                tableHeight = static_cast<float>(ui["tableHeight"].as_floating());
            }

            if (ui.contains("selectedFormType")) {
                selectedFormType = item_cheat::view::StringToFormType(ui["selectedFormType"].as_string());
            }
        }

        // --- item plugins ---
        if (v.contains("items") && v["items"].contains("selected_plugin")) {
            auto& selected = v["items"]["selected_plugin"];
            if (selected.contains("name")) {
                if (GetPlugins().empty()) {
                    BuildPluginList();
                }
                const auto& plugins = GetPlugins();

                const std::string& selectedName = selected["name"].as_string();
                for (size_t i = 0; i < plugins.size(); ++i) {
                    if (plugins[i].name == selectedName) {
                        selectedPlugin = static_cast<int>(i);
                        break;
                    }
                }
            }
        }
    }
}
