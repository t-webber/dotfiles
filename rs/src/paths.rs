use std::env;
use std::fs::read_dir;
use std::path::PathBuf;

use rayon::iter::{ParallelBridge, ParallelIterator as _};

use crate::{CollectVec, debug, envg, upanic};

fn char_to_dir(ch: char) -> Option<String> {
    Some(match ch {
        'a' => envg!(APPS),
        'b' => envg!(BLOB),
        'c' => envg!(CMD_SRC),
        'd' => envg!(DEV),
        'e' => "/etc".to_owned(),
        'f' => envg!(FILES),
        'g' => envg!(OCFG),
        'h' => envg!(HOME),
        'i' => "/boot/efi".to_owned(),

        'k' => envg!(DOT),
        'l' => envg!(LOGS),
        'm' => envg!(CMD),
        'o' => envg!(WORK),
        'p' => "/tmp".to_owned(),

        's' => envg!(SECRET),
        't' => envg!(DATA),

        'v' => "/var".to_owned(),
        'w' => envg!(WASTE),
        'x' => envg!(XDG_CONFIG_HOME),
        'y' => envg!(STUDY),

        '.' => "main".to_owned(),
        ':' => "master".to_owned(),
        '%' => "dev".to_owned(),
        _ => return None,
    })
}

fn match_segments<F>(dir: &str, func: F) -> Vec<String>
where
    F: Fn(String, bool) -> Vec<String>,
    F: Sync + Send,
{
    if !PathBuf::from(dir).is_dir()
        || dir.ends_with("/.git")
        || dir.ends_with("/target")
        || dir.ends_with("/node_modules")
    {
        return vec![];
    }
    match read_dir(dir) {
        Ok(files) => files
            .par_bridge()
            .map(Result::unwrap)
            .flat_map_iter(|f| {
                func(f.file_name().to_string_lossy().to_string(), f.file_type().unwrap().is_dir())
            })
            .collect(),
        Err(err) => {
            eprintln!("Failed to read {dir}: {err}");
            vec![]
        }
    }
}

fn match_exact_segments(dir: &str, name: &str) -> Vec<String> {
    let res = match_segments(dir, |segment, isdir| {
        debug!("   - :{segment}:");
        if name == segment {
            vec![format!("{dir}/{segment}")]
        } else if name.starts_with(&segment) && isdir {
            match_exact_segments(&format!("{dir}/{segment}"), &name[segment.len()..])
        } else {
            vec![]
        }
    });
    debug!("+ 1. exact segments :{name}: {dir} => {res:?}");
    res
}

fn match_partial_segments(dir: &str, name: &str) -> Vec<String> {
    let res = match_segments(dir, |segment, isdir| {
        debug!("   - :{segment}:");
        if name == segment {
            return vec![format!("{dir}/{segment}")];
        };
        if name.is_empty() {
            return vec![dir.to_owned()];
        }
        if !isdir {
            return vec![];
        }
        let dir = format!("{dir}/{segment}");
        let mut v = vec![];
        for (idx, ch) in name.char_indices() {
            if segment.chars().nth(idx) != Some(ch) {
                break;
            }
            v.extend(match_partial_segments(&dir, &name[idx + 1..]));
        }
        v
    });
    debug!(
        "+ 2. partial segments :{name}: {dir} => {:?}",
        res.iter().map(|s| s.split("/").last().unwrap().to_string()).colvec()
    );
    res
}

fn match_no_segments(dir: &str, name: &str) -> Vec<String> {
    let res = match_segments(dir, |segment, isdir| {
        debug!("   - :{segment}:");
        if name == segment {
            return vec![name.to_owned()];
        };
        if !isdir {
            return vec![];
        }
        let dir = format!("{dir}/{segment}");
        let mut v = vec![];
        for (idx, ch) in name.char_indices() {
            v.extend(match_no_segments(&dir, &name[idx..]));
            if segment.chars().nth(idx) != Some(ch) {
                break;
            }
        }
        v
    });
    debug!(
        "+ 3. no segments :{name}: {dir} => {:?}",
        res.iter().map(|s| s.split("/").last().unwrap().to_string()).colvec()
    );
    res
}

#[derive(Debug)]
pub enum FileKind {
    All,
    Folder,
    File,
}

impl FileKind {
    fn is_valid(&self, item: &str) -> bool {
        let path = PathBuf::from(item);
        match self {
            Self::All => path.exists(),
            Self::Folder => path.is_dir(),
            Self::File => path.is_file() || path.is_symlink(),
        }
    }
}

pub fn expand(name: String, kind: FileKind) -> Vec<String> {
    if std::env::var("DEBUG").is_ok() {
        rayon::ThreadPoolBuilder::new().num_threads(1).build_global().unwrap()
    };
    if PathBuf::from(&name).exists() {
        if kind.is_valid(&name) {
            return vec![name];
        } else {
            upanic!("{name} exists but isn't a {kind:?}")
        }
    }
    if name.starts_with("/") {
        upanic!("{name} doesn't exist")
    }
    if name.is_empty() {
        upanic!("Usage: {} <segments..>", env::args().next().unwrap());
    }
    if name.contains("/") || name.is_empty() {
        upanic!("not yet implemented");
    }
    let mut prefix = vec![".".to_owned()];
    if let Some(dir) = char_to_dir(name.chars().next().unwrap()) {
        prefix.push(dir);
    }
    let mut v = vec![];
    for f in [match_exact_segments, match_partial_segments, match_no_segments] {
        v.extend(f(".", &name));
        if let Some(dir) = char_to_dir(name.chars().next().unwrap()) {
            v.extend(f(&dir, &name[1..]));
        }
        v.retain(|item| kind.is_valid(item));
        if !v.is_empty() {
            return v;
        }
    }
    vec![]
}
