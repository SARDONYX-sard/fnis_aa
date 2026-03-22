//! Logging macros backed by spdlog via cxx bridge.
//!
//! These macros mirror spdlog's log levels and accept
//! standard Rust `format!` syntax.
//!
//! # Examples
//! ```rust
//! log_info!("Player level = {}", level);
//! log_warn!("Unexpected state: {:?}", state);
//! log_error!("Failed to load form {:X}", form_id);
//! ```

/// Logs a message at **trace** level.
///
/// Equivalent to `spdlog::trace`.
///
/// # Example
/// ```rust
/// log_trace!("Entering function {}", func_name);
/// ```
///
/// # Heap allocation cost(2 times)
/// - Rust: Once macro -> [`format!`] -> [`str`]
/// - C++: (Probably) Once `string_view` -> `spdlog`
#[macro_export]
macro_rules! log_trace {
    ($($arg:tt)*) => {{
        $crate::bridge::ffi::spdlog_trace(&format!($($arg)*));
    }};
}

/// Logs a message at **debug** level.
///
/// Equivalent to `spdlog::debug`.
///
/// # Example
/// ```rust
/// log_debug!("Loaded {} records", count);
/// ```
#[macro_export]
macro_rules! log_debug {
    ($($arg:tt)*) => {{
        $crate::bridge::ffi::spdlog_debug(&format!($($arg)*));
    }};
}

/// Logs a message at **info** level.
///
/// Equivalent to `spdlog::info`.
///
/// # Example
/// ```rust
/// log_info!("Plugin initialized successfully");
/// ```
#[macro_export]
macro_rules! log_info {
    ($($arg:tt)*) => {{
        $crate::bridge::ffi::spdlog_info(&format!($($arg)*));
    }};
}

/// Logs a message at **warn** level.
///
/// Equivalent to `spdlog::warn`.
///
/// # Example
/// ```rust
/// log_warn!("Missing optional file: {}", path);
/// ```
#[macro_export]
macro_rules! log_warn {
    ($($arg:tt)*) => {{
        $crate::bridge::ffi::spdlog_warn(&format!($($arg)*));
    }};
}

/// Logs a message at **error** level.
///
/// Equivalent to `spdlog::error`.
///
/// # Example
/// ```rust
/// log_error!("Failed to open file: {}", err);
/// ```
#[macro_export]
macro_rules! log_error {
    ($($arg:tt)*) => {{
        $crate::bridge::ffi::spdlog_error(&format!($($arg)*));
    }};
}
