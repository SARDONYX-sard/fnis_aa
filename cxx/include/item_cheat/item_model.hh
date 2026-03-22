#pragma once

#include <string>
#include <vector>

namespace item_cheat::model {
    struct PluginEntry {
    public:
        RE::TESFile* file{};
        std::string  name;

        PluginEntry(RE::TESFile* a_file, std::string a_name) : file(a_file), name(std::move(a_name)) {}
    };

    struct ItemEntry {
    public:
        RE::TESBoundObject* object{};
        RE::FormType        formType{};
        std::string         name;

        ItemEntry(
            RE::TESBoundObject* a_object,
            RE::FormType        a_formType,
            std::string         a_name) : object(a_object), formType(a_formType), name(std::move(a_name)) {}
    };

    // state
    const std::vector<PluginEntry>& GetPlugins();
    const std::vector<ItemEntry>&   GetItems();

    RE::TESBoundObject* GetSelectedItem();

    // actions
    void BuildPluginList();
    void BuildItemList(RE::TESFile* plugin);

    void SelectItem(RE::TESBoundObject* object);
    void AddSelectedItem(int count);
}
