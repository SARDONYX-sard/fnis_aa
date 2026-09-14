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
    local output_dir = [[D:\GAME\ModOrganizer Skyrim SE\mods\Dyn FNIS AA Functions]]

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
