use std::fs::read_dir;
use std::io::Write as _;
use std::path::Path;
use std::process::Command;

rs::cli! {
    a   => start(),
    au  => { wg("up"); start(); },
    o   => stop(),
    oa  => { stop(); start(); },
    oau => { stop(); wg("up"); start(); },

    en { id_or_all: Option<String> } => wpa("enable_network", vec![id_or_all.unwrap_or("all".into())]),
    dis { id: String } => wpa("disable_network", vec![id]),
    e   => spawn!("sudo", envg!(EDITOR), envg!(SECRET "/wpa.confg")),
    l   => wpa("list_networks", vec![]),
    sc  => wpa("scan", vec![]),
    scr => wpa("scan_results", vec![]),

    k  => spawn!("sudo", "chattr", "+i", "/etc/resolv.conf"),
    uk => spawn!("sudo", "chattr", "-i", "/etc/resolv.conf"),
    gk => spawn!("lsattr", "/etc/resolv.conf"),
    r  => spawn!("cat", "/etc/resolv.conf"),

    sv { value: Option<String> } => {
        let name = if let Some(inner) = value {
            envg!(SECRET "/vpn-", &inner, ".conf")
        } else if Path::new("/bin/fzf").exists() {
            let mut child = Command::new("fzf").arg("--ansi").stdin(Stdio::piped()).stdout(Stdio::piped()).spawn().unwrap();
            let mut stdin = child.stdin.take().unwrap();
            stdin.write_all(list_vpn().as_bytes()).unwrap();
            drop(stdin);
            let out = String::from_utf8_lossy_owned(child.wait_with_output().unwrap().stdout);
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
    get_vpn: String => Command::new("readlink").arg("-f").arg(envg!(SECRET "/vpn.conf")).output()
            .map(|c| String::from_utf8_lossy(c.stdout.trim_ascii()).to_string())
            .unwrap_or("No active vpn config".into())
    wg action: &str => spawn!("sudo", "wg-quick", action, envg!(SECRET "/vpn.conf"); ignore: 1)
    link from: &str, to: &str => spawn!("sudo", "ln", "-sf", from, to)
    stop => {
        spawn!("sudo" ,"pkill", "wpa_supplicant"; ignore: 1);
        spawn!("sudo", "pkill", "udhcpc"; ignore: 1);
        wg("down");
    }
    start => {
        spawn!("sudo", "rfkill", "unblock", "wlan");
        spawn!("sudo", "wpa_supplicant", "-i", "wlan0", "-B", "-c", envg!(SECRET "/wpa.conf"));
        spawn!("sudo", "udhcpc", "-i", "wlan0", "-x", "hostname:GreyBob", "-f")
    }
    wpa cmd: &str, args: Vec<String> => spawn!("sudo", cmd, "wpa_cli"; vec: args)
}
