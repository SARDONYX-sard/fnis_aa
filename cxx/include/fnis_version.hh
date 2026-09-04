#include <expected>
#include <optional>
#include <string_view>

namespace fnis_aa {

    class FNISVersionError {
    public:
        enum class Kind {
            VersionStringTooShort,
            ExpectedVersionPrefix,
            ExpectedVersionSeparator,
            ExpectedMajorNumber,
            ExpectedMinor1Number,
            ExpectedMinor2Number,
            ExpectedFlagsNumber,
        };

        constexpr explicit FNISVersionError(Kind kind) noexcept : kind_(kind) {}

        [[nodiscard]]
        constexpr Kind kind() const noexcept { return kind_; }

        /// Returns the static error message associated with this error.
        [[nodiscard]] constexpr std::string_view to_static_str() const noexcept;

        constexpr bool operator==(const FNISVersionError&) const noexcept = default;

    private:
        Kind kind_;
    };

    constexpr std::string_view FNISVersionError::to_static_str() const noexcept {
        switch (this->kind_) {
        case Kind::VersionStringTooShort:
            return "version string is too short";

        case Kind::ExpectedVersionPrefix:
            return "expected 'V' version prefix";

        case Kind::ExpectedVersionSeparator:
            return "expected '.' version separator";

        case Kind::ExpectedMajorNumber:
            return "expected major version number";

        case Kind::ExpectedMinor1Number:
            return "expected minor1 version number";

        case Kind::ExpectedMinor2Number:
            return "expected minor2 version number";

        case Kind::ExpectedFlagsNumber:
            return "expected flags value (0, 1, or 2)";
        }

        return "unknown FNIS version error";
    }

    enum VersionCompareResult : int32_t {
        NotInstalledOrOlder = -1,
        Match = 0,
        Newer = 1,
    };

    /// Represents an FNIS version.
    ///
    /// The FNIS version format is `V<DD>.<DD>.<DD>.<D>`.
    ///
    /// - `major` is the major version.
    /// - `minor1` is the first minor version.
    /// - `minor2` is the second minor version.
    /// - `flags` is the release state: `0` for release, `1` for alpha,
    ///   and `2` for beta.
    ///
    /// A `flags` value of `3` represents an invalid or unavailable version.
    struct FNISVersion {
        int32_t major = 0;
        int32_t minor1 = 0;
        int32_t minor2 = 0;
        int32_t flags = 3;

        constexpr bool operator==(const FNISVersion&) const noexcept = default;

        [[nodiscard]]
        inline constexpr bool is_invalid() const noexcept { return flags == 3; }

        [[nodiscard]]
        inline static constexpr FNISVersion latest() noexcept {
            return FNISVersion{ .major = 7, .minor1 = 6, .minor2 = 0, .flags = 0 };
        }

        /// Parses an FNIS version string.
        ///
        /// # Errors
        ///
        /// Returns `FNISVersionError` if the version string is too short,
        /// has an invalid prefix or separator, contains a non-numeric
        /// component, or contains an invalid flags value.
        [[nodiscard]]
        static constexpr std::expected<FNISVersion, FNISVersionError> from_str(std::string_view ver) noexcept;

        /// Compares this FNIS version against a required version.
        ///
        /// The flags component is not considered.
        ///
        /// An invalid version (`flags == 3`) is treated as not installed.
        ///
        /// # Returns
        ///
        /// Returns `VersionCompareResult::Newer` if this version is newer
        /// than the required version, `VersionCompareResult::Match` if the
        /// versions match, or `VersionCompareResult::NotInstalledOrOlder`
        /// if this version is older or not installed.
        [[nodiscard]]
        inline constexpr VersionCompareResult compare(int32_t required_major, int32_t required_minor1, int32_t required_minor2) const noexcept;
    };

    constexpr std::expected<FNISVersion, FNISVersionError> FNISVersion::from_str(std::string_view ver) noexcept {
        if (ver.size() < 11) {
            return std::unexpected(FNISVersionError{ FNISVersionError::Kind::VersionStringTooShort });
        }

        if (ver[0] != 'V') {
            return std::unexpected(FNISVersionError{ FNISVersionError::Kind::ExpectedVersionPrefix });
        }

        if (ver[3] != '.' || ver[6] != '.' || ver[9] != '.') {
            return std::unexpected(FNISVersionError{ FNISVersionError::Kind::ExpectedVersionSeparator });
        }

        const auto parse_two_digits = [](char tens, char ones) -> std::optional<int32_t> {
            if (tens < '0' || tens > '9' || ones < '0' || ones > '9') {
                return std::nullopt;
            }
            return (tens - '0') * 10 + (ones - '0');
        };

        const auto major = parse_two_digits(ver[1], ver[2]);
        if (!major) {
            return std::unexpected(FNISVersionError{ FNISVersionError::Kind::ExpectedMajorNumber });
        }

        const auto minor1 = parse_two_digits(ver[4], ver[5]);
        if (!minor1) {
            return std::unexpected(FNISVersionError{ FNISVersionError::Kind::ExpectedMinor1Number });
        }

        const auto minor2 = parse_two_digits(ver[7], ver[8]);
        if (!minor2) {
            return std::unexpected(FNISVersionError{ FNISVersionError::Kind::ExpectedMinor2Number });
        }

        if (ver[10] < '0' || ver[10] > '2') {
            return std::unexpected(
                FNISVersionError{
                    FNISVersionError::Kind::ExpectedFlagsNumber });
        }

        return FNISVersion{
            .major = *major,
            .minor1 = *minor1,
            .minor2 = *minor2,
            .flags = ver[10] - '0',
        };
    }

