-- Please change this for your settings.
local PLUGIN_NAME<const> = "fnis_aa" -- dll name
local AUTHOR_NAME<const> = "SARDONYX" -- NOTE: Including a space seems to break the rc.
local DESCRIPTION<const> = "Replaces `FNIS_aa2.pex` Papyrus natives at runtime via JSON, eliminating the need for PapyrusCompiler."
local VERSION<const> = "1.0.0"
local LICENSE<const> = "MIT OR Apache-2.0"
--

set_config("rex_json", true)
set_config("rex_toml", true)
includes("../extern/CommonLibVR_NG")

includes("../rust/bridge")

target(PLUGIN_NAME, function ()
    add_deps("commonlibsse-ng")
    add_deps("rust_bridge")

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
    set_version(VERSION)
    set_license(LICENSE)
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
        local skse_source = path.join(skyrim_dir, "Data/Scripts/Source")
        local src_dir     = path.join(os.scriptdir(), "papyrus")
        local dst_dir     = path.join(os.scriptdir(), "papyrus/prebuilt")

        local psc_files = {
            "FNIS_aa2.psc",
            "FNISVersionGenerated.psc",
        }

        if not os.isfile(compiler) then
            raise("[error] Papyrus compiler not found at: " .. compiler)
        end

        os.mkdir(dst_dir)

        for _, psc in ipairs(psc_files) do
            local src_psc = path.join(src_dir, psc)
            local dst_pex = path.join(dst_dir, psc:gsub("%.psc$", ".pex"))

            local ok, err = os.execv(compiler, {
                src_psc,
                "-output=" .. dst_dir,
                "-import=" .. src_dir .. ";" .. skse_source,
                "-optimize",
                "-quiet",
            })

            if not ok then
                print("[error] Papyrus compilation failed for " .. psc .. ": " .. tostring(err))
            end

            print("Compiled " .. psc .. " → " .. dst_pex)
        end
    end)
end)
