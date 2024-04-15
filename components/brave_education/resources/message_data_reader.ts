/* Copyright (c) 2024 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

import { Command, ClickInfo } from 'gen/brave/components/brave_education/common/brave_education.mojom.m.js'

const commandNames = new Map<string, Command>(Object.entries({
  'open-rewards-onboarding': Command.kOpenRewardsOnboarding
}))

interface CommandMessage {
  command: Command
  clickInfo: ClickInfo
}

function parseClickInfo (clickInfo: any): ClickInfo {
  if (!clickInfo || typeof clickInfo !== 'object') {
    clickInfo = {}
  }

  return {
    middleButton: Boolean(clickInfo.middleButton),
    altKey: Boolean(clickInfo.altKey),
    ctrlKey: Boolean(clickInfo.ctrlKey),
    metaKey: Boolean(clickInfo.metaKey),
    shiftKey: Boolean(clickInfo.shiftKey)
  }
}

export function readMessageData (data: any): CommandMessage | null {
  if (!data || typeof data !== 'object') {
    data = {}
  }

  const command = commandNames.get(String(data.command || ''))
  if (command === undefined) {
    return null
  }

  return { command, clickInfo: parseClickInfo(data) }
}
