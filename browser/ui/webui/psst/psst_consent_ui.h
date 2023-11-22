/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_UI_WEBUI_PSST_PSST_CONSENT_UI_H_
#define BRAVE_BROWSER_UI_WEBUI_PSST_PSST_CONSENT_UI_H_

#include <memory>

#include "brave/components/psst/common/psst_consent.mojom.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "ui/webui/mojo_bubble_web_ui_controller.h"

class PsstConsentUI : public ui::MojoBubbleWebUIController,
                      public psst::mojom::PsstConsentPageHandlerFactory {
 public:
  explicit PsstConsentUI(content::WebUI* web_ui);
  ~PsstConsentUI() override;

  PsstConsentUI(const PsstConsentUI&) = delete;
  PsstConsentUI& operator=(const PsstConsentUI&) = delete;

  void BindInterface(
      mojo::PendingReceiver<psst::mojom::PsstConsentPageHandlerFactory>
          receiver);

 private:
  // psst::mojom::PsstConsentPageHandlerFactory:
  void CreatePageHandler(
      mojo::PendingReceiver<psst::mojom::PsstConsentPageHandler> receiver)
      override;

  std::unique_ptr<psst::mojom::PsstConsentPageHandler> page_handler_;
  mojo::Receiver<PsstConsentPageHandlerFactory> page_factory_receiver_{this};

  WEB_UI_CONTROLLER_TYPE_DECL();
};

#endif  // BRAVE_BROWSER_UI_WEBUI_PSST_PSST_CONSENT_UI_H_
