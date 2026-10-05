#[macro_export]
macro_rules! cli {
    (
    $(=> $other:expr,)?
    $(
        $variant:ident $({  $(#[$vmeta:meta])* $(  $field:ident : $ty:ty ),* $(,)? })? => $body:expr,
    )*
    $(:
        $($name:ident $($arg:ident: $argty:ty),* $(: $ret:ty)? => $expr:expr)*
    )?
) => {
    use std::env;
    use clap::Parser;
    use rs::{envg, spawn, sspawn};


    #[allow(non_camel_case_types)]
    #[derive(Parser, Debug)]
    pub enum Cli {
       $(
            $variant $({  $(#[$vmeta])* $( $field : $ty ),* })?,
        )*
    }

    fn main() {
        use clap::{CommandFactory, FromArgMatches};

        let cmd = Cli::command().disable_help_subcommand(true);

        let names: Vec<String> = cmd
            .get_subcommands()
            .map(|s| s.get_name().to_string())
            .collect();
        let usage = format!("{} [{}] [--help]", env::args().next().unwrap(), names.join("|"));

        match env::args().nth(1) {
            Some(first) if names.contains(&first) || first != "--help"  => (),
            _ =>  { $($other)? }
        }

        let cmd = cmd
            .help_template(format!("Usage: {usage}\n"))
            .override_usage(usage);

        let cli = Cli::from_arg_matches(&cmd.get_matches())
            .unwrap_or_else(|e| e.exit());

        match cli {
            $( Cli::$variant $({ $($field),* })? => { $body; } )*
        }
    }

    $(
        $(
            fn $name($($arg: $argty),*) $(-> $ret)? { $expr }
        )*
    )?
};
}

#[macro_export]
macro_rules! spawn { (
    $prog:expr $(, $arg:expr)* $(; vec: $vec:expr)?
    $(; stdin: $in:expr)? $(; out: $out:expr)? $(; err: $err:expr)?
    $(; ignore: $ignore:expr)?
) => {{
        let dbg: u32 = env::var("DEBUG").unwrap_or("0".into()).parse().unwrap();
        let prog = $prog;
        let mut args = vec![];
        $( args.push($arg.to_string()); )*
        $( args.extend($vec); )?
        if dbg != 0 {
            println!("\x1b[35m{prog}: {args:?}\x1b[0m");
        }
        if dbg != 2 {
            let mut cmd = std::process::Command::new(&prog);
            cmd.args(args);
            $(cmd.stdin($in);)?
            $(cmd.stdout($out);)?
            $(cmd.stderr($err);)?
            let ret = cmd.spawn().unwrap().wait().unwrap();
            let mut ignore = false;
            $( ignore = $ignore;)?
            if !ret.success() && !ignore { eprintln!("\x1b[36m{prog}\x1b[31m: {ret:?}\x1b[0m"); std::process::exit(13); }
        }
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