    inline constexpr VersionCompareResult FNISVersion::compare(int32_t required_major, int32_t required_minor1, int32_t required_minor2) const noexcept {
        if (is_invalid()) {
            return VersionCompareResult::NotInstalledOrOlder;
        }
        if (this->major != required_major) {
            return this->major > required_major ? VersionCompareResult::Newer : VersionCompareResult::NotInstalledOrOlder;
        }
        if (this->minor1 != required_minor1) {
            return this->minor1 > required_minor1 ? VersionCompareResult::Newer : VersionCompareResult::NotInstalledOrOlder;
        }
        if (this->minor2 != required_minor2) {
            return this->minor2 > required_minor2 ? VersionCompareResult::Newer : VersionCompareResult::NotInstalledOrOlder;
        }
        return VersionCompareResult::Match;
    }

    template <typename T>
    consteval bool test_fnis_version(std::string_view input, const T& expected) {
        const auto result = FNISVersion::from_str(input);
        return result.has_value() && *result == expected;
    }

    consteval bool test_fnis_version_error(std::string_view input, FNISVersionError expected) {
        const auto result = FNISVersion::from_str(input);
        return !result.has_value() && result.error() == expected;
    }

    static_assert(test_fnis_version("V07.06.00.0", FNISVersion{ .major = 7, .minor1 = 6, .minor2 = 0, .flags = 0 }));
    static_assert(test_fnis_version("V07.06.00.1", FNISVersion{ .major = 7, .minor1 = 6, .minor2 = 0, .flags = 1 }));
    static_assert(test_fnis_version("V07.06.00.2", FNISVersion{ .major = 7, .minor1 = 6, .minor2 = 0, .flags = 2 }));

    static_assert(test_fnis_version_error("V07.06.00", FNISVersionError{ FNISVersionError::Kind::VersionStringTooShort }));
    static_assert(test_fnis_version_error("v07.06.00.0", FNISVersionError{ FNISVersionError::Kind::ExpectedVersionPrefix }));
    static_assert(test_fnis_version_error("V07-06.00.0", FNISVersionError{ FNISVersionError::Kind::ExpectedVersionSeparator }));
    static_assert(test_fnis_version_error("Vxx.06.00.0", FNISVersionError{ FNISVersionError::Kind::ExpectedMajorNumber }));
    static_assert(test_fnis_version_error("V07.xx.00.0", FNISVersionError{ FNISVersionError::Kind::ExpectedMinor1Number }));
    static_assert(test_fnis_version_error("V07.06.xx.0", FNISVersionError{ FNISVersionError::Kind::ExpectedMinor2Number }));
    static_assert(test_fnis_version_error("V07.06.00.3", FNISVersionError{ FNISVersionError::Kind::ExpectedFlagsNumber }));

    static_assert(FNISVersion::latest() == FNISVersion{ .major = 7, .minor1 = 6, .minor2 = 0, .flags = 0 });
    static_assert(FNISVersion{ .major = 7, .minor1 = 6, .minor2 = 0, .flags = 3 }.is_invalid());
    static_assert(!FNISVersion{ .major = 7, .minor1 = 6, .minor2 = 0, .flags = 0 }.is_invalid());

    static_assert(FNISVersion{ .major = 7, .minor1 = 6, .minor2 = 0, .flags = 0 }.compare(7, 2, 0) == VersionCompareResult::Newer);
    static_assert(FNISVersion{ .major = 7, .minor1 = 6, .minor2 = 0, .flags = 0 }.compare(7, 6, 0) == VersionCompareResult::Match);
    static_assert(FNISVersion{ .major = 7, .minor1 = 6, .minor2 = 0, .flags = 0 }.compare(8, 0, 0) == VersionCompareResult::NotInstalledOrOlder);
    static_assert(FNISVersion{ .major = 7, .minor1 = 6, .minor2 = 0, .flags = 3 }.compare(7, 6, 0) == VersionCompareResult::NotInstalledOrOlder);

}
