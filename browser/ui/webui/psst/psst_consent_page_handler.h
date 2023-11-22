/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_UI_WEBUI_PSST_PSST_CONSENT_PAGE_HANDLER_H_
#define BRAVE_BROWSER_UI_WEBUI_PSST_PSST_CONSENT_PAGE_HANDLER_H_

#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "brave/components/brave_shields/common/cookie_list_opt_in.mojom.h"
#include "brave/components/psst/common/psst_consent.mojom.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "ui/webui/mojo_bubble_web_ui_controller.h"

class Profile;

class PsstConsentPageHandler : public psst::mojom::PsstConsentPageHandler {
 public:
  PsstConsentPageHandler(
      mojo::PendingReceiver<psst::mojom::PsstConsentPageHandler> receiver,
      base::WeakPtr<ui::MojoBubbleWebUIController::Embedder> embedder,
      Profile* profile);

  PsstConsentPageHandler(const PsstConsentPageHandler&) = delete;
  PsstConsentPageHandler& operator=(const PsstConsentPageHandler&) = delete;

  ~PsstConsentPageHandler() override;

  // psst::mojom::PsstConsentPageHandler:
  void ShowUI() override;
  void CloseUI() override;

  void OnUINoClicked() override;
  void OnUIYesClicked() override;

 private:
  mojo::Receiver<psst::mojom::PsstConsentPageHandler> receiver_;
  base::WeakPtr<ui::MojoBubbleWebUIController::Embedder> embedder_;
  raw_ptr<Profile> profile_ = nullptr;
};

#endif  // BRAVE_BROWSER_UI_WEBUI_PSST_PSST_CONSENT_PAGE_HANDLER_H_
