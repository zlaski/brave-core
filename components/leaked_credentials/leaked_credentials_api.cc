/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/leaked_credentials/leaked_credentials_api.h"
#include "brave/components/leaked_credentials/rs/cxx/src/lib.rs.h"
#include "third_party/rust/cxx/v1/crate/include/cxx.h"

#include "services/network/public/cpp/resource_request.h"
#include "net/base/load_flags.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "url/gurl.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/storage_partition.h"

#include "base/logging.h"
#include "base/json/json_reader.h"

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
        leaked_credentials::ClientBucketParams cbp; // = ClientBucketParams.from (base_params)  TODO FIX ME
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

    LocalHashPrefixTable::LocalHashPrefixTable() = default;
    LocalHashPrefixTable::LocalHashPrefixTable(const LocalHashPrefixTable& other) = default;
    LocalHashPrefixTable::~LocalHashPrefixTable() = default;

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

    LeakedCredentialsNetworkCalls::LeakedCredentialsNetworkCalls() = default;
    LeakedCredentialsNetworkCalls::~LeakedCredentialsNetworkCalls() = default;

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
        std::cout << "Response body: " << response_body.get() << std::endl;

        if (!response_body || response_body->empty()) {
            std::string error_str = net::ErrorToShortString(simple_url_loader_->NetError());
            LOG(ERROR) << "no response body. net error: " << error_str;
            return;
        }

        base::JSONReader::Result parsed_value =
            base::JSONReader::ReadAndReturnValueWithError(
                *response_body, base::JSONParserOptions::JSON_PARSE_RFC);
        if (!parsed_value.has_value() || !parsed_value.value().is_dict()) {
            LOG(ERROR) << "LeakedCredentials: failed to parse get request json: "
               << parsed_value.error().message;
            return;
        }

        base::Value::Dict& root = parsed_value->GetDict();
        absl::optional<int> value = root.FindInt("life");

        //std::cout << "value: " << value << std::endl;

        this->test = value.value_or(21);

        // TODO build struct

        // signal URL load complete for blocking requests
        SignalUrlLoadCompleted();
    }

    void LeakedCredentialsNetworkCalls::WaitForUrlLoadToComplete() {
        if (url_loaded_) {
            return;
        }

        run_loop_ = std::make_unique<base::RunLoop>();
        run_loop_->Run();
    }

    void LeakedCredentialsNetworkCalls::SignalUrlLoadCompleted() {
        url_loaded_ = true;
        if (run_loop_) {
            run_loop_->Quit();
        }
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

    KeywordIndexingMappingMessage KeywordIndexingMappingMessage::from_local_hpt(LocalHashPrefixTable hpt) {
        // TODO implement
        KeywordIndexingMappingMessage message = KeywordIndexingMappingMessage();
        return message;
    }

    OfflineResponseResult::OfflineResponseResult(BaseParams bp, KeywordIndexingMappingMessage kimm) {
        this->base_params_ = ""; //bp; TODO json to string + base64 encode
        this->kimm_ = kimm;
    }

    std::pair<BaseParams, LocalHashPrefixTable> OfflineResponseResult::deseralize_local_hpt() {
        if (this->kimm_.method_ != "local_hpt") {
            //throw exception   // TODO
        }
        std::vector<uint8_t> sbp; //=  TODO decode base params
        BaseParams bp; //= from json?

        LocalHashPrefixTable local_hpt; // TODO from json

        return std::make_pair(bp, local_hpt);
    }

    ServerOfflineResponse::ServerOfflineResponse() = default;
    ServerOfflineResponse::ServerOfflineResponse(const ServerOfflineResponse& other) = default;
    ServerOfflineResponse::~ServerOfflineResponse() = default;

    ServerOfflineResponse ServerOfflineResponse::from_local_hpt(BaseParams base_params, LocalHashPrefixTable hpt, std::size_t id) {
        std::string jsonrpc = "2.0";
        
        ServerOfflineResponse err_resp;
        err_resp.jsonrpc_ = jsonrpc;
        err_resp.id_ = id;
        err_resp.error_ = ResponseError();   // TODO responseerror.offline_internal()
        err_resp.result_ = {};

        KeywordIndexingMappingMessage kimm_res;
        /*try
        {
            // TODO FIX ME
            // = KeywordIndexingMappingMessage::from_local_hpt(hpt);
        }
        catch(const std::exception& e)
        {
            return err_resp;
        }*/

        OfflineResponseResult prr_res;
        /*try
        {
            prr_res = OfflineResponseResult(base_params, kimm_res);
        }
        catch(const std::exception& e)
        {
            return err_resp;
        }*/

        ServerOfflineResponse response;
        response.jsonrpc_ = jsonrpc;
        response.id_ = id;
        response.result_ = prr_res;
        response.error_ = {};

        return response;
    }

} // namespace leaked_credentials
