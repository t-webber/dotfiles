//! This file was heavily inspired and modified from suckless init.
//!
//! Original licence:
//!
//! MIT/X Consortium License
//!
//! © 2014-2015 Dimitris Papastamos <sin@2f30.org>
//!
//! Permission is hereby granted, free of charge, to any person obtaining a
//! copy of this software and associated documentation files (the "Software"),
//! to deal in the Software without restriction, including without limitation
//! the rights to use, copy, modify, merge, publish, distribute, sublicense,
//! and/or sell copies of the Software, and to permit persons to whom the
//! Software is furnished to do so, subject to the following conditions:
//!
//! The above copyright notice and this permission notice shall be included in
//! all copies or substantial portions of the Software.
//!
//! THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//! IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//! FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
//! THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//! LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//! FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//! DEALINGS IN THE SOFTWARE.

use core::ffi::c_int;
use std::env::set_current_dir;
use std::os::unix::process::CommandExt;
use std::process::Command;
use std::ptr::null_mut;

type Set = [u64; 16];

const SIG_BLOCK: c_int = 0;
const SIG_UNBLOCK: c_int = 1;
const WNOHANG: c_int = 1;

const SIGINT: c_int = 2;
const SIGUSR1: c_int = 10;
const SIGALRM: c_int = 14;
const SIGCHLD: c_int = 17;

const SHUTDOWN: &str = "/del/.dot/otherconfig/shutdown";

unsafe extern "C" {
    fn sigfillset(s: *mut Set) -> i32;
    fn sigprocmask(how: i32, s: *const Set, old: *mut Set) -> i32;
    fn sigwait(s: *const Set, sig: *mut i32) -> i32;
    fn alarm(n: u32) -> u32;
    fn waitpid(pid: i32, st: *mut i32, opt: i32) -> i32;
    fn setsid() -> i32;
}

fn spawn(set: Set, prog: &str, args: &[&str]) {
    let _ = unsafe {
        Command::new(prog)
            .args(args)
            .pre_exec(move || {
                sigprocmask(SIG_UNBLOCK, &set, null_mut()); // liberate signals for new process
                setsid(); // separate process terminal
                Ok(())
            })
            .spawn()
            .inspect_err(|err| eprintln!("sinit ({prog}) error: {:?}", err))
    };
}

fn main() {
    set_current_dir("/").ok(); // cwd of process blocks umount
    let mut set: Set = [0; 16];
    let mut sig = 0;
    unsafe {
        sigfillset(&mut set); // select all signals
        sigprocmask(SIG_BLOCK, &set, null_mut()); //  block them all (needed for sigwait)
    }
    spawn(set, "/del/.dot/rs/bin/init-setup", &[]);
    loop {
        unsafe {
            alarm(30);
            sigwait(&set, &mut sig);
        }
        match sig {
            SIGALRM | SIGCHLD => unsafe { while waitpid(-1, null_mut(), WNOHANG) > 0 {} },
            SIGUSR1 => spawn(set, SHUTDOWN, &["poweroff"]),
            SIGINT => spawn(set, SHUTDOWN, &["reboot"]),
            _ => {}
        }
    }
}
