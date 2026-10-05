macro_rules! repl { ( $($v:expr);* ) => {
    concat!( $( "try: import ", stringify!($v), "\nexcept: 0\n" ,)* "from os import environ as env" )
}}

rs::cli! {
    =>  {
        if env::args().count() == 2 {
            spawn!("uv", "run", "python", env::args().nth(1).unwrap());
            std::process::exit(0);
        }
    },
    a { #[arg(required = true)] names: Vec<String> } => spawn!("uv", "add"         ; vec: names),

    d { #[arg(required = true)] names: Vec<String> } => spawn!("uv", "add", "--dev"; vec: names),
    r { #[arg(required = true)] names: Vec<String> } => spawn!("uv", "remove"      ; vec: names),
    k { name: Option<String> } => if let Some(inner) = name { spawn!("uv", "lock", "--upgrade-package", inner) } else { spawn!("uv", "lock", "--upgrade") },
    l => spawn!("uv", "run", "ty", "check"),
    f => spawn!("uv", "run", "ruff", "check"),
    y => spawn!("uv", "sync"),
    i => spawn!("uv", "init"),
    n => spawn!("uv", "venv"),
    t => spawn!("uv", "run", "pytest"),
    s => spawn!("uv", "run", "streamlit", "run"),
    m { package: String } => spawn!("uv", "run", "python", "-m", package),
    c { code: String } => spawn!("uv", "run", "python", "-c", code),
    e => spawn!("uv", "run", "python", "-ic", repl!(pandas as pd; polars as pl; numpy as np; matplotlib.pyplot as plt; time; os; math; sys)),
    w => { let file = envg!(WASTE "del.py"); spawn!("touch", file); spawn!(envg!(EDITOR), file); spawn!("uv", "run", "python", file); },
    v { #[arg(num_args = 0..=2)] args: Vec<String> } => {
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
