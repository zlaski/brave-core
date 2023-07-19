/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/leaked_credentials/leaked_credentials_api.h"
#include "brave/components/leaked_credentials/rs/cxx/src/lib.rs.h"
#include "third_party/rust/cxx/v1/crate/include/cxx.h"

/*#include "base/run_loop.h"
#include "net/base/net_errors.h"
#include "net/base/url_request.h"
#include "net/base/url_request_context.h"
#include "net/url_request/url_request_context_getter.h"
#include "net/url_request/url_request_context_builder.h"
#include "net/url_request/url_request_context_storage.h"
#include "net/url_request/url_fetcher.h"
#include "net/url_request/url_fetcher_delegate.h"*/

#include "services/network/public/cpp/resource_request.h"
#include "net/base/load_flags.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "url/gurl.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/storage_partition.h"

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

    std::vector<leaked_credentials::QueryParams> preprocess_queries(leaked_credentials::BaseParams base_params, 
                                                                     std::size_t n) {
        std::cout << "> Deriving full set of parameters." << std::endl;    
        leaked_credentials::ClientBucketParams cbp; // = ClientBucketParams.from (base_params)
        std::cout << "> Preprocessing " << n << " queries." << std::endl;
        return client_preproc_n_queries(cbp, n);
    }

    std::vector<leaked_credentials::QueryParams> client_preproc_n_queries(leaked_credentials::ClientBucketParams cbp, std::size_t n) {
        std::vector<leaked_credentials::QueryParams> query_params;
        for (std::size_t i = 0; i < n; i++) {
            query_params.push_back(QueryParams());  // TODO from clientbucketparams -> make another constructor for that
        }
        return query_params;
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

    LeakedCredentialsNetworkCalls::LeakedCredentialsNetworkCalls(
        scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory)
        : url_loader_factory_(std::move(url_loader_factory)) {}

    // This is a blocking request and might block the main thread.  
    void LeakedCredentialsNetworkCalls::send_blocking_get_request(const std::string& url) {
        // 1. URLLoaderFactory to use for request (either user profile oder system wide)
        // https://github.com/brave/brave-browser/wiki/Simple-Guide-to-SimpleURLLoader#deciding-on-a-urlloaderfactory

        // 2. Create network::ResourceRequest
        auto resource_request = std::make_unique<network::ResourceRequest>();
        resource_request->url = GURL(url);
        resource_request->method = "GET";

        // TODO check correct flags - does not compile
        //resource_request->load_flags = net::LOAD_DO_NOT_SEND_COOKIES | net::LOAD_DO_NOT_SAVE_COOKIES;
        // optional other headers -> services/network/public/cpp/resource_request.h

        // 3. Create a net::NetworkTrafficAnnotationTag
        net::NetworkTrafficAnnotationTag traffic_annotation =
            net::DefineNetworkTrafficAnnotation("leaked_credential_get_request", R"(
                semantics {
                sender: "Leaked Credentials Component"
                description:
                    "Test GET request"
                trigger:
                    "unittest"
                data: "None"
                destination: GOOGLE_OWNED_SERVICE
                }
                policy {
                cookies_allowed: NO
                setting: "This feature cannot be disabled by settings."
                chrome_policy {
                    BrowserNetworkTimeQueriesEnabled {
                        BrowserNetworkTimeQueriesEnabled: false
                    }
                }
                })");

        // 4. Create network::SimpleURLLoader and pass object 2/3
        simple_url_loader_ = network::SimpleURLLoader::Create(std::move(resource_request),
                                                              traffic_annotation);
        simple_url_loader_->SetAllowHttpErrorResults(true);
        // optional - set additional values

        // 5. For POST request attach data
        // not needed for GET request

        // 6. Call method from SimpleURLLoader to initiate network request
        simple_url_loader_->DownloadToStringOfUnboundedSizeUntilCrashAndDie(
            url_loader_factory_.get(),
            base::BindOnce(&LeakedCredentialsNetworkCalls::OnSimpleLoaderComplete, base::Unretained(this))
        );
    }

    void LeakedCredentialsNetworkCalls::OnSimpleLoaderComplete(std::unique_ptr<std::string> response_body) {
        // TODO does not compile - TODO implement correctly
        /*int response_code = -1;
        if (simple_url_loader_->ResponseInfo() &&
            simple_url_loader_->ResponseInfo()->headers) {
            response_code = simple_url_loader_->ResponseInfo()->headers->response_code();
        }

        std::cout << "Response code: " << response_code << std::endl;*/
        std::cout << "Response body: " << response_body.get() << std::endl;
        // TODO do something with the body
    }

    /*void LeakedCredentialsURLFetcherDelegate::OnURLFetchComplete(const net::URLFetcher* source) {
        run_loop_->Quit(); // Quit the run loop when the request is complete
        if (source->GetStatus().is_success()) {
            response_ = source->GetResponseAsString();
        } else {
            response_ = ""; // Request failed, set an appropriate value
        }
    }*/

    /*std::string LeakedCredentialsURLFetcherDelegate::WaitForResponse() {
        base::RunLoop run_loop;
        run_loop_ = &run_loop;

        run_loop.Run(); // Wait until the request is complete

        run_loop_ = nullptr;
        return response_;
    }*/

} // namespace leaked_credentials
