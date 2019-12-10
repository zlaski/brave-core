/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 3.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/vendor/brave-ios/components/bookmarks/bookmarks_api.h"

#include "base/task/post_task.h"
#include "base/time/time.h"
#include "components/bookmarks/browser/bookmark_client.h"
#include "components/bookmarks/browser/bookmark_model.h"
#include "components/bookmarks/browser/bookmark_utils.h"
#include "components/prefs/pref_service.h"
#include "components/sync_bookmarks/bookmark_sync_service.h"
#include "components/undo/bookmark_undo_service.h"
#include "ios/web/public/thread/web_thread.h"
#include "ios/web/public/thread/web_task_traits.h"
#include "url/gurl.h"

namespace bookmarks {

BookmarksAPI::BookmarksAPI(PrefService* prefs,
    const base::FilePath& state_path,
    const scoped_refptr<base::SequencedTaskRunner>& io_task_runner,
    std::unique_ptr<BookmarkClient> client)
    : model_(new bookmarks::BookmarkModel(std::move(client))),
      bookmark_undo_service_(new BookmarkUndoService()),
      bookmark_sync_service_(
          new sync_bookmarks::BookmarkSyncService(
              bookmark_undo_service_.get())) {
  model_->Load(
      prefs,
      state_path,
      io_task_runner,
      base::CreateSingleThreadTaskRunner({web::WebThread::UI}));
}

BookmarksAPI::~BookmarksAPI() {}

void BookmarksAPI::Create(const int64_t& parent_id,
                          size_t index,
                          const base::string16& title,
                          const GURL& url) {
  const BookmarkNode* parent =
      bookmarks::GetBookmarkNodeByID(model_.get(), parent_id);

  DCHECK(parent);

  const BookmarkNode* node;
  if (url.is_valid()) {
    node = model_->AddURL(parent, index, title, url, nullptr);
  } else {
    node = model_->AddFolder(parent, index, title, nullptr);
    model_->SetDateFolderModified(parent, base::Time::Now());
  }

  DCHECK(node);
}

}  // namespace bookmarks
