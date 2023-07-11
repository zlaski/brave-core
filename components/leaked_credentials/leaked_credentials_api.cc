/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/leaked_credentials/leaked_credentials_api.h"
#include "brave/components/leaked_credentials/rs/cxx/src/lib.rs.h"
#include "third_party/rust/cxx/v1/crate/include/cxx.h"

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

namespace leaked_credentials {

    std::string uint8_to_hex_string(const uint8_t *v, const int s) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (int i = 0; i < s; i++) {
            ss << std::hex << std::setw(2) << static_cast<int>(v[i]);
        }
        return ss.str();
    }

    // The `client_get_bucket_id` function returns the bucket index that
    // should be queried, based on the queried username and the
    // `bucket_prefix_len` which corresponds to the number of hex
    // characters that are used.
    std::size_t client_get_bucket_id(std::string &username, std::size_t bucket_prefix_len, uint32_t total_buckets) {
        std::vector<uint8_t> username_vector;
        rust::Vec<std::uint8_t> hashresult = leaked_credentials::sha256(username);
        std::copy(hashresult.begin(), hashresult.end(), std::back_inserter(username_vector));

        return static_cast<std::size_t>(get_mod_prefix(username_vector.data(), 
                                                       username_vector.size(), 
                                                       bucket_prefix_len, 
                                                       total_buckets));
    }

    uint32_t get_mod_prefix(uint8_t bytes[], std::size_t bytes_len, std::size_t hex_prefix_len, uint32_t bound) {
        std::string h = uint8_to_hex_string(bytes, bytes_len);
        uint64_t val = std::stoull(h.substr(0, hex_prefix_len), nullptr, 16);
        return static_cast<uint32_t>(val % bound);
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
