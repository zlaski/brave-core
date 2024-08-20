// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

use std::fmt;

use orchard::{
    builder:: {
        BuildError as OrchardBuildError,
        InProgress,
        Unproven,
        Unauthorized
    },
    bundle::Bundle,
    zip32::ChildIndex as OrchardChildIndex,
    keys::Scope as OrchardScope,
    keys::FullViewingKey as OrchardFVK,
    keys::PreparedIncomingViewingKey,
    zip32::Error as Zip32Error,
    zip32::ExtendedSpendingKey,
    tree::MerkleHashOrchard,
    note_encryption:: {
        OrchardDomain,
        CompactAction
    },
    note:: {
        Nullifier,
        ExtractedNoteCommitment
    }
};

use zcash_note_encryption::EphemeralKeyBytes;
use zcash_primitives::transaction::components::amount::Amount;
use zcash_protocol::consensus::BlockHeight;

use ffi::OrchardOutput;
use ffi::ShardStoreContext as ShardStoreContext;

use rand::rngs::OsRng;
use rand::{RngCore, Error as OtherError};
use rand::CryptoRng;

use brave_wallet::{
  impl_error
};
use std::sync::Arc;
use zcash_note_encryption::{
    batch, Domain, ShieldedOutput, COMPACT_NOTE_SIZE,
};

use crate::ffi::OrchardCompactAction;

use shardtree::{
    error::ShardTreeError,
    store::{Checkpoint, ShardStore, TreeState},
    LocatedPrunableTree, LocatedTree, PrunableTree, RetentionFlags,
    ShardTree,
};

use zcash_client_backend::serialization::shardtree::{read_shard, write_shard};
use cxx::UniquePtr;

use std::rc::Rc;
use std::cell::RefCell;
use std::marker::PhantomData;
use std::error;
use incrementalmerkletree::Address;
use zcash_primitives::merkle_tree::HashSer;

use crate::ffi::FfiShardTree;
use crate::ffi::FfiShardAddress;
use crate::ffi::FfiCheckpoint;
use crate::ffi::FfiCap;

use std::cmp::{Ord, Ordering};
use std::convert::TryFrom;
use std::ops::{Add, Bound, RangeBounds, Sub};

use crate::ffi::orchard_get_checkpoint;
use crate::ffi::orchard_remove_checkpoint;
use crate::ffi::orchard_min_checkpoint_id;
use crate::ffi::orchard_max_checkpoint_id;
use crate::ffi::orchard_checkpoint_count;
use crate::ffi::orchard_get_checkpoint_at_depth;
use crate::ffi::orchard_truncate_checkpoint;
use crate::ffi::orchard_get_cap;
use crate::ffi::orchard_last_shard;
use crate::ffi::orchard_get_shard;
use crate::ffi::orchard_get_shard_roots;
use crate::ffi::orchard_truncate;
use crate::ffi::orchard_put_cap;
use crate::ffi::orchard_add_checkpoint;
use crate::ffi::orchard_put_shard;
use crate::ffi::orchard_with_checkpoints;
use crate::ffi::orchard_update_checkpoint;
use crate::ffi::orchard_get_checkpoints;

use crate::ffi::ShardStoreStatusCode;
use incrementalmerkletree::Position;
use std::io::Cursor;
use std::collections::BTreeSet;
// pub use brave_wallet::orchard;

// The rest of the wallet code should be updated to use this version of unwrap
// and then this code can be removed
#[macro_export]
macro_rules! impl_result {
    ($t:ident, $r:ident, $f:ident) => {
        impl $r {
            fn error_message(self: &$r) -> String {
                match &self.0 {
                    Err(e) => e.to_string(),
                    Ok(_) => "".to_string(),
                }
            }

            fn is_ok(self: &$r) -> bool {
                match &self.0 {
                    Err(_) => false,
                    Ok(_) => true,
                }
            }

            // Unfortunately cxx doesn't support passing $r by value here so
            // we have to clone the inner value instead of passing ownership
            // This is not really a big deal because eventually we want to
            // replace this with mojo which would serialize this anyway
            fn unwrap(self: &$r) -> Box<$t> {
                Box::new(self.0.as_ref().expect(
                    "Unhandled error before unwrap call").clone())
            }
        }

        impl From<Result<$f, Error>> for $r {
            fn from(result: Result<$f, Error>) -> Self {
                match result {
                    Ok(v) => Self(Ok($t(v))),
                    Err(e) => Self(Err(e)),
                }
            }
        }
    };
}

pub(crate) const PRUNING_DEPTH: u8 = 100;
pub(crate) const SHARD_HEIGHT: u8 = 16;

#[derive(Clone)]
pub(crate) struct MockRng(u64);

impl CryptoRng for MockRng {}

// Used for determenistic zk-proof generation in the
// brave_wallet unittests.
// Must not be used in production see create_testing_orchard_bundle
impl RngCore for MockRng {
    fn next_u32(&mut self) -> u32 {
        self.next_u64() as u32
    }

    fn next_u64(&mut self) -> u64 {
        self.0 += 1;
        self.0
    }

    fn fill_bytes(&mut self, dest: &mut [u8]) {
        // https://github.com/rust-random/rand/blob/master/rand_core/src/impls.rs#L38
        let mut left = dest;
        while left.len() >= 8 {
            let (l, r) = { left }.split_at_mut(8);
            left = r;
            let chunk: [u8; 8] = self.next_u64().to_le_bytes();
            l.copy_from_slice(&chunk);
        }
        let n = left.len();
        if n > 4 {
            let chunk: [u8; 8] = self.next_u64().to_le_bytes();
            left.copy_from_slice(&chunk[..n]);
        } else if n > 0 {
            let chunk: [u8; 4] = self.next_u32().to_le_bytes();
            left.copy_from_slice(&chunk[..n]);
        }
    }

    fn try_fill_bytes(&mut self, dest: &mut [u8]) -> Result<(), OtherError> {
        Ok(self.fill_bytes(dest))
    }
}

