use std::env;
use std::fs::{OpenOptions, read_to_string};

use rs::{spawn, upanic};

fn main() {
    if read_to_string("/proc/self/status")
        .unwrap()
        .lines()
        .all(|l| !l.starts_with("Uid:") || l.split_whitespace().nth(2) != Some("0"))
    {
        upanic!("Please run this script as root");
    }
    let mut args = env::args();
    let prog = args.next().unwrap();
    let tty = args.next().unwrap_or_else(|| upanic!("Usage: {prog} <tty>"));
    let tty = if let Ok(number) = tty.parse::<u8>() { format!("/dev/tty{number}") } else { tty };
    let file = OpenOptions::new().write(true).read(true).open(tty).unwrap();
    spawn!("setsid", "-c", "init-login"; stdin: file.try_clone().unwrap(); both: file; discard: 1);
}
