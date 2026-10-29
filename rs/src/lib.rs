use core::time::Duration;
use std::thread::sleep;

#[macro_export]
macro_rules! modu { ($($name:ident)*) => {
    $( mod $name;
       #[allow(unused_imports)]
       pub use $name::*;
    )*
}; }

mod exec;
modu!(cli paths term);

#[macro_export]
macro_rules! envg {
    ($env:ident $($post:expr),* $(; pre: $first:expr $(, $pre:expr)*)?) => {
       $( $first.to_string() $( + $pre )* + & )? std::env::var(stringify!($env)).unwrap_or_else(|_| panic!("env var '{}' not set", stringify!($env))) $( + $post )*
    };
}

#[macro_export]
macro_rules! setenv {
    ($name:ident $base:ident $value:expr) => {
        setenv!($name {format!("{}{}/", $crate::envg!($base), $value)})
    };
    ($name:ident $value:expr) => {
        unsafe { std::env::set_var(stringify!($name), $value) }
    };
}

#[macro_export]
macro_rules! rsbin {
    ($val:expr) => {
        format!("{}{}", envg!(DOT "/rs/bin/"), $val)
    }
}

#[macro_export]
macro_rules! upanic {
    ($($arg:tt)*) => {{
        eprint!("\x1b[31m");
        eprint!($($arg)*);
        eprintln!("\x1b[0m");
        std::process::exit(1);
    }};
}

#[macro_export]
macro_rules! debug {
    ($($arg:tt)*) => {{
        if std::env::var("DEBUG").is_ok() {
            $crate::Ti::MIL.mul(500).slp();
            eprint!("\x1b[35m");
            eprint!($($arg)*);
            eprintln!("\x1b[0m");
        }
    }};
}

pub struct Ti(u64);

impl Ti {
    pub const MIL: Self = Self(1);
    pub const MIN: Self = Self::SEC.mul(60);
    pub const SEC: Self = Self::MIL.mul(1000);

    pub const fn mul(self, factor: u64) -> Self {
        Self(self.0 * factor)
    }

    pub fn slp(self) {
        sleep(Duration::from_millis(self.0));
    }
}

pub fn map<F: for<'a> Fn(&'a String) -> Option<&'a String>>(f: F) -> F {
    f
}

pub fn link(from: &str, to: &str) {
    spawn!("sudo", "ln", "-sf", from, to)
}

pub trait CollectVec<T>: Iterator<Item = T> + Sized {
    fn colvec(self) -> Vec<T> {
        self.collect()
    }
}

impl<T, I: Iterator<Item = T>> CollectVec<T> for I {}
