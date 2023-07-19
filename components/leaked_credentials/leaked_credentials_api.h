/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_LEAKED_CREDENTIALS_API_H_
#define BRAVE_COMPONENTS_LEAKED_CREDENTIALS_API_H_

#include "brave/components/leaked_credentials/rs/cxx/src/lib.rs.h"
#include "third_party/rust/cxx/v1/crate/include/cxx.h"

/*#include "base/run_loop.h"
#include "net/url_request/url_fetcher.h"
#include "net/url_request/url_fetcher_delegate.h"*/

#include <vector>
#include <string>

//#include <memory>
#include "base/memory/scoped_refptr.h"

namespace network {
class SimpleURLLoader;
class SharedURLLoaderFactory;
}  // namespace network

namespace leaked_credentials {
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

  struct ClientBucketParams {
        leaked_credentials::BaseParams base; 
        leaked_credentials::CommonParams common;
  };

  /*class LeakedCredentialsURLFetcherDelegate: public net::URLFetcherDelegate {
    public:
        void OnURLFetchComplete(const net::URLFetcher* source) override;
        std::string WaitForResponse();

    private:
        base::RunLoop* run_loop_ = nullptr;
        std::string response_;
  };*/

  std::size_t client_get_bucket_id(std::string &username, std::size_t bucket_prefix_len, uint32_t total_buckets);
  uint32_t get_mod_prefix(uint8_t bytes[], std::size_t bytes_len, std::size_t hex_prefix_len, uint32_t bound);
  std::vector<leaked_credentials::QueryParams> preprocess_queries(leaked_credentials::BaseParams base_params, std::size_t n);
  std::vector<leaked_credentials::QueryParams> client_preproc_n_queries(leaked_credentials::ClientBucketParams cbp, std::size_t n);

  
  // chromium network requests
  class LeakedCredentialsNetworkCalls {
      public:
          // Constructor
          LeakedCredentialsNetworkCalls(scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory);
          ~LeakedCredentialsNetworkCalls();

          // TODO description
          void send_blocking_get_request(const std::string& url);

      private: 
          scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory_;
          std::unique_ptr<network::SimpleURLLoader> simple_url_loader_;

          void OnSimpleLoaderComplete(std::unique_ptr<std::string> response_body);
  };

} // namespace leaked_credentials

#endif // BRAVE_COMPONENTS_LEAKED_CREDENTIALS_API_H_