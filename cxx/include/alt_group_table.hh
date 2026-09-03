#pragma once

#include <algorithm>
#include <array>

namespace fnis_aa {

    struct AAGroupInfo {
        std::string_view name;
        int32_t          id;
    };

    // NOTE: Sorting by name enables binary search group name, id
    inline constexpr std::array<AAGroupInfo, 54> ALT_GROUP_TABLE = { {
        { .name = "_1hmatk", .id = 19 },
        { .name = "_1hmatkpow", .id = 20 },
        { .name = "_1hmblock", .id = 21 },
        { .name = "_1hmeqp", .id = 37 },
        { .name = "_1hmidle", .id = 1 },
        { .name = "_1hmmt", .id = 13 },
        { .name = "_1hmstag", .id = 22 },
        { .name = "_2hmatk", .id = 23 },
        { .name = "_2hmatkpow", .id = 24 },
        { .name = "_2hmblock", .id = 25 },
        { .name = "_2hmeqp", .id = 39 },
        { .name = "_2hmidle", .id = 2 },
        { .name = "_2hmmt", .id = 14 },
        { .name = "_2hmstag", .id = 26 },
        { .name = "_2hwatk", .id = 27 },
        { .name = "_2hwatkpow", .id = 28 },
        { .name = "_2hwblock", .id = 29 },
        { .name = "_2hweqp", .id = 38 },
        { .name = "_2hwidle", .id = 3 },
        { .name = "_2hwstag", .id = 30 },
        { .name = "_axeeqp", .id = 40 },
        { .name = "_bowatk", .id = 31 },
        { .name = "_bowblock", .id = 32 },
        { .name = "_boweqp", .id = 41 },
        { .name = "_bowidle", .id = 4 },
        { .name = "_bowmt", .id = 15 },
        { .name = "_cboweqp", .id = 42 },
        { .name = "_cbowidle", .id = 5 },
        { .name = "_dageqp", .id = 43 },
        { .name = "_dw", .id = 50 },
        { .name = "_h2hatk", .id = 33 },
        { .name = "_h2hatkpow", .id = 34 },
        { .name = "_h2heqp", .id = 44 },
        { .name = "_h2hidle", .id = 6 },
        { .name = "_h2hstag", .id = 35 },
        { .name = "_jump", .id = 51 },
        { .name = "_maceqp", .id = 45 },
        { .name = "_magatk", .id = 36 },
        { .name = "_magcastmt", .id = 17 },
        { .name = "_magcon", .id = 49 },
        { .name = "_mageqp", .id = 46 },
        { .name = "_magidle", .id = 7 },
        { .name = "_magmt", .id = 16 },
        { .name = "_mt", .id = 10 },
        { .name = "_mtidle", .id = 0 },
        { .name = "_mtturn", .id = 12 },
        { .name = "_mtx", .id = 11 },
        { .name = "_shield", .id = 53 },
        { .name = "_shout", .id = 48 },
        { .name = "_sneakidle", .id = 8 },
        { .name = "_sneakmt", .id = 18 },
        { .name = "_sprint", .id = 52 },
        { .name = "_staffidle", .id = 9 },
        { .name = "_stfeqp", .id = 47 },
    } };
    static_assert(std::ranges::is_sorted(ALT_GROUP_TABLE, {}, &AAGroupInfo::name), "kAltGroupTable must be sorted by name");

    // Binary search for the first element whose projected key is not less than `name`.
    // Requires `ALT_GROUP_TABLE` to be sorted by `AAGroupInfo::name`.
    // Complexity: O(log N) comparisons.
    // - https://en.cppreference.com/w/cpp/algorithm/ranges/lower_bound
    [[nodiscard]] constexpr std::optional<std::reference_wrapper<const AAGroupInfo>> GetAltGroup(std::string_view name) noexcept {
        const auto iter = std::ranges::lower_bound(ALT_GROUP_TABLE, name, {}, &AAGroupInfo::name);
        if (iter == ALT_GROUP_TABLE.end() || iter->name != name) {
            return std::nullopt;
        }

        return std::cref(*iter);
    }

    static_assert([] {
        const auto a = GetAltGroup("_1hmatk");
        if (!a || a->get().name != "_1hmatk" || a->get().id != 19) {
            return false;
        }

        const auto b = GetAltGroup("_mtidle");
        if (!b || b->get().name != "_mtidle" || b->get().id != 0) {
            return false;
        }

        const auto c = GetAltGroup("_2hwatk");
        if (!c || c->get().name != "_2hwatk" || c->get().id != 27) {
            return false;
        }

        const auto d = GetAltGroup("_shield");
        if (!d || d->get().name != "_shield" || d->get().id != 53) {
            return false;
        }

        return !GetAltGroup("_does_not_exist");
    }());
}