#[allow(unused)]
#[allow(unsafe_op_in_unsafe_fn)]
#[cxx::bridge(namespace = brave_wallet::orchard)]
mod ffi {
    struct OrchardOutput {
        // Amount of zashi being spend
        value: u32,
        // Recipient raw Orchard address.
        // Array size should match kOrchardRawBytesSize
        addr: [u8; 43]
    }

    // Encoded orchard output extracted from the transaction
    struct OrchardCompactAction {
        nullifier: [u8; 32],  // kOrchardNullifierSize
        ephemeral_key: [u8; 32],  // kOrchardEphemeralKeySize
        cmx: [u8; 32],  // kOrchardCmxSize
        enc_cipher_text : [u8; 52]  // kOrchardCipherTextSize
    }

    #[repr(u32)]
    enum ShardStoreStatusCode {
        Ok = 0,
        None = 1,
        Error = 2
    }

    #[derive(Default)]
    struct FfiShardAddress {
        level: u8,
        index: u32
    }
    
    #[derive(Default)]
    struct FfiCap {
        data: Vec<u8>,
    }

    #[derive(Default)]
    struct FfiShardTree {
        address: FfiShardAddress,
        hash: [u8; 32],
        data: Vec<u8>,
        contains_marked: bool
    }

    #[derive(Default)]
    struct FfiCheckpoint {
        empty: bool,
        position: u32,
        mark_removed: Vec<u32>
    }

    struct FfiCheckpointBundle {
        checkpoint_id: u32,
        checkpoint: FfiCheckpoint
    }

    extern "Rust" {
        type OrchardExtendedSpendingKey;
        type OrchardUnauthorizedBundle;
        type OrchardAuthorizedBundle;

        type BatchOrchardDecodeBundle;

        type OrchardShardTreeBundle;

        type OrchardExtendedSpendingKeyResult;
        type OrchardUnauthorizedBundleResult;
        type OrchardAuthorizedBundleResult;

        type BatchOrchardDecodeBundleResult;

        type OrchardShardTreeBundleResult;

        // OsRng is used
        fn create_orchard_bundle(
            tree_state: &[u8],
            outputs: Vec<OrchardOutput>
        ) -> Box<OrchardUnauthorizedBundleResult>;

        // Creates orchard bundle with mocked rng using provided rng seed.
        // Must not be used in production, only in tests.
        fn create_testing_orchard_bundle(
            tree_state: &[u8],
            outputs: Vec<OrchardOutput>,
            rng_seed: u64
        ) -> Box<OrchardUnauthorizedBundleResult>;

        fn generate_orchard_extended_spending_key_from_seed(
            bytes: &[u8]
        ) -> Box<OrchardExtendedSpendingKeyResult>;

        fn is_ok(self: &OrchardExtendedSpendingKeyResult) -> bool;
        fn error_message(self: &OrchardExtendedSpendingKeyResult) -> String;
        fn unwrap(self: &OrchardExtendedSpendingKeyResult) -> Box<OrchardExtendedSpendingKey>;

        fn batch_decode(
            fvk_bytes: &[u8; 96],  // Array size should match kOrchardFullViewKeySize
            actions: Vec<OrchardCompactAction>
        ) -> Box<BatchOrchardDecodeBundleResult>;

        fn derive(
            self: &OrchardExtendedSpendingKey,
            index: u32
        ) -> Box<OrchardExtendedSpendingKeyResult>;
        // External addresses can be used for receiving funds from external
        // senders.
        fn external_address(
            self: &OrchardExtendedSpendingKey,
            diversifier_index: u32
        ) -> [u8; 43];  // Array size should match kOrchardRawBytesSize
        // Internal addresses are used for change or internal shielding and
        // shouldn't be exposed to public.
        fn internal_address(
            self: &OrchardExtendedSpendingKey,
            diversifier_index: u32
        ) -> [u8; 43];  // Array size should match kOrchardRawBytesSize
        fn full_view_key(
            self: &OrchardExtendedSpendingKey
        ) -> [u8; 96];  // Array size sohuld match kOrchardFullViewKeySize

        fn is_ok(self: &OrchardAuthorizedBundleResult) -> bool;
        fn error_message(self: &OrchardAuthorizedBundleResult) -> String;
        fn unwrap(self: &OrchardAuthorizedBundleResult) -> Box<OrchardAuthorizedBundle>;

        fn is_ok(self: &OrchardUnauthorizedBundleResult) -> bool;
        fn error_message(self: &OrchardUnauthorizedBundleResult) -> String;
        fn unwrap(self: &OrchardUnauthorizedBundleResult) -> Box<OrchardUnauthorizedBundle>;

        fn is_ok(self: &BatchOrchardDecodeBundleResult) -> bool;
        fn error_message(self: &BatchOrchardDecodeBundleResult) -> String;
        fn unwrap(self: &BatchOrchardDecodeBundleResult) -> Box<BatchOrchardDecodeBundle>;

        fn size(self :&BatchOrchardDecodeBundle) -> usize;
        fn note_value(self :&BatchOrchardDecodeBundle, index: usize) -> u32;
        // Result array size should match kOrchardNullifierSize
        // fvk array size should match kOrchardFullViewKeySize
        fn note_nullifier(self :&BatchOrchardDecodeBundle, fvk: &[u8; 96], index: usize) -> [u8; 32];

        fn is_ok(self: &OrchardShardTreeBundleResult) -> bool;
        fn error_message(self: &OrchardShardTreeBundleResult) -> String;
        fn unwrap(self: &OrchardShardTreeBundleResult) -> Box<OrchardShardTreeBundle>;

        // Orchard digest is desribed here https://zips.z.cash/zip-0244#t-4-orchard-digest
        // Used in constructing signature digest and tx id
        fn orchard_digest(self: &OrchardUnauthorizedBundle) -> [u8; 32];  // Array size should match kZCashDigestSize
        // Completes unauthorized bundle to authorized state
        // Signature digest should be constructed as desribed in https://zips.z.cash/zip-0244#signature-digest
        fn complete(self: &OrchardUnauthorizedBundle, sighash: [u8; 32]) -> Box<OrchardAuthorizedBundleResult>;  // Array size should match kZCashDigestSize

        // Orchard part of v5 transaction as described in
        // https://zips.z.cash/zip-0225
        fn raw_tx(self: &OrchardAuthorizedBundle) -> Vec<u8>;

        fn create_shard_tree(ctx: UniquePtr<ShardStoreContext>) -> Box<OrchardShardTreeBundleResult>;
        fn insert_subtree_roots(self: &OrchardShardTreeBundle, roots: Vec<FfiShardTree>) -> bool;
        fn insert_commitments(self: &OrchardShardTreeBundle, commitments: Vec<FfiShardTree>) -> bool;
    }

