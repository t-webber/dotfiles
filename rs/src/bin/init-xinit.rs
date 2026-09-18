use rs::{envg, rsbin, spawn};

fn main() {
    spawn!("bspwm", "-c", rsbin!("init-bspwm"));
}
