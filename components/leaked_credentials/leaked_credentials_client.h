/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_LEAKED_CREDENTIALS_CLIENT_H_
#define BRAVE_COMPONENTS_LEAKED_CREDENTIALS_CLIENT_H_

#include "brave/components/leaked_credentials/leaked_credentials_api.h"
#include "brave/components/leaked_credentials/rs/cxx/src/lib.rs.h"
#include "third_party/rust/cxx/v1/crate/include/cxx.h"

#include <string>
#include <vector>
#include <optional>

namespace leaked_credentials {

    #define HEX_PREFIX_LEN 15
    #define BUCKETS_TOTAL 16
    #define BUCKETS_PER_INSTANCE 4
    #define LOCAL_DATA_DIR "bucket_metadata"
    #define INSTANCE_URLS {"ec2-54-184-23-71.us-west-2.compute.amazonaws.com", "ec2-52-89-9-75.us-west-2.compute.amazonaws.com", "ec2-54-201-141-198.us-west-2.compute.amazonaws.com","ec2-54-212-69-254.us-west-2.compute.amazonaws.com"}
    #define INSTANCE_PORT "8080"

    struct Credential {
        std::string username_;
        std::string password_;
        std::size_t n_preprocess_;
    }; 

    struct ClientLocalStorage {
        leaked_credentials::BaseParams base_;  
        leaked_credentials::LocalHashPrefixTable local_hpt_;
        std::optional<std::vector<leaked_credentials::QueryParams>> preprocessed_queries_;

        ClientLocalStorage(); 
        ~ClientLocalStorage();
    };

    class BucketInfo {
        public:
            std::string params_url_;
            std::string query_url_;

            BucketInfo(std::size_t id);
    };

    // TODO remove. should be the main function
    int client();

} // namespace leaked_credentials

#endif // BRAVE_COMPONENTS_LEAKED_CREDENTIALS_CLIENT_H_