    unsafe extern "C++" {
        include!("brave/components/brave_wallet/browser/zcash/rust/cxx/src/shard_store.h");

        type ShardStoreContext;
       // type ShardStoreStatusCode;

        fn orchard_last_shard(
            ctx: &ShardStoreContext, into: &mut FfiShardTree) -> ShardStoreStatusCode;
        fn orchard_get_shard(
            ctx: &ShardStoreContext,
            addr: &FfiShardAddress,
            tree: &mut FfiShardTree) -> ShardStoreStatusCode;
        fn orchard_put_shard(
            ctx: Pin<&mut ShardStoreContext>,
            tree: &FfiShardTree) -> ShardStoreStatusCode;
        fn orchard_get_shard_roots(
            ctx: &ShardStoreContext, into: &mut Vec<FfiShardAddress>) -> ShardStoreStatusCode;
        fn orchard_truncate(
            ctx: Pin<&mut ShardStoreContext>,
            address: &FfiShardAddress) -> ShardStoreStatusCode;
        fn orchard_get_cap(
            ctx: &ShardStoreContext,
            into: &mut FfiCap) -> ShardStoreStatusCode;
        fn orchard_put_cap(
            ctx: Pin<&mut ShardStoreContext>,
            tree: &FfiCap) -> ShardStoreStatusCode;
        fn orchard_min_checkpoint_id(
            ctx: &ShardStoreContext, into: &mut u32) -> ShardStoreStatusCode;
        fn orchard_max_checkpoint_id(
            ctx: &ShardStoreContext, into: &mut u32) -> ShardStoreStatusCode;
        fn orchard_add_checkpoint(
            ctx: Pin<&mut ShardStoreContext>,
            checkpoint_id: u32,
            checkpoint: &FfiCheckpoint) -> ShardStoreStatusCode;
        fn orchard_update_checkpoint(
            ctx: Pin<&mut ShardStoreContext>,
            checkpoint_id: u32,
            checkpoint: &FfiCheckpoint) -> ShardStoreStatusCode;
        fn orchard_checkpoint_count(ctx: &ShardStoreContext, into: &mut usize) -> ShardStoreStatusCode;
        fn orchard_get_checkpoint_at_depth(
            ctx: &ShardStoreContext,
            depth: usize,
            into_checkpoint_id: &mut u32,
            into_checkpoint: &mut FfiCheckpoint) -> ShardStoreStatusCode;
        fn orchard_get_checkpoint(
            ctx: &ShardStoreContext,
            checkpoint_id: u32,
            into: &mut FfiCheckpoint) -> ShardStoreStatusCode;
        fn orchard_remove_checkpoint(
            ctx: Pin<&mut ShardStoreContext>,
            checkpoint_id: u32) -> ShardStoreStatusCode;
        fn orchard_truncate_checkpoint(
            ctx: Pin<&mut ShardStoreContext>,
            checkpoint_id: u32) -> ShardStoreStatusCode;
        fn orchard_with_checkpoints(
            ctx: &ShardStoreContext,
            limit: usize,
            callback: fn(u32, &FfiCheckpoint) -> ShardStoreStatusCode) ->ShardStoreStatusCode;
        fn orchard_get_checkpoints(
            ctx: &ShardStoreContext,
            limit: usize,
            into: &mut Vec<FfiCheckpoint>) -> ShardStoreStatusCode;
    }

}

#[derive(Debug)]
pub enum Error {
    Zip32(Zip32Error),
    OrchardBuilder(OrchardBuildError),
    WrongOutputError,
    BuildError,
    FvkError,
    OrchardActionFormatError,
    ShardTreeError,
}

impl_error!(Zip32Error, Zip32);
impl_error!(OrchardBuildError, OrchardBuilder);

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match &self {
            Error::Zip32(e) => write!(f, "Zip32 Error: {e}"),
            Error::OrchardBuilder(e) => write!(f, "Error: {}", e.to_string()),
            Error::WrongOutputError => write!(f, "Error: Can't parse output"),
            Error::BuildError => write!(f, "Error, build error"),
            Error::OrchardActionFormatError => write!(f, "Error, orchard action format error"),
            Error::FvkError => write!(f, "Error, fvk format error"),
            Error::ShardTreeError => write!(f, "Failed to create shard tree")
        }
    }
}

impl error::Error for Error {
    fn source(&self) -> Option<&(dyn error::Error + 'static)> {
        None
        // match &self {
        //     Error::Serialization(e) => Some(e),
        //     Error::Query(e) => Some(e),
        //     Error::CheckpointConflict { .. } => None,
        //     Error::SubtreeDiscontinuity { .. } => None,
        // }
    }
}


// Different random sources are used for testing and for release
// Since Orchard uses randomness we need to mock it to get
// deterministic resuluts in tests.
#[derive(Clone)]
enum OrchardRandomSource {
    OsRng(OsRng),
    MockRng(MockRng),
}

