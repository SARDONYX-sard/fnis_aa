#pragma once
#include <filesystem>

namespace skyrim_cheat::persistence {

    std::filesystem::path GetDataFile();
    void                  SaveAll();
    void                  LoadAll();
}
