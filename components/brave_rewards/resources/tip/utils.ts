/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import BigNumber from 'bignumber.js'

export const convertBalance = (tokens: string, rates: Record<string, number> | undefined, currency: string = 'USD'): string => {
  const tokensNum = parseFloat(tokens)
  if (tokensNum === 0 || !rates || !rates[currency]) {
    return '0.00'
  }

  const converted = tokensNum * rates[currency]

  if (isNaN(converted)) {
    return '0.00'
  }

  return converted.toFixed(2)
}

export const formatConverted = (converted: string, currency: string = 'USD'): string | null => {
  return `${converted} ${currency}`
}

export const convertProbiToFixed = (probi: string, places: number = 1) => {
  const result = new BigNumber(probi).dividedBy('1e18').toFixed(places, BigNumber.ROUND_DOWN)

  if (result === 'NaN') {
    return '0.0'
  }

  return result
}

const socialRegexes = {
  twitter: /^(https:\/\/)?(www\.)?twitter\.com\//,
  youtube: /^(https:\/\/)?(www\.)?youtube\.com\//,
  twitch: /^(https:\/\/)?(www\.)?twitch\.tv\//
}

/**
 * Normalizes a social media URL from the publishers database. Returns empty
 * if input is not a valid social URL.
 * @param {string} type - social media type
 * @param {string} input - input social url
 * @returns {string} - the normalized url
 */
export const normalizeSocialUrl = (type: string, input: string) => {
  const regex = socialRegexes[type]
  if (regex && regex.test(input)) {
    if (input.startsWith('https://')) {
      return input
    }
    return `https://${input}`
  }
  return ''
}
