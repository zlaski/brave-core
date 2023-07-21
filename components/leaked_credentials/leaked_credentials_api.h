/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_LEAKED_CREDENTIALS_API_H_
#define BRAVE_COMPONENTS_LEAKED_CREDENTIALS_API_H_

#include "base/run_loop.h"
#include "brave/components/leaked_credentials/rs/cxx/src/lib.rs.h"
#include "third_party/rust/cxx/v1/crate/include/cxx.h"

/*#include "base/run_loop.h"
#include "net/url_request/url_fetcher.h"
#include "net/url_request/url_fetcher_delegate.h"*/

#include <optional>
#include <vector>
#include <string>
#include <utility>

//#include <memory>
#include "base/memory/scoped_refptr.h"

namespace network {
    class SimpleURLLoader;
    class SharedURLLoaderFactory;
}  // namespace network

namespace leaked_credentials {
  class LocalHashPrefixTable {
      std::vector<uint32_t> prefixes_;
      //uint32_t prefix_bit_len_;

      public:
        LocalHashPrefixTable(); // BUCKET -> Shard -> FrodoPIR
        LocalHashPrefixTable(const LocalHashPrefixTable& other);
        ~LocalHashPrefixTable();  // Destructor needed because of complex class error
        LocalHashPrefixTable new_from_file(std::string path, uint32_t prefix_bit_len);
        LocalHashPrefixTable load_from_external(std::vector<std::string> &vector, uint32_t prefix_bit_len);
        uint32_t get_prefix(uint8_t bytes[], uint32_t prefix_bit_len);
        std::size_t len();
        bool is_empty();
        std::vector<std::size_t> get_indices(uint8_t *keyword[]);
  };

  struct ClientBucketParams {
        leaked_credentials::BaseParams base_; 
        leaked_credentials::CommonParams common_;
  };

  struct KeywordIndexingMappingMessage {
      std::string method_;
      std::string data_;

      KeywordIndexingMappingMessage from_local_hpt(LocalHashPrefixTable hpt);
  };

  struct OfflineResponseResult {
      std::string base_params_;  // base64-encoded base parameters
      KeywordIndexingMappingMessage kimm_;

      // explicit constructor
      OfflineResponseResult(BaseParams bp, KeywordIndexingMappingMessage kimm);
      OfflineResponseResult();

      std::pair<BaseParams, LocalHashPrefixTable> deseralize_local_hpt();
  };

  // TODO replace with explicit error class in C++?
  struct ResponseError {
      int32_t code_;
      std::string message_;
  };

  struct ServerOfflineResponse {
      std::string jsonrpc_;
      std::optional<OfflineResponseResult> result_;
      std::optional<ResponseError> error_;
      std::size_t id_;

      ServerOfflineResponse();
      ServerOfflineResponse(const ServerOfflineResponse& other);
      ~ServerOfflineResponse();
      ServerOfflineResponse from_local_hpt(BaseParams base_params, LocalHashPrefixTable hpt, std::size_t id);
  };



  // TODO remove
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

          // TODO remove default construtor
          LeakedCredentialsNetworkCalls();
          ~LeakedCredentialsNetworkCalls();

          void WaitForUrlLoadToComplete();

          // TODO description
          void send_blocking_get_request(const std::string& url);

          // should be private?
          void OnSimpleLoaderComplete(std::unique_ptr<std::string> response_body);

          int test = 0;

      private: 
          scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory_;
          std::unique_ptr<network::SimpleURLLoader> simple_url_loader_;

          std::unique_ptr<base::RunLoop> run_loop_;
          bool url_loaded_ = false;
          
          void SignalUrlLoadCompleted(); 
  };

} // namespace leaked_credentials

#endif // BRAVE_COMPONENTS_LEAKED_CREDENTIALS_API_H_