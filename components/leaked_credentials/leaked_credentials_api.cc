/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/leaked_credentials/leaked_credentials_api.h"

// TODO further includes here
#include <iostream>

namespace leaked_credentials {

    // The `client_get_bucket_id` function returns the bucket index that
    // should be queried, based on the queried username and the
    // `bucket_prefix_len` which corresponds to the number of hex
    // characters that are used.
    std::size_t client_get_bucket_id(std::string &username, std::size_t bucket_prefix_len, uint32_t total_buckets) {
        // TODO implement
        // calls get_mod_prefix
        return 0;
    }

    uint32_t get_mod_prefix(uint8_t bytes[], std::size_t hex_prefix_len, uint32_t bound) {
        // TODO implement
        return 0;
    }

    LocalHashPrefixTable::LocalHashPrefixTable() {
        //TODO implement
    }

    LocalHashPrefixTable::~LocalHashPrefixTable() {
        //TODO implement
    }

    LocalHashPrefixTable LocalHashPrefixTable::new_from_file(std::string path, uint32_t prefix_bit_len) {
        // TODO implement
        return LocalHashPrefixTable();
    }

    LocalHashPrefixTable LocalHashPrefixTable::load_from_external(std::vector<std::string> &vector, uint32_t prefix_bit_len) {
        // TODO implement
        return LocalHashPrefixTable();
    }

    uint32_t LocalHashPrefixTable::get_prefix(uint8_t bytes[], uint32_t prefix_bit_len) {
        // TODO implement
        return 0;
    }

    std::size_t LocalHashPrefixTable::len() {
        // TODO implement
        return 0;
    }

    bool LocalHashPrefixTable::is_empty() {
        // TODO implement
        return false;
    }

    std::vector<std::size_t> LocalHashPrefixTable::get_indices(uint8_t *keyword[]) {
        std::vector<std::size_t> indices;
        // TODO implement
        return indices;
    }

} // namespace leaked_credentials
