#pragma once

#include "config.hh"

namespace fnis_aa::FNIS_aa2 {
    bool Register(RE::BSScript::IVirtualMachine* vm);
}

namespace fnis_aa::FNIS_aa {
    bool Register(RE::BSScript::IVirtualMachine* vm);
}

namespace fnis_aa::FNIS {
    using namespace fnis_aa::config;

    enum VersionCompareResult : int32_t {
        NotInstalledOrOlder = -1,
        Match = 0,
        Newer = 1,
    };

    /// Compares the installed FNIS version against the required version.
    constexpr VersionCompareResult fnis_version_comp(const FNISVersion& ver, int32_t comp_major, int32_t comp_minor1, int32_t comp_minor2) {
        if (ver.flags == 3) {
            return VersionCompareResult::NotInstalledOrOlder;
        }

        if (ver.major != comp_major) {
            return ver.major > comp_major ? VersionCompareResult::Newer : VersionCompareResult::NotInstalledOrOlder;
        }
        if (ver.minor1 != comp_minor1) {
            return ver.minor1 > comp_minor1 ? VersionCompareResult::Newer : VersionCompareResult::NotInstalledOrOlder;
        }
        if (ver.minor2 != comp_minor2) {
            return ver.minor2 > comp_minor2 ? VersionCompareResult::Newer : VersionCompareResult::NotInstalledOrOlder;
        }

        return VersionCompareResult::Match;
    }

    static_assert([] {
        constexpr FNISVersion version{
            .major = 7,
            .minor1 = 6,
            .minor2 = 0,
            .flags = 0,
        };
        return fnis_version_comp(version, 7, 2, 0) == VersionCompareResult::Newer;
    }());

    static_assert([] {
        constexpr FNISVersion version{
            .major = 7,
            .minor1 = 6,
            .minor2 = 0,
            .flags = 0,
        };
        return fnis_version_comp(version, 7, 6, 0) == VersionCompareResult::Match;
    }());

    static_assert([] {
        constexpr FNISVersion version{
            .major = 7,
            .minor1 = 6,
            .minor2 = 0,
            .flags = 0,
        };
        return fnis_version_comp(version, 8, 0, 0) == VersionCompareResult::NotInstalledOrOlder;
    }());

    static_assert([] {
        constexpr FNISVersion version{
            .major = 7,
            .minor1 = 6,
            .minor2 = 0,
            .flags = 3,
        };
        return fnis_version_comp(version, 7, 6, 0) == VersionCompareResult::NotInstalledOrOlder;
    }());

    bool Register(RE::BSScript::IVirtualMachine* vm);
}

namespace fnis_aa::menu {
    void UpdateSnapshot();
    void Register();
}
