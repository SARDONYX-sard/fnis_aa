#pragma once
#include <string>
#include <vector>

namespace cmd_cheat::model {
    enum class CmdTiming {
        OnGameLoadOnce,
        OnEveryLoad
    };

    struct CmdEntry {
        std::string cmd;
        CmdTiming   timing;
        bool        executedOnce = false;
        bool        enabled = true;
    };

    const std::vector<CmdEntry>& GetCommands();
    std::vector<CmdEntry>&       GetCommandsMut();
    void                         AddCommand(std::string cmd, CmdTiming timing);
    void                         RemoveCommand(size_t index);

    void OnGameLoaded();
    void OnLoadGame();

    inline std::vector<CmdEntry> g_cmds{};
    inline bool                  g_gameLoadedOnce = false;
}