// Unauthorized bundle is a bundle without generated proof, but it
// contains Orchard digest needed to calculate tx signature digests.
#[derive(Clone)]
pub struct OrchardUnauthorizedBundleValue {
    unauthorized_bundle: Bundle<InProgress<Unproven, Unauthorized>, Amount>,
    rng: OrchardRandomSource
}

// Authorized bundle is a bundle where inputs are signed with signature digests
// and proof is generated.
#[derive(Clone)]
pub struct OrchardAuthorizedBundleValue {
    raw_tx: Vec<u8>
}

#[derive(Clone)]
pub struct DecryptedOrchardOutput {
    note: <OrchardDomain as Domain>::Note
}
#[derive(Clone)]
pub struct BatchOrchardDecodeBundleValue {
    outputs: Vec<DecryptedOrchardOutput>
}

#[allow(dead_code)]
pub struct OrchardShardTreeBundleValue {
    tree: ShardTree<OrchardCxxShardStoreImpl, PRUNING_DEPTH, SHARD_HEIGHT>
}

impl Clone for OrchardShardTreeBundleValue {
    fn clone(&self) -> Self {
        OrchardShardTreeBundleValue { tree : ShardTree::new(self.tree.store().clone(), PRUNING_DEPTH.try_into().unwrap()) }
    }
}

#[derive(Clone)]
struct OrchardExtendedSpendingKey(ExtendedSpendingKey);
#[derive(Clone)]
struct OrchardAuthorizedBundle(OrchardAuthorizedBundleValue);
#[derive(Clone)]
struct OrchardUnauthorizedBundle(OrchardUnauthorizedBundleValue);
#[derive(Clone)]
struct BatchOrchardDecodeBundle(BatchOrchardDecodeBundleValue);
#[allow(dead_code)]
#[derive(Clone)]
struct OrchardShardTreeBundle(OrchardShardTreeBundleValue);

struct OrchardExtendedSpendingKeyResult(Result<OrchardExtendedSpendingKey, Error>);
struct OrchardAuthorizedBundleResult(Result<OrchardAuthorizedBundle, Error>);
struct OrchardUnauthorizedBundleResult(Result<OrchardUnauthorizedBundle, Error>);
struct BatchOrchardDecodeBundleResult(Result<BatchOrchardDecodeBundle, Error>);
struct OrchardShardTreeBundleResult(Result<OrchardShardTreeBundle, Error>);

impl_result!(OrchardExtendedSpendingKey, OrchardExtendedSpendingKeyResult, ExtendedSpendingKey);
impl_result!(OrchardAuthorizedBundle, OrchardAuthorizedBundleResult, OrchardAuthorizedBundleValue);
impl_result!(OrchardUnauthorizedBundle, OrchardUnauthorizedBundleResult, OrchardUnauthorizedBundleValue);

impl_result!(BatchOrchardDecodeBundle, BatchOrchardDecodeBundleResult, BatchOrchardDecodeBundleValue);
impl_result!(OrchardShardTreeBundle, OrchardShardTreeBundleResult, OrchardShardTreeBundleValue);

fn generate_orchard_extended_spending_key_from_seed(
    bytes: &[u8]
) -> Box<OrchardExtendedSpendingKeyResult> {
  Box::new(OrchardExtendedSpendingKeyResult::from(
    ExtendedSpendingKey::master(bytes).map_err(Error::from))
  )
}

impl OrchardExtendedSpendingKey {
    fn derive(
        self: &OrchardExtendedSpendingKey,
        index: u32
    ) -> Box<OrchardExtendedSpendingKeyResult> {
        Box::new(OrchardExtendedSpendingKeyResult::from(
            self.0.derive_child(
                OrchardChildIndex::hardened(index))
                .map_err(Error::from)))
    }

    fn external_address(
        self: &OrchardExtendedSpendingKey,
        diversifier_index: u32
    ) -> [u8; 43] {
        let address = OrchardFVK::from(&self.0).address_at(
            diversifier_index, OrchardScope::External);
        address.to_raw_address_bytes()
    }

    fn internal_address(
        self: &OrchardExtendedSpendingKey,
        diversifier_index: u32
    ) -> [u8; 43] {
        let address = OrchardFVK::from(&self.0).address_at(
            diversifier_index, OrchardScope::Internal);
        address.to_raw_address_bytes()
    }

    fn full_view_key(
        self: &OrchardExtendedSpendingKey
    ) -> [u8; 96] {
        OrchardFVK::from(&self.0).to_bytes()
    }
}

impl OrchardAuthorizedBundle {
    fn raw_tx(self: &OrchardAuthorizedBundle) -> Vec<u8> {
        self.0.raw_tx.clone()
    }
}

fn create_orchard_builder_internal(
    orchard_tree_bytes: &[u8],
    outputs: Vec<OrchardOutput>,
    random_source: OrchardRandomSource
) -> Box<OrchardUnauthorizedBundleResult> {
    use orchard::Anchor;
    use zcash_primitives::merkle_tree::read_commitment_tree;

    // To construct transaction orchard tree state of some block should be provided
    // But in tests we can use empty anchor.
    let anchor = if orchard_tree_bytes.len() > 0 {
        match read_commitment_tree::<MerkleHashOrchard, _, { orchard::NOTE_COMMITMENT_TREE_DEPTH as u8 }>(
                &orchard_tree_bytes[..]) {
            Ok(tree) => Anchor::from(tree.root()),
            Err(_e) => return Box::new(OrchardUnauthorizedBundleResult::from(Err(Error::from(OrchardBuildError::AnchorMismatch)))),
        }
    } else {
        orchard::Anchor::empty_tree()
    };

    let mut builder = orchard::builder::Builder::new(
        orchard::builder::BundleType::DEFAULT,
        anchor);

    for out in outputs {
        let _ = match Option::from(orchard::Address::from_raw_address_bytes(&out.addr)) {
            Some(addr) => {
                builder.add_output(None, addr,
                    orchard::value::NoteValue::from_raw(u64::from(out.value)), None)
            },
            None => return Box::new(OrchardUnauthorizedBundleResult::from(Err(Error::WrongOutputError)))
        };
    }

    Box::new(OrchardUnauthorizedBundleResult::from(match random_source {
        OrchardRandomSource::OsRng(mut rng) => {
            builder.build(&mut rng)
                .map_err(Error::from)
                .and_then(|builder| {
                    builder.map(|bundle| OrchardUnauthorizedBundleValue {
                        unauthorized_bundle: bundle.0,
                        rng: OrchardRandomSource::OsRng(rng) }).ok_or(Error::BuildError)
                })
        },
        OrchardRandomSource::MockRng(mut rng) => {
            builder.build(&mut rng)
                .map_err(Error::from)
                .and_then(|builder| {
                    builder.map(|bundle| OrchardUnauthorizedBundleValue {
                        unauthorized_bundle: bundle.0,
                        rng: OrchardRandomSource::MockRng(rng) }).ok_or(Error::BuildError)
                })
        }
    }))
}

