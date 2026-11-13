use crate::spawn;

const ISSUE: &str = include_str!(concat!(env!("OCFG"), "/issue"));

pub fn issue() -> String {
    let issue = ISSUE.trim_start();
    let issue_len = issue.lines().map(|l| l.len()).max().unwrap();
    let empty_lines = "\n".repeat(5);
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
        .map(|line| format!("{hpad}│{hpad}{line}{}{hpad}│", " ".repeat(issue_len - line.len())))
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
