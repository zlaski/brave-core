// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/web_view/internal/web_view_global_state_util.h"

#import <UIKit/UIKit.h>
#include <memory>

#include "ios/web/public/init/web_main.h"
#import "ios/web_view/internal/web_view_web_client.h"
#import "ios/web_view/internal/web_view_web_main_delegate.h"
#import "testing/coverage_util_ios.h"

namespace ios_web_view {

void InitializeGlobalState() {
  // Stub out everything!
  // It will all be handled in brave_core_main.mm
}

}  // namespace ios_web_view
