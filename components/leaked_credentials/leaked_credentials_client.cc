/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/leaked_credentials/leaked_credentials_client.h"

#include <iostream>
#include <filesystem>
#include <fstream>
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
        base_url += PORT;

        this->params_url = base_url + "/params";
        this->query_url = base_url + "/query";
    }

    int client() {
        Credential cred;
        cred.username = "test";
        cred.password = "password";
        cred.n_preprocess = 5;

        std::cout << "******* OFFLINE PHASE *******" ;
        std::size_t bucket_id = leaked_credentials::client_get_bucket_id(cred.username, HEX_PREFIX_LEN, BUCKETS_TOTAL);
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
            std::cout << "> Retrieving params to check credential with username=" << cred.username 
            << "(bucket:" << bucket_id << ")" << std::endl;

            // ServerOfflineReponse
            // TODO chromium networking part
            //TODO do request, extract response
            
            // 1. send GET request
            // 2. expect ServerOfflineResponse
            // 3. Extract OfflineResponseResult

            std::cout << "> Deserializing response" << std::endl;
            // ClientLocalstorage
        }

        // online phase
        std::cout << "******* LOCAL ONLINE PHASE *******" ;
        std::cout << "> Checking credential locally" ;

        std::string data_to_hash = cred.username + cred.password;
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
            if (cls.preprocessed_queries.empty() == false) {
                // attempt to use existing preprocessed data
                if (cls.preprocessed_queries.size() < indices.size()) {
                    // preprocess minimum amount of extra queries
                    std::cout << "> Have " << cls.preprocessed_queries.size() <<  
                                 " preprocessed queries, but need " << indices.size() << "." << std::endl;

                    std::size_t n = cred.n_preprocess - cls.preprocessed_queries.size();
                    std::vector<leaked_credentials::QueryParams> extra_qps = preprocess_queries(cls.base, n);

                    std::vector<leaked_credentials::QueryParams> qps;
                    qps.reserve(cred.n_preprocess);
                    qps.insert(qps.end(), cls.preprocessed_queries.begin(), cls.preprocessed_queries.end());
                    qps.insert(qps.end(), extra_qps.begin(), extra_qps.end());
                    query_params = qps;
                } else {
                    // use existing preprocessed data
                    std::cout << "> Using previously derived preprocessed query data." << std::endl;
                    query_params = cls.preprocessed_queries;
                }
            } else {
                std::cout << "> Need to preprocess query parameters." << std::endl;
                // derive a whole new set of preprocessed queries
                std::size_t n = 0;
                if (cred.n_preprocess < indices.size()) {
                    n = indices.size();
                } else {
                    n = cred.n_preprocess;
                }
                
                query_params = preprocess_queries(cls.base, n);
            }

            std::cout << "******* REMOTE ONLINE PHASE *******" << std::endl;
            std::vector<QueryParams> params_to_use (query_params.begin(), query_params.begin() + indices.size());
            std::vector<QueryParams> unused_params (query_params.begin() + indices.size(), query_params.end());
            
            // TODO client_prepare_queries
            // function needs to be in rust as it contains a lot of cryptography code

            // TODO ClientOnlineRequest

            std::cout << "> Sending query to " << bucket_info.query_url << std::endl;

            // TODO chromium networking stuff

            std::cout << "> Parsing results from server" << std::endl;
            // TODO client_process_output
            // TODO the function need to be in rust as it contains a lot of cryptography code

            // update preprocessed query data to remove used parameters
            cls.preprocessed_queries = unused_params;
        }

        // write local storage params back to file
        std::cout << "> Update local storage data" ;
        // TODO json to file ClientLocalStorage

        return 0;
    }

} // namespace leaked_credentials
