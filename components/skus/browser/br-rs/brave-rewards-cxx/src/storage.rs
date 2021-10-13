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
        let tmp = Ok(ffi::shim_get(key)
            .as_ref()
            .map(|v| v.to_string_lossy().to_string()));
        debug!("{:?}", tmp);
        tmp
    }
}
