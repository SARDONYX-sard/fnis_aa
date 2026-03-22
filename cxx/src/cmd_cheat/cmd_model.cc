#include "cmd_cheat/cmd_model.hh"

namespace cmd_cheat::model {
    namespace {
        static void Execute(CmdEntry& entry) {
            RE::Console::ExecuteCommand(entry.cmd.c_str());
        }
    }

    const std::vector<CmdEntry>& GetCommands() {
        return g_cmds;
    }

    std::vector<CmdEntry>& GetCommandsMut() {
        return g_cmds;
    }

    void AddCommand(std::string cmd, CmdTiming timing) {
        g_cmds.emplace_back(CmdEntry{ .cmd = std::move(cmd), .timing = timing });
    }

    void RemoveCommand(size_t index) {
        if (index < g_cmds.size()) {
            g_cmds.erase(g_cmds.begin() + static_cast<int>(index));
        }
    }

    void OnGameLoaded() {
        if (g_gameLoadedOnce)
            return;

        g_gameLoadedOnce = true;

        for (auto& cmd : g_cmds) {
            if (cmd.enabled && cmd.timing == CmdTiming::OnGameLoadOnce && !cmd.executedOnce) {
                spdlog::info("[OnGameLoaded] Executing command: {}", cmd.cmd);
                Execute(cmd);
                cmd.executedOnce = true;
            } else if (!cmd.enabled && cmd.timing == CmdTiming::OnGameLoadOnce) {
                spdlog::info("[OnGameLoaded] Skipped disabled command: {}", cmd.cmd);
            }
        }
    }

    void OnLoadGame() {
        for (auto& cmd : g_cmds) {
            if (cmd.enabled && cmd.timing == CmdTiming::OnEveryLoad) {
                spdlog::info("[OnLoadGame] Executing command: {}", cmd.cmd);
                Execute(cmd);
            } else if (!cmd.enabled && cmd.timing == CmdTiming::OnEveryLoad) {
                spdlog::info("[OnLoadGame] Skipped disabled command: {}", cmd.cmd);
            }
        }
    }

}
