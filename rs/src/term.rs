use crate::spawn;

const ISSUE: &str = r#"
oooooooooo.             .o8         .oooooo.    .oooooo..o
`888'   `Y8b           "888        d8P'  `Y8b  d8P'    `Y8
 888     888  .ooooo.   888oooo.  888      888 Y88bo.     
 888oooo888' d88' `88b  d88' `88b 888      888  `"Y8888o. 
 888    `88b 888   888  888   888 888      888      `"Y88b
 888    .88P 888   888  888   888 `88b    d88' oo     .d8P
o888bood8P'  `Y8bod8P'  `Y8bod8P'  `Y8bood8P'  8""88888P' 
"#;

pub fn issue() -> String {
    let issue = ISSUE.trim_start();
    let issue_len = issue.lines().next().unwrap().len();
    let empty_lines = format!("{}\n", " ".repeat(issue_len)).repeat(5);
    let issue = format!("{empty_lines}{issue}{empty_lines}");

    let cols = spawn!("stty", "size"; output: 1)
        .split_whitespace()
        .last()
        .unwrap()
        .parse::<usize>()
        .unwrap();

    let hpad_len = (cols - issue_len - 2) / 4;
    let hpad = " ".repeat(hpad_len);
    let issue = issue
        .lines()
        .map(|line| format!("{hpad}│{hpad}{line}{hpad}│"))
        .collect::<Vec<_>>()
        .join("\n");

    let hline = "─".repeat(hpad_len * 2 + issue_len);
    let first = format!("{hpad}┌{hline}┐");
    let last = format!("{hpad}└{hline}┘");
    format!("\n\x1b[33m{first}\n{issue}\n{last}\n\n")
}

pub const ORANGE: &str = "\x1b[38;2;255;153;85m";

pub fn setfont() {
    spawn!("setfont", "/usr/share/kbd/consolefonts/sun12x22.psfu.gz");
}
