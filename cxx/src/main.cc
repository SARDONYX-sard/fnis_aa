#include <bridge/src/bridge.rs.h>

#include "config.hh"
#include "fnis.hh"
#include "fnis_aa.hh"
#include "fnis_aa2.hh"
// #include "fnis_quest.hh"

namespace {
    static void skse_listener(SKSE::MessagingInterface::Message* a_msg) {
        switch (a_msg->type) {
        case SKSE::MessagingInterface::kPostLoadGame:  // Fired after loading a game save.
            {
                bridge::bridge_init();
                // fnis_quest::OnPostLoadGame();
                return;
            }

        case SKSE::MessagingInterface::kNewGame:     // Fired when starting a new game.
        case SKSE::MessagingInterface::kDataLoaded:  // Fired after all game data has loaded.
            {
                config::OnLoaded();
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
    spdlog::set_level(spdlog::level::trace);
    // [%T.%e] time ms
    // [%t] thread id
    // [%l] (Trace/Debug...)
    // [%s:%#] file:line (when use macros)
    // [!] function
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [thread_id:%t] %l [%s:%#] %v");  // like `tracing` crate display

    auto msg = SKSE::GetMessagingInterface();
    if (msg == nullptr) {
        return false;
    }

    msg->RegisterListener("SKSE", ::skse_listener);

    SKSE::GetPapyrusInterface()->Register(FNIS_aa2::Register);
    SKSE::GetPapyrusInterface()->Register(FNIS_aa::Register);
    SKSE::GetPapyrusInterface()->Register(FNIS::Register);

    return true;
}
