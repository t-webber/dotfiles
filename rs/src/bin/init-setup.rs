use std::fs::{self, File, create_dir_all, read_dir, remove_dir_all, rename, write};
use std::io::{Write, stdout};
use std::time::Instant;

use rs::{CollectVec, envg, issue, rsbin, setenv, setfont, spawn};

macro_rules! wrap {
    ($msg:ident => $code:expr) => {
        fn $msg() {
            let start = Instant::now();
            display_function_name(stringify!($msg));
            $code;
            let end = Instant::now().duration_since(start).as_millis();
            print!(" ({end}ms)\n")
        }
    };
    ($msg:ident $out:ident => $code:expr) => {
        wrap!($msg => {
            #[allow(unused_variables)]
            let $out = logfile(stringify!($msg));
            $code
        });
    };
}

fn display_function_name(msg: &str) {
    print!("\x1b[33m├ {}", msg);
    stdout().flush().unwrap();
}

fn logfile(name: &str) -> File {
    File::create(envg!(LOGS name)).unwrap()
}

wrap!(env => {
    // depth 1
    setenv!(FILES "/del/");
    // depth 2
    setenv!(DOT FILES ".dot");
    setenv!(DEV FILES ".dev");
    setenv!(BIN FILES ".bin");
    setenv!(TMP FILES ".temp");
    setenv!(BLOB FILES ".blob");
    setenv!(WORK FILES ".work");
    setenv!(APPS FILES ".apps");
    setenv!(DATA FILES ".data");
    setenv!(WASTE FILES ".waste");
    setenv!(STUDY FILES ".study");
    setenv!(SECRET FILES ".secret");
    // depth 3
    setenv!(CMD DOT "cmd");
    setenv!(CMD_SRC CMD "src");
    setenv!(XCFG DOT "xdgconfig");
    setenv!(OCFG DOT "otherconfig");
    setenv!(ETC DOT "etc");
    setenv!(LOGS DATA "logs");
    setenv!(OLOGS DATA "ologs");
    setenv!(XDG_CONFIG_HOME {envg!(XCFG)});
    setenv!(XDG_DATA_HOME DATA ".shared");
    setenv!(XDG_CACHE_HOME DATA ".cache");
    setenv!(XDG_STATE_HOME DATA ".state");
    setenv!(XDG_RUNTIME_DIR DATA ".rt");
    // apps
    setenv!(RUSTUP_HOME DATA "rustup");
    setenv!(CARGO_HOME DATA "cargo");
    setenv!(BUN_INSTALL DATA "bun");
    setenv!(ANDROID_HOME "/opt/android-sdk/");
    // needed
    setenv!(SHELL "/bin/bash");
    setenv!(EDITOR "/bin/nvim");
    setenv!(DEVICE "/bin/nvim");
    // x
    setenv!(GTK_THEME "Adwaita:dark");
    setenv!(PASSWORD_STORE_BACKEND "plain");
    setenv!(CHROME_SECRET_STORE "gnome");
    setenv!(XCURSOR_THEME "HUD_Machine_Red_Cursors");
    setenv!(XCURSOR_SIZE "50");
    // path
    let mut path = Vec::new();
    for arg in [
        "/bin".into(),
        envg!(CMD "sh"),
        envg!(CMD "bin"),
        envg!(DOT "rs/bin"),
        envg!(BIN),
        envg!(CARGO_HOME "bin"),
        envg!(BUN_INSTALL "bin"),
        envg!(APPS "flutter/bin"),
        envg!(ANDROID_HOME "emulator"),
        envg!(ANDROID_HOME "platform-tools"),
        envg!(ANDROID_HOME "cmdline-tools/latest/bin"),
        envg!(ANDROID_HOME "build-tools/36.0.0"),
    ] {
        if std::fs::exists(&arg).unwrap() {
            path.push(arg)
        };
    }
    setenv!(PATH path.join(":"));
    //
    let out = File::create("/env").unwrap();
    spawn!("env"; both: out);
});

wrap!(logs => {
    let mut logs = Vec::new();
    remove_dir_all(envg!(OLOGS)).inspect_err(|e| logs.push(e.to_string())).ok();
    rename(envg!(LOGS), envg!(OLOGS)).inspect_err(|e| logs.push(e.to_string())).ok();
    create_dir_all(envg!(LOGS)).inspect_err(|e| println!("log removal err: {e}")).ok();
    write(envg!(LOGS "date"), spawn!("date"; output: 1)).unwrap();
    write(envg!(LOGS "lwd"), "/\n").unwrap();
    write(envg!(LOGS "version"), "rs-0\n").unwrap();
    write(envg!(LOGS "log"), logs.join("\n======\n")).unwrap();
    spawn!("chmod", 666, "-R", envg!(LOGS));
    spawn!("chmod", 777, envg!(LOGS));
});

wrap!(stage1 out => {
    let mut files: Vec<_> = read_dir("/etc/rc/sysinit/").unwrap().filter_map(|e| e.ok())
        .filter(|e| e.file_type().is_ok_and(|t| t.is_symlink()))
        .map(|e| e.path().into_string().unwrap()).collect();
    files.sort();
    for f in files {
        spawn!(f, "start"; both: out; ignore: 1);
    }
});

wrap!(agetty => {
    create_dir_all(envg!(TMP)).unwrap();
    let issue = issue();
    let stripped_issue = issue.replace("\x1b[33m", "").replace("\x1b[0m", "").trim()
        .lines().map(|l| l.trim().to_owned()).colvec().join("\n");
    write(&envg!(TMP "issue-lock"), stripped_issue).unwrap();
    let f = envg!(TMP "issue");
    write(&f, issue).unwrap();
    for x in [2, 3] {
        spawn!("agetty", "--noclear", format!("tty{x}"), "-f", f, "linux"; nowait: 1);
    }
});

wrap!(net out => spawn!("nt", "au"; both: out; discard: 1));

wrap!(tty => fs::read_dir("/dev/").unwrap()
                .map(|f| f.unwrap().path().to_str().unwrap().to_string())
                .filter(|p| p.starts_with("/dev/tty"))
                .for_each(|p| spawn!("chmod", 777, p)));

wrap!(xinit out => {
    println!("\x1b[0m");
    spawn!("runuser", "-u", "b", "--", "xinit", rsbin!("init-xinit"), "--", "-logfile", envg!(LOGS "xorg")
            ; both: out; discard: 1);
});

fn main() {
    let t = Instant::now();
    setfont();
    print!("{}", issue());
    println!("\x1b[33m┌ display ({}ms)", Instant::now().duration_since(t).as_millis());
    spawn!("loadkeys", "fr-latin1"; discard: 1);
    env();
    logs();
    stage1();
    net();
    tty();
    agetty();
    xinit();
}
