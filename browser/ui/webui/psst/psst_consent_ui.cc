/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/webui/psst/psst_consent_ui.h"

#include <memory>
#include <utility>

#include "base/feature_list.h"
#include "brave/browser/ui/webui/psst/psst_consent_page_handler.h"
#include "brave/components/brave_shields/resources/cookie_list_opt_in/grit/cookie_list_opt_in_generated_map.h"
#include "brave/components/psst/common/features.h"
// #include
// "brave/components/psst/resources/consent_opt_in/grit/consent_opt_in_generated_map.h"
#include "brave/components/constants/webui_url_constants.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/webui/favicon_source.h"
#include "chrome/browser/ui/webui/webui_util.h"
#include "components/favicon_base/favicon_url_parser.h"
#include "components/grit/brave_components_resources.h"
#include "components/grit/brave_components_strings.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"

namespace {

static constexpr webui::LocalizedString kStrings[] = {
    {"cookieListTitle", IDS_BRAVE_SHIELDS_COOKIE_LIST_TITLE},
    {"cookieListHeader", IDS_BRAVE_SHIELDS_COOKIE_LIST_HEADER},
    {"cookieListText", IDS_BRAVE_SHIELDS_COOKIE_LIST_TEXT},
    {"cookieListButtonText", IDS_BRAVE_SHIELDS_COOKIE_LIST_BUTTON_TEXT},
    {"cookieListNoThanks", IDS_BRAVE_SHIELDS_COOKIE_LIST_NO_THANKS}};

}  // namespace

PsstConsentUI::PsstConsentUI(content::WebUI* web_ui)
    : MojoBubbleWebUIController(web_ui, true) {
  auto* profile = Profile::FromWebUI(web_ui);

  auto* source = content::WebUIDataSource::CreateAndAdd(
      web_ui->GetWebContents()->GetBrowserContext(), kPsstConsentHost);
  source->AddLocalizedStrings(kStrings);

  webui::SetupWebUIDataSource(
      source,
      base::make_span(kCookieListOptInGenerated, kCookieListOptInGeneratedSize),
      IDR_COOKIE_LIST_OPT_IN_HTML);

  content::URLDataSource::Add(
      profile, std::make_unique<FaviconSource>(
                   profile, chrome::FaviconUrlFormat::kFavicon2));
}

PsstConsentUI::~PsstConsentUI() = default;

WEB_UI_CONTROLLER_TYPE_IMPL(PsstConsentUI)

void PsstConsentUI::BindInterface(
    mojo::PendingReceiver<PsstConsentPageHandlerFactory> receiver) {
  page_factory_receiver_.reset();
  page_factory_receiver_.Bind(std::move(receiver));
}

void PsstConsentUI::CreatePageHandler(
    mojo::PendingReceiver<psst::mojom::PsstConsentPageHandler> receiver) {
  page_handler_ = std::make_unique<PsstConsentPageHandler>(
      std::move(receiver), embedder(), Profile::FromWebUI(web_ui()));
}
