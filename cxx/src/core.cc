#include <bridge/src/bridge.rs.h>

#include "core.hh"

namespace skse_plugin_core {
    // Hello Rust!
    void init() {
        bridge::bridge_init();
    }
}  // namespace skse_plugin_core
