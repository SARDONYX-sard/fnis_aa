local PLUGIN_NAME<const> = "fnis_aa" -- dll name
local AUTHOR_NAME<const> = "SARDONYX" -- NOTE: Including a space seems to break the rc.
local DESCRIPTION<const> = "Dyn defines FNIS AA functions. SkyrimSE, AE(<=1.7.104.0), VR"
local VERSION<const> = "3.1.1"
local LICENSE<const> = "GPL-3.0-or-later" -- changed CommonLibSSE-NG GPL-3.0-or-later: https://github.com/alandtse/CommonLibSSE-NG/tree/7a60f4de794095d7b0f8928d1b930a52e9a7da83#license
--

set_version(VERSION)
set_license(LICENSE)

-- Local checkout can be used with: `XMAKE_SARD_REPO=D:/Programming/cpp/xmake_repo_sard xmake f -c`
add_repositories("sard_repo " .. (os.getenv("XMAKE_SARD_REPO") or "https://github.com/SARDONYX-sard/xmake_repo_sard.git main"))

-- Split by options so that `xmake f` installs only the packages that are actually needed.
--   e.g. `xmake f --skse_plugin=n` -> only test packages are installed
option("skse_plugin", function ()
    set_default(true)
    set_showmenu(true)
    set_description("Build the SKSE plugin (installs commonlibsse-ng etc.)")
end)

-- NOTE: Default on so that `xmake-requires.lock` always contains every package.
option("tests", function ()
    set_default(true)
    set_showmenu(true)
    set_description("Build unit tests (installs catch2 etc.)")
end)

if has_config("skse_plugin") then
    -- tag or commit sha (e.g. "commonlibsse-ng adb3e2c4dff61d151370a6777ed9e77f8e005c62")
    -- spdlog, toml11(rex_toml) and nlohmann_json(rex_json) are provided as deps of commonlibsse-ng.
    add_requires("commonlibsse-ng v8.4.0", {configs = {rex_json = true, rex_toml = true}})
    add_requires("skse-menu-framework 2026.10.31")

    target(PLUGIN_NAME, function ()
        add_packages("commonlibsse-ng", "skse-menu-framework")
        add_defines("SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE")

        add_includedirs("include")
        add_headerfiles("include/**.hh")
        set_pcxxheader("include/pch.hh")

        add_files("src/**.cc")

        -- SKSEMenuFramework.h warn
        add_cxxflags("/wd5054", {tools = "cl"})
        add_cxxflags("cl::/Zc:char8_t")
        add_cxxflags("clang_cl::/Zc:char8_t")

        -- This setting automatically creates `SKSE/Plugins/<target_NAME>.dll` during `xmake install`.
        add_rules("@commonlibsse-ng/plugin", {
            name = PLUGIN_NAME,
            author = AUTHOR_NAME,
            description = DESCRIPTION,
        })

        after_install(function (target)
            local src_dir = path.join(target:scriptdir(), "papyrus/prebuilt")
            local dst_dir = path.join(target:installdir(), "scripts")
            os.mkdir(dst_dir)

            for _, pex in ipairs(os.files(path.join(src_dir, "*.pex"))) do
                os.cp(pex, dst_dir)
                print("Installed " .. path.filename(pex) .. " -> " .. dst_dir)
            end
        end)
    end)
end

if has_config("tests") then
    -- For testing
    add_requires("catch2 3.13.0")
    -- NOTE: Keep the same version/configs as the commonlibsse-ng deps to share the same package instance.
    add_requires("spdlog v1.16.0", {configs = {header_only = false, wchar = true, std_format = true}})
    add_requires("nlohmann_json v3.12.0")

    target("test_fnis_aa", function ()
        set_default(false) -- To exclude `xmake build`
        set_kind("binary")
        add_packages("catch2", "spdlog", "nlohmann_json")

        add_defines("TEST")

        add_includedirs("src", "include")
        set_pcxxheader("tests/pch.hh")

        add_files("src/config.cc")
        add_files("tests/test_config.cc")

        -- xmake test
        add_tests("config", {timeout = 20 * 1000, trim_output = false})
    end)
end
