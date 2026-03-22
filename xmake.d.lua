---@meta

---@alias Mode
---| '"mode.asan"'
---| '"mode.check"'
---| '"mode.coverage"'
---| '"mode.debug"'
---| '"mode.lsan"'
---| '"mode.minsizerel"'
---| '"mode.msan"'
---| '"mode.profile"'
---| '"mode.release"'
---| '"mode.releasedbg"'
---| '"mode.tsan"'
---| '"mode.ubsan"'
---| '"mode.valgrind"'

---Add build rules like debug/release
---
---@param ...
---| "android.native_app"
---| "asm"
---| "asm.build"
---| "asn1c"
---| "c"
---| "c++"
---| "c++.build"
---| "c++.build.modules"
---| "c++.build.modules.builder"
---| "c++.build.modules.install"
---| "c++.build.modules.scanner"
---| "c++.build.pcheader"
---| "c++.openmp"
---| "c++.unity_build"
---| "c.build"
---| "c.build.pcheader"
---| "c.openmp"
---| "c.unity_build"
---| "c51.binary"
---| "capnproto.cpp"
---| "cppfront"
---| "cppfront.build.cpp2"
---| "cppfront.build.h2"
---| "cuda"
---| "cuda.build"
---| "cuda.build.devlink"
---| "cuda.env"
---| "cuda.gencodes"
---| "dlang"
---| "dlang.build"
---| "dlang.build.optimization"
---| "fortran"
---| "fortran.build"
---| "fortran.build.modules"
---| "gnu-rm.binary"
---| "gnu-rm.object"
---| "gnu-rm.shared"
---| "gnu-rm.static"
---| "go"
---| "go.build"
---| "iverilog.binary"
---| "kotlin-native"
---| "kotlin-native.build"
---| "lex"
---| "linker"
---| "linker.link_scripts"
---| "linker.soname"
---| "linker.version_scripts"
---| "lua.module"
---| "lua.native-objects"
---| "luarocks.module"
---| "mdk.binary"
---| "mdk.console"
---| "mdk.static"
---| "mode.asan"
---| "mode.check"
---| "mode.coverage"
---| "mode.debug"
---| "mode.lsan"
---| "mode.minsizerel"
---| "mode.msan"
---| "mode.profile"
---| "mode.release"
---| "mode.releasedbg"
---| "mode.tsan"
---| "mode.ubsan"
---| "mode.valgrind"
---| "module.binary"
---| "module.shared"
---| "nim"
---| "nim.build"
---| "nodejs.module"
---| "objc++"
---| "objc++.build"
---| "objc++.build.pcheader"
---| "objc.build"
---| "objc.build.pcheader"
---| "pascal"
---| "pascal.build"
---| "platform.linux.bpf"
---| "platform.linux.driver"
---| "platform.linux.module"
---| "platform.wasm"
---| "platform.wasm.installfiles"
---| "platform.wasm.preloadfiles"
---| "platform.windows"
---| "platform.windows.def"
---| "platform.windows.idl"
---| "platform.windows.manifest"
---| "platform.windows.subsystem"
---| "plugin.compile_commands.autoupdate"
---| "plugin.vsxmake.autoupdate"
---| "protobuf.c"
---| "protobuf.cpp"
---| "python.cython"
---| "python.library"
---| "python.module"
---| "qt._wasm_app"
---| "qt.application"
---| "qt.console"
---| "qt.env"
---| "qt.moc"
---| "qt.qmlplugin"
---| "qt.qmltyperegistrar"
---| "qt.qrc"
---| "qt.quickapp"
---| "qt.quickapp_static"
---| "qt.shared"
---| "qt.static"
---| "qt.ts"
---| "qt.ui"
---| "qt.widgetapp"
---| "qt.widgetapp_static"
---| "rust"
---| "rust.build"
---| "rust.cxxbridge"
---| "swift"
---| "swift.build"
---| "swift.interop"
---| "swig.base"
---| "swig.c"
---| "swig.cpp"
---| "utils.bin2c"
---| "utils.bin2obj"
---| "utils.compiler.runtime"
---| "utils.glsl2spv"
---| "utils.hlsl2spv"
---| "utils.inherit.links"
---| "utils.install.cmake_importfiles"
---| "utils.install.pkgconfig_importfiles"
---| "utils.ispc"
---| "utils.merge.archive"
---| "utils.merge.object"
---| "utils.symbols.export_all"
---| "utils.symbols.export_list"
---| "utils.symbols.extract"
---| "vala"
---| "vala.build"
---| "verilator.binary"
---| "verilator.shared"
---| "verilator.static"
---| "wdk.binary"
---| "wdk.driver"
---| "wdk.env"
---| "wdk.env.kmdf"
---| "wdk.env.umdf"
---| "wdk.env.wdm"
---| "wdk.inf"
---| "wdk.man"
---| "wdk.mc"
---| "wdk.mof"
---| "wdk.package.cab"
---| "wdk.shared"
---| "wdk.sign"
---| "wdk.static"
---| "wdk.tracewpp"
---| "win.sdk.application"
---| "win.sdk.dotnet"
---| "win.sdk.mfc.env"
---| "win.sdk.mfc.shared"
---| "win.sdk.mfc.shared_app"
---| "win.sdk.mfc.static"
---| "win.sdk.mfc.static_app"
---| "win.sdk.resource"
---| "xcode.application"
---| "xcode.bundle"
---| "xcode.framework"
---| "xcode.info_plist"
---| "xcode.metal"
---| "xcode.storyboard"
---| "xcode.xcassets"
---| "xmake.cli"
---| "yacc"
---| "zig"
---| "zig.build"
function add_rules(...) end

