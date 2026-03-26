-- Please change this for your settings.
local PLUGIN_NAME<const> = "fnis_aa" -- dll name
local AUTHOR_NAME<const> = "SARDONYX" -- NOTE: Including a space seems to break the rc.
local DESCRIPTION<const> = "Replaces `FNIS_aa2.pex` Papyrus natives at runtime via JSON, eliminating the need for PapyrusCompiler."
local VERSION<const> = "1.0.1"
local LICENSE<const> = "MIT OR Apache-2.0"
--

set_version(VERSION)
set_license(LICENSE)

set_config("rex_json", true)
set_config("rex_toml", true)
includes("../extern/CommonLibVR_NG")

includes("../rust/bridge")

target(PLUGIN_NAME, function ()
    add_deps("commonlibsse-ng")
    add_deps("rust_bridge")

    add_defines("SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE")

    add_includedirs("include")
    add_headerfiles("include/**.hh")
    set_pcxxheader("include/pch.hh")

    add_files("src/**.cc")

    add_cxxflags("cl::/Zc:char8_t")
    add_cxxflags("clang-cl::/Zc:char8_t")

    -- Rust bridge
    add_includedirs("../target/cxxbridge", { public = true })
    add_headerfiles("../target/cxxbridge/**.h")
    add_files("../target/cxxbridge/**.cc")
    add_linkdirs("../target/release")
    add_links("bridge", { public = true })
    -- Rust std deps
    if is_plat("windows") then
        add_links("ntdll", "userenv", "ws2_32", { public = true })
    end
    --

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
            print("Installed " .. path.filename(pex) .. " → " .. dst_dir)
        end
    end)
end)

target("regenerate_pex", function()
    set_kind("phony")
    on_build(function (target)
        local skyrim_dir  = os.getenv("SKYRIM_DIR") or "D:/STEAM/steamapps/common/Skyrim Special Edition"
        local compiler    = path.join(skyrim_dir, "Papyrus Compiler/PapyrusCompiler.exe")
        local skse_source = path.join(skyrim_dir, "Data/Source/Scripts")
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
            print("Compiled " .. psc .. " → " .. dst_pex)
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