fn create_orchard_bundle(
    orchard_tree_bytes: &[u8],
    outputs: Vec<OrchardOutput>
) -> Box<OrchardUnauthorizedBundleResult> {
    create_orchard_builder_internal(orchard_tree_bytes, outputs, OrchardRandomSource::OsRng(OsRng))
}

fn create_testing_orchard_bundle(
    orchard_tree_bytes: &[u8],
    outputs: Vec<OrchardOutput>,
    rng_seed: u64
) -> Box<OrchardUnauthorizedBundleResult> {
    create_orchard_builder_internal(orchard_tree_bytes, outputs, OrchardRandomSource::MockRng(MockRng(rng_seed)))
}

impl OrchardUnauthorizedBundle {
    fn orchard_digest(self: &OrchardUnauthorizedBundle) -> [u8; 32] {
        self.0.unauthorized_bundle.commitment().into()
    }

    fn complete(self: &OrchardUnauthorizedBundle, sighash: [u8; 32]) -> Box<OrchardAuthorizedBundleResult> {
        use zcash_primitives::transaction::components::orchard::write_v5_bundle;
        Box::new(OrchardAuthorizedBundleResult::from(match self.0.rng.clone() {
            OrchardRandomSource::OsRng(mut rng) => {
                self.0.unauthorized_bundle.clone()
                .create_proof(&orchard::circuit::ProvingKey::build(), &mut rng)
                .and_then(|b| {
                            b.apply_signatures(
                                &mut rng,
                                sighash,
                                &[],
                            )
                        })
            },
            OrchardRandomSource::MockRng(mut rng) => {
                self.0.unauthorized_bundle.clone()
                .create_proof(&orchard::circuit::ProvingKey::build(), &mut rng)
                .and_then(|b| {
                            b.apply_signatures(
                                &mut rng,
                                sighash,
                                &[],
                            )
                        })
            }
        }.map_err(Error::from).and_then(|authorized_bundle| {
            let mut result = OrchardAuthorizedBundleValue {raw_tx : vec![]};
            let _ = write_v5_bundle(Some(&authorized_bundle), &mut result.raw_tx);
            Ok(result)
        })))
    }
}

impl ShieldedOutput<OrchardDomain, COMPACT_NOTE_SIZE> for OrchardCompactAction {
    fn ephemeral_key(&self) -> EphemeralKeyBytes {
        EphemeralKeyBytes(self.ephemeral_key)
    }

    fn cmstar_bytes(&self) -> [u8; 32] {
        self.cmx
    }

    fn enc_ciphertext(&self) -> &[u8; COMPACT_NOTE_SIZE] {
        &self.enc_cipher_text
    }
}

fn batch_decode(
    fvk_bytes: &[u8; 96],
    actions: Vec<OrchardCompactAction>
) -> Box<BatchOrchardDecodeBundleResult> {
    let fvk = match OrchardFVK::from_bytes(fvk_bytes) {
        Some(fvk) => fvk,
        None => return Box::new(BatchOrchardDecodeBundleResult::from(Err(Error::FvkError)))
    };

    let ivks = [
        PreparedIncomingViewingKey::new(&fvk.to_ivk(OrchardScope::External)),
        PreparedIncomingViewingKey::new(&fvk.to_ivk(OrchardScope::Internal))
    ];

    let input_actions: Result<Vec<(OrchardDomain, OrchardCompactAction)>, Error> = actions
        .into_iter()
        .map(|v| {
            let nullifier_ctopt = Nullifier::from_bytes(&v.nullifier);
            let nullifier = if nullifier_ctopt.is_none().into() {
                Err(Error::OrchardActionFormatError)
            } else {
                Ok(nullifier_ctopt.unwrap())
            }?;

            let cmx_ctopt = ExtractedNoteCommitment::from_bytes(&v.cmx);
            let cmx = if cmx_ctopt.is_none().into() {
                Err(Error::OrchardActionFormatError)
            } else {
                Ok(cmx_ctopt.unwrap())
            }?;

            let ephemeral_key = EphemeralKeyBytes(v.ephemeral_key);
            let enc_cipher_text = v.enc_cipher_text;

            let compact_action = CompactAction::from_parts(nullifier, cmx, ephemeral_key, enc_cipher_text);
            let orchard_domain = OrchardDomain::for_compact_action(&compact_action);

            Ok((orchard_domain, v))
        })
        .collect();

    let input_actions = match input_actions {
        Ok(actions) => actions,
        Err(e) => return Box::new(BatchOrchardDecodeBundleResult::from(Err(e.into())))
    };

    let decrypted_outputs = batch::try_compact_note_decryption(&ivks, &input_actions.as_slice())
    .into_iter()
    .map(|res| {
        res.map(|((note, _recipient), _ivk_idx)| DecryptedOrchardOutput {
            note: note
        })
    })
    .filter_map(|x| x)
    .collect::<Vec<_>>();

    Box::new(BatchOrchardDecodeBundleResult::from(Ok(BatchOrchardDecodeBundleValue { outputs: decrypted_outputs })))
}

