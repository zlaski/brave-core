// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

export const log = new class {
  verbose = false

  info(...args: any[]) {
    if (this.verbose) {
      console.info('[Rewards]', ...args)
    }
  }

  error(...args: any[]) {
    console.error('[Rewards]', ...args)
  }
}

interface PollOptions {
  timeout: number
  interval: number
}

export async function pollFor<T>(fn: () => T, opts: PollOptions) {
  const startTime = Date.now()
  while (Date.now() - startTime < opts.timeout) {
    const result = fn()
    if (result) {
      return result
    }
    await new Promise((resolve) => setTimeout(resolve, opts.interval))
  }
  log.info('Polling timed out')
  return null
}

let currentURL = ''

export function waitForURLChange() {
  return pollFor(() => {
    if (currentURL !== location.href) {
      currentURL = location.href
      return true
    }
    return false
  }, {
    interval: 300,
    timeout: 1000
  })
}

export function singleCall<T>(fn: () => Promise<T> | T) {
  let p: Promise<T> | null = null
  return () => {
    if (!p) {
      p = Promise.resolve(fn())
      p.finally(() => { p = null })
    }
    return p
  }
}

export function urlPath(url: string) {
  try { return new URL(url, location.href).pathname }
  catch { return '' }
}

export function absoluteURL(url: string) {
  try { return new URL(url, location.href).toString() }
  catch { return '' }
}

export function getPathComponents(path: string) {
  return path
    .split('/')
    .filter((part) => part)
    .map((part) => part.toLocaleLowerCase())
}

export interface CreatorInfo {
  id: string
  name: string
  url: string
  imageURL: string
}
