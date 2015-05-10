extern crate pkg_config;

use std::env;
use std::fs::{self, File};
use std::io::ErrorKind;
use std::io::prelude::*;
use std::path::{Path, PathBuf};
use std::process::Command;

macro_rules! t {
    ($e:expr) => (match $e {
        Ok(n) => n,
        Err(e) => fail(&format!("\n{} failed with {}\n", stringify!($e), e)),
    })
}

fn main() {
    let mut cflags = env::var("CFLAGS").unwrap_or(String::new());
    let mut cflags = env::var("CFLAGS").unwrap_or(String::new());
    let target = env::var("TARGET").unwrap();
    let mingw = target.contains("windows-gnu");
    cflags.push_str(" -ffunction-sections -fdata-sections");

    if target.contains("i686") {
        cflags.push_str(" -m32");
    } else if target.contains("x86_64") {
        cflags.push_str(" -m64");
    }
    if !target.contains("i686") {
        cflags.push_str(" -fPIC");
    }

    if mingw {
        panic!("Windows building not supported!");
    }
    if target.contains("apple") {
        panic!("OS X building not supported!");
    }

    let src = PathBuf::from(&env::var("CARGO_MANIFEST_DIR").unwrap());
    let dst = PathBuf::from(&env::var("OUT_DIR").unwrap());

    let mut cmd = Command::new("./configure");
    run(cmd.env("CXXFLAGS", &cflags)
           .env("CFLAGS", &cflags)
           .env("CPPFLAGS", &cflags)
           .arg(format!("--prefix={}", dst.display())), "configure");

    run(&mut Command::new("make"), "make");
    run(Command::new("make").arg("install"), "make");

    println!("cargo:root={}", dst.display());
    println!("cargo:rustc-link-lib=dylib=stdc++");

    if env::var("HOST") == env::var("TARGET") {
        prepend("PKG_CONFIG_PATH", dst.join("lib"));
        if pkg_config::Config::new().statik(true).find("google-breakpad-client-c").is_ok() {
            return
        }
    }

    println!("cargo:rustc-link-lib=static=breakpad_client_c");
    println!("cargo:rustc-link-search=native={}/lib", dst.display());
    println!("cargo:rustc-flags=-L {}", dst.join("lib").display());
}

fn run(cmd: &mut Command, program: &str) {
    println!("running: {:?}", cmd);
    let status = match cmd.status() {
        Ok(status) => status,
        Err(ref e) if e.kind() == ErrorKind::NotFound => {
            fail(&format!("failed to execute command: {}\nis `{}` not installed?",
                          e, program));
        }
        Err(e) => fail(&format!("failed to execute command: {}", e)),
    };
    if !status.success() {
        fail(&format!("command did not execute successfully, got: {}", status));
    }
}

fn register_dep(dep: &str) {
    match env::var(&format!("DEP_{}_ROOT", dep)) {
        Ok(s) => {
            prepend("CMAKE_PREFIX_PATH", PathBuf::from(&s));
            prepend("PKG_CONFIG_PATH", Path::new(&s).join("lib/pkgconfig"));
        }
        Err(..) => {}
    }
}

fn prepend(var: &str, val: PathBuf) {
    let prefix = env::var(var).unwrap_or(String::new());
    let mut v = vec![val];
    v.extend(env::split_paths(&prefix));
    env::set_var(var, &env::join_paths(v).unwrap());
}

fn fail(s: &str) -> ! {
    panic!("\n{}\n\nbuild script failed, must exit now", s)
}

