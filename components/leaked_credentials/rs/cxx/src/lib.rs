
#[cxx::bridge(namespace = leaked_credentials)]
mod ffi {
    // Shared functions/datatypes between rust/c++ go in here directly

    // functions that are only known to rust go in here
    extern "Rust" {
        // TODO
        fn HelloCPlusPlus() -> u8;
    }

    // functions that are only known to C++ go in here
    unsafe extern "C++" {
        // TODO
    }
}

pub fn HelloCPlusPlus() -> u8 {
    println!("Hello C++!");
    return 42;
}