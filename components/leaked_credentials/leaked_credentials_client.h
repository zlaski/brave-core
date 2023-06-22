/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_LEAKED_CREDENTIALS_CLIENT_H_
#define BRAVE_COMPONENTS_LEAKED_CREDENTIALS_CLIENT_H_

#include "brave/components/leaked_credentials/leaked_credentials_api.h"
#include <string>

namespace leaked_credentials {

    #define HEX_PREFIX_LEN 15
    #define BUCKETS_TOTAL 16
    #define BUCKETS_PER_INSTANCE 4
    #define LOCAL_DATA_DIR "bucket_metadata"
    #define INSTANCE_URLS ["ec2-54-184-23-71.us-west-2.compute.amazonaws.com", "ec2-52-89-9-75.us-west-2.compute.amazonaws.com", "ec2-54-201-141-198.us-west-2.compute.amazonaws.com","ec2-54-212-69-254.us-west-2.compute.amazonaws.com"]
    #define PORT "8080"

    struct Credential {
        std::string username;
        std::string password;
        std::size_t n_preprocess;
    }; 

    struct ClientLocalStorage {
        // base BaseParams -> from FrodoPIR TODO
        leaked_credentials::LocalHashPrefixTable local_hpt;
        // preprocessed queries -> from FRODOPIR TODO
    };

    class BucketInfo {
        std::string params_url;
        std::string query_url;

        public:
            BucketInfo(std::size_t id);
    };

    // TODO remove. should be the main function
    int client();

} // namespace leaked_credentials

#endif // BRAVE_COMPONENTS_LEAKED_CREDENTIALS_CLIENT_H_