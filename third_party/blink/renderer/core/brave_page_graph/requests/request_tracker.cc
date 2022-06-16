/* Copyright (c) 2019 The Brave Software Team. Distributed under the MPL2
 * license. This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/third_party/blink/renderer/core/brave_page_graph/requests/request_tracker.h"

#include <map>
#include <memory>
#include <string>
#include <utility>

#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/request/edge_request_start.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/node.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/node_resource.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/logging.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/requests/tracked_request.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/types.h"
#include "third_party/blink/renderer/platform/loader/fetch/resource.h"

using ::std::make_pair;
using ::std::make_unique;
using ::std::move;
using ::std::unique_ptr;

namespace brave_page_graph {

RequestTracker::RequestTracker() {}

RequestTracker::~RequestTracker() {}

scoped_refptr<const TrackedRequestRecord> RequestTracker::RegisterRequestStart(
    const InspectorId request_id,
    Node* const requester,
    NodeResource* const resource,
    const blink::ResourceType resource_type,
    const RequestType request_type) {
  if (tracked_requests_.count(request_id) == 0) {
    auto request_record = make_unique<TrackedRequest>(request_id, requester,
                                                      resource, request_type);
    CheckTracedRequestAgainstHistory(request_record.get());
    auto tracking_record = base::MakeRefCounted<TrackedRequestRecord>();
    tracking_record->request = move(request_record);
    tracked_requests_.emplace(request_id, tracking_record);
    return tracking_record;
  }

  tracked_requests_.at(request_id)
      ->request->AddRequest(requester, resource, resource_type, request_type);
  return ReturnTrackingRecord(request_id);
}

scoped_refptr<const TrackedRequestRecord>
RequestTracker::RegisterRequestComplete(const InspectorId request_id) {
  tracked_requests_.at(request_id)->request->SetCompleted();
  return ReturnTrackingRecord(request_id);
}

scoped_refptr<const TrackedRequestRecord> RequestTracker::RegisterRequestError(
    const InspectorId request_id) {
  tracked_requests_.at(request_id)->request->SetIsError();
  return ReturnTrackingRecord(request_id);
}

void RequestTracker::RegisterDocumentRequestStart(
    const InspectorId request_id,
    const blink::DOMNodeId frame_id,
    const std::string& url,
    const bool is_main_frame,
    const std::chrono::milliseconds timestamp) {
  // Any previous document requests from this root should have been canceled.
  if (document_request_initiators_.count(frame_id) != 0) {
    PG_LOG_ASSERT(document_request_initiators_.at(frame_id) == request_id);
    return;
  }

  // If we get to this point, there should be no previous request with this
  // request ID.
  PG_LOG_ASSERT(document_requests_.count(request_id) == 0);

  auto request_record = DocumentRequest{
      request_id,
      url,
      is_main_frame,
      timestamp,
      absl::nullopt,
      0,
      std::chrono::milliseconds::zero(),
  };
  document_request_initiators_.emplace(frame_id, request_id);
  document_requests_.emplace(request_id, request_record);
}

void RequestTracker::RegisterDocumentRequestComplete(
    const InspectorId request_id,
    const int64_t size,
    const std::chrono::milliseconds timestamp) {
  // The request should have been started previously.
  PG_LOG_ASSERT(document_requests_.count(request_id) != 0);

  auto& request_record = document_requests_.at(request_id);

  // The request should not have been completed previously.
  PG_LOG_ASSERT(request_record.size == 0);
  PG_LOG_ASSERT(request_record.complete_timestamp ==
                std::chrono::milliseconds::zero());

  request_record.size = size;
  request_record.complete_timestamp = timestamp;
}

DocumentRequest* RequestTracker::GetDocumentRequestInfo(
    const InspectorId request_id) {
  auto request_it = document_requests_.find(request_id);
  return request_it != document_requests_.end() ? &request_it->second : nullptr;
}

TrackedRequestRecord* RequestTracker::GetTrackingRecord(
    const InspectorId request_id) {
  auto record_it = tracked_requests_.find(request_id);
  return record_it != tracked_requests_.end() ? record_it->second.get()
                                              : nullptr;
}

scoped_refptr<const TrackedRequestRecord> RequestTracker::ReturnTrackingRecord(
    const InspectorId request_id) {
  auto record_it = tracked_requests_.find(request_id);
  auto& record = record_it->second;
  TrackedRequest* request = record->request.get();

  if (!request->IsComplete()) {
    return record_it->second;
  }

  const size_t num_requestors = request->GetRequesters().size();
  record->is_first_reply = (record->num_complete_replies == 0);
  record->num_complete_replies += 1;

  // If this is the last requester we need to reply, then
  // we want to set things up so that we loose our handle on the unique_ptr
  // on return.  Otherwise, we can just leave it in place.
  if (record->num_complete_replies < num_requestors) {
    return tracked_requests_.at(request_id);
  }

  AddTracedRequestToHistory(request);
  auto record_shared = std::move(record);
  tracked_requests_.erase(record_it);
  return record_shared;
}

void RequestTracker::AddTracedRequestToHistory(const TrackedRequest* request) {
  PG_LOG_ASSERT(request->GetRequestId() > 0);
  PG_LOG_ASSERT(request->GetResource() != nullptr);
  completed_requests_.emplace(request->GetRequestId(), request->GetResource());
}

void RequestTracker::CheckTracedRequestAgainstHistory(
    const TrackedRequest* request) {
  PG_LOG_ASSERT(completed_requests_.count(request->GetRequestId()) == 0 ||
                completed_requests_.at(request->GetRequestId()) ==
                    request->GetResource());
}

}  // namespace brave_page_graph
