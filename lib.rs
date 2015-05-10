#![allow(bad_style)]
extern crate libc;

use libc::{size_t, off_t, uintptr_t};
pub type uint8_t = u8;
pub type siginfo_t = ();

#[cfg(any(target_os = "linux", target_os = "android"))]
include!("linux.rs");
