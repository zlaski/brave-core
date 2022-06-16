/* Copyright (c) 2019 The Brave Software Team. Distributed under the MPL2
 * license. This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_THIRD_PARTY_BLINK_RENDERER_CORE_BRAVE_PAGE_GRAPH_REQUESTS_REQUEST_TRACKER_H_
#define BRAVE_THIRD_PARTY_BLINK_RENDERER_CORE_BRAVE_PAGE_GRAPH_REQUESTS_REQUEST_TRACKER_H_

#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include "base/memory/ref_counted.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/types.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/utilities/response_metadata.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace brave_page_graph {

class NodeResource;
class Node;
class TrackedRequest;

struct TrackedRequestRecord : public base::RefCounted<TrackedRequestRecord> {
  std::unique_ptr<TrackedRequest> request;
  uint16_t num_complete_replies = 0;
  bool is_first_reply;
};

struct DocumentRequest {
  // Information available at request start
  InspectorId request_id;
  std::string url;
  bool is_main_frame;
  std::chrono::milliseconds start_timestamp;

  // Information available at response
  absl::optional<ResponseMetadata> response_metadata;

  // Information available at request completion
  int64_t size;
  std::chrono::milliseconds complete_timestamp;
};

class RequestTracker {
  friend class PageGraph;

 public:
  typedef std::pair<InspectorId, const NodeResource*> RequestKey;

  RequestTracker();
  ~RequestTracker();

  scoped_refptr<const TrackedRequestRecord> RegisterRequestStart(
      const InspectorId request_id,
      Node* const requester,
      NodeResource* const resource,
      const blink::ResourceType resource_type,
      const RequestType request_type);
  scoped_refptr<const TrackedRequestRecord> RegisterRequestComplete(
      const InspectorId request_id);
  scoped_refptr<const TrackedRequestRecord> RegisterRequestError(
      const InspectorId request_id);

  void RegisterDocumentRequestStart(const InspectorId request_id,
                                    const blink::DOMNodeId frame_id,
                                    const std::string& url,
                                    const bool is_main_frame,
                                    const std::chrono::milliseconds timestamp);
  void RegisterDocumentRequestComplete(
      const InspectorId request_id,
      const int64_t size,
      const std::chrono::milliseconds timestamp);
  DocumentRequest* GetDocumentRequestInfo(const InspectorId request_id);
  TrackedRequestRecord* GetTrackingRecord(const InspectorId request_id);

 private:
  std::map<InspectorId, scoped_refptr<TrackedRequestRecord>> tracked_requests_;

  std::map<blink::DOMNodeId, InspectorId> document_request_initiators_;
  std::map<InspectorId, DocumentRequest> document_requests_;

  // Returns the record from the above map, and cleans up the record
  // if the final requester has been responded to.
  scoped_refptr<const TrackedRequestRecord> ReturnTrackingRecord(
      const InspectorId request_id);

  // This structure is just included for debugging, to make sure the
  // assumptions built into this request tracking system (e.g. that
  // request ids will not repeat, etc.).
  std::map<InspectorId, const NodeResource*> completed_requests_;

  // These methods manage writing to and from the above structure.
  void AddTracedRequestToHistory(const TrackedRequest* request);
  // Checks to make sure that either 1) this request id hasn't been seen
  // before, or 2) that if its been seen before, its to the same Resource.
  void CheckTracedRequestAgainstHistory(const TrackedRequest* request);
};

}  // namespace brave_page_graph

#endif  // BRAVE_THIRD_PARTY_BLINK_RENDERER_CORE_BRAVE_PAGE_GRAPH_REQUESTS_REQUEST_TRACKER_H_
