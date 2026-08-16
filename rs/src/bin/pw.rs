use std::fs;

#[derive(PartialEq)]
enum Show {
    Low,
    Time,
    Value,
}

rs::cli! {
    => if args().count() == 1 { println!("{}{} {}\x1b[0m", dec().0, dec().1, lvl()); exit(0); };
    notif => notif(Show::Time)
    check => notif(Show::Low)
    activeloop => loop {
        if stat() == "Discharging" && lvl() <= 10 {
            spawn!("sudo", "zzz");
        } else if spawn!("date", "+%H"; output: 1).trim().parse::<u32>().unwrap() >= 21 {
            spawn!("herbe", "br:#ff0000", "Time is up, shutting down in 2 minutes");
            Ti::MIN.slp();
            spawn!("herbe", "br:#ff0000", "Time is up, shutting down in 1 minutes");
            Ti::MIN.slp();
            spawn!("sudo", "kill", "-USR1", "1");
        }
        notif(Show::Value);
        Ti::MIN.mul(5).slp();
    }
    daemon => spawn!("setsid", "-f", args().next().unwrap(), "activeloop")
    ;
    lvl: u32 => fs::read_to_string("/sys/class/power_supply/BAT1/capacity")
                    .unwrap().trim().parse().unwrap()
    stat: String => fs::read_to_string("/sys/class/power_supply/BAT1/status").unwrap()
    dec: (&'static str, &'static str) => match stat().trim() {
        "Discharging" => ("\x1b[31m", "🪫"),
        "Charging" => ("\x1b[32m", "🔋"),
        "Full" => ("\x1b[35m", "💥"),
        _ => ("\x1b[36m","❓")
    }
    notif show: Show => if stat().trim() == "Discharging" && lvl() <= 10 {
            spawn!("sudo", "zzz");
        } else if stat().trim() == "Discharging" && lvl() <= 20 {
            spawn!("herbe", "br:#ff0000", format!("Battery low: {}%", lvl()));
        } else if show == Show::Time {
            spawn!("herbe", format!("{}% ({})", lvl(), spawn!("date", "+%H:%M:%S"; output: 1).trim()))
        } else if show == Show::Value {
            spawn!("herbe", "em:", format!("{}{}", dec().1, lvl()))
        }
}
