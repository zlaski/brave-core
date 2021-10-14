use crate::{ffi, NativeClient, LOCAL_POOL};
use brave_rewards::{errors, KVClient, KVStore};
use tracing::debug;

impl KVClient<NativeClient> for NativeClient {
    fn get_store() -> Result<Self, errors::InternalError> {
        // FIXME
        return Ok(NativeClient());
    }
}

impl KVStore for NativeClient {
    fn purge(&mut self) -> Result<(), errors::InternalError> {
        ffi::shim_purge();
        Ok(())
    }
    fn set(&mut self, key: &str, value: &str) -> Result<(), errors::InternalError> {
        ffi::shim_set(key, value);
        Ok(())
    }
    fn get(&mut self, key: &str) -> Result<Option<String>, errors::InternalError> {
        let key_copy = key;
        let ret = ffi::shim_get(key_copy).to_string_lossy();
        Ok(if ret.len() > 0 {
            Some(ret.to_string())
        } else {
            None
        })
    }
}
