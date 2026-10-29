#[macro_export]
macro_rules! cli {(
    $(=> $other:expr;)?
    $(
        $($vdoc:literal)? $variant:ident
        $({ $( $(#[$fmeta:meta])* $($fdoc:literal)? $field:ident : $ty:ty $(= $default:expr)? ),* })?
        => $body:expr
    )*
    $(;
        $($name:ident $($arg:ident: $argty:ty),* $(: $ret:ty)? => $expr:expr)*
    )?
) => {
    use std::env::args;
    use std::process::{exit, Stdio};
    use clap::Parser;
    use rs::*;

    type S = String;
    type OS = Option<String>;

    #[allow(non_camel_case_types)]
    #[derive(Parser, Debug)]
    pub enum Cli { $(
        $( #[doc = $vdoc] )?
        $variant $({ $(
            $(#[$fmeta])*
            $(#[arg(default_value = stringify!($default))])?
            $(#[doc = $fdoc])?
            $field : $ty ),*
        })?,
    )* }

    fn main() {
        use clap::{CommandFactory, FromArgMatches};

        let cmd = Cli::command().disable_help_subcommand(true);

        let names: Vec<String> = cmd
            .get_subcommands()
            .map(|s| s.get_name().to_string())
            .collect();

        let prog = args().next().unwrap();
        let colnames = names.join("\x1b[31m|\x1b[33m");
        let usage = format!("\x1b[36m{prog}\x1b[31m [\x1b[33m{colnames}\x1b[31m]\x1b[0m");
        // let cmd = cmd.help_template(format!("\x1b[31muse {usage}\n")).override_usage(&usage);

        let first = args().nth(1);
        if first.as_ref().is_none_or(|inner| inner != "--help" && inner != "@" && !names.contains(inner)) { $($other)? }
        if first.is_none_or(|inner| inner == "@") {
            println!("\x1b[31muse {usage}");
            exit(2);
        }

        let cli = Cli::from_arg_matches(&cmd.get_matches()).unwrap_or_else(|e| e.exit());

        match cli { $(
            Cli::$variant $({ $(mut $field),* })? => { $body; }
        )* }
    }

    $(
        $(
            fn $name($($arg: $argty),*) $(-> $ret)? { $expr }
        )*
    )?
};
}
