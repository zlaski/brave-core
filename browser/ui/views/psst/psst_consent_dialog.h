/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_UI_VIEWS_PSST_PSST_CONSENT_DIALOG_H_
#define BRAVE_BROWSER_UI_VIEWS_PSST_PSST_CONSENT_DIALOG_H_

#include "ui/views/window/dialog_delegate.h"
#include "base/functional/callback.h"

class PsstConsentDialog : public views::DialogDelegateView {
 public:
  PsstConsentDialog(base::OnceCallback<void()> yes_callback, base::OnceCallback<void()> no_callback);
  ~PsstConsentDialog() override;

  // views::DialogDelegateView:
  gfx::Size CalculatePreferredSize() const override;
  void WindowClosing() override;

 private:
  void DisableAdBlockForSite();
};

#endif  // BRAVE_BROWSER_UI_VIEWS_PSST_PSST_CONSENT_DIALOG_H_
