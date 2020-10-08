/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/ntp_widget_utils/browser/ntp_widget_utils_network.h"

#include <memory>
#include <string>
#include <utility>

#include "content/public/browser/browser_context.h"
#include "content/public/browser/storage_partition.h"
#include "net/base/load_flags.h"
#include "net/base/url_util.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"

NTPWidgetNetworkUtil::NTPWidgetNetworkUtil(
    const std::string& service_key,
    content::BrowserContext* context)
    : service_key_(service_key),
      context_(context) {
}

NTPWidgetNetworkUtil::~NTPWidgetNetworkUtil() {
}

net::NetworkTrafficAnnotationTag
NTPWidgetNetworkUtil::GetNetworkTrafficAnnotationTag() {
  return net::DefineNetworkTrafficAnnotation("ntp_widget", R"(
      semantics {
        sender: "ntp_widget"
        description:
        "This service is used to communicate with an exchange widget network"
        trigger:
        "Triggered by user connecting to the associated NTP widget."
        data:
        "May include account balances, deposit info, market data."
        destination: WEBSITE
      }
      policy {
        cookies_allowed: NO
        setting:
        "You can enable or disable this feature on the new tab page."
        policy_exception_justification:
        "Not implemented."
      }
    )");
}

bool NTPWidgetNetworkUtil::NetworkRequest(const GURL &url,
                                          const std::string& method,
                                          const std::string& post_data,
                                          bool auto_retry_on_network_change,
                                          bool send_save_cookies,
                                          bool use_json_encoding,
                                          const RequestHeaders& headers,
                                          URLRequestCallback callback) {
  auto request = std::make_unique<network::ResourceRequest>();
  auto url_loader = network::SimpleURLLoader::Create(
      std::move(request), GetNetworkTrafficAnnotationTag());

  request->url = url;
  request->method = method;
  request->load_flags = net::LOAD_BYPASS_CACHE |
                        net::LOAD_DISABLE_CACHE;

  if (!send_save_cookies) {
    request->load_flags |= net::LOAD_DO_NOT_SEND_COOKIES;
    request->load_flags |= net::LOAD_DO_NOT_SAVE_COOKIES;
  }

  for (const auto& header : headers) {
    request->headers.SetHeader(header.first, header.second);
  }

  if (!post_data.empty()) {
    const std::string encoding = use_json_encoding ?
        "application/json" : "application/x-www-form-urlencoded";
    url_loader->AttachStringForUpload(post_data, encoding);
  }

  url_loader->SetRetryOptions(
      retry_count_on_network_change,
      auto_retry_on_network_change ?
          network::SimpleURLLoader::RetryMode::RETRY_ON_NETWORK_CHANGE :
          network::SimpleURLLoader::RetryMode::RETRY_NEVER);

  auto iterator =
      url_loaders_.insert(url_loaders_.begin(), std::move(url_loader));
  auto* default_storage_partition =
      content::BrowserContext::GetDefaultStoragePartition(context_);
  auto* url_loader_factory =
      default_storage_partition->GetURLLoaderFactoryForBrowserProcess().get();

  iterator->get()->DownloadToStringOfUnboundedSizeUntilCrashAndDie(
      url_loader_factory, base::BindOnce(
          &NTPWidgetNetworkUtil::OnURLLoaderComplete,
          base::Unretained(this), std::move(iterator), std::move(callback)));

  return true;
}

void NTPWidgetNetworkUtil::OnURLLoaderComplete(
    SimpleURLLoaderList::iterator iterator,
    URLRequestCallback callback,
    const std::unique_ptr<std::string> body) {
  RequestHeaders headers;
  auto* loader = iterator->get();
  auto headers_list = loader->ResponseInfo()->headers;
  const std::string response_body = body ? *body : std::string();

  if (!headers_list) {
    url_loaders_.erase(iterator);
    std::move(callback).Run(-1, response_body, headers);
    return;
  }

  std::string key;
  std::string value;
  size_t index = 0;
  while (headers_list->EnumerateHeaderLines(&index, &key, &value)) {
    key = base::ToLowerASCII(key);
    headers[key] = value;
  }

  url_loaders_.erase(iterator);
  std::move(callback).Run(headers_list->response_code(),
      response_body, headers);
}
