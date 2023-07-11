/*pub use frodo_pir::{
    api::{BaseParams},
};*/

use sha2::{Digest, Sha256};

#[cxx::bridge(namespace = leaked_credentials)]
mod ffi {
    // Shared functions/datatypes between rust/c++ go in here directly

    // from rs/lib/db.rs
    struct BaseParams {
        dim: usize,
        m: usize,
        lhs_seed: [u8; 32],
        //rhs: Vec<Vec<u32>>,   // TODO FIX ME
        ele_size: usize,
        plaintext_bits: usize,
    }

    // from rs/lib/api.rs
    struct QueryParams {
        lhs: Vec<u32>,
        rhs: Vec<u32>,
        ele_size: usize,
        plaintext_bits: usize,
        pub used: bool,
    }

    // functions that are only known to rust go in here
    extern "Rust" {

        fn sha256(value: &str) -> Vec<u8>;
    }

    // functions that are only known to C++ go in here
    /*unsafe extern "C++" {
        // TODO
    }*/
}

pub fn sha256(value: &str) -> Vec<u8> {
    let digest = Sha256::digest(&value);
    let mut hash: Vec<u8> = vec![];
    for byte in digest.iter() {
        hash.push((*byte).into());
    }
    hash
}
