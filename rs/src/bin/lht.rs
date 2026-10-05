use std::process::{Command, Stdio};

const STEP: u32 = 9;

rs::cli! {
    => if args().count() == 1 { println!("\x1b[33m☀️ {}\x1b[0m", get().trim()); exit(0); },
    notif => spawn!("herbe", "em:", format!("☀️{}", get().trim())),
    set { amount: u32 } => set(amount.to_string()),
    up { step: Option<u32> } => set(format!("{}+", step.unwrap_or(STEP))),
    down { step: Option<u32> } => set(format!("{}-", step.unwrap_or(STEP))),
    :
    get: String => String::from_utf8_lossy_owned(Command::new("sudo").args(["brightnessctl", "g"]).output().unwrap().stdout)
    set val: String => spawn!("sudo", "brightnessctl", "s", val; out: Stdio::null())
}
