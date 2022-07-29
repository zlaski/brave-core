/* Copyright (c) 2019 The Brave Software Team. Distributed under the MPL2
 * license. This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/third_party/blink/renderer/core/brave_page_graph/types.h"
#include <map>
#include <string>
#include "brave/third_party/blink/renderer/core/brave_page_graph/logging.h"
#include "third_party/blink/renderer/core/dom/dom_node_ids.h"
#include "third_party/blink/renderer/platform/loader/fetch/resource.h"

namespace brave_page_graph {

std::string GraphMLAttrTypeToString(const GraphMLAttrType type) noexcept {
  switch (type) {
    case kGraphMLAttrTypeString:
      return "string";
    case kGraphMLAttrTypeBoolean:
      return "boolean";
    case kGraphMLAttrTypeInt:
      return "int";
    case kGraphMLAttrTypeFloat:
      return "float";
    case kGraphMLAttrTypeDouble:
      return "double";
    case kGraphMLAttrTypeUnknown:
    default:
      return "unknown";
  }
}

std::string GraphMLForTypeToString(const GraphMLAttrForType type) noexcept {
  switch (type) {
    case kGraphMLAttrForTypeNode:
      return "node";
    case kGraphMLAttrForTypeEdge:
      return "edge";
    case kGraphMLAttrForTypeUnknown:
    default:
      return "unknown";
  }
}

std::string RequestTypeToString(const RequestType type) noexcept {
  switch (type) {
    case kRequestTypeAJAX:
      return "AJAX";
    case kRequestTypeImage:
      return "Image";
    case kRequestTypeScriptClassic:
      return "ScriptClassic";
    case kRequestTypeScriptModule:
      return "ScriptModule";
    case kRequestTypeCSS:
      return "CSS";
    case kRequestTypeVideo:
      return "Video";
    case kRequestTypeAudio:
      return "Audio";
    case kRequestTypeSVG:
      return "SVG";
    case kRequestTypeFont:
      return "Font";
    case kRequestTypeDocument:
      return "Document";
    case kRequestTypeSubdocument:
      return "Subdocument";
    case kRequestTypeUnknown:
    default:
      return "Unknown";
  }
}

std::string ResourceTypeToString(const blink::ResourceType type) noexcept {
  switch (type) {
    case blink::ResourceType::kImage:
      return "image";
    case blink::ResourceType::kCSSStyleSheet:
      return "css";
    case blink::ResourceType::kScript:
      return "script";
    case blink::ResourceType::kFont:
      return "font";
    case blink::ResourceType::kRaw:
      return "raw";
    case blink::ResourceType::kSVGDocument:
      return "svg";
    case blink::ResourceType::kXSLStyleSheet:
      return "XSL style sheet";
    case blink::ResourceType::kLinkPrefetch:
      return "link prefetch";
    case blink::ResourceType::kTextTrack:
      return "text track";
    case blink::ResourceType::kAudio:
      return "audio";
    case blink::ResourceType::kVideo:
      return "video";
    case blink::ResourceType::kManifest:
      return "manifest";
    case blink::ResourceType::kMock:
      return "mock";
  }
}

std::string RequestStatusToString(const RequestStatus status) noexcept {
  switch (status) {
    case kRequestStatusStart:
      return "started";
    case kRequestStatusComplete:
      return "complete";
    case kRequestStatusError:
      return "error";
    case kRequestStatusBlocked:
      return "blocked";
  }
}

std::string StorageLocationToString(const StorageLocation location) noexcept {
  switch (location) {
    case kStorageLocationCookie:
      return "cookie";
    case kStorageLocationLocalStorage:
      return "localStorage";
    case kStorageLocationSessionStorage:
      return "sessionStorage";
  }
}

FingerprintingRule::FingerprintingRule(const std::string& primary_pattern,
                                       const std::string& secondary_pattern,
                                       const std::string& source,
                                       const bool incognito)
    : primary_pattern(primary_pattern),
      secondary_pattern(secondary_pattern),
      source(source),
      incognito(incognito) {}

bool FingerprintingRule::operator==(const FingerprintingRule& other) const {
  return primary_pattern == other.primary_pattern &&
         secondary_pattern == other.secondary_pattern &&
         source == other.source && incognito == other.incognito;
}

bool FingerprintingRule::operator<(const FingerprintingRule& other) const {
  return primary_pattern < other.primary_pattern &&
         secondary_pattern < other.secondary_pattern && source < other.source &&
         incognito < other.incognito;
}

bool FingerprintingRule::operator>(const FingerprintingRule& other) const {
  return primary_pattern > other.primary_pattern &&
         secondary_pattern > other.secondary_pattern && source > other.source &&
         incognito > other.incognito;
}

std::string FingerprintingRule::ToString() const {
  return "primary pattern: " + primary_pattern +
         ", secondary pattern: " + secondary_pattern + ", source: " + source +
         ", incognito: " + base::NumberToString(incognito);
}

bool ScriptSource::operator==(const ScriptSource& rhs) const {
  auto tie = [](const ScriptSource& v) {
    return std::tie(v.node_id, v.parent_script_id, v.url, v.function_name,
                    v.location_type, v.is_module, v.is_eval);
  };
  return tie(*this) == tie(rhs);
};

bool ScriptData::operator==(const ScriptData& rhs) const {
  auto tie = [](const ScriptData& v) { return std::tie(v.code, v.source); };
  return tie(*this) == tie(rhs);
}

}  // namespace brave_page_graph
