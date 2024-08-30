/* Copyright (c) 2024 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/browser/infobars/creative_search_result_ad_infobar_delegate.h"

#include <memory>

#include "brave/grit/brave_generated_resources.h"
#include "chrome/browser/infobars/confirm_infobar_creator.h"
#include "components/infobars/content/content_infobar_manager.h"
#include "components/infobars/core/infobar.h"
#include "components/strings/grit/components_strings.h"
#include "components/vector_icons/vector_icons.h"
#include "content/public/browser/web_contents.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/views/vector_icons.h"
#include "url/gurl.h"

// static
void CreativeSearchResultAdInfoBarDelegate::Create(
    content::WebContents* web_contents) {
  CHECK(web_contents);
  infobars::ContentInfoBarManager* infobar_manager =
      infobars::ContentInfoBarManager::FromWebContents(web_contents);
  if (!infobar_manager) {
    return;
  }

  infobar_manager->AddInfoBar(
      CreateConfirmInfoBar(std::unique_ptr<ConfirmInfoBarDelegate>(
          new CreativeSearchResultAdInfoBarDelegate())));
}

CreativeSearchResultAdInfoBarDelegate::CreativeSearchResultAdInfoBarDelegate() =
    default;

CreativeSearchResultAdInfoBarDelegate::
    ~CreativeSearchResultAdInfoBarDelegate() = default;

infobars::InfoBarDelegate::InfoBarIdentifier
CreativeSearchResultAdInfoBarDelegate::GetIdentifier() const {
  return CREATIVE_SEARCH_RESULT_AD_INFOBAR_DELEGATE;
}

const gfx::VectorIcon& CreativeSearchResultAdInfoBarDelegate::GetVectorIcon()
    const {
  return views::kInfoIcon;
}

std::u16string CreativeSearchResultAdInfoBarDelegate::GetMessageText() const {
  return l10n_util::GetStringUTF16(
      IDS_BRAVE_ADS_CREATIVE_SEARCH_RESULT_AD_INFOBAR_MESSAGE);
}

int CreativeSearchResultAdInfoBarDelegate::GetButtons() const {
  return BUTTON_NONE;
}

std::u16string CreativeSearchResultAdInfoBarDelegate::GetLinkText() const {
  return l10n_util::GetStringUTF16(IDS_LEARN_MORE);
}

GURL CreativeSearchResultAdInfoBarDelegate::GetLinkURL() const {
  return GURL("https://brave.com");
}
