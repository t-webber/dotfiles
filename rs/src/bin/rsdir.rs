use std::env;

use rs::{FileKind, expand, upanic};

// rsdir to something in ./*/
// rsdir to bin

fn main() {
    if env::args().count() != 2 {
        upanic!("Usage: {} <path>", env::args().next().unwrap())
    }
    println!("{}", expand(env::args().nth(1).unwrap(), FileKind::Folder).join("\n"))
}
