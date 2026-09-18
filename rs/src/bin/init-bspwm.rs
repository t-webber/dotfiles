use std::fs::File;
use std::thread::spawn;

use rs::{envg, slp, spawn};

macro_rules! conf {
    ($($a:ident $b:expr;)*) => {
        $(spawn!("bspc", "config", stringify!($a), $b; ret: 1))&&*
    };
}

macro_rules! rule {
    ($($a:literal $($b:literal)*;)*) => {
        $(spawn!("bspc", "rule", "-a", $a $(,$b)*; ret: 1))&&*
    };
}

fn main() {
    spawn(|| {
        if !(spawn!("vl", "a"; both: File::create(envg!(LOGS "vl")).unwrap(); ret: 1) && spawn!("pw", "daemon"; ret: 1)
            // xinput
            && spawn!("xinput", "set-prop", 8, "libinput Tapping Enabled", 1; ret: 1)
            && spawn!("xinput", "set-prop", 8, "libinput Click Method Enabled", 1, 0; ret: 1)
            // bspc
            && spawn!("bspc", "monitor", "-d", "term", "edit", "browser", "com", "agents"; ret: 1)
            && conf!(
                border_width 1;
                window_gap 0;
                split_ratio 0.5;
                borderless_monocle true;
                gapless_monocle true;
                normal_border_color "#000000";
                focused_border_color "#440000";
                presel_feedback_color "#000000";
                top_padding 0;
                top_monocle_padding 0;
            )
            && rule!(
                "Beeper" "desktop=com";
                "discord" "desktop=com";
                "Remote-viewer" "desktop=com" "state=fullscreen";
                "Qemu-system-x86_64" "desktop=agents" "state=fullscreen";
            ))
        {
            eprintln!("\x1b[31m>>>One or more bspwm setting failed.\x1b[0m");
        }
        // apps
        spawn!("alacritty"; both: File::create(envg!(LOGS "alacritty")).unwrap(); discard: 1);
        spawn!("brave", "--no-startup-window"; both: File::create(envg!(LOGS "brave")).unwrap(); discard: 1);
        slp(10000);
        spawn!("pkill", "chrome_crashpad");
    });
    spawn!("setxkbmap", "fr");
    spawn!("slock");
    spawn!("sxhkd"; discard: 1);
}
