#[macro_export]
macro_rules! cli {
    ($(
        $variant:ident $({ $(  $field:ident : $ty:ty ),* $(,)? })? => $body:expr,
    )*
    $(:
        $($name:ident $($arg:ident: $argty:ty),* => $expr:expr)*
    )?

        ) => {
        use std::env;
        use clap::Parser;
        use rs::{envg, spawn, sspawn};


        #[allow(non_camel_case_types)]
        #[derive(Parser, Debug)]
        pub enum Cli {
           $(
                $variant $({ $( $field : $ty ),* })?,
            )*
        }

        fn main() {
            use clap::{CommandFactory, FromArgMatches};

            let cmd = Cli::command().disable_help_subcommand(true);

            let names: Vec<String> = cmd
                .get_subcommands()
                .map(|s| s.get_name().to_string())
                .collect();
            let usage = format!("net [{}] [--help]", names.join("|"));

            let cmd = cmd
                .help_template(format!("Usage: {usage}\n"))
                .override_usage(usage);

            let cli = Cli::from_arg_matches(&cmd.get_matches())
                .unwrap_or_else(|e| e.exit());

            match cli {
                $( Cli::$variant $({ $($field),* })? => { $body; } )*
            }


            match Cli::parse() {
                $( Cli::$variant $({ $($field),* })? => { $body; } )*
            }
        }

        $(
            $(
                fn $name($($arg: $argty),*) { $expr }
            )*
        )?
    };
}

#[macro_export]
macro_rules! spawn {
    ($prog:expr $(, $arg:expr)*) => {{
        let ret = std::process::Command::new($prog)$(.arg($arg))*.spawn().unwrap().wait().unwrap();
        if !ret.success() { panic!("{ret:?}") }
    }};
}

#[macro_export]
macro_rules! sspawn {
    ($($arg:expr),*) => {
        $crate::spawn!("sudo", $($arg),*)
    }
    ;
}

#[macro_export]
macro_rules! envg {
    ($env:ident $($val:expr),*) => {
        env::var(stringify!($env)).unwrap() $( + $val )*
    };
}
