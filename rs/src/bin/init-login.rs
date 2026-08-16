use core::ffi::{CStr, c_char};
use std::ffi::CString;
use std::fs::read_to_string;
use std::io::{Write, stdin, stdout};
use std::process;

use rs::{Ti, issue, setfont, spawn, upanic};

#[link(name = "crypt")]
unsafe extern "C" {
    fn crypt(key: *const c_char, salt: *const c_char) -> *const c_char;
}

fn is_setsid() -> bool {
    let s = read_to_string("/proc/self/stat").unwrap();
    let s = s.rsplit_once(") ").unwrap().1;
    s.split_whitespace().nth(3) == Some(process::id().to_string().as_str())
}

fn verify(pass: &str, hash: &str) -> bool {
    unsafe {
        let r = crypt(CString::new(pass).unwrap().as_ptr(), CString::new(hash).unwrap().as_ptr());
        !r.is_null() && CStr::from_ptr(r).to_bytes() == hash.as_bytes()
    }
}

fn readline() -> String {
    print!("Password:");
    stdout().flush().unwrap();
    spawn!("stty", "-echo");
    let mut buf = String::new();
    stdin().read_line(&mut buf).unwrap();
    spawn!("stty", "echo");
    println!();
    buf
}

fn login() {
    let hash = read_to_string("/etc/shadow")
        .unwrap()
        .lines()
        .find(|l| l.starts_with("b:"))
        .unwrap()
        .split(":")
        .nth(1)
        .unwrap()
        .trim()
        .to_owned();
    for i in 0.. {
        Ti::SEC.mul(i * i).slp();
        let pass = readline();
        if verify(pass.trim(), &hash) {
            break;
        }
    }
}

fn main() {
    if !is_setsid() {
        upanic!("Can't run in tty with setsid: use init-tty");
    }
    loop {
        spawn!("clear");
        setfont();
        println!("{}", issue());
        login();
        spawn!("runuser", "-u", "b", "--", "bash", "-i");
        Ti::SEC.mul(2).slp();
    }
}
