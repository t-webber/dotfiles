use core::sync::atomic::AtomicBool;
use core::sync::atomic::Ordering::AcqRel;
use std::fs::{File, create_dir_all, read_dir, rename, write};
use std::process::Stdio;

use rs::{date, envg, rsbin, setenv, slp, spawn};

macro_rules! wrap {
    ($msg:ident => $code:expr) => {
        fn $msg() {
            display_function_name(stringify!($msg));
            $code
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

const ISSUE: &str = r###"
oooooooooo.             .o8         .oooooo.    .oooooo..o
`888'   `Y8b           "888        d8P'  `Y8b  d8P'    `Y8
 888     888  .ooooo.   888oooo.  888      888 Y88bo.     
 888oooo888' d88' `88b  d88' `88b 888      888  `"Y8888o. 
 888    `88b 888   888  888   888 888      888      `"Y88b
 888    .88P 888   888  888   888 `88b    d88' oo     .d8P
o888bood8P'  `Y8bod8P'  `Y8bod8P'  `Y8bood8P'  8""88888P'
"###;

static FIRST: AtomicBool = AtomicBool::new(true);

fn display_function_name(msg: &str) {
    println!("\x1b[31m{}{}\x1b[0m", if FIRST.swap(false, AcqRel) { '┌' } else { '├' }, msg);
}

fn logfile(name: &str) -> File {
    File::create(envg!(LOGS name)).unwrap()
}

fn issue() -> String {
    let cols = spawn!("stty", "size"; output: 1)
        .split_whitespace()
        .last()
        .unwrap()
        .parse::<usize>()
        .unwrap();
    let issue_len = ISSUE.lines().next().unwrap().len();
    let hpad_len = (cols - issue_len) / 4;
    let hpad = " ".repeat(hpad_len);
    let fixed_issue =
        ISSUE.lines().map(|line| format!("{hpad}│{hpad}{line}")).collect::<Vec<_>>().join("\n");
    let hline = "─".repeat(hpad_len * 2 + issue_len);
    let vpad = "\n".repeat(5);
    format!("\x1b[33m{hpad}┌{hline}┐{vpad}{fixed_issue}{vpad}└{hline}┘\x1b[0m")
}

wrap!(environment => {
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
        rsbin!(""),
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
});

wrap!(logs => {
    rename(envg!(LOGS), envg!(OLOGS)).ok();
    create_dir_all(envg!(LOGS)).unwrap();
    spawn!("chmod", 777, envg!(LOGS));
    write(envg!(LOGS "date"), date()).unwrap();
    write(envg!(LOGS "lwd"), "/").unwrap();
    write(envg!(LOGS "version"), "rs-0").unwrap();
});

wrap!(stage1 out => {
    let mut files: Vec<_> = read_dir("/etc/rc/sysinit/").unwrap().filter_map(|e| e.ok())
        .filter(|e| e.file_type().is_ok_and(|t| t.is_file())).map(|e| e.file_name()).collect();
    files.sort();
    for f in files {
        spawn!(f.into_string().unwrap(), "start"; both: out);
    }
});

wrap!(agetty => {
    create_dir_all(envg!(TMP)).unwrap();
    let f = envg!(TMP "issue");
    write(&f, issue()).unwrap();
    for x in [2, 3] {
        spawn!("agetty", "--noclear", format!("tty{x}"), "-f", f, "linux"; nowait: 1);
    }
});

// wrap!(net out => {
//     spawn!("nt", "au"; both: out; nowait: 1);
//     while !spawn!("getent", "hosts", "example.com"; ret: 1; both: out) {
//         slp(3000);
//     }
// });

wrap!(xinit out => {
    spawn!("runuser", "-u", "b", "--", "xinit", rsbin!("init-xinit"), "--", "-logfile", envg!(LOGS "xorg"); both: out);
});

fn main() {
    spawn!("setfont"; ignore: 1);
    println!("{}", issue());
    environment();
    slp(3000);
    logs();
    slp(3000);
    stage1();
    slp(3000);
    agetty();
    //         net();
    slp(3000);
    xinit();
}
