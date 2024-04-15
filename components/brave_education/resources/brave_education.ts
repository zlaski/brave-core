/* Copyright (c) 2024 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

import { BraveEducationProxy } from './brave_education_proxy'
import { CommandHandlerProxy } from './command_handler_proxy'
import { readMessageData } from './message_data_reader'

function initializeContent (serverURL: string) {
  // TODO(zenparsing): Show some kind of default or redirect if a server
  // URL cannot be found.
  if (!serverURL) {
    return
  }

  const frame = document.getElementById('content') as HTMLIFrameElement
  frame.src = serverURL

  window.addEventListener('message', (event) => {
    if (event.origin !== new URL(serverURL).origin) {
      console.warn('Invalid sender origin')
      return
    }

    const commandData = readMessageData(event.data)
    if (!commandData) {
      console.warn('Invalid message data')
      return
    }

    const { handler } = CommandHandlerProxy.getInstance()
    handler.canExecuteCommand(commandData.command).then(({ canExecute }) => {
      if (canExecute) {
        handler.executeCommand(commandData.command, commandData.clickInfo);
      } else {
        console.warn('Received invalid command: ' + commandData.command);
      }
    });
  })
}

function onReady () {
  BraveEducationProxy
      .getInstance()
      .initialize(location.href)
      .then(initializeContent)
}

if (document.readyState === 'loading') {
  document.addEventListener('DOMContentLoaded', onReady)
} else {
  onReady()
}
