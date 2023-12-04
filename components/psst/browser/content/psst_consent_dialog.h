/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_PSST_BROWSER_CONTENT_PSST_CONSENT_DIALOG_H_
#define BRAVE_COMPONENTS_PSST_BROWSER_CONTENT_PSST_CONSENT_DIALOG_H_

#include "ui/views/window/dialog_delegate.h"

class PsstConsentDialog : public views::DialogDelegateView {
 public:
  PsstConsentDialog();
  ~PsstConsentDialog() override;

  // views::DialogDelegateView:
  gfx::Size CalculatePreferredSize() const override;
  void WindowClosing() override;

 private:
  void DisableAdBlockForSite();
};

#endif  // BRAVE_COMPONENTS_PSST_BROWSER_CONTENT_PSST_CONSENT_DIALOG_H_
