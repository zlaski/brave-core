/* Copyright (c) 2019 The Brave Software Team. Distributed under the MPL2
 * license. This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_THIRD_PARTY_BLINK_RENDERER_CORE_BRAVE_PAGE_GRAPH_TYPES_H_
#define BRAVE_THIRD_PARTY_BLINK_RENDERER_CORE_BRAVE_PAGE_GRAPH_TYPES_H_

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "third_party/blink/renderer/bindings/core/v8/script_source_location_type.h"
#include "third_party/blink/renderer/core/core_export.h"
#include "third_party/blink/renderer/platform/graphics/dom_node_id.h"
#include "third_party/blink/renderer/platform/weborigin/kurl.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"

namespace blink {
enum class ResourceType : uint8_t;
}  // namespace blink

namespace brave_page_graph {

class Edge;
class GraphItem;
class Node;
class NodeHTML;

typedef std::string ItemDesc;
typedef std::string ItemName;
typedef std::string GraphMLId;

using JSBuiltIn = std::string;

typedef const char* BindingType;
CORE_EXPORT extern const BindingType kBindingTypeAttribute;
CORE_EXPORT extern const BindingType kBindingTypeConstant;
CORE_EXPORT extern const BindingType kBindingTypeConstructor;
CORE_EXPORT extern const BindingType kBindingTypeMethod;

typedef const char* BindingEvent;
CORE_EXPORT extern const BindingEvent kBindingEventAttributeGet;
CORE_EXPORT extern const BindingEvent kBindingEventAttributeSet;
CORE_EXPORT extern const BindingEvent kBindingEventConstantGet;
CORE_EXPORT extern const BindingEvent kBindingEventConstructorCall;
CORE_EXPORT extern const BindingEvent kBindingEventMethodCall;

typedef const char* Binding;

typedef unsigned SourceCodeHash;
typedef unsigned UrlHash;
typedef int ScriptId;
typedef int ScriptPosition;
typedef int EventListenerId;
typedef uint64_t PageGraphId;
typedef std::string MethodName;
typedef std::string RequestURL;
typedef uint64_t InspectorId;

struct ScriptSource {
  blink::DOMNodeId node_id = blink::kInvalidDOMNodeId;
  ScriptId parent_script_id = 0;
  blink::KURL url;
  String function_name;
  blink::ScriptSourceLocationType location_type =
      blink::ScriptSourceLocationType::kUnknown;
  bool is_module = false;
  bool is_eval = false;

  bool operator==(const ScriptSource& rhs) const;
};

struct ScriptData {
  String code;
  ScriptSource source;

  bool operator==(const ScriptData& rhs) const;
};

typedef enum {
  kGraphMLAttrDefAttrName = 0,
  kGraphMLAttrDefBeforeNodeId,
  kGraphMLAttrDefBinding,
  kGraphMLAttrDefBindingEvent,
  kGraphMLAttrDefBindingType,
  kGraphMLAttrDefBlockType,
  kGraphMLAttrDefCallArgs,
  kGraphMLAttrDefEdgeType,
  kGraphMLAttrDefEventListenerId,
  kGraphMLAttrDefFrameId,
  kGraphMLAttrDefHost,
  kGraphMLAttrDefIncognito,
  kGraphMLAttrDefIsDeleted,
  kGraphMLAttrDefIsStyle,
  kGraphMLAttrDefKey,
  kGraphMLAttrDefMethodName,
  kGraphMLAttrDefNodeId,
  kGraphMLAttrDefNodeTag,
  kGraphMLAttrDefNodeText,
  kGraphMLAttrDefNodeType,
  kGraphMLAttrDefPageGraphEdgeId,
  kGraphMLAttrDefPageGraphNodeId,
  kGraphMLAttrDefPageGraphEdgeTimestamp,
  kGraphMLAttrDefPageGraphNodeTimestamp,
  kGraphMLAttrDefParentNodeId,
  kGraphMLAttrDefPrimaryPattern,
  kGraphMLAttrDefRequestId,
  kGraphMLAttrDefRequestType,
  kGraphMLAttrDefResourceType,
  kGraphMLAttrDefResponseHash,
  kGraphMLAttrDefRule,
  kGraphMLAttrDefScriptIdForEdge,
  kGraphMLAttrDefScriptIdForNode,
  kGraphMLAttrDefScriptPosition,
  kGraphMLAttrDefScriptType,
  kGraphMLAttrDefSecondaryPattern,
  kGraphMLAttrDefSource,
  kGraphMLAttrDefStatus,
  kGraphMLAttrDefSuccess,
  kGraphMLAttrDefURL,
  kGraphMLAttrDefValue,
  kGraphMLAttrDefUnknown,
  kGraphMLAttrDefSize,
  kGraphMLAttrDefHeaders,
} GraphMLAttrDef;

typedef enum {
  kGraphMLAttrTypeString = 0,
  kGraphMLAttrTypeBoolean,
  kGraphMLAttrTypeInt,
  kGraphMLAttrTypeFloat,
  kGraphMLAttrTypeDouble,
  kGraphMLAttrTypeUnknown
} GraphMLAttrType;
CORE_EXPORT std::string GraphMLAttrTypeToString(
    const GraphMLAttrType type) noexcept;

typedef enum {
  kGraphMLAttrForTypeNode = 0,
  kGraphMLAttrForTypeEdge,
  kGraphMLAttrForTypeUnknown
} GraphMLAttrForType;
CORE_EXPORT std::string GraphMLForTypeToString(
    const GraphMLAttrForType type) noexcept;

typedef enum {
  kRequestTypeAJAX = 0,
  kRequestTypeAudio,
  kRequestTypeCSS,
  kRequestTypeDocument,
  kRequestTypeFont,
  kRequestTypeImage,
  kRequestTypeScriptClassic,
  kRequestTypeScriptModule,
  kRequestTypeSubdocument,
  kRequestTypeSVG,
  kRequestTypeVideo,
  kRequestTypeUnknown
} RequestType;
CORE_EXPORT std::string RequestTypeToString(const RequestType type) noexcept;

CORE_EXPORT std::string ResourceTypeToString(
    const blink::ResourceType type) noexcept;

typedef enum {
  kRequestStatusStart = 0,
  kRequestStatusComplete,
  kRequestStatusError,
  kRequestStatusBlocked
} RequestStatus;
CORE_EXPORT std::string RequestStatusToString(
    const RequestStatus status) noexcept;

typedef enum {
  kStorageLocationCookie = 0,
  kStorageLocationLocalStorage,
  kStorageLocationSessionStorage
} StorageLocation;
CORE_EXPORT std::string StorageLocationToString(
    const StorageLocation location) noexcept;

struct CORE_EXPORT FingerprintingRule {
  const std::string& primary_pattern;
  const std::string& secondary_pattern;
  const std::string& source;
  const bool incognito;

  FingerprintingRule(const std::string& primary_pattern,
                     const std::string& secondary_pattern,
                     const std::string& source,
                     const bool incognito);

  bool operator==(const FingerprintingRule& other) const;
  bool operator<(const FingerprintingRule& other) const;
  bool operator>(const FingerprintingRule& other) const;

  std::string ToString() const;
};

typedef std::vector<const Edge*> EdgeList;
typedef std::vector<Node*> NodeList;
typedef std::vector<std::unique_ptr<const Edge>> EdgeUniquePtrList;
typedef std::vector<std::unique_ptr<Node>> NodeUniquePtrList;
typedef std::vector<const GraphItem*> GraphItemList;
typedef std::vector<NodeHTML*> HTMLNodeList;
typedef std::vector<blink::DOMNodeId> DOMNodeIdList;
typedef std::vector<ScriptId> ScriptIdList;
typedef std::map<const std::string, const std::string> AttributeMap;

typedef std::map<SourceCodeHash, ScriptId> HashToScriptIdMap;
typedef std::map<ScriptId, SourceCodeHash> ScriptIdToHashMap;
typedef std::map<SourceCodeHash, UrlHash> SourceToUrlMap;
typedef std::map<UrlHash, SourceCodeHash> UrlToSourceMap;

}  // namespace brave_page_graph

#endif  // BRAVE_THIRD_PARTY_BLINK_RENDERER_CORE_BRAVE_PAGE_GRAPH_TYPES_H_
