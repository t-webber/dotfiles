macro_rules! repl { ( $($v:expr);* ) => {
    concat!( $( "try: import ", stringify!($v), "\nexcept: 0\n" ,)* "from os import environ as env" )
}}

rs::cli! {
    =>  if args().count() == 2 { spawn!("uv", "run", "python", args().nth(1).unwrap()); exit(0); },
    a { #[arg(required = true)] names: Vec<String> } => spawn!("uv", "add"         ; vec: names),
    d { #[arg(required = true)] names: Vec<String> } => spawn!("uv", "add", "--dev"; vec: names),
    r { #[arg(required = true)] names: Vec<String> } => spawn!("uv", "remove"      ; vec: names),
    k { name: Option<String> } => if let Some(inner) = name { spawn!("uv", "lock", "--upgrade-package", inner) } else { spawn!("uv", "lock", "--upgrade") },
    "pip" p { args: Vec<String> } => spawn!("uv", "pip"; vec: args),
    "freeze" z => spawn!("uv", "pip", "freeze"),
    "show" o => spawn!("uv", "pip", "show"),
    "ty" l => spawn!("uv", "run", "ty", "check"),
    "ruff" f => spawn!("uv", "run", "ruff", "check"),
    y => spawn!("uv", "sync"),
    i => spawn!("uv", "init"),
    n => spawn!("uv", "venv"),
    t => spawn!("uv", "run", "pytest"),
    "streamlit" s => spawn!("uv", "run", "streamlit", "run"),
    m { package: String } => spawn!("uv", "run", "python", "-m", package),
    c { code: String } => spawn!("uv", "run", "python", "-c", code),
    e => spawn!("uv", "run", "python", "-ic", repl!(pandas as pd; polars as pl; numpy as np; matplotlib.pyplot as plt; time; os; math; sys)),
    "run in $WASTE" w => { let file = envg!(WASTE "del.py"); spawn!("touch", file); spawn!(envg!(EDITOR), file); spawn!("uv", "run", "python", file); },
    "serve" v { #[arg(num_args = 0..=2)] args: Vec<String> } => {
        let mut port = 5000;
        let mut host = "localhost".into();
        for inner in args {
            if let Ok(new) = inner.parse::<u32>() {
                port = new
            } else {
                host = inner;
            }
        }
        spawn!("uv", "run", "python", "-m", "http.serve", port, "-b", host);
    },
}
