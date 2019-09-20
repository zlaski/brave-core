/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/profiles/brave_profile_impl.h"

#include "base/task/post_task.h"
#include "brave/browser/profiles/profile_util.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/chrome_notification_types.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "content/public/browser/notification_source.h"
#include "extensions/buildflags/buildflags.h"

#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_registry_observer.h"
#include "extensions/browser/extension_prefs.h"
#endif

class BASE_EXPORT Data {
 public:
  virtual ~Data() = default;

  // Returns a copy of |this|; null if copy is not supported.
  virtual std::unique_ptr<Data> Clone();
};

#if BUILDFLAG(ENABLE_EXTENSIONS)
class BraveProfileImpl::SessionProfileExtensionRegistryObserver : public extensions::ExtensionRegistryObserver {
 public:
  SessionProfileExtensionRegistryObserver(content::BrowserContext* browser_context) :
      browser_context_(browser_context) {
    auto* registry = extensions::ExtensionRegistry::Get(browser_context);
    if (registry)
      registry->AddObserver(this);
  }

  ~SessionProfileExtensionRegistryObserver() override {
    auto* registry = extensions::ExtensionRegistry::Get(browser_context_);
    if (registry)
      registry->RemoveObserver(this);
  }

  void OnExtensionReady(content::BrowserContext* browser_context,
                                const extensions::Extension* extension) override {
    if (browser_context == browser_context_) {
      // TODO(bridiver) - need to deal with extension whitelist both
      // here and for extensions enabled on startup (from prefs)
      auto* registry = extensions::ExtensionRegistry::Get(browser_context_);
      if (registry)
        registry->AddEnabled(extension);
    }
  }

  void OnExtensionUnloaded(content::BrowserContext* browser_context,
                                   const extensions::Extension* extension,
                                   extensions::UnloadedExtensionReason reason) override {
    if (browser_context == browser_context_) {
      auto* registry = extensions::ExtensionRegistry::Get(browser_context_);
      if (registry)
        registry->RemoveEnabled(extension->id());
    }
  }

 private:
  content::BrowserContext* browser_context_;
  DISALLOW_COPY_AND_ASSIGN(SessionProfileExtensionRegistryObserver);
};
#endif

BraveProfileImpl::BraveProfileImpl(
    const base::FilePath& path,
    Delegate* delegate,
    CreateMode create_mode,
    scoped_refptr<base::SequencedTaskRunner> io_task_runner)
    : ProfileImpl(path, delegate, create_mode, io_task_runner),
      weak_ptr_factory_(this) {
  // In sessions profiles, prefs are created from the original profile like how
  // incognito profile works. By the time chromium start to observe prefs
  // initialization in ProfileImpl constructor for the async creation case,
  // prefs are already initialized and it's too late for the observer to get
  // the notification, so we manually trigger the OnPrefsLoaded here. For the
  // sync cases, OnPrefsLoaded will always be called at the end of ProfileImpl
  // constructor, so there is no need to trigger it here.
  //
  // This need to be posted instead of running directly here because we need to
  // finish the construction and register this profile first in ProfileManager
  // before OnPrefsLoaded is called, otherwise we would hit a DCHECK in
  // ProfileManager::OnProfileCreated which is called inside OnPrefsLoaded and
  // is expecting the profile_info is already added then.
  if (brave::IsSessionProfilePath(path) &&
      create_mode == CREATE_MODE_ASYNCHRONOUS) {
    auto* parent_profile = brave::CreateParentProfileData(this);

    notification_registrar_.Add(this,
                                chrome::NOTIFICATION_PROFILE_DESTROYED,
                                content::Source<Profile>(parent_profile));

#if BUILDFLAG(ENABLE_EXTENSIONS)
    if (!IsOffTheRecord()) {
      observer_.reset(
          new SessionProfileExtensionRegistryObserver(parent_profile));
    }
#endif

    base::PostTaskAndReply(
        FROM_HERE, base::DoNothing(),
        base::BindOnce(&ProfileImpl::OnPrefsLoaded,
                       weak_ptr_factory_.GetWeakPtr(), create_mode, true));
  }
}

BraveProfileImpl::~BraveProfileImpl() {}

void BraveProfileImpl::Observe(
    int type,
    const content::NotificationSource& source,
    const content::NotificationDetails& details) {
  switch (type) {
    case chrome::NOTIFICATION_PROFILE_DESTROYED: {
      // this only happens when a profile is deleted because the profile manager
      // ensures that session profiles are destroyed before their parents
      // passing false for `success` removes the profile from the info cache
      g_browser_process->profile_manager()->OnProfileCreated(
          this, false, false);
      break;
    }
    default: {
      NOTREACHED();  // Unexpected notification.
      break;
    }
  }
}
