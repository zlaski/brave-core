// Copyright 2018 The Brave Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_BOOKMARKS_BOOKMARK_CONTEXT_MENU_CONTROLLER_H_OVERRIDE_
#define CHROME_BROWSER_UI_BOOKMARKS_BOOKMARK_CONTEXT_MENU_CONTROLLER_H_OVERRIDE_

// Get the Chromium declaration.
#define BookmarkContextMenuController BookmarkContextMenuController_Chromium
#include "../../../../../../chrome/browser/ui/bookmarks/bookmark_context_menu_controller.h"
#undef BookmarkContextMenuController

// Declare our own subclass with overridden methods.
class BraveBookmarkContextMenuController
    : public BookmarkContextMenuController_Chromium {
 public:
  BraveBookmarkContextMenuController(
      gfx::NativeWindow parent_window,
      BookmarkContextMenuControllerDelegate* delegate,
      Browser* browser,
      Profile* profile,
      content::PageNavigator* navigator,
      const bookmarks::BookmarkNode* parent,
      const std::vector<const bookmarks::BookmarkNode*>& selection);
 private:
  void AddBraveMenuItems1() override;
  void AddBraveMenuItemsN(int count) override;
};

// Use our own subclass as the real BookmarkContextMenuController.
#define BookmarkContextMenuController BraveBookmarkContextMenuController

#endif  // CHROME_BROWSER_UI_BOOKMARKS_BOOKMARK_CONTEXT_MENU_CONTROLLER_H_OVERRIDE_
