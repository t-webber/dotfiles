use core::time::Duration;
use std::thread::sleep;

#[macro_export]
macro_rules! cli {(
    $(=> $other:expr,)?
    $(
        $($doc:literal)? $variant:ident $({ $(#[$vmeta:meta])* $( $field:ident : $ty:ty ),* })? => $body:expr,
    )*
    $(:
        $($name:ident $($arg:ident: $argty:ty),* $(: $ret:ty)? => $expr:expr)*
    )?
) => {
    use std::env::args;
    use std::process::{exit, Stdio};
    use clap::Parser;
    use rs::{envg, spawn, setenv, slp};

    #[allow(non_camel_case_types)]
    #[derive(Parser, Debug)]
    pub enum Cli { $(
        $( #[doc = $doc] )?
        $variant $({ $(#[$vmeta])* $( $field : $ty ),* })?,
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
        if first.as_ref().is_none_or(|inner| inner != "--help" && inner != "@") { $($other)? }
        if first.is_none_or(|inner| inner == "@") {
            println!("\x1b[31muse {usage}");
            exit(0);
        }

        let cli = Cli::from_arg_matches(&cmd.get_matches()).unwrap_or_else(|e| e.exit());

        match cli { $(
            Cli::$variant $({ $($field),* })? => { $body; }
        )* }
    }

    $(
        $(
            fn $name($($arg: $argty),*) $(-> $ret)? { $expr }
        )*
    )?
};
}

#[macro_export]
macro_rules! proc {
    (
        $prog:expr
        $(, $arg:expr)*
        $(; vec: $vec:expr)?
        $(; stdin: $in:expr)?
        $(; out: $out:expr)?
        $(; err: $err:expr)?
        $(; ignore: $ignore:expr)?
        $(; filter: $filter:expr; $disp:expr)?
    ) => {{}};
}

#[macro_export]
macro_rules! spawn {

    (res $retdisp:ident; ret: $ret:expr) => {{
        $retdisp.unwrap().0.success()
    }};

    (res $retdisp:ident $(; ignore: $ignore:expr)?) => {{
        if let Some((ret, disp)) = $retdisp {
            let mut ignore = false;
            $( ignore = true; let _ = $ignore;)?
            if !ret.success() && !ignore { eprintln!("{}: {:?}\x1b[0m", disp, ret); std::process::exit(13); }
        }
    }};

    (finish $cmdisp:ident $(; ignore: $ignore:expr)? $(; filter: $filter:expr)? $(; ret: $ret:expr)?) => {{
        let retdisp = $cmdisp.map(|(mut child, disp)| {
            $( {
                use std::io::{BufRead, BufReader, Write};
                let f = $filter;
                let reader = BufReader::new(child.stdout.take().unwrap());
                for line in reader.lines() {
                    let line = line.unwrap();
                    if f(&line) {
                        println!("{line}")
                    }
                }
            } )?
            (child.wait().unwrap(), disp)
        });
        $crate::spawn!(res retdisp $(; ignore: $ignore)? $(; ret: $ret)?)
    }};


    (finish $cmdisp:ident $(output: $output:expr)?) => {{
        if let Some((mut child, _)) = $cmdisp {
            String::from_utf8_lossy_owned(child.wait_with_output().unwrap().stdout)
        } else {
            panic!("DEBUG=2 can't be used when expecting a string")
        }
    }};

    (
        $prog:expr $(, $arg:expr)*
        $(; vec: $vec:expr)?
        $(; stdin: $in:expr)? $(; out: $out:expr)? $(; err: $err:expr)?
        $(; feed: $feed:expr)?
        $(; ignore: $ignore:expr)?
        $(; filter: $filter:expr)?
        $(; output: $output:expr)?
        $(; ret: $ret:expr)?
    ) => {{
        let dbg: u32 = std::env::var("DEBUG").unwrap_or("0".into()).parse().unwrap();
        let prog = $prog;
        let mut args: Vec<String> = vec![];
        $( args.push($arg.to_string()); )*
        $( args.extend($vec); )?
        if dbg != 0 {
            println!("\x1b[35m+++ {prog} {}\x1b[0m", args.join(" "));
        }
        let cmdisp = if dbg != 2 {
            let mut cmd = std::process::Command::new(&prog);
            cmd.args(&args);
            $(cmd.stdin($in);)?
            $(cmd.stdout($out);)?
            $(cmd.stderr($err);)?
            $(cmd.stdout(Stdio::piped()); let _ = &$filter;)?
            $(cmd.stdout(Stdio::piped()); let _ = $output;)?
            $(cmd.stdin(Stdio::piped()); let _ = &$feed;)?
            let mut child = cmd.spawn().unwrap();
            $(
                let mut stdin = child.stdin.take().unwrap();
                stdin.write_all($feed.as_bytes()).unwrap();
                drop(stdin);
            )?
            Some((child, format!("\x1b[37m{prog} {}", args.join(" "))))
        } else {
            None
        };
        $crate::spawn!(finish cmdisp $(output: $output)? $(; ignore: $ignore)? $(; filter: $filter)? $(; ret: $ret)?)
    }};
}

#[macro_export]
macro_rules! envg {
    ($env:ident $($val:expr),*) => {
        std::env::var(stringify!($env)).unwrap() $( + $val )*
    };
}

#[macro_export]
macro_rules! setenv {
    ($name:ident $value:expr) => {
        unsafe { std::env::set_var(stringify!($name), $value) }
    };
}

pub fn slp(ms: u64) {
    sleep(Duration::from_millis(ms));
}
