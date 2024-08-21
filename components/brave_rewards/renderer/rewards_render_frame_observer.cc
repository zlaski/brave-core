// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#include "brave/components/brave_rewards/renderer/rewards_render_frame_observer.h"

#include <array>
#include <optional>
#include <utility>

#include "base/feature_list.h"
#include "base/functional/bind.h"
#include "brave/components/brave_rewards/common/features.h"
#include "brave/components/brave_rewards/common/publisher_utils.h"
#include "brave/components/brave_rewards/resources/grit/creator_detection_generated.h"
#include "content/public/renderer/render_frame.h"
#include "gin/function_template.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"
#include "third_party/blink/public/platform/platform.h"
#include "third_party/blink/public/platform/scheduler/web_agent_group_scheduler.h"
#include "third_party/blink/public/web/blink.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "third_party/blink/public/web/web_script_source.h"

namespace brave_rewards {

namespace {

v8::Isolate* GetIsolate(content::RenderFrame* render_frame) {
  DCHECK(render_frame);
  return render_frame->GetWebFrame()->GetAgentGroupScheduler()->Isolate();
}

v8::Local<v8::Context> GetMainWorldContext(content::RenderFrame* render_frame) {
  DCHECK(render_frame);
  return render_frame->GetWebFrame()->MainWorldScriptContext();
}

std::optional<std::string> GetDetectionScript(
    content::RenderFrame* render_frame) {
  if (!render_frame || !render_frame->IsMainFrame()) {
    return std::nullopt;
  }

  auto* web_frame = render_frame->GetWebFrame();
  DCHECK(web_frame);
  if (web_frame->IsProvisional()) {
    return std::nullopt;
  }

  GURL origin = url::Origin(web_frame->GetSecurityOrigin()).GetURL();
  if (!origin.is_valid() || !origin.SchemeIs(url::kHttpsScheme) ||
      !IsMediaPlatformURL(origin)) {
    return std::nullopt;
  }

  // TODO(zeparsing): Do we need to run this on a separate thread? Do we need to
  // optimize the loading of this string (or its JS compilation)?
  return blink::Platform::Current()->GetDataResourceString(
      IDR_CREATOR_DETECTION_CREATOR_DETECTION_BUNDLE_JS);
}

v8::MaybeLocal<v8::Function> CompileDetectionInitializer(
    v8::Local<v8::Context> context,
    std::string_view function_body) {
  v8::Context::Scope context_scope(context);
  v8::MicrotasksScope microtasks_scope(
      context, v8::MicrotasksScope::kDoNotRunMicrotasks);

  auto* isolate = context->GetIsolate();
  v8::ScriptCompiler::Source source(gin::StringToV8(isolate, function_body));

  v8::Local<v8::String> parameters[] = {
      gin::StringToV8(isolate, "setPageUpdatedCallback"),
      gin::StringToV8(isolate, "onCreatorDetected"),
      gin::StringToV8(isolate, "verboseLogging")};

  return v8::ScriptCompiler::CompileFunction(context, &source, 3, parameters);
}

template <typename... Args>
void CallFunctionWithArgs(v8::Local<v8::Context> context,
                          v8::Local<v8::Function> function,
                          Args... args) {
  v8::Context::Scope context_scope(context);
  v8::MicrotasksScope microtasks_scope(
      context, v8::MicrotasksScope::kDoNotRunMicrotasks);
  std::array<v8::Local<v8::Value>, sizeof...(Args)> arg_list = {
      args.template As<v8::Value>()...};
  std::ignore = function->Call(context, context->Global(), arg_list.size(),
                               arg_list.empty() ? nullptr : arg_list.data());
}

}  // namespace

RewardsRenderFrameObserver::RewardsRenderFrameObserver(
    content::RenderFrame* render_frame,
    int32_t world_id)
    : RenderFrameObserver(render_frame), world_id_(world_id) {}

RewardsRenderFrameObserver::~RewardsRenderFrameObserver() = default;

void RewardsRenderFrameObserver::DidCreateScriptContext(
    v8::Local<v8::Context> context,
    int32_t world_id) {
  if (world_id != world_id_) {
    return;
  }

  GetDetectionHost()->ShouldDetectCreator(
      base::BindOnce(&RewardsRenderFrameObserver::MaybeInjectDetectionScript,
                     weak_factory_.GetWeakPtr()));
}

void RewardsRenderFrameObserver::WillReleaseScriptContext(
    v8::Local<v8::Context> context,
    int32_t world_id) {
  if (world_id == world_id_) {
    page_updated_callback_.Reset();
  }
}

void RewardsRenderFrameObserver::DidFinishSameDocumentNavigation() {
  NotifyPageUpdated();
}

void RewardsRenderFrameObserver::OnDestruct() {
  delete this;
}

mojo::AssociatedRemote<mojom::CreatorDetectionHost>&
RewardsRenderFrameObserver::GetDetectionHost() {
  if (!detection_host_) {
    render_frame()->GetRemoteAssociatedInterfaces()->GetInterface(
        &detection_host_);
    detection_host_.reset_on_disconnect();
  }

  return detection_host_;
}

void RewardsRenderFrameObserver::MaybeInjectDetectionScript(
    bool should_detect) {
  if (!should_detect) {
    return;
  }

  if (!page_updated_callback_.IsEmpty()) {
    return;
  }

  auto* isolate = GetIsolate(render_frame());
  v8::Isolate::Scope isolate_scope(isolate);
  v8::HandleScope handle_scope(isolate);

  auto context = GetMainWorldContext(render_frame());
  if (context.IsEmpty()) {
    return;
  }

  auto detection_script = GetDetectionScript(render_frame());
  if (!detection_script) {
    return;
  }

  auto initializer = CompileDetectionInitializer(context, *detection_script);
  if (initializer.IsEmpty()) {
    return;
  }

  auto js_callback = [&](auto fn) {
    auto function_template = gin::CreateFunctionTemplate(
        isolate, base::BindRepeating(fn, weak_factory_.GetWeakPtr()));
    return function_template->GetFunction(context).ToLocalChecked();
  };

  CallFunctionWithArgs(
      context, initializer.ToLocalChecked(),
      js_callback(&RewardsRenderFrameObserver::SetPageUpdatedCallback),
      js_callback(&RewardsRenderFrameObserver::OnCreatorDetected),
      v8::Boolean::New(isolate, base::FeatureList::IsEnabled(
                                    features::kVerboseLoggingFeature)));

  DCHECK(!page_updated_callback_.IsEmpty());

  NotifyPageUpdated();
}

void RewardsRenderFrameObserver::NotifyPageUpdated() {
  if (page_updated_callback_.IsEmpty()) {
    return;
  }

  auto* isolate = GetIsolate(render_frame());
  v8::Isolate::Scope isolate_scope(isolate);
  v8::HandleScope handle_scope(isolate);

  auto context = GetMainWorldContext(render_frame());
  if (context.IsEmpty()) {
    return;
  }

  CallFunctionWithArgs(context, page_updated_callback_.Get(isolate));
}

void RewardsRenderFrameObserver::SetPageUpdatedCallback(gin::Arguments* args) {
  v8::HandleScope handle_scope(args->isolate());
  v8::Local<v8::Function> callback;
  if (!args->GetNext(&callback)) {
    args->ThrowError();
    return;
  }
  page_updated_callback_.Reset(args->isolate(), callback);
}

void RewardsRenderFrameObserver::OnCreatorDetected(
    const std::string& id,
    const std::string& name,
    const std::string& url,
    const std::string& image_url) {
  GetDetectionHost()->OnCreatorDetected(id, name, url, image_url);
}

}  // namespace brave_rewards
