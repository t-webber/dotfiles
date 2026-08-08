rs::cli! {
    => if args().count() == 1 { println!("\x1b[33m☀️ {}\x1b[0m", get()); exit(0); };
    notif => spawn!("herbe", "em:", format!("☀️{}", get()))
    full => set("400")
    set { amount: u32 } => set(&amount.to_string())
    up { step: u32 = 9 } => set(&format!("{step}+"))
    down { step: u32 = 9 } => set(&format!("{step}-"))
    ;
    get: String => spawn!("sudo", "brightnessctl", "g"; output: true).trim().to_string()
    set val: &str => spawn!("sudo", "brightnessctl", "s", val; out: Stdio::null())
}
