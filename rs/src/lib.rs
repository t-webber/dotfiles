use core::time::Duration;
use std::process::Stdio;
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
    use rs::{envg, spawn, setenv, slp, map, link, date, upanic, rsbin};

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
                eprintln!("{}: {:?}\x1b[0m", disp, ret);
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
        $(; stdin: $in:expr)? $(; out: $out:expr)? $(; err: $err:expr)?
        $(; feed: $feed:expr)?
        $(; ignore: $ignore:literal)?
        $(; map: $map:expr)?
        $(; output: $output:literal)?
        $(; ret: $ret:literal)?
        $(; both: $both:expr)?
        $(; nowait: $nowait:literal)?
        $(; discard: $discard:literal)?
    ) => {{ #[allow(unused_mut)] {
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
            $(cmd.stdout(Stdio::from($out));)?
            $(cmd.stderr(Stdio::from($err));)?
            $(
                let f = $both.try_clone().unwrap();
                cmd.stdout(Stdio::from(f.try_clone().unwrap()));
                cmd.stdout(Stdio::from(f.try_clone().unwrap()));
            )?
            $(cmd.stdout(Stdio::piped()); let _ = &$map;)?
            $(cmd.stdout(Stdio::piped()); let _ = $output;)?
            $(cmd.stdin(Stdio::piped()); let _ = &$feed;)?
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
            Some((child, format!("\x1b[37m{prog} {}", args.join(" "))))
        } else {
            None
        };
        $crate::spawn!(finish cmdisp $(discard: $discard)? $(nowait: $nowait)? $(output: $output)? $(; ignore: $ignore)? $(; ret: $ret)?)
    }}};
}

#[macro_export]
macro_rules! envg {
    ($env:ident $($post:expr),* $(; pre: $first:expr $(, $pre:expr)*)?) => {
       $( $first.to_string() $( + $pre )* + & )? std::env::var(stringify!($env)).unwrap() $( + $post )*
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

pub fn date() -> String {
    spawn!("date", "+%H:%M:%S"; output: 1)
}

pub trait CollectVec<T>: Iterator<Item = T> + Sized {
    fn colvec(self) -> Vec<T> {
        self.collect()
    }
}

impl<T, I: Iterator<Item = T>> CollectVec<T> for I {}
