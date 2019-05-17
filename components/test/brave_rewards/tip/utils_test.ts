/* global describe, it */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import { convertBalance, convertProbiToFixed, normalizeSocialUrl } from '../../../brave_rewards/resources/tip/utils'

describe('Rewards Panel extension - Utils', () => {
  describe('convertBalance', () => {
    it('token has letters', () => {
      expect(convertBalance('test', { 'USD': 10 })).toBe('0.00')
    })

    it('rates are empty', () => {
      expect(convertBalance('10', {})).toBe('0.00')
    })

    it('rate is missing', () => {
      expect(convertBalance('10', { 'USD': 10 }, 'EUR')).toBe('0.00')
    })

    it('all good', () => {
      expect(convertBalance('10', { 'USD': 10 })).toBe('100.00')
    })

    it('currency is provided', () => {
      expect(convertBalance('10', { 'USD': 10, 'EUR': 4 }, 'EUR')).toBe('40.00')
    })
  })

  describe('convertProbiToFixed', () => {
    it('probi is not in correct format', () => {
      expect(convertProbiToFixed('sdfs')).toBe('0.0')
    })

    it('probi is not 10^18', () => {
      expect(convertProbiToFixed('9')).toBe('0.0')
    })

    it('we should always round down', () => {
      expect(convertProbiToFixed('0999999999999999999')).toBe('0.9')
    })

    it('regular convert', () => {
      expect(convertProbiToFixed('1559999999999999990')).toBe('1.5')
    })

    it('regular convert two places', () => {
      expect(convertProbiToFixed('1559999999999999990', 2)).toBe('1.55')
    })

    it('big convert', () => {
      expect(convertProbiToFixed('150000000000000000000000000')).toBe('150000000.0')
    })
  })

  describe('normalizeSocialUrl', () => {
    it('normalize with null inputs', () => {
      expect(normalizeSocialUrl(undefined, undefined)).toBe('')
    })

    it('normalize with null second input', () => {
      expect(normalizeSocialUrl('twitter', null)).toBe('')
    })

    it('normalize with invalid type', () => {
      expect(normalizeSocialUrl('soundcloud', 'https://soundcloud.com/')).toBe('')
    })

    it('normalize with invalid URLs', () => {
      expect(normalizeSocialUrl('twitch', 'https://twitch.com/')).toBe('')
      expect(normalizeSocialUrl('twitch', 'https://twitchxtv/')).toBe('')
      expect(normalizeSocialUrl('twitter', 'https://www.twitch.com/')).toBe('')
      expect(normalizeSocialUrl('twitter', 'ahttps://www.twitter.com/')).toBe('')
      expect(normalizeSocialUrl('twitter', 'https://www.twitter.com')).toBe('')
      expect(normalizeSocialUrl('youtube', 'http://youtube.com/')).toBe('')
      expect(normalizeSocialUrl('youtube', 'youtube.com')).toBe('')
      expect(normalizeSocialUrl('youtube', 'www.youtube.com')).toBe('')
      expect(normalizeSocialUrl('youtube', 'wwwyoutube.com/')).toBe('')
      expect(normalizeSocialUrl('twitch', 'https://youtube.com/')).toBe('')
    })

    it('normalize with valid URLs', () => {
      expect(normalizeSocialUrl('twitch', 'https://twitch.tv/'))
        .toBe('https://twitch.tv/')
      expect(normalizeSocialUrl('twitch', 'https://www.twitch.tv/test'))
        .toBe('https://www.twitch.tv/test')
      expect(normalizeSocialUrl('youtube', 'www.youtube.com/foo.bar'))
        .toBe('https://www.youtube.com/foo.bar')
      expect(normalizeSocialUrl('youtube', 'youtube.com/'))
        .toBe('https://youtube.com/')
      expect(normalizeSocialUrl('twitter', 'twitter.com/omg/omg/?test#hashtag'))
        .toBe('https://twitter.com/omg/omg/?test#hashtag')
      expect(normalizeSocialUrl('twitter', 'https://www.twitter.com/?test#hashtag'))
        .toBe('https://www.twitter.com/?test#hashtag')
    })
  })
})
