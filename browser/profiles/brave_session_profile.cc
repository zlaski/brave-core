/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/profiles/brave_session_profile.h"

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/path_service.h"
#include "chrome/browser/chrome_notification_types.h"
#include "chrome/browser/profiles/profile_destroyer.h"
#include "chrome/common/chrome_constants.h"
#include "chrome/grit/chromium_strings.h"
#include "components/zoom/zoom_event_manager.h"
#include "ui/base/l10n/l10n_util.h"

using content::DownloadManagerDelegate;

namespace {

// Text content of README file created in each profile directory. Both %s
// placeholders must contain the product name. This is not localizable and hence
// not in resources.
const char kReadmeText[] =
    "%s settings and storage represent user-selected preferences and "
    "information and MUST not be extracted, overwritten or modified except "
    "through %s defined APIs.";

void CreateProfileReadme(const base::FilePath& profile_path) {
  base::AssertBlockingAllowed();
  base::FilePath readme_path = profile_path.Append(chrome::kReadmeFilename);
  std::string product_name = l10n_util::GetStringUTF8(IDS_PRODUCT_NAME);
  std::string readme_text = base::StringPrintf(
      kReadmeText, product_name.c_str(), product_name.c_str());
  if (base::WriteFile(readme_path, readme_text.data(), readme_text.size()) ==
      -1) {
    LOG(ERROR) << "Could not create README file.";
  }
}

// Creates the profile directory synchronously if it doesn't exist. If
// |create_readme| is true, the profile README will be created asynchronously in
// the profile directory.
void CreateProfileDirectory(base::SequencedTaskRunner* io_task_runner,
                            const base::FilePath& path,
                            bool create_readme) {
  // Create the profile directory synchronously otherwise we would need to
  // sequence every otherwise independent I/O operation inside the profile
  // directory with this operation. base::PathExists() and
  // base::CreateDirectory() should be lightweight I/O operations and avoiding
  // the headache of sequencing all otherwise unrelated I/O after these
  // justifies running them on the main thread.
  base::ThreadRestrictions::ScopedAllowIO allow_io_to_create_directory;

  // If the readme exists, the profile directory must also already exist.
  if (base::PathExists(path.Append(chrome::kReadmeFilename)))
    return;

  DVLOG(1) << "Creating directory " << path.value();
  if (base::CreateDirectory(path) && create_readme) {
    base::PostTaskWithTraits(FROM_HERE,
                             {base::MayBlock(), base::TaskPriority::BACKGROUND,
                              base::TaskShutdownBehavior::BLOCK_SHUTDOWN},
                             base::Bind(&CreateProfileReadme, path));
  }
}

}

BraveSessionProfile::BraveSessionProfile(const base::FilePath& path,
                                         Delegate* delegate,
                                         CreateMode create_mode,
                                         Profile* profile)
    : path_(path),
      parent_(profile),
      is_off_the_record_(profile->IsOffTheRecord()),
      start_time_(base::Time::Now()),
      media_device_id_salt_(
          content::BrowserContext::CreateRandomMediaDeviceIDSalt()) {
  content::BrowserContext::Initialize(this, path);
  notification_registrar_.Add(this, chrome::NOTIFICATION_PROFILE_DESTROYED,
                 content::NotificationService::AllBrowserContextsAndSources());
  LOG(ERROR) << "posting";

  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::BindOnce(&BraveSessionProfile::OnLoad, base::Unretained(this), base::Unretained(delegate)));


  // base::PostTaskWithTraits(
  //     FROM_HERE, {base::MayBlock(), base::TaskPriority::USER_VISIBLE},
  //     base::BindOnce(&BraveSessionProfile::OnLoad, base::Unretained(this), base::Unretained(delegate)));
  LOG(ERROR) << "posted";
}

void BraveSessionProfile::OnLoad(Delegate* delegate) {
  LOG(ERROR) << "on load";
  if (delegate) {
    // TRACE_EVENT0("browser", "ProfileImpl::DoFileInit:DelegateOnProfileCreated")
    LOG(ERROR) << "on create profile";
    delegate->OnProfileCreated(this, true, false);
  }

  LOG(ERROR) << "send notification " << content::NotificationService::current();
  // {
  //   SCOPED_UMA_HISTOGRAM_TIMER("Profile.NotifyProfileCreatedTime");
    content::NotificationService::current()->Notify(
        chrome::NOTIFICATION_PROFILE_CREATED,
        content::Source<Profile>(this),
        content::NotificationService::NoDetails());
  // }
    LOG(ERROR) << "done";
}
base::FilePath BraveSessionProfile::last_selected_directory() {
  return parent_->last_selected_directory();
}

void BraveSessionProfile::set_last_selected_directory(const base::FilePath& path) {
  parent_->set_last_selected_directory(path);
}

