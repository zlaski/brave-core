/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_LEAKED_CREDENTIALS_API_H_
#define BRAVE_COMPONENTS_LEAKED_CREDENTIALS_API_H_

#include <vector>
#include <string>

namespace leaked_credentials {

  std::size_t client_get_bucket_id(std::string &username, std::size_t bucket_prefix_len, uint32_t total_buckets);
  uint32_t get_mod_prefix(uint8_t bytes[], std::size_t bytes_len, std::size_t hex_prefix_len, uint32_t bound);

  class LocalHashPrefixTable {
      std::vector<uint32_t> prefixes;
      //uint32_t prefix_bit_len;

      public:
        LocalHashPrefixTable(); // BUCKET -> Shard -> FrodoPIR
        ~LocalHashPrefixTable();  // Destructor needed because of complex class error
        LocalHashPrefixTable new_from_file(std::string path, uint32_t prefix_bit_len);
        LocalHashPrefixTable load_from_external(std::vector<std::string> &vector, uint32_t prefix_bit_len);
        uint32_t get_prefix(uint8_t bytes[], uint32_t prefix_bit_len);
        std::size_t len();
        bool is_empty();
        std::vector<std::size_t> get_indices(uint8_t *keyword[]);
  };

// TODO remove?
  /*class BucketInfo {
    std::string params_url;
    std::string query_url;

    public:
      BucketInfo(std::size_t id);
  };*/

} // namespace leaked_credentials

#endif // BRAVE_COMPONENTS_LEAKED_CREDENTIALS_API_H_