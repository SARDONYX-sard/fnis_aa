task("build-all", function()
    on_run(function()
        os.exec("git submodule update --init --recursive --depth=1")
        os.exec("cargo build --release")
        os.exec("xmake build -y fnis_aa")
    end)

    set_menu({
        usage = "xmake build",
        description = "Build Rust project and fnis_aa",
    })
end)

task("deploy", function()
    local DEFAULT_OUTPUT_DIR <const> = [[D:\GAME\ModOrganizer Skyrim SE\mods\Dyn FNIS AA Functions]]
    local output_dir = os.getenv("OUTPUT_DIR") or DEFAULT_OUTPUT_DIR

    on_run(function()
        os.exec("cargo build --release")
        os.exec("xmake build -y fnis_aa")
        os.exec([[xmake install -o "]] .. output_dir .. [[" fnis_aa]])
    end)

    set_menu({
        usage = "xmake deploy",
        description = "Build and install fnis_aa",
    })
end)

task("update-submodules", function()
    on_run(function()
        os.exec("git submodule update --init --remote")
    end)

    set_menu({
        usage = "xmake update-submodules",
        description = "Update git submodules to their remote revisions",
    })
end)

task("lsp", function()
    on_run(function()
        os.exec("xmake project -k compile_commands --lsp=clangd --outputdir=.vscode -y")
    end)

    set_menu({
        usage = "xmake lsp",
        description = "generate compile_commands.json",
    })
end)
