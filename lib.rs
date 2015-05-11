#![allow(bad_style)]

extern crate libc;
extern crate winapi;

use winapi::*;

use libc::size_t;
#[cfg(any(target_os = "linux", target_os = "android"))]
use libc::{off_t, uintptr_t};
pub type uint8_t = u8;
pub type siginfo_t = (); // Temp hack

#[cfg(target_os = "windows")]
pub type MINIDUMP_TYPE = DWORD; // Temp hack

#[cfg(any(target_os = "linux", target_os = "android"))]
include!("linux.rs");

#[cfg(target_os = "windows")]
include!("windows.rs");