impl BatchOrchardDecodeBundle {
    fn size(self :&BatchOrchardDecodeBundle) -> usize {
      self.0.outputs.len()
    }

    fn note_value(self :&BatchOrchardDecodeBundle, index: usize) -> u32 {
      u32::try_from(self.0.outputs[index].note.value().inner()).expect(
          "Outputs are always created from a u32, so conversion back will always succeed")
    }

    fn note_nullifier(self :&BatchOrchardDecodeBundle, fvk: &[u8; 96], index: usize) -> [u8; 32] {
      self.0.outputs[index].note.nullifier(&OrchardFVK::from_bytes(fvk).unwrap()).to_bytes()
    }
}


impl OrchardShardTreeBundle {
    fn insert_subtree_roots(self: &OrchardShardTreeBundle, _roots: Vec<FfiShardTree>) -> bool {
        false
    }

    fn insert_commitments(self: &OrchardShardTreeBundle, _commitments: Vec<FfiShardTree>) -> bool {
        false
    }
}
#[allow(dead_code)]
#[derive(Clone)]
pub struct CxxShardStoreImpl<H, const SHARD_HEIGHT: u8>  {
    native_context: Rc<RefCell<UniquePtr<ShardStoreContext>>>,
    _hash_type: PhantomData<H>,
}

impl From<&FfiCheckpoint> for Checkpoint {
    fn from(item: &FfiCheckpoint) -> Self {
        let tree_state : TreeState = if item.empty { TreeState::Empty } else { TreeState::AtPosition((item.position as u64).into()) };
        let marks_removed : BTreeSet<Position> = item.mark_removed.iter().map(|x| Position::from(*x as u64)).collect();
        Checkpoint::from_parts(tree_state, marks_removed)
    }
}

impl TryFrom<&Checkpoint> for FfiCheckpoint {
    type Error = Error;
    fn try_from(item: &Checkpoint) -> Result<Self, Self::Error> {
        let position: u32 = match item.tree_state() {
            TreeState::Empty => 0,
            TreeState::AtPosition(pos) => (u64::from(pos)).try_into().map_err(|_| Error::ShardTreeError)?
        };
        let marks_removed : Result<Vec<u32>, Error> = item.marks_removed().into_iter().map(
            |x| u32::try_from(u64::from(*x)).map_err(|_| Error::ShardTreeError)).collect();
        Ok(FfiCheckpoint {
            empty: item.is_tree_empty(),
            position: position,
            mark_removed: marks_removed?
        })
    }
}

impl TryFrom<&Address> for FfiShardAddress {
    type Error = Error;

    fn try_from(item: &Address) -> Result<Self, Self::Error> {
        let index : u32 =  item.index().try_into().map_err(|_| Error::ShardTreeError)?;
        Ok(FfiShardAddress{
            level: item.level().into(),
            index: index })
    }
}

impl From<&FfiShardAddress> for Address {
    fn from(item: &FfiShardAddress) -> Self {
        Address::from_parts(item.level.into(), item.index.into())
    }
}

impl<H: HashSer> TryFrom<&FfiShardTree> for LocatedPrunableTree<H> {
    type Error = Error;

    fn try_from(item: &FfiShardTree) -> Result<Self, Self::Error> {
        let shard_tree = read_shard(&mut Cursor::new(&item.data)).map_err(|_| Error::ShardTreeError)?;
        let located_tree: LocatedTree<_, (_, RetentionFlags)> = LocatedPrunableTree::from_parts(Address::from(&item.address), shard_tree);
        if !item.hash.is_empty() {
            let root_hash = H::read(Cursor::new(item.hash)).map_err(|_| Error::ShardTreeError)?;
            Ok(located_tree.reannotate_root(Some(Arc::new(root_hash))))
        } else {
            Ok(located_tree)
        }
    }
}

impl <H: HashSer> TryFrom<&FfiCap> for PrunableTree<H> {
    type Error = Error;

    fn try_from(item: &FfiCap) -> Result<Self, Self::Error> {
        read_shard(&mut Cursor::new(&item.data)).map_err(|_| Error::ShardTreeError)
    }  
}

impl <H: HashSer> TryFrom<&PrunableTree<H>> for FfiCap {
    type Error = Error;

    fn try_from(item: &PrunableTree<H>) -> Result<Self, Self::Error> {
        let mut data = vec![];
        write_shard(&mut data, item).map_err(|_| Error::ShardTreeError)?;
        Ok(FfiCap {
            data: data
        })
    }
}

impl<H: HashSer> TryFrom<&LocatedPrunableTree<H>> for FfiShardTree {    
    type Error = Error;

    fn try_from(item: &LocatedPrunableTree<H>) -> Result<Self, Self::Error> {
        let subtree_root_hash : Option<Vec<u8>> = item
        .root()
        .annotation()
        .and_then(|ann| {
            ann.as_ref().map(|rc| {
                let mut root_hash = vec![];
                rc.write(&mut root_hash)?;
                Ok(root_hash)
            })
        })
        .transpose()
        .map_err(|_err : std::io::Error| Error::ShardTreeError)?;

        let mut result = FfiShardTree {
            address: FfiShardAddress::try_from(&item.root_addr()).map_err(|_| Error::ShardTreeError)?,
            hash: subtree_root_hash.unwrap_or_else(|| vec![0; 32]).try_into().map_err(|_| Error::ShardTreeError)?,
            data: vec![],
            contains_marked: false,
        };

        write_shard(&mut result.data, &item.root()).map_err(|_| Error::ShardTreeError)?;
        Ok(result)
    }
}

type OrchardCxxShardStoreImpl = CxxShardStoreImpl<orchard::tree::MerkleHashOrchard, SHARD_HEIGHT>;

