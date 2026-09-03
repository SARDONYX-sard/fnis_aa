fn main() {
    let crate_root = std::path::PathBuf::from(env!("CARGO_MANIFEST_DIR"));
    cxx_build::bridge("src/bridge.rs")
        .std("c++20")
        .include(crate_root.join("../../cxx/include"))
        .compile("bridge");
}
