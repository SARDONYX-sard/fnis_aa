-- Global settings
set_arch("x64")
set_languages("c++23")
set_encodings("utf-8")
set_warnings("allextra")
set_policy("package.requires_lock", true)

add_rules("mode.debug", "mode.releasedbg")
set_defaultmode("releasedbg") -- To find crash location

includes("./tasks.lua")
includes("cxx")