chrome_browser_net::Predictor* BraveSessionProfile::GetNetworkPredictor() {
  return parent_->GetNetworkPredictor();
}

GURL BraveSessionProfile::GetHomePage() {
  return parent_->GetHomePage();
}

BraveSessionProfile::~BraveSessionProfile() {}

std::string BraveSessionProfile::GetProfileUserName() const {
  return parent_->GetProfileUserName();
}

Profile::ProfileType BraveSessionProfile::GetProfileType() const {
  return parent_->GetProfileType();
}

#if !defined(OS_ANDROID)
std::unique_ptr<content::ZoomLevelDelegate>
BraveSessionProfile::CreateZoomLevelDelegate(const base::FilePath& partition_path) {
  return std::make_unique<ChromeZoomLevelPrefs>(
      GetPrefs(), GetPath(), partition_path,
      zoom::ZoomEventManager::GetForBrowserContext(this)->GetWeakPtr());
  // return parent_->CreateZoomLevelDelegate(partition_path);
}
#endif  // !defined(OS_ANDROID)

base::FilePath BraveSessionProfile::GetPath() const {
  return path_;
}

scoped_refptr<base::SequencedTaskRunner> BraveSessionProfile::GetIOTaskRunner() {
  return parent_->GetIOTaskRunner();
}

bool BraveSessionProfile::IsOffTheRecord() const {
  return is_off_the_record_;
}

Profile* BraveSessionProfile::GetOffTheRecordProfile() {
  return parent_->GetOffTheRecordProfile();
}

void BraveSessionProfile::DestroyOffTheRecordProfile() {}

bool BraveSessionProfile::HasOffTheRecordProfile() {
  return parent_->HasOffTheRecordProfile();
}

Profile* BraveSessionProfile::GetOriginalProfile() {
  return parent_;
}

const Profile* BraveSessionProfile::GetOriginalProfile() const {
  return parent_;
}

bool BraveSessionProfile::IsSupervised() const {
  return parent_->IsSupervised();
}

bool BraveSessionProfile::IsChild() const {
  return parent_->IsChild();
}

bool BraveSessionProfile::IsLegacySupervised() const {
  return parent_->IsLegacySupervised();
}

ExtensionSpecialStoragePolicy*
    BraveSessionProfile::GetExtensionSpecialStoragePolicy() {
  return parent_->GetExtensionSpecialStoragePolicy();
}

bool BraveSessionProfile::WasCreatedByVersionOrLater(const std::string& version) {
  return parent_->WasCreatedByVersionOrLater(version);
}

void BraveSessionProfile::SetExitType(ExitType exit_type) {
  parent_->SetExitType(exit_type);
}

Profile::ExitType BraveSessionProfile::GetLastSessionExitType() {
  return parent_->GetLastSessionExitType();
}

bool BraveSessionProfile::ShouldRestoreOldSessionCookies() {
  return parent_->ShouldRestoreOldSessionCookies();
}

bool BraveSessionProfile::ShouldPersistSessionCookies() {
  return parent_->ShouldPersistSessionCookies();
}

PrefService* BraveSessionProfile::GetPrefs() {
  return parent_->GetPrefs();
}

const PrefService* BraveSessionProfile::GetPrefs() const {
  return parent_->GetPrefs();
}

// #if !defined(OS_ANDROID)
// ChromeZoomLevelPrefs* BraveSessionProfile::GetZoomLevelPrefs() {
//   return parent_->GetZoomLevelPrefs();
// }
// #endif  // !defined(OS_ANDROID)

PrefService* BraveSessionProfile::GetOffTheRecordPrefs() {
  return parent_->GetOffTheRecordPrefs();
}

PrefService* BraveSessionProfile::GetReadOnlyOffTheRecordPrefs() {
  return parent_->GetReadOnlyOffTheRecordPrefs();
}

content::ResourceContext* BraveSessionProfile::GetResourceContext() {
  return parent_->GetResourceContext();
}

net::URLRequestContextGetter* BraveSessionProfile::GetRequestContext() {
  return parent_->GetRequestContext();
}

net::URLRequestContextGetter* BraveSessionProfile::GetRequestContextForExtensions() {
  return parent_->GetRequestContextForExtensions();
}

net::SSLConfigService* BraveSessionProfile::GetSSLConfigService() {
  return parent_->GetSSLConfigService();
}

content::BrowserPluginGuestManager* BraveSessionProfile::GetGuestManager() {
  return parent_->GetGuestManager();
}

DownloadManagerDelegate* BraveSessionProfile::GetDownloadManagerDelegate() {
  return parent_->GetDownloadManagerDelegate();
}

