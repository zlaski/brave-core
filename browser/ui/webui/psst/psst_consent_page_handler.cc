/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/webui/psst/psst_consent_page_handler.h"

#include <utility>

#include "base/metrics/histogram_functions.h"
#include "brave/browser/brave_browser_process.h"
#include "brave/components/psst/common/psst_consent.mojom.h"

PsstConsentPageHandler::PsstConsentPageHandler(
    mojo::PendingReceiver<psst::mojom::PsstConsentPageHandler> receiver,
    base::WeakPtr<ui::MojoBubbleWebUIController::Embedder> embedder,
    Profile* profile)
    : receiver_(this, std::move(receiver)),
      embedder_(embedder),
      profile_(profile) {
  DCHECK(embedder_);
  DCHECK(profile_);
}

PsstConsentPageHandler::~PsstConsentPageHandler() = default;

void PsstConsentPageHandler::ShowUI() {
  if (embedder_) {
    embedder_->ShowUI();
  }
}

void PsstConsentPageHandler::CloseUI() {
  if (embedder_) {
    embedder_->CloseUI();
  }
}

void PsstConsentPageHandler::OnUINoClicked() {
  std::cerr << " PsstConsentPageHandler::OnUINoClicked" << std::endl;
}

void PsstConsentPageHandler::OnUIYesClicked() {
  std::cerr << " PsstConsentPageHandler::OnUIYesClicked" << std::endl;
}
