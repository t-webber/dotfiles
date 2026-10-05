use std::fs::OpenOptions;
use std::process::Command;

const STEP: u32 = 13;
const SINK: &str = "@DEFAULT_SINK@";

rs::cli! {
    => if args().count() == 1 { println!("{}", get(true).unwrap_or("Pipewire not running...".into())); exit(0); },
    notif => if let Some(inner) = get(false) { spawn!("herbe", "em:", inner) } else { spawn!("herbe", "Pipewire not running..."); },
    a => {
        setenv!(DBUS_SESSION_BUS_ADDRESS "unix:path=/dev/null");
        setenv!(DISABLE_RTKIT "true");
        let logs_file = OpenOptions::new().create(true).truncate(true).write(true).open(envg!(LOGS "/vl")).unwrap();
        let log = || logs_file.try_clone().unwrap();
        for cmd in ["pipewire", "pipewire-pulse", "wireplumber"] {
            spawn!("setsid", "-f", cmd; stdin: Stdio::null(); out: log(); err: log());
        }
        brave_audio();
    },
    o => { spawn!("pkill", "pipewire"); spawn!("pkill", "pipewire-pulse"); spawn!("pkill", "wireplumber"); brave_audio(); },
    "list" l => spawn!("pactl", "list"),
    mute => pactl("set-sink-mute", "toggle"),
    up { step: Option<u32> } => pactl("set-sink-volume", &format!("+{}%", step.unwrap_or(STEP))),
    down { step: Option<u32> } => pactl("set-sink-volume", &format!("-{}%", step.unwrap_or(STEP))),
    "status" s => if read("get-sink-mute").is_empty() { println!("Off") } else { println!("On") },
    test => spawn!("pw-play", "/usr/share/sounds/alsa/Front_Center.wav"),
    debug => {
        setenv!(DEBUG "1");
        spawn!("id"; ignore: 1);
        spawn!("pacman", "-Q", "sof-firmware", "alsa-utils", "alsa-ucm-conf"; ignore: 1);
        spawn!("aplay", "-l"; ignore: 1);
        spawn!("aplay", "-L"; ignore: 1);
        spawn!("cat", "/proc/asound/cards"; ignore: 1);
        spawn!("wpctl", "status"; ignore: 1);
    },
    :
    brave_audio => spawn!("pkill", "-f", "brave.*--type=utility.*--utility-sub-type=audio.mojom.AudioService"; ignore: 1)
    pactl cmd: &str, action: &str => spawn!("pactl", cmd, SINK, action; err: Stdio::null(); ignore: 1)
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
