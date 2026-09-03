// C++ to Rust bridge
// NOTE: bridge Cannot inline function!
#pragma once

#include <rust/cxx.h>

namespace fnis_aa::bridge {
    void spdlog_trace(rust::Str msg);
    void spdlog_debug(rust::Str msg);
    void spdlog_info(rust::Str msg);
    void spdlog_warn(rust::Str msg);
    void spdlog_error(rust::Str msg);

    void message_box(rust::Str msg);
}
