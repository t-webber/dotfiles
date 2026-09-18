use std::env::{self, args};
use std::fs::{File, read_dir};
use std::io::Read;
use std::path::{Path, PathBuf};

use flate2::read::GzDecoder;
use rs::{CollectVec, spawn, upanic};

fn usage() -> ! {
    let prog = args().next().unwrap();
    upanic!(
        "Usage:
        - {prog} <path>: umount
        - {prog} <path> <path> [-m]: mount [--mkdir]
        - {prog} <names...> [-c|-g]: man"
    );
}

fn find_c_pages(dir: &Path) -> Vec<PathBuf> {
    let mut res = vec![];
    for entry in read_dir(dir).unwrap().map(Result::unwrap) {
        let path = entry.path();
        let ft = entry.file_type().unwrap();
        if ft.is_dir() {
            res.extend(find_c_pages(&path));
        } else if ft.is_file() {
            let mut buf = String::new();
            GzDecoder::new(File::open(path).unwrap()).read_to_string(&mut buf).unwrap();
            if buf.contains("#include <") {
                res.push(entry.path())
            }
        }
    }
    res
}

fn main() {
    let mut args = env::args().skip(1).colvec();
    let len = args.len();
    if len == 0 {
        usage()
    }
    if args.iter().any(|p| p.contains('/')) {
        if len == 1 {
            spawn!("sudo", "umount", args[0]);
        } else {
            spawn!("sudo", "mount"; vec: args.into_iter().map(|x| if x == "-m" { "--mkdir".to_owned()} else { x }));
        }
        return;
    }
    args.retain(|x| x != "-mg");
    if len != args.len() {
        spawn!("man"; vec: args.into_iter().map(|x| format!("git-{x}")));
        return;
    }
    args.retain(|x| x != "-mc");
    if len != args.len() {
        spawn!("man"; vec: find_c_pages(Path::new("/usr/share/man/")).into_iter().map(|x|x.display().to_string()));
    }
    spawn!("man"; vec: args);
}
