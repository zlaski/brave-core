/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_PROFILES_BRAVE_SESSION_PROFILE_H_
#define BRAVE_BROWSER_PROFILES_BRAVE_SESSION_PROFILE_H_

#include <string>

#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/timer/timer.h"
#include "build/build_config.h"
#include "brave/browser/profiles/brave_profile.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "chrome/common/buildflags.h"

// The default profile implementation.
class BraveSessionProfile : public Profile,
                            public content::NotificationObserver {
 public:
  static BraveSessionProfile* CreateProfile(const base::FilePath& path,
                                            Delegate* delegate,
                                            CreateMode create_mode,
                                            Profile* parent);
  ~BraveSessionProfile() override;

  // content::BrowserContext implementation:
#if !defined(OS_ANDROID)
  std::unique_ptr<content::ZoomLevelDelegate> CreateZoomLevelDelegate(
      const base::FilePath& partition_path) override;
#endif
  base::FilePath GetPath() const override;
  content::DownloadManagerDelegate* GetDownloadManagerDelegate() override;
  content::ResourceContext* GetResourceContext() override;
  content::BrowserPluginGuestManager* GetGuestManager() override;
  storage::SpecialStoragePolicy* GetSpecialStoragePolicy() override;
  content::PushMessagingService* GetPushMessagingService() override;
  content::SSLHostStateDelegate* GetSSLHostStateDelegate() override;
  content::BrowsingDataRemoverDelegate* GetBrowsingDataRemoverDelegate()
      override;
  content::PermissionManager* GetPermissionManager() override;
  content::BackgroundFetchDelegate* GetBackgroundFetchDelegate() override;
  content::BackgroundSyncController* GetBackgroundSyncController() override;
  net::URLRequestContextGetter* CreateRequestContext(
      content::ProtocolHandlerMap* protocol_handlers,
      content::URLRequestInterceptorScopedVector request_interceptors) override;
  net::URLRequestContextGetter* CreateRequestContextForStoragePartition(
      const base::FilePath& partition_path,
      bool in_memory,
      content::ProtocolHandlerMap* protocol_handlers,
      content::URLRequestInterceptorScopedVector request_interceptors) override;
  net::URLRequestContextGetter* CreateMediaRequestContext() override;
  net::URLRequestContextGetter* CreateMediaRequestContextForStoragePartition(
      const base::FilePath& partition_path,
      bool in_memory) override;
  void RegisterInProcessServices(StaticServiceMap* services) override;
  std::string GetMediaDeviceIDSalt() override;

  // Profile implementation:
  scoped_refptr<base::SequencedTaskRunner> GetIOTaskRunner() override;
  std::string GetProfileUserName() const override;
  ProfileType GetProfileType() const override;
  bool IsOffTheRecord() const override;
  Profile* GetOffTheRecordProfile() override;
  void DestroyOffTheRecordProfile() override;
  bool HasOffTheRecordProfile() override;
  Profile* GetOriginalProfile() override;
  const Profile* GetOriginalProfile() const override;
  bool IsSupervised() const override;
  bool IsChild() const override;
  bool IsLegacySupervised() const override;
  ExtensionSpecialStoragePolicy* GetExtensionSpecialStoragePolicy() override;
  PrefService* GetPrefs() override;
  const PrefService* GetPrefs() const override;
// #if !defined(OS_ANDROID)
//   ChromeZoomLevelPrefs* GetZoomLevelPrefs() override;
// #endif
  PrefService* GetOffTheRecordPrefs() override;
  PrefService* GetReadOnlyOffTheRecordPrefs() override;
  net::URLRequestContextGetter* GetRequestContext() override;
  net::URLRequestContextGetter* GetRequestContextForExtensions() override;
  net::SSLConfigService* GetSSLConfigService() override;
  bool IsSameProfile(Profile* profile) override;
  base::Time GetStartTime() const override;
  base::FilePath last_selected_directory() override;
  void set_last_selected_directory(const base::FilePath& path) override;
  chrome_browser_net::Predictor* GetNetworkPredictor() override;
  GURL GetHomePage() override;
  bool WasCreatedByVersionOrLater(const std::string& version) override;
  void SetExitType(ExitType exit_type) override;
  ExitType GetLastSessionExitType() override;
  bool ShouldRestoreOldSessionCookies() override;
  bool ShouldPersistSessionCookies() override;

  void Observe(int type,
               const content::NotificationSource& source,
               const content::NotificationDetails& details) override;
 private:
  BraveSessionProfile(const base::FilePath& path,
                      Delegate* delegate,
                      CreateMode create_mode,
                      Profile* parent);
  void OnLoad(Delegate* delegate);

  base::FilePath path_;
  Profile* parent_;
  bool is_off_the_record_;
  base::Time start_time_;
  std::string media_device_id_salt_;
  content::NotificationRegistrar notification_registrar_;

  DISALLOW_COPY_AND_ASSIGN(BraveSessionProfile);
};

#endif  // BRAVE_BROWSER_PROFILES_BRAVE_SESSION_PROFILE_H_
