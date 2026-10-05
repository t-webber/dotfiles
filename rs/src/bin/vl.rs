use std::fs::OpenOptions;
use std::process::{Command, Stdio};

const STEP: u32 = 13;
const SINK: &str = "@DEFAULT_SINK@";

rs::cli! {
    => if args().count() == 1 { println!("{}", get(true).unwrap_or("Pipewire not running...".into())); exit(0); },
    notif => if let Some(inner) = get(false) { spawn!("herbe", "em:", inner) } else { spawn!("herbe", "Pipewire not running..."); },
    a => {
        unsafe { std::env::set_var("DBUS_SESSION_BUS_ADDRESS", "unix:path=/dev/null") }
        unsafe { std::env::set_var("DISABLE_RTKIT", "true") }
        let logs_file = OpenOptions::new().create(true).truncate(true).write(true).open(envg!(LOGS "/vl")).unwrap();
        let log = || logs_file.try_clone().unwrap();
        for cmd in ["pipewire", "pipewire-pulse", "wireplumber"] {
            spawn!("setsid", "-f", cmd; stdin: Stdio::null(); out: log(); err: log());
        }
        brave_audio();
    },
    o => { spawn!("pkill", "pipewire"); spawn!("pkill", "pipewire-pulse"); spawn!("pkill", "wireplumber"); brave_audio(); },
    l => spawn!("pactl", "list"),
    mute => pactl("set-sink-mute", "toggle"),
    up { step: Option<u32> } => pactl("set-sink-volume", &format!("+{}%", step.unwrap_or(STEP))),
    down { step: Option<u32> } => pactl("set-sink-volume", &format!("-{}%", step.unwrap_or(STEP))),
    :
    brave_audio => spawn!("pkill", "-f", "brave.*--type=utility.*--utility-sub-type=audio.mojom.AudioService"; ignore: true)
    pactl cmd: &str, action: &str => spawn!("pactl", cmd, SINK, action; err: Stdio::null(); ignore: true)
    read cmd: &str : String => String::from_utf8_lossy_owned(Command::new("pactl").args([cmd, SINK]).stderr(Stdio::null()).output().unwrap().stdout)
    get term: bool: Option<String> => {
        let mute = read("get-sink-mute");
        if mute.is_empty() {
            None
        } else {
            let (col, em) = if mute.trim().ends_with("yes") {  ("\x1b[31m", "🔇") } else {  ("\x1b[32m", "🔊") };
            let level = read("get-sink-volume").split_whitespace().nth(4).unwrap().to_owned();
            Some(format!("{}{em} {level}", if term { col } else { "" }))
        }
    }
}
