// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef BRAVE_COMPONENTS_BRAVE_REWARDS_RENDERER_REWARDS_RENDER_FRAME_OBSERVER_H_
#define BRAVE_COMPONENTS_BRAVE_REWARDS_RENDERER_REWARDS_RENDER_FRAME_OBSERVER_H_

#include <string>
#include <string_view>
#include <vector>

#include "base/memory/weak_ptr.h"
#include "brave/components/brave_rewards/common/mojom/rewards.mojom.h"
#include "content/public/renderer/render_frame_observer.h"
#include "mojo/public/cpp/bindings/associated_remote.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"
#include "v8/include/v8.h"

namespace gin {
class Arguments;
}

namespace brave_rewards {

class RewardsRenderFrameObserver : public content::RenderFrameObserver {
 public:
  RewardsRenderFrameObserver(content::RenderFrame* render_frame,
                             int32_t world_id);

  // RenderFrameObserver:
  void DidCreateScriptContext(v8::Local<v8::Context> context,
                              int32_t world_id) override;
  void WillReleaseScriptContext(v8::Local<v8::Context> context,
                                int32_t world_id) override;
  void DidFinishSameDocumentNavigation() override;
  void OnDestruct() override;

 private:
  ~RewardsRenderFrameObserver() override;

  mojo::AssociatedRemote<mojom::CreatorDetectionHost>& GetDetectionHost();

  void MaybeInjectDetectionScript(bool should_inject);

  void NotifyPageUpdated();

  void SetPageUpdatedCallback(gin::Arguments* args);

  void OnCreatorDetected(const std::string& id,
                         const std::string& name,
                         const std::string& url,
                         const std::string& image_url);

  int32_t world_id_ = 0;
  mojo::AssociatedRemote<mojom::CreatorDetectionHost> detection_host_;
  v8::Global<v8::Function> page_updated_callback_;
  base::WeakPtrFactory<RewardsRenderFrameObserver> weak_factory_{this};
};

}  // namespace brave_rewards

#endif  // BRAVE_COMPONENTS_BRAVE_REWARDS_RENDERER_REWARDS_RENDER_FRAME_OBSERVER_H_
