set_project("skse_plugin_rust_bridge")
set_version("0.1.0")
set_license("Unlicense OR Apache-2.0")

-- Global settings
set_arch("x64")
set_languages("c++23")
set_encodings("utf-8")
set_warnings("allextra")

add_rules("mode.debug", "mode.releasedbg")

set_defaultmode("releasedbg")

includes("cxx")
