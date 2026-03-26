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

        /// # Example
        ///
        /// ```rust,no_compile
        /// ffi::message_box("Hello from Rust!");
        /// ```
        pub fn message_box(msg: &str);
    }
}

fn bridge_init() {
    crate::log_trace!("bridge_init() has been called.");
}

#[cfg(test)]
mod tests {
    // For CI
    #[test]
    const fn test_dummy() {
        assert!(2 + 2 == 4);
    }
}
