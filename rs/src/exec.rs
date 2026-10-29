#[macro_export]
macro_rules! spawn {

    (res $retdisp:ident; ret: $ret:expr) => {{
        if let Some((ret, _)) = $retdisp {
            ret.success()
        } else {
            panic!("DEBUG=2 can't be used when expecting a success value")
        }
    }};

    (res $retdisp:ident $(; ignore: $ignore:expr)?) => {{
        if let Some((ret, disp)) = $retdisp {
            let mut ignore = false;
            $( ignore = true; let _ = $ignore;)?
            if !ret.success() && !ignore {
                eprintln!("{}[{}] {}: {:?}\x1b[0m", $crate::ORANGE, std::env::args().next().unwrap(), disp, ret.code());
                std::process::exit(3);
            }
        }
    }};

    (finish $cmdisp:ident $(; ignore: $ignore:expr)? $(; ret: $ret:expr)?) => {{
        let retdisp = $cmdisp.map(|(mut child, disp)| {
            (child.wait().unwrap(), disp)
        });
        $crate::spawn!(res retdisp $(; ignore: $ignore)? $(; ret: $ret)?)
    }};


    (finish $cmdisp:ident output: $output:expr) => {{
        if let Some((mut child, _)) = $cmdisp {
            String::from_utf8_lossy_owned(child.wait_with_output().unwrap().stdout)
        } else {
            panic!("DEBUG=2 can't be used when expecting a string")
        }
    }};

    (finish $cmdisp:ident discard: $discard:expr) => {{}};

    (finish $cmdisp:ident nowait: $nowait:expr) => {{
        if let Some((mut child, _)) = $cmdisp {
            child
        } else {
            panic!("DEBUG=2 can't be used when expecting a child")
        }
    }};


    (
        $prog:expr $(, $arg:expr)*
        $(; vec: $vec:expr)?
        // in/ou
        $(; stdin: $in:expr)? $(; out: $out:expr)? $(; err: $err:expr)?
        $(; feed: $feed:expr)? $(; both: $both:expr)?
        $(; map: $map:expr)?
        $(; output: $output:literal)?
        // ret
        $(; ignore: $ignore:literal)?
        $(; discard: $discard:literal)?
        $(; ret: $ret:literal)?
        $(; nowait: $nowait:literal)?
    ) => {{ #[allow(unused_mut)] {
        use std::process as P;
        let dbg: u32 = std::env::var("DEBUG").unwrap_or("0".into()).parse().unwrap();
        let prog = $prog;
        let mut args: Vec<String> = vec![];
        $( args.push($arg.to_string()); )*
        $( args.extend($vec); )?
        $crate::debug!("\x1b[35m+++ {prog} {}\x1b[0m", args.join(" "));
        let cmdisp = if dbg != 2 {
            let mut cmd = P::Command::new(&prog);
            cmd.args(&args);
            $(cmd.stdin($in);)?
            $(cmd.stdout(P::Stdio::from($out));)?
            $(cmd.stderr(P::Stdio::from($err));)?
            $(
                let f = $both.try_clone().unwrap();
                cmd.stdout(P::Stdio::from(f.try_clone().unwrap()));
                cmd.stderr(P::Stdio::from(f.try_clone().unwrap()));
            )?
            $(cmd.stdout(P::Stdio::piped()); let _ = &$map;  )?
            $(cmd.stdout(P::Stdio::piped()); let _ = $output;)?
            $(cmd.stdin (P::Stdio::piped()); let _ = &$feed; )?
            let mut child = cmd.spawn().unwrap();
            $(
                let mut stdin = child.stdin.take().unwrap();
                stdin.write_all($feed.as_bytes()).unwrap();
                drop(stdin);
            )?
            $( {
                use std::io::{BufRead, BufReader, Write};
                let f = $map;
                let reader = BufReader::new(child.stdout.take().unwrap());
                for line in reader.lines() {
                    let line = line.unwrap();
                    if let Some(modified) = f(&line) {
                        println!("{modified}")
                    }
                }
            } )?
            Some((child, format!("{prog} {}", args.join(" "))))
        } else {
            None
        };
        $crate::spawn!(finish cmdisp $(discard: $discard)? $(nowait: $nowait)? $(output: $output)? $(; ignore: $ignore)? $(; ret: $ret)?)
    }}};
}
