#include <bridge/src/bridge.rs.h>

#include "config.hh"
#include "fnis_aa.hh"

namespace {
    void SkseListener(SKSE::MessagingInterface::Message* a_msg) {
        switch (a_msg->type) {
        case SKSE::MessagingInterface::kPostLoadGame:  // Fired after loading a game save.
            {
                fnis_aa::bridge::init();
                return;
            }
        case SKSE::MessagingInterface::kNewGame:     // Fired when starting a new game.
        case SKSE::MessagingInterface::kDataLoaded:  // Fired after all game data has loaded.
            {
                fnis_aa::config::NewGlobalConfig();
                fnis_aa::menu::UpdateSnapshot();
                return;
            }

        default:
            return;
        }
    }
}

extern "C" __declspec(dllexport) bool SKSEPlugin_Load(const SKSE::LoadInterface* a_interface) {
    SKSE::Init(a_interface);

    // - [%T.%e]: time ms
    // - [%t]:    thread id
    // - [%l]:    (Trace/Debug...)
    // - [%s:%#]: file:line (when use macros)
    // - [!]:     function
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [thread_id:%t] %l [%s:%#] %v");  // like `tracing` crate display
    spdlog::set_level(spdlog::level::debug);

    auto msg = SKSE::GetMessagingInterface();
    if (msg == nullptr) {
        return false;
    }

    msg->RegisterListener("SKSE", ::SkseListener);
    SKSE::GetPapyrusInterface()->Register(
        fnis_aa::FNIS_aa2::Register,
        fnis_aa::FNIS_aa::Register,
        fnis_aa::FNIS::Register  //
    );

    fnis_aa::menu::Register();

    return true;
}
