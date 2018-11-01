// Copyright 2018 The Brave Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/bookmarks/bookmark_context_menu_controller.h"

// Our .h file creates a masquerade for BookmarkContextMenuController.
// Switch back to the Chromium one for the Chromium implementation.
#undef BookmarkContextMenuController
#define BookmarkContextMenuController BookmarkContextMenuController_Chromium

#include "../../../../../../chrome/browser/ui/bookmarks/bookmark_context_menu_controller.cc"

// Make it clear which class we mean here.
#undef BookmarkContextMenuController

BraveBookmarkContextMenuController::BraveBookmarkContextMenuController(
      gfx::NativeWindow parent_window,
      BookmarkContextMenuControllerDelegate* delegate,
      Browser* browser,
      Profile* profile,
      content::PageNavigator* navigator,
      const bookmarks::BookmarkNode* parent,
      const std::vector<const bookmarks::BookmarkNode*>& selection)
  : BookmarkContextMenuController_Chromium(
      parent_window, delegate, browser, profile, navigator, parent,
      selection) {
}

void BookmarkContextMenuController_Chromium::AddBraveMenuItems1() {
}

void BookmarkContextMenuController_Chromium::AddBraveMenuItemsN(int count) {
}

void BraveBookmarkContextMenuController::AddBraveMenuItems1() {
  AddItem(IDC_BOOKMARK_BAR_OPEN_ALL_PRIVATE_TOR,
          IDS_BOOKMARK_BAR_OPEN_PRIVATE_TOR);
}

void BraveBookmarkContextMenuController::AddBraveMenuItemsN(int count) {
  AddItem(IDC_BOOKMARK_BAR_OPEN_ALL_PRIVATE_TOR,
          l10n_util::GetPluralStringFUTF16(
              IDS_BOOKMARK_BAR_OPEN_ALL_COUNT_PRIVATE_TOR, count));
}
