#pragma once
#include <array>
#include <cstdint>
#include <string_view>

struct AAGroupInfo {
    std::string_view name;
    int32_t          id;
};

// NOTE: Sorting by name enables binary search
// group name, id
inline constexpr std::array<AAGroupInfo, 54> kAltGroupTable = { {
    { "_1hmatk", 19 },
    { "_1hmatkpow", 20 },
    { "_1hmblock", 21 },
    { "_1hmeqp", 37 },
    { "_1hmidle", 1 },
    { "_1hmmt", 13 },
    { "_1hmstag", 22 },
    { "_2hmatk", 23 },
    { "_2hmatkpow", 24 },
    { "_2hmblock", 25 },
    { "_2hmeqp", 39 },
    { "_2hmidle", 2 },
    { "_2hmmt", 14 },
    { "_2hmstag", 26 },
    { "_2hwatk", 27 },
    { "_2hwatkpow", 28 },
    { "_2hwblock", 29 },
    { "_2hweqp", 38 },
    { "_2hwidle", 3 },
    { "_2hwstag", 30 },
    { "_axeeqp", 40 },
    { "_bowatk", 31 },
    { "_bowblock", 32 },
    { "_boweqp", 41 },
    { "_bowidle", 4 },
    { "_bowmt", 15 },
    { "_cboweqp", 42 },
    { "_cbowidle", 5 },
    { "_dageqp", 43 },
    { "_dw", 50 },
    { "_h2hatk", 33 },
    { "_h2hatkpow", 34 },
    { "_h2heqp", 44 },
    { "_h2hidle", 6 },
    { "_h2hstag", 35 },
    { "_jump", 51 },
    { "_maceqp", 45 },
    { "_magatk", 36 },
    { "_magcastmt", 17 },
    { "_magcon", 49 },
    { "_mageqp", 46 },
    { "_magidle", 7 },
    { "_magmt", 16 },
    { "_mt", 10 },
    { "_mtidle", 0 },
    { "_mtturn", 12 },
    { "_mtx", 11 },
    { "_shield", 53 },
    { "_shout", 48 },
    { "_sneakidle", 8 },
    { "_sneakmt", 18 },
    { "_sprint", 52 },
    { "_staffidle", 9 },
    { "_stfeqp", 47 },
} };

static_assert([] {
    for (std::size_t i = 1; i < kAltGroupTable.size(); ++i)
        if (kAltGroupTable[i].name <= kAltGroupTable[i - 1].name)
            return false;
    return true;
}(),
    "kAltGroupTable must be sorted by name");

[[nodiscard]] constexpr const AAGroupInfo* GetAltGroup(std::string_view name) noexcept {
    for (const auto& entry : kAltGroupTable)
        if (entry.name == name)
            return &entry;
    return nullptr;
}
