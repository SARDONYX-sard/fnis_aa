// C++ to Rust bridge
#pragma once

#include <rust/cxx.h>  // From Rust bridge

namespace bridge {
    void spdlog_trace(rust::Str msg);
    void spdlog_debug(rust::Str msg);
    void spdlog_info(rust::Str msg);
    void spdlog_warn(rust::Str msg);
    void spdlog_error(rust::Str msg);

    // Bridge test fn
    rust::String get_calendar_time();
    void         message_box(rust::Str msg);
}  // namespace bridge
