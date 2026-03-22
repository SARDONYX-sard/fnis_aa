#include "item_cheat/item_model.hh"

namespace item_cheat::model {

    namespace {
        static std::vector<PluginEntry> g_plugins{};       // All esm,esp list
        static std::vector<ItemEntry>   g_items{};         // selected plugin esp forms
        static RE::TESBoundObject*      g_selectedItem{};  // current selected table cell item
    }

    const std::vector<PluginEntry>& GetPlugins() { return g_plugins; }

    // One esm,esp forms
    const std::vector<ItemEntry>& GetItems() { return g_items; }
    RE::TESBoundObject*           GetSelectedItem() { return g_selectedItem; }

    void BuildPluginList() {
        g_plugins.clear();

        auto dh = RE::TESDataHandler::GetSingleton();
        if (!dh)
            return;

        for (auto* file : dh->files) {
            if (!file)
                continue;
            g_plugins.emplace_back(file, std::string{ file->GetFilename() });
        }
    }

    void BuildItemList(RE::TESFile* plugin) {
        g_items.clear();
        g_selectedItem = nullptr;
        if (!plugin)
            return;

        auto [map, lock] = RE::TESForm::GetAllFormsByEditorID();
        auto _guard(lock.get());

        for (auto& [_, form] : *map) {
            if (!form || form->GetFile(0) != plugin) {
                continue;
            }

            auto bound = form->As<RE::TESBoundObject>();
            if (!bound) {
                continue;
            }

            auto trim = [](const std::string_view& s) {
                size_t start = 0;
                while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
                size_t end = s.size();
                while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
                return s.substr(start, end - start);
            };

            const char* name = bound->GetName();
            if (!name) {
                continue;
            }

            auto trimmed = trim(name);
            if (trimmed.empty())
                continue;

            if (bound->formType.any(
                    RE::FormType::Weapon,
                    RE::FormType::Armor,
                    RE::FormType::Ammo,
                    RE::FormType::Ingredient,
                    RE::FormType::AlchemyItem,
                    RE::FormType::Book,
                    RE::FormType::Scroll,
                    RE::FormType::Misc,
                    RE::FormType::NPC)) {
                g_items.emplace_back(bound, form->formType.get(), std::string{ name });
            }
        }
    }

    void SelectItem(RE::TESBoundObject* object) {
        g_selectedItem = object;
    }

    void AddSelectedItem(int count) {
        if (!g_selectedItem)
            return;

        auto player =
            RE::PlayerCharacter::GetSingleton()
                ->As<RE::TESObjectREFR>();

        player->AddObjectToContainer(
            g_selectedItem, nullptr, count, nullptr);
    }
}
