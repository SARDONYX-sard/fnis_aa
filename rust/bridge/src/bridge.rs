#[cxx::bridge(namespace = "bridge")]
pub(crate) mod ffi {
    extern "Rust" {
        fn bridge_init();
    }

    #[allow(unused)]
    unsafe extern "C++" {
        include!("bridge.hh");

        pub fn spdlog_trace(msg: &str);
        pub fn spdlog_debug(msg: &str);
        pub fn spdlog_info(msg: &str);
        pub fn spdlog_warn(msg: &str);
        pub fn spdlog_error(msg: &str);

        pub fn get_calendar_time() -> String;
        pub fn message_box(msg: &str);
    }
}

pub fn bridge_init() {
    crate::log_trace!("[rust] bridge_init()");

    crate::log_info!("[From Rust] Current time: {}", ffi::get_calendar_time());
    // ffi::message_box("Hello from Rust!");
}

#[cfg(test)]
mod tests {
    // For CI
    #[test]
    const fn test_dummy() {
        assert!(2 + 2 == 4);
    }
}
