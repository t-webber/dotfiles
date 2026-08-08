use std::{env, fs};

use rs::{envg, spawn, upanic};

const PROG_C: &str = r#"
#include "lib.h"

int main(const int argc, Args argv) {
    return 0;
}
"#;

const PROG_RS: &str = r#"
rs::cli! {}

fn main() {}
"#;

const PROG_SH: &str = r#"
#!/bin/sh

set -euo pipefail

"#;

fn new(folder: &str, file: &str, content: &str) {
    let path = format!("{}/{folder}/{file}", envg!(DOT));
    if !fs::exists(&path).unwrap() {
        fs::write(&path, content).unwrap();
    }
    spawn!(envg!(EDITOR), path);
}

fn main() {
    let mut args = env::args();
    let prog = args.next().unwrap();
    if env::args().count() != 2 {
        upanic!("Usage: {prog} <filename>[.c|.rs]");
    };
    let fname = args.next().unwrap();
    if fname.ends_with(".c") {
        new("cmd/src", &fname, PROG_C);
    } else if fname.ends_with(".rs") {
        new("rs/src/bin", &fname, PROG_RS);
    } else {
        new("cmd/sh", &fname, PROG_SH);
    }
}
