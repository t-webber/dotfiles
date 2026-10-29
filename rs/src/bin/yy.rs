use std::env::set_current_dir;
use std::fs::create_dir_all;

use rayon::iter::{IntoParallelIterator, ParallelIterator};

macro_rules! repl { ( $($v:expr);* ) => {
    concat!( $( "try: import ", stringify!($v), "\nexcept: 0\n" ,)* "from os import environ as env" )
}}

macro_rules! uv {
    ($($b:ident)* $(,$c:expr)* $(; vec: $vec:expr)? $(; ret: $ret:expr)? ) => {
        spawn!("uv" $(,stringify!($b))* $(,$c)* $(; vec: $vec)? $(; ret: $ret)?)
    };
}

rs::cli! {
    =>  if args().count() == 2 { spawn!("uv", "run", "python", args().nth(1).unwrap()); exit(0); };
    // packages
    a { #[arg(required = true)] names: Vec<String> } => add(names, false)
    d { #[arg(required = true)] names: Vec<String> } => add(names, true)
    r { #[arg(required = true)] names: Vec<String> } => uv!(remove; vec: names)
    k { name: Option<String> } => if let Some(inner) = name { uv!(lock, "--upgrade-package", inner) } else { uv!(lock, "--upgrade") }
    y => uv!(sync)
    // run
    m { package: String } => uv!(run python, "-m", package)
    c { code: String } => uv!(run python, "-c", code)
    e => uv!(run python, "-ic", repl!(pandas as pd; polars as pl; numpy as np; matplotlib.pyplot as plt; time; os; math; sys))
    "streamlit" s => uv!(run streamlit run)
    "waste" w => { let file = envg!(WASTE "del.py"); spawn!("touch", file); spawn!(envg!(EDITOR), file); uv!(run python, file); }
    "serve" v { #[arg(num_args = 0..=2)] args: Vec<String> } => serve(args)
    "interpret" i { file: String } => uv!(run python, "-i", file)
    // checks
    "ty" l => uv!(run ty check)
    "ruff" f => uv!(run ruff  check)
    "test" t => uv!(run pytest)
    // utils
    "pip" p { args: Vec<String> } => uv!(pip; vec: args)
    "show" o => uv!(pip show)
    "freeze" z => uv!(pip freeze)
    // project
    "init" ii => uv!(init)
    "venv" n => uv!(venv)
    "new"  u {name: String} => {
        create_dir_all(&name).unwrap(); set_current_dir(&name).unwrap();
        uv!(venv); uv!(init); uv!(sync); spawn!("uv", "d", "ruff", "ty");
    }
    ;
    serve args: Vec<String> => {
        let mut port = 5000;
        let mut host = "localhost".into();
        for inner in args {
            if let Ok(new) = inner.parse::<u32>() {
                port = new
            } else {
                host = inner;
            }
        }
        uv!(run python, "-m", "http.serve", port, "-b", host);
    }
}

fn add(names: Vec<String>, dev: bool) {
    let mut remainder = names
        .into_par_iter()
        .filter(|name| {
            let mut v = vec![name.to_owned()];
            if dev {
                v.push("--dev".to_owned());
            }
            !spawn!("uv","--offline","--no-sync"; vec:v; err:Stdio::null(); ret:1)
        })
        .collect::<Vec<_>>();
    if dev {
        remainder.push("--dev".to_owned());
    }
    uv!(add,"--dev"; vec: remainder);
}
