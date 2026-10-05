use core::time::Duration;
use std::fs;
use std::thread::sleep;

rs::cli! {
    => if args().count() == 1 { let (col, em) = dec(); println!("{col}{em} {}\x1b[0m", lvl()); exit(0); },
    notif => notif(true),
    check => notif(false),
    activeloop => loop {notif(true); sleep(Duration::from_secs(10));  },
    demon => spawn!("setsid", "-f", args().next().unwrap(), "activeloop"),
    :
    lvl: u32 => fs::read_to_string("/sys/class/power_supply/BAT1/capacity").unwrap().trim().parse().unwrap()
    stat: String => fs::read_to_string("/sys/class/power_supply/BAT1/status").unwrap()
    dec: (&'static str, &'static str) => match stat().trim() {
        "Charging" => ("\x1b[32m", "🔋"),
        "Discharging" => ("\x1b[31m", "🪫"),
        "Full" => ("\x1b[35m", "💥"),
        _ => ("\x1b[36m","❓")
    }
    notif ok: bool => {
        if stat().trim() == "Discharging" && lvl() <= 10 {
            spawn!("sudo", "zzz");
        } else if stat().trim() == "Discharging" && lvl() <= 20 {
            spawn!("herbe", "br:#ff0000", "Battery is low");
        } else if ok {
            spawn!("herbe", "em:", format!("{}{}", dec().1, lvl()))}
        }
}
