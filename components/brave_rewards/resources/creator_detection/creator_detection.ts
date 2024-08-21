// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

import { log, waitForURLChange, singleCall, CreatorInfo } from './helpers'

import * as github from './github'
import * as reddit from './reddit'
import * as twitch from './twitch'
import * as twitter from './twitter'
import * as vimeo from './vimeo'
import * as youtube from './youtube'

// The following identifiers are injected as arguments by the browser.
declare function setPageUpdatedCallback(
  callback: () => any): void
declare function onCreatorDetected(
  id: string,
  name: string,
  url: string,
  imageURL: string): void
declare const verboseLogging: boolean

if (verboseLogging) {
  log.verbose = true
}

function initializeDetector() {
  switch (location.origin.toLocaleLowerCase()) {
    case 'https://github.com':
    case 'https://www.github.com':
    case 'https://gist.github.com':
      return github.initialize()
    case 'https://reddit.com':
    case 'https://www.reddit.com':
      return reddit.initialize()
    case 'https://twitch.tv':
    case 'https://www.twitch.tv':
      return twitch.initialize()
    case 'https://twitter.com':
    case 'https://x.com':
      return twitter.initialize()
    case 'https://vimeo.com':
      return vimeo.initialize()
    case 'https://www.youtube.com':
    case 'https://m.youtube.com':
      return youtube.initialize()
    default:
      log.error('Creator detection not supported for origin', location.origin)
      return null
  }
}

const detectCreator = initializeDetector()

setPageUpdatedCallback(singleCall(async () => {
  if (!detectCreator) {
    return
  }

  await waitForURLChange()

  let creator: CreatorInfo | null = null
  try {
    creator = await detectCreator()
  } catch (err) {
    log.error('Error detecting creator', err)
  }

  if (creator) {
    log.info('Found creator:', creator)
    onCreatorDetected(
      creator.id,
      creator.name,
      creator.url,
      creator.imageURL
    )
  } else {
    log.info('No creator found for current URL')
    onCreatorDetected('', '', '', '')
  }
}))
