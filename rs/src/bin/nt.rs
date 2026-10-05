rs::cli! {
    a   => start(),
    au  => { wg("up"); start(); },
    o   => stop(),
    oa  => { stop(); start(); },
    oau => { stop(); wg("up"); start(); },

    en { id_or_all: Option<String> } => sspawn!("enable_network", id_or_all.unwrap_or("all".into())),
    di { id: String } => sspawn!("disable_network", id),
    e   => sspawn!(envg!(EDITOR), envg!(SECRET "/wpa.confg")),
    l   => sspawn!("wpa_cli", "list_networks"),
    sc  => sspawn!("wpa_cli", "scan"),
    scr => sspawn!("wpa_cli", "scan_results"),

    k  => sspawn!("chattr", "+i", "/etc/resolv.conf"),
    uk => sspawn!("chattr", "-i", "/etc/resolv.conf"),
    gk => spawn!("lsattr", "/etc/resolv.conf"),
    r  => spawn!("cat", "/etc/resolv.conf"),

    sv { value: String } => link(&envg!(SECRET "/vpn-", &value, ".conf"), &envg!(SECRET "/vpn.conf")),
    gv => spawn!("readlink", "-f", concat!(env!("SECRET"), "/vpn.conf") ),
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
    wg action: &str => sspawn!("wg-quick", action, envg!(SECRET "/vpn.conf"))
    link from: &str, to: &str => sspawn!("ln", "-sf", from, to)
    stop => { sspawn!("pkill", "wpa_supplicant"); sspawn!("pkill", "udhcpc"); wg("down"); }
    start => {
        sspawn!("rfkill", "unblock", "wlan");
        sspawn!("wpa_supplicant", "-i", "wlan0", "-B", "-c", envg!(SECRET "/wpa.conf"));
        sspawn!("udhcpc", "-i", "wlan0", "-x", "hostname:GreyBob", "-f")
    }
}
