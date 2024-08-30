/* Copyright (c) 2024 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_INFOBARS_CREATIVE_SEARCH_RESULT_AD_INFOBAR_DELEGATE_H_
#define BRAVE_BROWSER_INFOBARS_CREATIVE_SEARCH_RESULT_AD_INFOBAR_DELEGATE_H_

#include "components/infobars/core/confirm_infobar_delegate.h"

namespace content {
class WebContents;
}  // namespace content

class CreativeSearchResultAdInfoBarDelegate : public ConfirmInfoBarDelegate {
 public:
  CreativeSearchResultAdInfoBarDelegate();
  CreativeSearchResultAdInfoBarDelegate(
      const CreativeSearchResultAdInfoBarDelegate&) = delete;
  CreativeSearchResultAdInfoBarDelegate& operator=(
      const CreativeSearchResultAdInfoBarDelegate&) = delete;
  ~CreativeSearchResultAdInfoBarDelegate() override;

  static void Create(content::WebContents* web_contents);

 private:
  // ConfirmInfoBarDelegate:
  infobars::InfoBarDelegate::InfoBarIdentifier GetIdentifier() const override;
  const gfx::VectorIcon& GetVectorIcon() const override;
  std::u16string GetMessageText() const override;
  int GetButtons() const override;
  std::u16string GetLinkText() const override;
  GURL GetLinkURL() const override;
};

#endif  // BRAVE_BROWSER_INFOBARS_CREATIVE_SEARCH_RESULT_AD_INFOBAR_DELEGATE_H_
