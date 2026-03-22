#include "cmd_cheat/cmd_model.hh"
#include "core.hh"
#include "persistence.hh"
#include "ui.hh"

namespace {
    static void skse_listener(SKSE::MessagingInterface::Message* a_msg) {
        switch (a_msg->type) {
        case SKSE::MessagingInterface::kPostLoadGame:  // Fired after loading a game save.
            {
                skse_plugin_core::init();
                ui::Register();

                skyrim_cheat::persistence::LoadAll();
                cmd_cheat::model::OnLoadGame();
                return;
            }

        case SKSE::MessagingInterface::kNewGame:     // Fired when starting a new game.
        case SKSE::MessagingInterface::kDataLoaded:  // Fired after all game data has loaded.
            {
                skyrim_cheat::persistence::LoadAll();
                cmd_cheat::model::OnGameLoaded();
                return;
            }

        case SKSE::MessagingInterface::kPostLoad:      // Fired after all plugins are loaded.
        case SKSE::MessagingInterface::kPostPostLoad:  // Fired after all `PostLoad` events have completed.
        case SKSE::MessagingInterface::kInputLoaded:   // Fired when the input system is loaded.
        case SKSE::MessagingInterface::kPreLoadGame:   // Fired before loading a game save.
        case SKSE::MessagingInterface::kSaveGame:      // Fired before saving a game.
        case SKSE::MessagingInterface::kDeleteGame:    // Fired before deleting a game save.
        default:
            return;
        }
    }
}

extern "C" __declspec(dllexport) bool
    SKSEPlugin_Load(const SKSE::LoadInterface* a_interface) {
    SKSE::Init(a_interface);

    auto msg = SKSE::GetMessagingInterface();
    if (msg == nullptr) {
        return false;
    }

    msg->RegisterListener("SKSE", ::skse_listener);

    return true;
}
