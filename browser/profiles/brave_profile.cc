/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/profiles/brave_profile.h"

#include "base/bind.h"
// #include "base/callback.h"
// #include "base/command_line.h"
#include "base/compiler_specific.h"
// #include "base/environment.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
// #include "base/logging.h"
// #include "base/memory/ptr_util.h"
// #include "base/memory/weak_ptr.h"
#include "base/metrics/histogram_macros.h"
#include "base/path_service.h"
#include "base/sequenced_task_runner.h"
#include "chrome/common/chrome_constants.h"
#include "chrome/grit/chromium_strings.h"
#include "ui/base/l10n/l10n_util.h"

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

// BraveProfile::BraveProfile() {}
BraveProfile::BraveProfile(const base::FilePath& path,
            Delegate* delegate,
            CreateMode create_mode,
            scoped_refptr<base::SequencedTaskRunner> io_task_runner) :
  ProfileImpl(path, delegate, create_mode, io_task_runner) {}

BraveProfile* BraveProfile::CreateProfile(const base::FilePath& path,
                                Delegate* delegate,
                                CreateMode create_mode) {
  LOG(ERROR) << "create brave profile" << path.AsUTF8Unsafe();
  TRACE_EVENT1("browser,startup",
               "Profile::CreateProfile",
               "profile_path",
               path.AsUTF8Unsafe());

  // Get sequenced task runner for making sure that file operations of
  // this profile are executed in expected order (what was previously assured by
  // the FILE thread).
  scoped_refptr<base::SequencedTaskRunner> io_task_runner =
      base::CreateSequencedTaskRunnerWithTraits(
          {base::TaskShutdownBehavior::BLOCK_SHUTDOWN, base::MayBlock()});
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

  return new BraveProfile(path, delegate, create_mode, io_task_runner);
}