---Set the target architecture
---@param arch string
function set_arch(arch) end

---Set the target encoding
---@param encoding string
function set_encodings(encoding) end

---Set the warnings
---@param options string[]
function set_warnings(options) end

---Set runtime library types, e.g., MT/MD
---@param runtime "MD"|"MT"
function set_runtimes(runtime) end

---Set the project name
---@param name string
function set_project(name) end

---Include another target or library
---@param name string
function includes(name) end

---Add a callback after build
---@param callback fun(target: nil)
function on_build(callback) end

---Define a build target
---@param name string
---@param callback fun(target: nil)
function target(name, callback) end

---Check the current build mode
---@param mode Mode
---@return boolean
function is_mode(mode) end

---Check the current platform
---@param plat string
---@return boolean
function is_plat(plat) end

---Check the target kind (binary, static, shared)
---@param kind string
---@return boolean
function is_kind(kind) end

---Add dependencies
---@param ... string
function add_deps(...) end

---Add dependencies files
---@param ... string
function add_depfiles(...) end

---Set header files
---@param header string
function add_headerfiles(header) end

---Set precompiled header
---@param header string
function set_pcxxheader(header) end

---Set precompiled header
---@param kind "static"|"dynamic"
function set_kind(kind) end

---Set default build mode
---@param mode Mode
function set_defaultmode(mode) end

---Set version e.g., `"0.1.0"`
---@param version string
function set_version(version) end

---Set license. e.g., `"MIT OR APache2.0"`
---@param license string
function set_license(license) end

---Set precompiled header
---@param lang "cxx14"
---@param lang2 string|nil
function set_languages(lang, lang2) end

---Add include directories
---@param ... string|string[]|{public: boolean}  # xmake supports tables with `public` field
function add_includedirs(...) end

---Add source files
---@param ... string
function add_files(...) end

---Add preprocessor defines
---@param ... string
function add_defines(...) end

---Add compiler flags
---@param ... string|table
function add_cxxflags(...) end

---Add C/C++ flags (MSVC)
---@param ... string|table
function add_cxflags(...) end

---Add library link directories
---@param ... string
function add_linkdirs(...) end

---Add libraries to link
---@overload fun(...:string)
---@overload fun(string, options:{public: boolean})
---@overload fun(...:string[], options:{public: boolean})
function add_links(...) end

---Add a callback before build
---@param callback fun(target: nil)
function before_build(callback) end

---Is file the path?
---@param path string
---@return boolean
function os.isfile(path) end

---Execute command with format string.
---@param ... string
---@return boolean
function os.execf(...) end
