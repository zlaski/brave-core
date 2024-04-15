/* Copyright (c) 2024 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_UI_WEBUI_BRAVE_EDUCATION_EDUCATION_COMMAND_HANDLER_H_
#define BRAVE_BROWSER_UI_WEBUI_BRAVE_EDUCATION_EDUCATION_COMMAND_HANDLER_H_

#include <vector>

#include "base/memory/raw_ptr.h"
#include "brave/components/brave_education/common/brave_education.mojom.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/receiver.h"

class Profile;

namespace brave_education {

class EducationCommandHandler : public mojom::CommandHandler {
 public:
  EducationCommandHandler(
      mojo::PendingReceiver<mojom::CommandHandler> pending_handler,
      Profile* profile,
      std::vector<mojom::Command> supported_commands);

  ~EducationCommandHandler() override;

  // mojom::CommandHandler:
  void CanExecuteCommand(mojom::Command command,
                         CanExecuteCommandCallback callback) override;

  void ExecuteCommand(mojom::Command command,
                      mojom::ClickInfoPtr click_info,
                      ExecuteCommandCallback callback) override;

 private:
  mojo::Receiver<mojom::CommandHandler> command_handler_;
  std::vector<mojom::Command> supported_commands_;
  raw_ptr<Profile> profile_;
};

}  // namespace brave_education

#endif  // BRAVE_BROWSER_UI_WEBUI_BRAVE_EDUCATION_EDUCATION_COMMAND_HANDLER_H_
