#pragma once

namespace config {
    /// Checks if two strings are equal, ignoring ASCII case.
    /// Optimized to avoid heap allocations and locale dependency.
    [[nodiscard]] constexpr bool iequals_ascii(std::string_view a, std::string_view b) noexcept {
        if (a.size() != b.size())
            return false;

        for (std::size_t i = 0; i < a.size(); ++i) {
            auto ca = static_cast<unsigned char>(a[i]);
            auto cb = static_cast<unsigned char>(b[i]);

            if (ca != cb) {
                // If characters don't match, try to normalize both to lowercase.
                // Only 'A'-'Z' (0x41-0x5A) are affected by '| 0x20'.
                auto la = (ca >= 'A' && ca <= 'Z') ? (ca | 0x20) : ca;
                auto lb = (cb >= 'A' && cb <= 'Z') ? (cb | 0x20) : cb;

                if (la != lb)
                    return false;
            }
        }
        return true;
    }

    // --- Compile-time Tests ---
    static_assert(iequals_ascii("trace", "TRACE"), "Failed: Basic case insensitivity");
    static_assert(iequals_ascii("debug", "debug"), "Failed: Exact match");
    static_assert(!iequals_ascii("info", "warn"), "Failed: Different strings");
    static_assert(!iequals_ascii("err", "error"), "Failed: Length mismatch");
    static_assert(iequals_ascii("Critical", "CRITICAL"), "Failed: Mixed case");

    // Boundary check for non-alpha characters (Ensures bit 0x20 doesn't cause false positives)
    static_assert(!iequals_ascii("@", "`"), "Failed: Symbol boundary @ vs `");
    static_assert(!iequals_ascii("[", "{"), "Failed: Symbol boundary [ vs {");
    static_assert(iequals_ascii("log_1", "LOG_1"), "Failed: Numbers and underscores");
}
