/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_NTP_WIDGET_UTILS_BROWSER_NTP_WIDGET_UTILS_NETWORK_H_
#define BRAVE_COMPONENTS_NTP_WIDGET_UTILS_BROWSER_NTP_WIDGET_UTILS_NETWORK_H_

#include <list>
#include <map>
#include <memory>
#include <string>

#include "base/callback_forward.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "url/gurl.h"

namespace content {
class BrowserContext;
}  // namespace content

namespace network {
class SharedURLLoaderFactory;
class SimpleURLLoader;
}  // namespace network

const unsigned int retry_count_on_network_change = 1;

typedef std::map<std::string, std::string> RequestHeaders;

class NTPWidgetNetworkUtil {
 public:
  NTPWidgetNetworkUtil(const std::string& service_key,
                        content::BrowserContext* context);
  ~NTPWidgetNetworkUtil();

  using URLRequestCallback =
      base::OnceCallback<void(const int,
                              const std::string&,
                              const RequestHeaders&)>;

  bool NetworkRequest(const GURL &url,
                      const std::string& method,
                      const std::string& post_data,
                      bool auto_retry_on_network_change,
                      bool send_save_cookies,
                      bool use_json_encoding,
                      const RequestHeaders& headers,
                      URLRequestCallback callback);

 private:
  using SimpleURLLoaderList =
      std::list<std::unique_ptr<network::SimpleURLLoader>>;

  net::NetworkTrafficAnnotationTag GetNetworkTrafficAnnotationTag();

  void OnURLLoaderComplete(SimpleURLLoaderList::iterator iterator,
                           URLRequestCallback callback,
                           const std::unique_ptr<std::string> body);

  std::string service_key_;
  content::BrowserContext* context_;

  SimpleURLLoaderList url_loaders_;
};

#endif  // BRAVE_COMPONENTS_NTP_WIDGET_UTILS_BROWSER_NTP_WIDGET_UTILS_NETWORK_H_
