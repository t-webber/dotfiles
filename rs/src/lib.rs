use core::time::Duration;
use std::thread::sleep;

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
            $(#[arg(short, long, default_value = stringify!($default))])?
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

#[macro_export]
macro_rules! spawn {

    (res $retdisp:ident; ret: $ret:expr) => {{
        if let Some((ret, _)) = $retdisp {
            ret.success()
        } else {
            panic!("DEBUG=2 can't be used when expecting a success value")
        }
    }};

    (res $retdisp:ident $(; ignore: $ignore:expr)?) => {{
        if let Some((ret, disp)) = $retdisp {
            let mut ignore = false;
            $( ignore = true; let _ = $ignore;)?
            if !ret.success() && !ignore {
                eprintln!("{}[{}] {}: {:?}\x1b[0m", $crate::ORANGE, std::env::args().next().unwrap(), disp, ret.code());
                std::process::exit(3);
            }
        }
    }};

    (finish $cmdisp:ident $(; ignore: $ignore:expr)? $(; ret: $ret:expr)?) => {{
        let retdisp = $cmdisp.map(|(mut child, disp)| {
            (child.wait().unwrap(), disp)
        });
        $crate::spawn!(res retdisp $(; ignore: $ignore)? $(; ret: $ret)?)
    }};


    (finish $cmdisp:ident output: $output:expr) => {{
        if let Some((mut child, _)) = $cmdisp {
            String::from_utf8_lossy_owned(child.wait_with_output().unwrap().stdout)
        } else {
            panic!("DEBUG=2 can't be used when expecting a string")
        }
    }};

    (finish $cmdisp:ident discard: $discard:expr) => {{}};

    (finish $cmdisp:ident nowait: $nowait:expr) => {{
        if let Some((mut child, _)) = $cmdisp {
            child
        } else {
            panic!("DEBUG=2 can't be used when expecting a child")
        }
    }};


    (
        $prog:expr $(, $arg:expr)*
        $(; vec: $vec:expr)?
        // in/ou
        $(; stdin: $in:expr)? $(; out: $out:expr)? $(; err: $err:expr)?
        $(; feed: $feed:expr)? $(; both: $both:expr)?
        $(; map: $map:expr)?
        $(; output: $output:literal)?
        // ret
        $(; ignore: $ignore:literal)?
        $(; discard: $discard:literal)?
        $(; ret: $ret:literal)?
        $(; nowait: $nowait:literal)?
    ) => {{ #[allow(unused_mut)] {
        use std::process as P;
        let dbg: u32 = std::env::var("DEBUG").unwrap_or("0".into()).parse().unwrap();
        let prog = $prog;
        let mut args: Vec<String> = vec![];
        $( args.push($arg.to_string()); )*
        $( args.extend($vec); )?
        if dbg != 0 {
            println!("\x1b[35m+++ {prog} {}\x1b[0m", args.join(" "));
        }
        let cmdisp = if dbg != 2 {
            let mut cmd = P::Command::new(&prog);
            cmd.args(&args);
            $(cmd.stdin($in);)?
            $(cmd.stdout(P::Stdio::from($out));)?
            $(cmd.stderr(P::Stdio::from($err));)?
            $(
                let f = $both.try_clone().unwrap();
                cmd.stdout(P::Stdio::from(f.try_clone().unwrap()));
                cmd.stderr(P::Stdio::from(f.try_clone().unwrap()));
            )?
            $(cmd.stdout(P::Stdio::piped()); let _ = &$map;  )?
            $(cmd.stdout(P::Stdio::piped()); let _ = $output;)?
            $(cmd.stdin (P::Stdio::piped()); let _ = &$feed; )?
            let mut child = cmd.spawn().unwrap();
            $(
                let mut stdin = child.stdin.take().unwrap();
                stdin.write_all($feed.as_bytes()).unwrap();
                drop(stdin);
            )?
            $( {
                use std::io::{BufRead, BufReader, Write};
                let f = $map;
                let reader = BufReader::new(child.stdout.take().unwrap());
                for line in reader.lines() {
                    let line = line.unwrap();
                    if let Some(modified) = f(&line) {
                        println!("{modified}")
                    }
                }
            } )?
            Some((child, format!("{prog} {}", args.join(" "))))
        } else {
            None
        };
        $crate::spawn!(finish cmdisp $(discard: $discard)? $(nowait: $nowait)? $(output: $output)? $(; ignore: $ignore)? $(; ret: $ret)?)
    }}};
}

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

pub fn slp(ms: u64) {
    sleep(Duration::from_millis(ms));
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

const ISSUE: &str = r#"
oooooooooo.             .o8         .oooooo.    .oooooo..o
`888'   `Y8b           "888        d8P'  `Y8b  d8P'    `Y8
 888     888  .ooooo.   888oooo.  888      888 Y88bo.     
 888oooo888' d88' `88b  d88' `88b 888      888  `"Y8888o. 
 888    `88b 888   888  888   888 888      888      `"Y88b
 888    .88P 888   888  888   888 `88b    d88' oo     .d8P
o888bood8P'  `Y8bod8P'  `Y8bod8P'  `Y8bood8P'  8""88888P' 
"#;

pub fn issue() -> String {
    let issue = ISSUE.trim_start();
    let issue_len = issue.lines().next().unwrap().len();
    let empty_lines = format!("{}\n", " ".repeat(issue_len)).repeat(5);
    let issue = format!("{empty_lines}{issue}{empty_lines}");

    let cols = spawn!("stty", "size"; output: 1)
        .split_whitespace()
        .last()
        .unwrap()
        .parse::<usize>()
        .unwrap();

    let hpad_len = (cols - issue_len - 2) / 4;
    let hpad = " ".repeat(hpad_len);
    let issue = issue
        .lines()
        .map(|line| format!("{hpad}│{hpad}{line}{hpad}│"))
        .collect::<Vec<_>>()
        .join("\n");

    let hline = "─".repeat(hpad_len * 2 + issue_len);
    let first = format!("{hpad}┌{hline}┐");
    let last = format!("{hpad}└{hline}┘");
    format!("\n\x1b[33m{first}\n{issue}\n{last}\n\n")
}

pub const ORANGE: &str = "\x1b[38;2;255;153;85m";

pub fn setfont() {
    spawn!("setfont", "/usr/share/kbd/consolefonts/sun12x22.psfu.gz");
}
