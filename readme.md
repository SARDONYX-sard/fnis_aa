# xmake + SKSE Plugin Template (C++ <- bridge -> Rust)

## Requirements

- Rust: >= v1.91
- MSVC
- xmake: >= 3.0.6

## Build

```shell
git submodule update --init --recursive --depth=1 && xmake
```

- And then install(`./build/artifact/SKSE/Data/<PLUGIN_NAME>.dll`)

NOTE: The `MyPlugin` is `PLUGIN_NAME` value in cxx/xmake.lua.

```shell
xmake install -o ./build/artifact MyPlugin
```

## Language server(For `clangd`)

```shell
xmake project -k compile_commands --lsp=clangd --outputdir=build
```

## Change build mode

```shell
xmake f -m release
```

## Format

- NOTE: Need LLVM(clang-format.exe path)

```shell
xmake format
```

## License

There is no license specified for this template.
It is okay to delete this item, but we recommend MIT OR APACHE2.0. This is because the license terms are clear when there is a documented license.
