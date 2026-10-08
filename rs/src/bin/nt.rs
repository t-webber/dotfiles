use std::fs::read_dir;
use std::io::{Write as _, stdout};
use std::path::Path;

rs::cli! {
    a   => start(),
    au  => { wg("up"); start(); },
    o   => stop(),
    oa  => { stop(); start(); },
    oau => { stop(); wg("up"); start(); },
    dns => dns(),
    chk => {
        for i in 0.. {
            match spawn!("sudo", "ip", "addr"; output: true).lines().find(|l|l.contains(" wlan0: ")) {
                Some(l) if l.contains(" UP ") => {println!("\rwlan0 is up"); break},
                Some(l) if l.contains(" DOWN ") => print!("\r{i} wlan0 is down"),
                Some(l) if l.contains(" DORMANT ") => print!("\r{i} wlan0 is dormant"),
                Some(l) => panic!("{i} wlan is in invalid state: {l}"),
                None => print!("\r{i} wlan0 not found"),
            }
            stdout().flush().unwrap();
            slp(1000);
        }
        for i in 0.. {
            if spawn!("getent", "ahosts", "1.1.1.1"; out: Stdio::null(); ret: 1) {
                 println!("\rDHCP is up");
                 break;
             }
            print!("\r{i} DHCP is down");
            stdout().flush().unwrap();
            slp(1000);
        }
        for i in 0.. {
            if spawn!("getent", "hosts", "std.rs"; out: Stdio::null(); ret: 1) {
                 println!("\rDNS is up");
                 break;
            }
            print!("\r{i} DNS is down");
            stdout().flush().unwrap();
            slp(1000);
        }
        if spawn!("wt", "show", "interfaces"; output: 1).trim().is_empty() {
            println!("VPN is down");
        } else {
            println!("VPN is up");
        }
    },

    en { id_or_all: Option<String> } => wpa("enable_network", vec![id_or_all.unwrap_or("all".into())]),
    dis { id: String } => wpa("disable_network", vec![id]),
    sel { id: String } => wpa("select_network", vec![id]),
    e   => spawn!(envg!(EDITOR), envg!(SECRET "/wpa.conf")),
    l   => wpa("list_networks", vec![]),
    sc  => wpa("scan", vec![]),
    scr => wpa("scan_results", vec![]),
    cli => spawn!("sudo", "wpa_cli"),

    k  => spawn!("sudo", "chattr", "+i", "/etc/resolv.conf"),
    uk => spawn!("sudo", "chattr", "-i", "/etc/resolv.conf"),
    gk => spawn!("lsattr", "/etc/resolv.conf"),
    r  => spawn!("cat", "/etc/resolv.conf"),

    sv { value: Option<String> } => {
        let name = if let Some(inner) = value {
            envg!(SECRET "/vpn-", &inner, ".conf")
        } else if Path::new("/bin/fzf").exists() {
            let list = list_vpn();
            let out = spawn!("fzf", "--ansi"; feed: list; output: 1);
            if out.trim().is_empty() {
                exit(1);
            }
            out
        } else {
            panic!("Install fzf or specify vpn name")
        };
        link(name.trim(), &envg!(SECRET "/vpn.conf"));
    },
    gv => println!("{}", get_vpn()),
    lv => println!("{}", list_vpn()),
    gt => spawn!("readlink", "-f", "/etc/localtime"),
    eu => link("/usr/share/zoneinfo/Europe/Berlin",    "/etc/localtime"),
    na => link("/usr/share/zoneinfo/Amercia/New_York", "/etc/localtime"),

    u { value: Option<String> } => {
        if let Some(inner) = value {
            link(&envg!(SECRET "/vpn-", &inner, ".conf"), &envg!(SECRET "/vpn.conf"));
        }
        wg("up");
    },
    d  => wg("down"),
    du => { wg("down"); wg("up"); },

    :
    list_vpn: String => {
        let current = get_vpn();
        let mut v: Vec<String> = read_dir(envg!(SECRET)).unwrap()
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
    wg action: &str => spawn!("sudo", "wg-quick", action, envg!(SECRET "/vpn.conf"); ignore: 1)
    link from: &str, to: &str => spawn!("sudo", "ln", "-sf", from, to)
    stop => {
        spawn!("sudo" ,"pkill", "wpa_supplicant"; ignore: 1);
        spawn!("sudo", "pkill", "udhcpc"; ignore: 1);
        wg("down");
    }
    dns => spawn!("sudo", "udhcpc", "-i", "wlan0", "-x", "hostname:GreyBob", "-f")
    start => {
        spawn!("sudo", "rfkill", "unblock", "wlan");
        spawn!("sudo", "wpa_supplicant", "-i", "wlan0", "-B", "-c", envg!(SECRET "/wpa.conf"));
        dns()
    }
    wpa cmd: &str, args: Vec<String> => {
        spawn!("sudo", "wpa_cli", cmd; vec: args; filter: |l: &String| l != "Selected interface 'wlan0'");
        spawn!("sudo", "wpa_cli", "save_config"; filter: |l: &String| l != "Selected interface 'wlan0'");
    }
}

// To get the password for the private key, your can use this program from
// within the /tmp/ SecureNow folder:
//
// >>> from keystore import Certificate
// ... from secrets import generate_secret
// ... c = Certificate.load(file.crt)
// ... print(generate_secret(c, salt=bytes()).decode())
//
