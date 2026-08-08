use std::fs::File;
use std::process::Stdio;
use std::thread::spawn;

use rs::{envg, spawn};

macro_rules! bspc {
    ($($a:ident $b:expr;)*) => {
        $(spawn!("bspc", "config", stringify!($a), $b);)*
    };
}

fn main() {
    spawn(|| {
        // xinput
        spawn!("xinput", "set-prop", "libinput Tapping Enabled", 1);
        spawn!("xinput", "set-prop", "libinput Click Method Enabled", 1, 0);
        spawn!("xinput", "set-prop", "libinput Natural Scrolling Enabled", 1);
        // bspc
        spawn!("bspc", "monitor", "-d", "term", "edit", "browser", "com", "agents");
        bspc!(
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
        );
        // apps
        spawn!("alacritty"; both: File::create(envg!(LOGS "alacritty")).unwrap(); nowait: 1);
        spawn!("brave", "--no-startup-window"; both: File::create(envg!(LOGS "brave")).unwrap(); nowait: 1);
        spawn!("vl", "start"; both: File::create(envg!(LOGS "vl")).unwrap());
        spawn!("pw", "daemon");
    });
    spawn!("setxkbmap", "fr");
    spawn!("slock");
    spawn!("sxhkd"; nowait: 1);
}
