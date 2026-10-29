use core::fmt::Display;
use core::sync::atomic::AtomicU8;
use core::sync::atomic::Ordering::{Acquire, Release};
use std::fs::read_dir;
use std::io::{Write as _, stdout};
use std::path::{Path, PathBuf};
use std::sync::Arc;
use std::sync::atomic::AtomicBool;
use std::thread::spawn;

macro_rules! checker {
    ($upd:ident $($name:ident $do:expr;)*) => {
        $(
            let $name = State::atom();
            let this_upd = $upd.clone();
            let this_val = $name.clone();
            spawn(move || {
                loop {
                    let state = $do;
                    this_val.store(state as u8, Release);
                    this_upd.store(true, Release);
                    Ti::SEC.mul(1).slp();
                }
            });
        )*
    };
}

#[derive(Debug)]
#[repr(u8)]
enum State {
    Up,
    Down,
    Dormant,
    None,
    Ukn,
}

impl State {
    const ALL: [State; 5] = [Self::Up, Self::Down, Self::Dormant, Self::None, Self::Ukn];

    fn atom() -> Arc<AtomicU8> {
        Arc::from(AtomicU8::from(Self::Ukn as u8))
    }
}

impl Display for State {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        if matches!(&self, State::Up) {
            f.write_str("\x1b[32m")?;
        } else if matches!(&self, State::Ukn) {
            f.write_str("\x1b[33m")?;
        } else {
            f.write_str("\x1b[31m")?;
        }
        write!(f, "{self:?}")?;
        f.write_str("\x1b[0m")
    }
}

macro_rules! fmt {
    ($($name:ident)*) => {
        $(print!(" {}:{}", stringify!($name), State::ALL[$name.load(Acquire) as usize]);)*
    };
}

rs::cli! {
    a   => start()
    au  => { wg("up"); start(); }
    o   => stop()
    oa  => { stop(); start(); }
    oau => { stop(); wg("up"); start(); }
    dns => dns()
    chk => {
        let updated = Arc::from(AtomicBool::default());
        checker!(updated
            wlan match spawn!("sudo", "ip", "addr"; output: true).lines().find(|l|l.contains(" wlan0: ")) {
                Some(l) if l.contains(" UP ") =>  State::Up,
                Some(l) if l.contains(" DOWN ") => State::Down,
                Some(l) if l.contains(" DORMANT ") => State::Dormant,
                Some(l) => upanic!("\n\x1b[31mwlan0: invalid state: {l}\n"),
                None => State::None };
            ping chkcmd(spawn!("ping", "-c", 1, "1.1.1.1" ; out: Stdio::null(); err: Stdio::null();  ret: 1));
            dhcp chkcmd(spawn!("getent", "ahosts", "1.1.1.1"; out: Stdio::null(); ret: 1));
            lock if spawn!("lsattr", "/etc/resolv.conf"; output: 1).contains("i") { State::Up } else { State::Down };
            dns chkcmd(spawn!("getent", "hosts", "std.rs"; out: Stdio::null(); ret: 1));
            vpn chkcmd(!spawn!("wg", "show", "interfaces"; output: 1).trim().is_empty());
        );
        let mut i = 0;
        loop {
            if updated.swap(false, Acquire) {
                print!("\x1b[2K\r{i}");
                fmt!(wlan ping dhcp dns vpn lock);
                stdout().flush().unwrap();
                i += 1;
            }
            Ti::SEC.slp();
        }
    }

    en  { id: String = "all" } => wpa("enable_network", vec![id])
    dis { id: String } => wpa("disable_network", vec![id])
    sel { id: String } => wpa("select_network", vec![id])
    e   => spawn!(envg!(EDITOR), envg!(SECRET "/wpa.conf"))
    l   => wpa("list_networks", vec![])
    sc  => wpa("scan", vec![])
    scr => wpa("scan_results", vec![])
    cli => spawn!("sudo", "wpa_cli")

    k  => spawn!("sudo", "chattr", "+i", "/etc/resolv.conf")
    uk => spawn!("sudo", "chattr", "-i", "/etc/resolv.conf")
    gk => spawn!("lsattr", "/etc/resolv.conf")
    r  => spawn!("cat", "/etc/resolv.conf")

    sv { value: Option<String> } => set_vpn(value)
    cv { value: Option<String> } => { set_vpn(value); wg("down"); wg("up") }
    gv => println!("{}", get_vpn())
    lv => println!("{}", list_vpn())
    gt => spawn!("readlink", "-f", "/etc/localtime")
    eu => link("/usr/share/zoneinfo/Europe/Berlin",    "/etc/localtime")
    na => link("/usr/share/zoneinfo/Amercia/New_York", "/etc/localtime")

    u => wg("up")
    d  => wg("down")
    du => { wg("down"); wg("up"); }

    ;
    list_vpn: String => {
        let current = PathBuf::from(get_vpn()); // needed for comparing w/ // in path
        let mut v: Vec<String> = read_dir(envg!(SECRET "/vpn")).unwrap()
            .filter_map(|e| e.ok())
            .map(|e| e.path())
            .filter(|p| {
                let n = p.file_name().unwrap().to_string_lossy();
                n.starts_with("vpn-") && n.ends_with(".conf")
            })
            .map(|p| p.display().to_string()).collect();
        v.sort();
        v.into_iter().map(|p| if p == current { format!("\x1b[35m{p}\x1b[0m") } else { p })
            .collect::<Vec<_>>().join("\n")
    }
    get_vpn: String => spawn!("readlink", "-f", envg!(SECRET "/vpn.conf"); output: 1).trim().to_owned()
    set_vpn value: Option<String> => {
        let name = if let Some(inner) = value {
            envg!(SECRET "/vpn/vpn-", &inner, ".conf")
        } else if Path::new("/bin/fzf").exists() {
            let list = list_vpn();
            let out = spawn!("fzf", "--ansi"; feed: list; output: 1);
            if out.trim().is_empty() {
                exit(10);
            }
            out
        } else {
            panic!("Install fzf or specify vpn name")
        };
        link(name.trim(), &envg!(SECRET "/vpn.conf"));
    }
    wg action: &str => spawn!("sudo", "wg-quick", action, envg!(SECRET "/vpn.conf"); ignore: 1)
    stop => {
        spawn!("sudo" ,"pkill", "wpa_supplicant"; ignore: 1);
        spawn!("sudo", "pkill", "udhcpc"; ignore: 1);
        wg("down");
    }
    dns => spawn!("sudo", "udhcpc", "-i", "wlan0", "-f"; discard: 1)
    start => {
        spawn!("sudo", "rfkill", "unblock", "wlan");
        spawn!("sudo", "wpa_supplicant", "-i", "wlan0", "-B", "-c", envg!(SECRET "/wpa.conf"));
        dns()
    }
    wpa_raw cmd: &str, args: Vec<String> => spawn!("sudo", "wpa_cli", cmd; vec: args;
        map: map(|l| if l != "Selected interface 'wlan0'" && l != "OK" { Some(l) } else { None }))
    wpa cmd: &str, args: Vec<String> => { wpa_raw(cmd, args); wpa_raw("save_config", vec![]); }
    chkcmd cmd: bool: State => if cmd { State::Up } else { State::Down }
}

// To get the password for the private key, your can use this program from
// within the /tmp/ SecureNow folder:
//
// >>> from keystore import Certificate
// ... from secrets import generate_secret
// ... c = Certificate.load(file.crt)
// ... print(generate_secret(c, salt=bytes()).decode())
