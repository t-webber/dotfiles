use std::collections::HashMap;
use std::fs::{self, remove_dir_all};
use std::io::{BufRead, BufReader, Read as _, Write as _, stdout};
use std::process::Command;
use std::thread::{self, JoinHandle};

rs::cli! {
    => if args().count() == 1 { build(true); exit(0); };
    cr => { clean(); build(true); }
    cd => { clean(); build(false); }
    d => { build(false); }
    ;
    build release: bool => {
        let r = rs(release); let m = make(release); m.join().unwrap(); r.join().unwrap();
        let dest = envg!(BIN "/init");
        fs::remove_file(&dest).unwrap();
        fs::copy(rsbin!("init"), dest).unwrap();
    }
    rs release: bool: JoinHandle<()> => thread::spawn(move || {
        let mut cmd = Command::new("cargo");
        cmd.args(["-Z", "unstable-options", "-C"]).arg(envg!(DOT "/rs")).arg("build");
        cmd.stderr(Stdio::piped());
        cmd.env("CARGO_TERM_PROGRESS_WHEN", "always").env("CARGO_TERM_PROGRESS_WIDTH", "120");
        if release { cmd.arg("--release"); }
        let mut child = cmd.spawn().unwrap();
        let mut map = HashMap::<String, bool>::new();
        let mut buf = Vec::new();
        for b in BufReader::new(child.stderr.take().unwrap()).bytes() {
            let b = b.unwrap();
            if b != b'\r' && b != b'\n' {
                buf.push(b);
                continue;
            }
            let line = String::from_utf8_lossy(&buf).to_string();
            buf.clear();
            let actions = [" Compiling ", " Building ", " Downloading ", " Finished "];
            if actions.iter().all(|s| !line.contains(s)) {
                eprintln!("\x1b[31m{line}\x1b[0m");
            }
            for val in map.values_mut() {
                *val = false;
            }
            for word in line.split_whitespace() {
                let word = word.trim_suffix(",");
                if !word.ends_with("(bin)") { continue }
                let bin = &word[..word.len()-5];
                map.entry(bin.to_owned()).and_modify(|val| *val = true).or_insert(true);
            }
            for (name, val) in map.clone().iter() {
                if !*val {
                    println!("\x1b[38;2;255;153;85m>>> Compiled {name}\x1b[0m");
                    map.remove(name);
                    stdout().flush().unwrap();
                }
            }
        }
        child.wait().unwrap();
    })
    make release: bool : JoinHandle<()> => thread::spawn(move || {
        let mut cmd = Command::new("make");
        cmd.args([if release { "release" } else { "debug" }, "--no-print-directory", "-C"]);
        cmd.arg(envg!(CMD "/src")).stdout(Stdio::piped());
        let mut child = cmd.spawn().unwrap();
        for line in BufReader::new(child.stdout.take().unwrap()).lines() {
            let line = line.unwrap();
            if line.starts_with("/bin/gcc -std=c99 ") {
                let last = line.split_whitespace().last().unwrap().split("/").last().unwrap();
                if !last.contains(".") {
                    println!("\x1b[36m>>> Compiled {last}\x1b[0m");
                    stdout().flush().unwrap();
                }
            } else if !line.starts_with("sudo ch") && !line.ends_with(" && true") {
                eprintln!("\x1b[31m{line}\x1b[0m");
            }
        }
        child.wait().unwrap();
    })
    clean => {
         spawn!("make", "-C", envg!(CMD "/src/"), "--no-print-directory", "clean"
             ; out: Stdio::null(); ignore: 1);
         spawn!("cargo", "-Z", "unstable-options", "-C", envg!(DOT "/rs/"), "clean"
             ; out: Stdio::null(); ignore: 1);
         remove_dir_all(rsbin!("")).ok();
         slp(1000);
    }

}
