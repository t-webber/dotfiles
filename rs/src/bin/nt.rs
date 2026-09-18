use core::fmt::Display;
use core::sync::atomic::AtomicU8;
use core::sync::atomic::Ordering::{Acquire, Release};
use std::fs::read_dir;
use std::io::{Write as _, stdout};
use std::path::Path;
use std::sync::Arc;
use std::sync::atomic::AtomicBool;
use std::thread::spawn;

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
    ($name:ident) => {
        print!(" {}:{}", stringify!($name), State::ALL[$name.load(Acquire) as usize])
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
        let wlan = State::atom();
        let w_updated = updated.clone();
        let w_wlan = wlan.clone();
        spawn(move || loop {
            let state = match spawn!("sudo", "ip", "addr"; output: true)
                                .lines()
                                .find(|l|l.contains(" wlan0: ")) {
                Some(l) if l.contains(" UP ") =>  State::Up,
                Some(l) if l.contains(" DOWN ") => State::Down,
                Some(l) if l.contains(" DORMANT ") => State::Dormant,
                Some(l) => upanic!("\n\x1b[31mwlan0: invalid state: {l}\n"),
                None => State::None
            };
            w_wlan.store(state as u8, Release);
            w_updated.store(true, Release);
            slp(2000);
        });
        let ping = State::atom();
        let p_updated = updated.clone();
        let p_ping = ping.clone();
        spawn(move || loop {
            let state = chkcmd(spawn!("ping", "-c", 1, "1.1.1.1"
                    ; out: Stdio::null(); err: Stdio::null();  ret: 1));
            p_ping.store(state as u8, Release);
            p_updated.store(true, Release);
            slp(2000);
        });
        let dhcp = State::atom();
        let d_updated = updated.clone();
        let d_dhcp = dhcp.clone();
        spawn(move || loop {
            let state = chkcmd(spawn!("getent", "ahosts", "1.1.1.1"; out: Stdio::null(); ret: 1));
            d_dhcp.store(state as u8, Release);
            d_updated.store(true, Release);
            slp(2000);
        });
        let lock = State::atom();
        let l_updated = updated.clone();
        let l_lock = lock.clone();
        spawn(move || loop {
            let lsattr = spawn!("lsattr", "/etc/resolv.conf"; output: 1);
            let state = if lsattr.contains("i") { State::Up } else { State::Down };
            l_lock.store(state as u8, Release);
            l_updated.store(true, Release);
            slp(2000);
        });
        let dns = State::atom();
        let n_updated = updated.clone();
        let n_dns = dns.clone();
        spawn(move || loop {
            let state = chkcmd(spawn!("getent", "hosts", "std.rs"; out: Stdio::null(); ret: 1));
            n_dns.store(state as u8, Release);
            n_updated.store(true, Release);
            slp(2000);
        });
        let vpn = State::atom();
        let v_updated = updated.clone();
        let v_vpn = vpn.clone();
        spawn(move || loop {
            let state = chkcmd(!spawn!("wg", "show", "interfaces"; output: 1).trim().is_empty());
            v_vpn.store(state as u8, Release);
            v_updated.store(true, Release);
            slp(2000);
        });
        let mut i = 0;
        loop {
            if updated.swap(false, Acquire) {
                print!("\x1b[2K\r{i}");
                fmt!(wlan);
                fmt!(ping);
                fmt!(dhcp);
                fmt!(dns);
                fmt!(vpn);
                fmt!(lock);
                stdout().flush().unwrap();
                i += 1;
            }
            slp(1000);
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

    u { value: Option<String> } => {
        if let Some(inner) = value {
            link(&envg!(SECRET "/vpn-", &inner, ".conf"), &envg!(SECRET "/vpn.conf"));
        }
        wg("up");
    }
    d  => wg("down")
    du => { wg("down"); wg("up"); }

    ;
    list_vpn: String => {
        let current = get_vpn();
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