storage::SpecialStoragePolicy* BraveSessionProfile::GetSpecialStoragePolicy() {
  return parent_->GetSpecialStoragePolicy();
}

content::PushMessagingService* BraveSessionProfile::GetPushMessagingService() {
  return parent_->GetPushMessagingService();
}

content::SSLHostStateDelegate* BraveSessionProfile::GetSSLHostStateDelegate() {
  return parent_->GetSSLHostStateDelegate();
}

content::BrowsingDataRemoverDelegate*
BraveSessionProfile::GetBrowsingDataRemoverDelegate() {
  return parent_->GetBrowsingDataRemoverDelegate();
}

content::PermissionManager* BraveSessionProfile::GetPermissionManager() {
  return parent_->GetPermissionManager();
}

content::BackgroundFetchDelegate* BraveSessionProfile::GetBackgroundFetchDelegate() {
  return parent_->GetBackgroundFetchDelegate();
}

content::BackgroundSyncController* BraveSessionProfile::GetBackgroundSyncController() {
  return parent_->GetBackgroundSyncController();
}

net::URLRequestContextGetter* BraveSessionProfile::CreateRequestContext(
    content::ProtocolHandlerMap* protocol_handlers,
    content::URLRequestInterceptorScopedVector request_interceptors) {
  return parent_->GetRequestContext();
}

net::URLRequestContextGetter*
BraveSessionProfile::CreateRequestContextForStoragePartition(
    const base::FilePath& partition_path,
    bool in_memory,
    content::ProtocolHandlerMap* protocol_handlers,
    content::URLRequestInterceptorScopedVector request_interceptors) {
  return parent_->CreateRequestContextForStoragePartition(
      partition_path, in_memory, protocol_handlers,
      std::move(request_interceptors));
}

net::URLRequestContextGetter* BraveSessionProfile::CreateMediaRequestContext() {
  return parent_->CreateMediaRequestContext();
}

net::URLRequestContextGetter*
BraveSessionProfile::CreateMediaRequestContextForStoragePartition(
    const base::FilePath& partition_path,
    bool in_memory) {
  return parent_->CreateMediaRequestContextForStoragePartition(partition_path,
                                                                in_memory);
}

void BraveSessionProfile::RegisterInProcessServices(StaticServiceMap* services) {}

std::string BraveSessionProfile::GetMediaDeviceIDSalt() {
  return media_device_id_salt_;
}

bool BraveSessionProfile::IsSameProfile(Profile* profile) {
  return (profile == GetOriginalProfile()) || parent_->IsSameProfile(profile);
}

base::Time BraveSessionProfile::GetStartTime() const {
  return start_time_;
}

void BraveSessionProfile::Observe(
    int type,
    const content::NotificationSource& source,
    const content::NotificationDetails& details) {
  switch (type) {
    case chrome::NOTIFICATION_PROFILE_DESTROYED: {
      LOG(ERROR) << "profile is getting destroyed";
      Profile* profile = content::Source<Profile>(source).ptr();
      if (profile != parent_)
        return;

      LOG(ERROR) << "force destroy";
      // force destuction if parent is being destroyed
      is_off_the_record_ = true;
      ProfileDestroyer::DestroyOffTheRecordProfileNow(this);
      base::ThreadTaskRunnerHandle::Get()->DeleteSoon(FROM_HERE, this);
      break;
    }
    default: {
      NOTREACHED();  // Unexpected notification.
      break;
    }
  }
}

// static
BraveSessionProfile* BraveSessionProfile::CreateProfile(
                                                  const base::FilePath& path,
                                                  Delegate* delegate,
                                                  CreateMode create_mode,
                                                  Profile* profile) {
  TRACE_EVENT1("browser,startup",
               "Profile::CreateProfile",
               "profile_path",
               path.AsUTF8Unsafe());

  LOG(ERROR) << "create profile in " << path.AsUTF8Unsafe();
  // Get sequenced task runner for making sure that file operations of
  // this profile are executed in expected order (what was previously assured by
  // the FILE thread).
  scoped_refptr<base::SequencedTaskRunner> io_task_runner =
      profile->GetIOTaskRunner();
  if (create_mode == CREATE_MODE_ASYNCHRONOUS) {
    DCHECK(delegate);
    CreateProfileDirectory(io_task_runner.get(), path, true);
  } else if (create_mode == CREATE_MODE_SYNCHRONOUS) {
    if (!base::PathExists(path)) {
      // TODO(rogerta): http://crbug/160553 - Bad things happen if we can't
      // write to the profile directory.  We should eventually be able to run in
      // this situation.
      if (!base::CreateDirectory(path))
        return NULL;

      CreateProfileReadme(path);
    }
  } else {
    NOTREACHED();
  }

  return new BraveSessionProfile(path, delegate, create_mode, profile);
}