impl<H: HashSer, const SHARD_HEIGHT: u8> ShardStore
    for CxxShardStoreImpl<H, SHARD_HEIGHT>
{
    type H = H;
    type CheckpointId = BlockHeight;
    type Error = Error;

    fn get_shard(
        &self,
        addr: Address,
    ) -> Result<Option<LocatedPrunableTree<Self::H>>, Self::Error> {
        let ctx = self.native_context.clone();    
        let mut into = FfiShardTree::default();
        let result = orchard_get_shard(&*ctx.try_borrow().unwrap(),
            &FfiShardAddress::try_from(&addr).map_err(|_| Error::ShardTreeError)?,
            &mut into);
        if result == ShardStoreStatusCode::Ok {
            let tree = LocatedPrunableTree::<H>::try_from(&into)?;
            return Ok(Some(tree));
        } else if result == ShardStoreStatusCode::None {
            return Ok(Option::None);
        } else {
            return Err(Error::ShardTreeError);
        }
    }

    fn last_shard(&self) -> Result<Option<LocatedPrunableTree<Self::H>>, Self::Error> {    
        let ctx = self.native_context.clone();    
        let mut into = FfiShardTree::default();
        let result = orchard_last_shard(&*ctx.try_borrow().unwrap(), &mut into);
        if result == ShardStoreStatusCode::Ok {
            let tree = LocatedPrunableTree::<H>::try_from(&into)?;
            return Ok(Some(tree));
        } else if result == ShardStoreStatusCode::None {
            return Ok(Option::None);
        } else {
            return Err(Error::ShardTreeError);
        }
    }

    fn put_shard(&mut self, subtree: LocatedPrunableTree<Self::H>) -> Result<(), Self::Error> {
        let ctx = self.native_context.clone();
        let ffi_tree = FfiShardTree::try_from(&subtree).map_err(|_| Error::ShardTreeError)?;
        let result = orchard_put_shard(ctx.try_borrow_mut().unwrap().pin_mut(), &ffi_tree);
        if result == ShardStoreStatusCode::Ok {
          return Ok(());
        }
        return Err(Error::ShardTreeError);
    }

    fn get_shard_roots(&self) -> Result<Vec<Address>, Self::Error> {
        let ctx = self.native_context.clone();
        let mut input : Vec<FfiShardAddress> = vec![];
        let result = orchard_get_shard_roots(&*ctx.try_borrow().unwrap(), &mut input);
        if result == ShardStoreStatusCode::Ok {
          return Ok(input.into_iter().map(|res| {
            Address::from_parts(res.level.into(), res.index.into())
          }).collect())
        } else if result == ShardStoreStatusCode::None {
          return Ok(vec![])
        } else {
          return Err(Error::ShardTreeError)
        }
    }

    fn truncate(&mut self, from: Address) -> Result<(), Self::Error> {
        let ctx = self.native_context.clone();
        let result = orchard_truncate(ctx.try_borrow_mut().unwrap().pin_mut(),
          &FfiShardAddress::try_from(&from).map_err(|_| Error::ShardTreeError)?);
        if result == ShardStoreStatusCode::Ok || result == ShardStoreStatusCode::None {
          return Ok(());
        } else {
          return Err(Error::ShardTreeError)
        }
    }

    fn get_cap(&self) -> Result<PrunableTree<Self::H>, Self::Error> {
        let ctx = self.native_context.clone();
        let mut input = FfiCap::default();
        let result = orchard_get_cap(&*ctx.try_borrow().unwrap(), &mut input);
        
        if result == ShardStoreStatusCode::Ok {
            let tree = PrunableTree::<H>::try_from(&input)?;
            return Ok(tree)
        } else 
        if result == ShardStoreStatusCode::None {
            return Ok(PrunableTree::empty());
        } else {
            return Err(Error::ShardTreeError);
        }
    }

    fn put_cap(&mut self, cap: PrunableTree<Self::H>) -> Result<(), Self::Error> {
        let ctx = self.native_context.clone();
        let mut ffi_cap = FfiCap::default();
        write_shard(&mut ffi_cap.data, &cap).map_err(|_| Error::ShardTreeError)?;

        let result = orchard_put_cap(ctx.try_borrow_mut().unwrap().pin_mut(), &ffi_cap);
        if result == ShardStoreStatusCode::Ok {
            return Ok(());  
        }
        return Err(Error::ShardTreeError);
    }

    fn min_checkpoint_id(&self) -> Result<Option<Self::CheckpointId>, Self::Error> {
        let ctx = self.native_context.clone();
        let mut input : u32 = 0;
        let result = orchard_min_checkpoint_id(&*ctx.try_borrow().unwrap(), &mut input);
        if result == ShardStoreStatusCode::Ok {
            return Ok(Some(input.into()));
        } else if result == ShardStoreStatusCode::None {
            return Ok(Option::None);
        }
        return Err(Error::ShardTreeError);
    }

    fn max_checkpoint_id(&self) -> Result<Option<Self::CheckpointId>, Self::Error> {
        let ctx = self.native_context.clone();
        let mut input : u32 = 0;
        let result = orchard_max_checkpoint_id(&*ctx.try_borrow().unwrap(), &mut input);
        if result == ShardStoreStatusCode::Ok {
            return Ok(Some(input.into()));
        } else if result == ShardStoreStatusCode::None {
            return Ok(Option::None);
        }
        return Err(Error::ShardTreeError);
    }

    fn add_checkpoint(
        &mut self,
        checkpoint_id: Self::CheckpointId,
        checkpoint: Checkpoint,
    ) -> Result<(), Self::Error> {
        let ctx = self.native_context.clone();
        let ffi_checkpoint_id : u32 = checkpoint_id.try_into().map_err(|_| Error::ShardTreeError)?;
        let result = orchard_add_checkpoint(ctx.try_borrow_mut().unwrap().pin_mut(), ffi_checkpoint_id, &FfiCheckpoint::try_from(&checkpoint)?);
        if result == ShardStoreStatusCode::Ok {
          return Ok(());
        }
        return Err(Error::ShardTreeError);
    }

    fn checkpoint_count(&self) -> Result<usize, Self::Error> {
        let ctx = self.native_context.clone();
        let mut input : usize = 0;
        let result = orchard_checkpoint_count(&*ctx.try_borrow().unwrap(), &mut input);
        if result == ShardStoreStatusCode::Ok {
            return Ok(input.into());
        } else if result == ShardStoreStatusCode::None {
            return Ok(0);
        }
        return Ok(0);
    }

    fn get_checkpoint_at_depth(
        &self,
        checkpoint_depth: usize,
    ) -> Result<Option<(Self::CheckpointId, Checkpoint)>, Self::Error> {
        let ctx = self.native_context.clone();
        let mut input_checkpoint_id : u32 = 0;
        let mut input_checkpoint : FfiCheckpoint = FfiCheckpoint::default();

        let result = orchard_get_checkpoint_at_depth(&*ctx.try_borrow().unwrap(),
            checkpoint_depth,
            &mut input_checkpoint_id,
            &mut input_checkpoint);
        
        if result == ShardStoreStatusCode::Ok {
            return Ok(Some((BlockHeight::from(input_checkpoint_id), Checkpoint::from(&input_checkpoint))));
        } else if result == ShardStoreStatusCode::None {
            return Ok(Option::None);
        }
        return Ok(Option::None);
    }

    fn get_checkpoint(
        &self,
        checkpoint_id: &Self::CheckpointId,
    ) -> Result<Option<Checkpoint>, Self::Error> {
        let ctx = self.native_context.clone();
        let mut input_checkpoint : FfiCheckpoint = FfiCheckpoint::default();

        let result = orchard_get_checkpoint(&*ctx.try_borrow().unwrap(),
            (*checkpoint_id).into(),
            &mut input_checkpoint);

        if result == ShardStoreStatusCode::Ok {
            return Ok(Some(Checkpoint::from(&input_checkpoint)));
        } else if result == ShardStoreStatusCode::None {
            return Ok(Option::None);
        }
        return Ok(Option::None);
    }

    fn with_checkpoints<F>(&mut self, limit: usize, mut callback: F) -> Result<(), Self::Error>
    where
        F: FnMut(&Self::CheckpointId, &Checkpoint) -> Result<(), Self::Error>,
    {
        let ctx = self.native_context.clone();
        let mut into : Vec<FfiCheckpoint> = vec![];
        let result = orchard_get_checkpoints(&*ctx.try_borrow().unwrap(), limit, &mut into);
        if result == ShardStoreStatusCode::Ok {
            for item in into {
                let checkpoint = Checkpoint::from(&item);
                callback(&BlockHeight::from(item.position), &checkpoint).map_err(|_| Error::ShardTreeError)?;
            }
            return Ok(())
        } else if result == ShardStoreStatusCode::None {
            return Ok(())
        }
        Err(Error::ShardTreeError)
    }

    fn update_checkpoint_with<F>(
        &mut self,
        checkpoint_id: &Self::CheckpointId,
        update: F,
    ) -> Result<bool, Self::Error>
    where
        F: Fn(&mut Checkpoint) -> Result<(), Self::Error>,
    {
        let ctx = self.native_context.clone();
        let mut input_checkpoint = FfiCheckpoint::default();
        let result_get_checkpoint = orchard_get_checkpoint(&*ctx.try_borrow().unwrap(), (*checkpoint_id).into(), &mut input_checkpoint);
        if result_get_checkpoint == ShardStoreStatusCode::Ok {
            return Ok(true);
        } else if result_get_checkpoint == ShardStoreStatusCode::None {
            return Ok(false);
        }

        let mut checkpoint = Checkpoint::from(&input_checkpoint);

        update(&mut checkpoint).map_err(|_| Error::ShardTreeError)?;
        let result_update_checkpoint = orchard_update_checkpoint(ctx.try_borrow_mut().unwrap().pin_mut(), (*checkpoint_id).into(), &FfiCheckpoint::try_from(&checkpoint)?);
        if result_update_checkpoint == ShardStoreStatusCode::Ok {
            return Ok(true);
        } else if result_update_checkpoint == ShardStoreStatusCode::None {
            return Ok(false);
        }
        return Err(Error::ShardTreeError);
    }

    fn remove_checkpoint(&mut self, checkpoint_id: &Self::CheckpointId) -> Result<(), Self::Error> {
        let ctx = self.native_context.clone();
        let result = orchard_remove_checkpoint(ctx.try_borrow_mut().unwrap().pin_mut(), (*checkpoint_id).into());
        if result == ShardStoreStatusCode::Ok {
            return Ok(());
        } else if result == ShardStoreStatusCode::None {
            return Ok(());
        }
        return Err(Error::ShardTreeError);
    }

    fn truncate_checkpoints(
        &mut self,
        checkpoint_id: &Self::CheckpointId,
    ) -> Result<(), Self::Error> {
        let ctx = self.native_context.clone();
        let result = orchard_truncate_checkpoint(ctx.try_borrow_mut().unwrap().pin_mut(), (*checkpoint_id).into());
        if result == ShardStoreStatusCode::Ok {
            return Ok(());
        } else if result == ShardStoreStatusCode::None {
            return Ok(());
        }
        return Err(Error::ShardTreeError);
    }
}

fn create_shard_tree(context: UniquePtr<ShardStoreContext>) -> Box<OrchardShardTreeBundleResult> {
    let shard_store = OrchardCxxShardStoreImpl {
        native_context: Rc::new(RefCell::new(context)),
        _hash_type: Default::default()
    };
    let shardtree = ShardTree::new(shard_store, PRUNING_DEPTH.try_into().unwrap());
    Box::new(OrchardShardTreeBundleResult::from(Ok(OrchardShardTreeBundleValue{tree: shardtree})))
}