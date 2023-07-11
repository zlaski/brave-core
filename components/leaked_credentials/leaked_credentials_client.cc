/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/leaked_credentials/leaked_credentials_client.h"

#include <iostream>
#include <filesystem>
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
        // TODO implement

        // attempt to read parameters locally or retrieve from server
        std::string bmd_path = LOCAL_DATA_DIR + std::to_string(bucket_id);
        ClientLocalStorage cls;// = ClientLocalStorage();

        // todo file system check
        /*if (true == true) {
            std::cout << ">> Reading existing params from file for bucket: " << bucket_id << std::endl;
            // TODO json string to ClientLocalStorage
        } else {
            std::cout << "> Retrieving params to check credential with username=" << cred.username << "(bucket:" << bucket_id << ")" << std::endl;

            // ServerOfflineReponse

            std::cout << "> Deserializing response" << std::endl;
            // ClientLocalstorage
        }*/

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
            //TODO
            
        }

        // write local storage params back to file
        std::cout << "> Update local storage data" ;
        // TODO json to file ClientLocalStorage

        return 0;
    }

} // namespace leaked_credentials
