rs::cli! {
    => if args().count() == 1 { make("release"); exit(0); },
    cr => { make("clean"); make("release"); },
    cd => { make("clean"); make("debug"); },
    d => { make("debug"); },
    :
    make cmd: &str => {
        spawn!("make", "-C", envg!(CMD "/src/"), "--no-print-directory", cmd,  "-j", 50;
            filter: |l: &String| !l.ends_with("is up to date.") && !l.ends_with(" && true"));
        let rsflag = if cmd == "release" { vec!["--release".into()] } else { vec![] };
        spawn!("cargo", "-Z", "unstable-options", "-C", envg!(DOT "/rs"), "build"; vec: rsflag);
    }
}
