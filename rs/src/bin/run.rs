use std::env;
use std::fs::{read_dir, read_to_string};
use std::process::exit;

use rs::{CollectVec as _, spawn, upanic};

fn main() {
    let mut args = env::args().skip(1).colvec();

    if let Some(index) = args.iter().position(|x| x.split("/").last().unwrap().contains(".")) {
        let file = args.remove(index);
        let (stem, ext) = file.rsplit_once(".").unwrap();
        let stem = stem.split("/").last().unwrap();
        if let Ok(content) = read_to_string(&file)
            && content.starts_with("#!")
        {
            spawn!(content.lines().next().unwrap()[2..].to_string(), "file"; vec: args);
            exit(0);
        }
        let release = if let Some(i) = args.iter().position(|x| x == "-r") {
            args.remove(i);
            true
        } else {
            false
        };
        match ext {
            "typ" => {
                if args.len() >= 2 || release {
                    upanic!("typst runner can only take input and output")
                }
                let out = if args.is_empty() {
                    format!("target/{}.pdf", stem)
                } else {
                    args.pop().unwrap()
                };
                spawn!("typst", "compile", file, out);
                exit(0);
            }
            "c" | "cpp" | "cc" | "C" => {
                let prog = if ext == "c" { "gcc" } else { "g++" };
                let run_args = args.split_off(index);
                let out = format!("/tmp/{stem}.out");
                let (flag, opt) = if release { ("-flto", "-Ofast") } else { ("-g", "-O0") };
                spawn!(prog, flag, opt, file, "-o", out; vec: args);
                spawn!(out; vec: run_args);
                exit(0);
            }
            "rs" => {
                let run_args = args.split_off(index);
                let out = format!("/tmp/{stem}.out");
                if release {
                    args.push("-C".to_owned());
                    args.push("opt-level=3".to_owned());
                }
                spawn!("rustc",  file, "-o", out; vec: args);
                spawn!(out; vec: run_args);
                exit(0);
            }
            "go" => {
                spawn!("go", "run", file; vec: args);
                exit(0);
            }
            _ => (),
        }
    }

    let dir = read_dir(".").unwrap().map(Result::unwrap).colvec();

    if dir.iter().any(|f| f.file_name().to_string_lossy().starts_with("Makefile")) {
        spawn!("make"; vec: args.into_iter().map(|x| match x.as_str() {
            "c" => "clean".to_owned(),
            "d" => "dev".to_owned(),
            "g" => "debug".to_owned(),
            "r" => "release".to_owned(),
            "u" => "run".to_owned(),
            "ii" => "init".to_owned(),
            "i" => "install".to_owned(),
            "a" => "start".to_owned(),
            "o" => "stop".to_owned(),
            "e" => "enable".to_owned(),
            "s" => "disable".to_owned(),
            "k" => "check".to_owned(),
            "t" => "test".to_owned(),
            "v" =>"valgrind".to_owned(),
            _ => x,
        }));
        exit(0);
    }

    if dir.iter().any(|f| f.file_name().to_string_lossy() == "go.mod") {
        spawn!("go", "run", "."; vec: args);
        exit(0);
    }

    if dir.iter().any(|f| f.file_name().to_string_lossy() == "package.json") {
        for x in match args.pop().unwrap_or_default().as_str() {
            "" | "d" => vec!["run", "dev"],
            "i" => vec!["install"],
            "b" => vec!["run", "build"],
            "a" => vec!["run", "start"],
            x => vec![x],
        }
        .iter()
        .rev()
        {
            args.insert(0, x.to_string());
        }
        spawn!("bun", "run"; vec: args);
        exit(0);
    }
}
