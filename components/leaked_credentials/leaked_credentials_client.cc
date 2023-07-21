/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/leaked_credentials/leaked_credentials_client.h"

#include "chrome/browser/net/system_network_context_manager.h"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace leaked_credentials {

    ClientLocalStorage::ClientLocalStorage() {
        // TODO inititalize elements
    }

    ClientLocalStorage::~ClientLocalStorage() {
        // TODO 
    }

    BucketInfo::BucketInfo(size_t id) {
        std::string instance_urls[] = INSTANCE_URLS;
        std::string base_url = "http://";
        if (id >= BUCKETS_PER_INSTANCE * (sizeof(instance_urls)/sizeof(instance_urls[0]))) {
            // TODO panic!
            std::cout << "Bad index mapping selected" << std::endl;
        }

        uint8_t i = 0;
        for (auto &instance: instance_urls) {
            if (id < BUCKETS_PER_INSTANCE * (i+1)) {
                base_url += instance; 
                break;
            }
            i++;
        }
        base_url += ":";
        base_url += INSTANCE_PORT;

        this->params_url_ = base_url + "/params";
        this->query_url_ = base_url + "/query";
    }

    int client() {
        Credential cred;
        cred.username_ = "test";
        cred.password_ = "password";
        cred.n_preprocess_ = 5;

        std::cout << "******* OFFLINE PHASE *******" ;
        std::size_t bucket_id = leaked_credentials::client_get_bucket_id(cred.username_, HEX_PREFIX_LEN, BUCKETS_TOTAL);
        std::cout << "bucket id: " << bucket_id << std::endl;
        BucketInfo bucket_info = BucketInfo(bucket_id);

        // If no params directory then create it
        std::filesystem::directory_entry entry{LOCAL_DATA_DIR};
        if (entry.exists() == false) {
            std::filesystem::create_directories(LOCAL_DATA_DIR);
        }

        // attempt to read parameters locally or retrieve from server
        ClientLocalStorage cls;// = ClientLocalStorage();   // TODO FIX ME
        std::string bmd_path = LOCAL_DATA_DIR + std::to_string(bucket_id);
        std::ifstream file(bmd_path);
        
        if (file.is_open()) {
            std::string content;
            std::string line;
            while (getline(file, line)) {
                content += line;
            }
            file.close();

            std::cout << ">> Reading existing params from file for bucket: " << bucket_id << std::endl;
            // TODO json string to ClientLocalStorage
        } else {
            std::cout << "> Retrieving params to check credential with username=" << cred.username_ 
            << "(bucket:" << bucket_id << ")" << std::endl;

            // ServerOfflineReponse
            // 1. send GET request TODO
            //scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory;
            // TODO where do I get the url_loader_factory from?
            //url_loader_factory = context->GetDefaultStoragePartition()->GetURLLoaderFactoryForBrowserProcess();
            LeakedCredentialsNetworkCalls network_request;// = LeakedCredentialsNetworkCalls(url_loader_factory);
            std::stringstream server_url;
            server_url << bucket_info.params_url_ << "/" << bucket_id;
            network_request.send_blocking_get_request(server_url.str());
            network_request.WaitForUrlLoadToComplete();

            // 3. Extract OfflineResponseResult
            ServerOfflineResponse sor;  // TODO construct from JSON result of GET request
            OfflineResponseResult result = sor.result_.value();

            std::cout << "> Deserializing response" << std::endl;
            std::pair<BaseParams, LocalHashPrefixTable> res = result.deseralize_local_hpt();

            cls.base_ = res.first;
            cls.local_hpt_ = res.second;
            cls.preprocessed_queries_ = {};
        }

        // online phase
        std::cout << "******* LOCAL ONLINE PHASE *******" ;
        std::cout << "> Checking credential locally" ;

        std::string data_to_hash = cred.username_ + cred.password_;
        // TODO FIX ME
        std::vector<std::size_t> indices;// = cls.local_hpt.get_indices(&reinterpret_cast<const uint8_t*>(&data_to_hash[0]));
        bool do_remote = true;
        if (indices.empty()) {
            std::cout << "> Credential not referenced as potential match, no query required" ;
            std::cout << "******* CREDENTIAL APPEARS SAFE *******" ;
            do_remote = false;
        } 

        // do remote check if required
        if (do_remote) {
            std::cout << "> Credential is a potential match, launching online query" ;

            // preprocess queries
            std::vector<QueryParams> query_params;
            if (cls.preprocessed_queries_.value().empty() == false) {
                // attempt to use existing preprocessed data
                if (cls.preprocessed_queries_.value().size() < indices.size()) {
                    // preprocess minimum amount of extra queries
                    std::cout << "> Have " << cls.preprocessed_queries_.value().size() <<  
                                 " preprocessed queries, but need " << indices.size() << "." << std::endl;

                    std::size_t n = cred.n_preprocess_ - cls.preprocessed_queries_.value().size();
                    std::vector<leaked_credentials::QueryParams> extra_qps = preprocess_queries(cls.base_, n);

                    std::vector<leaked_credentials::QueryParams> qps;
                    qps.reserve(cred.n_preprocess_);
                    qps.insert(qps.end(), cls.preprocessed_queries_.value().begin(), cls.preprocessed_queries_.value().end());
                    qps.insert(qps.end(), extra_qps.begin(), extra_qps.end());
                    query_params = qps;
                } else {
                    // use existing preprocessed data
                    std::cout << "> Using previously derived preprocessed query data." << std::endl;
                    query_params = cls.preprocessed_queries_.value();
                }
            } else {
                std::cout << "> Need to preprocess query parameters." << std::endl;
                // derive a whole new set of preprocessed queries
                std::size_t n = 0;
                if (cred.n_preprocess_ < indices.size()) {
                    n = indices.size();
                } else {
                    n = cred.n_preprocess_;
                }
                
                query_params = preprocess_queries(cls.base_, n);
            }

            std::cout << "******* REMOTE ONLINE PHASE *******" << std::endl;
            std::vector<QueryParams> params_to_use (query_params.begin(), query_params.begin() + indices.size());
            std::vector<QueryParams> unused_params (query_params.begin() + indices.size(), query_params.end());
            
            // TODO client_prepare_queries
            // function needs to be in rust as it contains a lot of cryptography code

            // TODO ClientOnlineRequest

            std::cout << "> Sending query to " << bucket_info.query_url_ << std::endl;

            // TODO chromium networking stuff

            std::cout << "> Parsing results from server" << std::endl;
            // TODO client_process_output
            // TODO the function need to be in rust as it contains a lot of cryptography code

            // update preprocessed query data to remove used parameters
            cls.preprocessed_queries_ = unused_params;
        }

        // write local storage params back to file
        std::cout << "> Update local storage data" ;
        // TODO json to file ClientLocalStorage

        return 0;
    }

} // namespace leaked_credentials
