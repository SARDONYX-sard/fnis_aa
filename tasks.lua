-- /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
-- General tasks

task("lsp", function ()
    on_run(function ()
        os.exec("xmake project -k compile_commands --lsp=clangd --outputdir=.vscode -y")
    end)

    set_menu({
        usage = "xmake lsp",
        description = "generate compile_commands.json",
    })
end)

task("update-lock", function ()
    on_run(function ()
        os.exec("xmake f -c -y --skse_plugin=y --tests=y") -- lock records only the requires of the current config
        os.exec("xmake repo -u")
        os.exec("xmake require --upgrade -y")
    end)

    set_menu({
        usage = "xmake update-lock",
        description = "Re-resolve packages and rewrite xmake-requires.lock",
    })
end)

-- /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
-- Target specific tasks

-- Find the SKSE plugin target name (= the target using the `@commonlibsse-ng/plugin` rule).
--
-- # NOTE
-- The top level of this file is the description scope: `import`/`raise` are unavailable and `os` is restricted.
---@param os os
---@param config core.project.config
---@param project core.project.project
---@return string|nil
local function find_plugin_name(os, config, project)
    if not os.isfile(config.filepath()) then
        os.vrun("xmake f -y") -- quiet: keep stdout clean for `xmake plugin-name`
    end
    config.load()

    -- NOTE: Package rules are not loaded here (packages are not loaded), so check the raw rule names.
    for _, target in pairs(project.targets()) do
        if table.contains(table.wrap(target:get("rules")), "@commonlibsse-ng/plugin") then
            return target:name()
        end
    end
    return nil
end

local PLUGIN_NOT_FOUND <const> = "SKSE plugin target not found. Configure with `xmake f -y --skse_plugin=y`."

task("deploy", function ()
    local DEFAULT_OUTPUT_DIR <const> = [[D:\GAME\ModOrganizer Skyrim SE\mods\Dyn FNIS AA Functions]]
    local output_dir = os.getenv("OUTPUT_DIR") or DEFAULT_OUTPUT_DIR

    on_run(function ()
        os.exec("xmake build -y fnis_aa")
        os.exec([[xmake install -o "]] .. output_dir .. [[" fnis_aa]])
    end)

    set_menu({
        usage = "xmake deploy",
        description = "Build and install fnis_aa",
    })
end)

task("gen-pex", function ()
    on_run(function ()
        local skyrim_dir = os.getenv("SKYRIM_DIR") or "D:/STEAM/steamapps/common/Skyrim Special Edition"
        local compiler = path.join(skyrim_dir, "Papyrus Compiler/PapyrusCompiler.exe")
        local skse_source = path.join(skyrim_dir, "Data/Scripts/Source")
        local flags_file = path.join(skse_source, "TESV_Papyrus_Flags.flg")
        local src_dir = path.join(os.projectdir(), "cxx/papyrus")
        local dst_dir = path.join(src_dir, "prebuilt")

        if not os.isdir(skse_source) then
            raise("Non exits scripts dir: %s", skse_source)
        end
        if not os.isfile(compiler) then
            raise("Papyrus compiler not found at: %s", compiler)
        end

        os.mkdir(dst_dir)

        for _, psc in ipairs(os.files(path.join(src_dir, "*.psc"))) do
            -- NOTE: `os.execv` raises on a non-zero exit code.
            os.execv(compiler, {
                psc,
                "-f=" .. flags_file,
                "-output=" .. dst_dir,
                "-import=" .. src_dir .. ";" .. skse_source,
                "-optimize",
                "-quiet",
            })

            local dst_pex = path.join(dst_dir, (path.filename(psc):gsub("%.psc$", ".pex")))
            print("Compiled " .. psc .. " -> " .. dst_pex)
        end
    end)

    set_menu({
        usage = "xmake gen-pex",
        description = "Compile cxx/papyrus/*.psc into cxx/papyrus/prebuilt/*.pex (env: SKYRIM_DIR)",
    })
end)

task("plugin-name", function ()
    on_run(function ()
        import("core.project.config")
        import("core.project.project")
        local name = find_plugin_name(os, config, project)
        if not name then
            raise(PLUGIN_NOT_FOUND)
        end
        print(name)
    end)

    set_menu({
        usage = "xmake plugin-name",
        description = "Print the SKSE plugin target name",
    })
end)

task("release-zip", function ()
    on_run(function ()
        import("core.project.config")
        import("core.project.project")
        import("utils.archive")

        local name = find_plugin_name(os, config, project)
        if not name then
            raise(PLUGIN_NOT_FOUND)
        end
        local installdir = path.join(os.projectdir(), "build/artifact")
        local zipfile = path.join(os.projectdir(), "build", name .. "-windows.zip")
        os.tryrm(installdir)
        os.tryrm(zipfile)

        os.exec("xmake build -y %s", name)
        os.exec("xmake install -y -o \"%s\" %s", installdir, name)

        local oldir = os.cd(installdir)
        archive.archive(zipfile, os.files("**"), {curdir = installdir})
        os.cd(oldir)
        cprint("${color.success}created %s", zipfile)
    end)

    set_menu({
        usage = "xmake release-zip",
        description = "Build, install to build/artifact and zip it as build/<plugin>-windows.zip",
    })
end)

-- Override the builtin `xmake test`.
task("test", function ()
    on_run(function ()
        os.exec("xmake f -y --skse_plugin=n --tests=y")

        -- Run the builtin test action.
        import("main", {rootdir = path.join(os.programdir(), "actions/test"), anonymous = true})()
    end)

    set_menu({usage = "xmake test [options]", description = "Run unit tests"})
end)
