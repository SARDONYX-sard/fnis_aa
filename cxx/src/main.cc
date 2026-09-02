#include <bridge/src/bridge.rs.h>

#include "config.hh"
#include "fnis.hh"
#include "fnis_aa.hh"
#include "fnis_aa2.hh"

namespace {
    void skse_listener(SKSE::MessagingInterface::Message* a_msg) {
        switch (a_msg->type) {
        case SKSE::MessagingInterface::kPostLoadGame:  // Fired after loading a game save.
            {
                bridge::bridge_init();
                return;
            }

        case SKSE::MessagingInterface::kNewGame:     // Fired when starting a new game.
        case SKSE::MessagingInterface::kDataLoaded:  // Fired after all game data has loaded.
            {
                config::OnLoaded();
                fnis_aa::menu::UpdateSnapshot();
                fnis_aa::menu::Register();
                return;
            }
        default:
            return;
        }
    }
}

extern "C" __declspec(dllexport) bool
    SKSEPlugin_Load(const SKSE::LoadInterface* a_interface) {
    SKSE::Init(a_interface);
    spdlog::set_level(spdlog::level::debug);

    // - [%T.%e]: time ms
    // - [%t]:    thread id
    // - [%l]:    (Trace/Debug...)
    // - [%s:%#]: file:line (when use macros)
    // - [!]:     function
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
