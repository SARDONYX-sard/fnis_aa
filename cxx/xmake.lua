-- Please change this for your settings.
local PLUGIN_NAME<const> = "RustBridgeTemplate" -- dll name
local AUTHOR_NAME<const> = "SARDONYX" -- NOTE: Including a space seems to break the rc.
local DESCRIPTION<const> = "Plugin Description"
local VERSION<const> = "0.1.0"
local LICENSE<const> = "Unlicense OR Apache-2.0"
--

set_config("rex_toml", true)
includes("../extern/CommonLibVR_NG")

includes("../extern/SKSEMenuFramework")
includes("../rust/bridge")

target(PLUGIN_NAME, function ()
    add_deps("commonlibsse-ng")
    add_deps("SKSEMenuFramework")
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
end)
