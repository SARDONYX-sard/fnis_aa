local PLUGIN_NAME<const> = "fnis_aa" -- dll name
local AUTHOR_NAME<const> = "SARDONYX" -- NOTE: Including a space seems to break the rc.
local DESCRIPTION<const> = "Dyn defines FNIS AA functions. SkyrimSE, AE(<=1.7.104.0), VR"
local VERSION<const> = "3.0.2"
local LICENSE<const> = "GPL-3.0-or-later" -- changed CommonLibSSE-NG GPL-3.0-or-later: https://github.com/alandtse/CommonLibSSE-NG/tree/7a60f4de794095d7b0f8928d1b930a52e9a7da83#license
--

set_version(VERSION)
set_license(LICENSE)

includes("../extern/CommonLibSSE_NG")
includes("../extern/SKSEMenuFramework")
includes("../rust/bridge")

add_requires("toml11 v4.4.0")
add_requires("nlohmann_json v3.12.0")

target(PLUGIN_NAME, function ()
    add_packages("toml11", "nlohmann_json")
    add_deps("commonlibsse-ng")
    add_deps("SKSEMenuFramework")
    add_deps("rust_bridge")
    add_defines("SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE")

    add_includedirs("include")
    add_headerfiles("include/**.hh")
    set_pcxxheader("include/pch.hh")

    add_files("src/**.cc")

    -- SKSEMenuFramework.h warn
    add_cxxflags("/wd5054", { tools = "cl" })

    add_cxxflags("cl::/Zc:char8_t")
    add_cxxflags("clang-cl::/Zc:char8_t")

    -- Rust bridge -----------------------------------------------------------------------------------------------------
    add_includedirs("../target/cxxbridge", { public = true })
    add_headerfiles("../target/cxxbridge/**.h")
    add_files("../target/cxxbridge/**.cc")
    add_linkdirs("../target/release")
    add_links("bridge", { public = true })
    -- Rust std deps
    if is_plat("windows") then
        add_links("ntdll", "userenv", "ws2_32", { public = true })
    end
    --------------------------------------------------------------------------------------------------------------------

    -- This setting automatically creates `SKSE/Plugins/<target_NAME>.dll` during `xmake install`.
    add_rules("commonlibsse-ng.plugin", {
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

target("regenerate_pex", function()
    set_kind("phony")
    on_build(function (target)
        local skyrim_dir  = os.getenv("SKYRIM_DIR") or "D:/STEAM/steamapps/common/Skyrim Special Edition"
        local compiler    = path.join(skyrim_dir, "Papyrus Compiler/PapyrusCompiler.exe")
        local skse_source = path.join(skyrim_dir, "Data/Scripts/Source")
        local flags_file  = path.join(skse_source, "TESV_Papyrus_Flags.flg")
        local src_dir     = path.join(os.scriptdir(), "papyrus")
        local dst_dir     = path.join(os.scriptdir(), "papyrus/prebuilt")

        if not os.isdir(skse_source) then
            print("[error] Non exits scripts dir: " .. skse_source)
        end

        if not os.isfile(compiler) then
            print("[error] Papyrus compiler not found at: " .. compiler)
        end

        os.mkdir(dst_dir)

        local psc_files = os.files(path.join(src_dir, "*.psc"))

        for _, psc in ipairs(psc_files) do
            local file_name = path.filename(psc)
            local src_psc = path.join(src_dir, file_name)

            local ok, err = os.execv(compiler, {
                src_psc,
                "-f=" .. flags_file,
                "-output=" .. dst_dir,
                "-import=" .. src_dir .. ";" .. skse_source,
                "-optimize",
                "-quiet",
            })

            if not ok then
                print("[error] Papyrus compilation failed for " .. psc .. ": " .. tostring(err))
            end

            local dst_pex = path.join(dst_dir, file_name:gsub("%.psc$", ".pex"))
            print("Compiled " .. psc .. " -> " .. dst_pex)
        end
    end)
end)

add_requires("catch2 3.13.0") -- For testing

target("test_fnis_aa", function()
    set_default(false)  -- To exclude `xmake build`
    set_kind("binary")
    add_packages("catch2", "nlohmann_json", "spdlog")

    add_defines("TEST")

    add_includedirs("src", "include")
    set_pcxxheader("tests/pch.hh")

    add_files("src/config.cc")
    add_files("tests/test_config.cc")

    -- xmake test
    add_tests("config", {
        run_timeout = 10000,
        trim_output = false
    })
end)
