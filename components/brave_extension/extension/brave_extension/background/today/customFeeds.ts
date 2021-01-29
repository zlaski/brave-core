// Copyright (c) 2020 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import fetchRssChannel, { RssChannel, RssItem } from './parseRss'

type RssSourceContent = {
  publisherId: string
  publisherName: string
  lastUpdatedTimestamp?: number
  items: BraveToday.Article[]
}

// TODO(petemill): not test data!
const customFeeds = [
  'https://css-tricks.com/feed/',
  'https://feeds.feedburner.com/HighScalability',
  'https://blog.producthunt.com/feed',
  'https://www.nba.com/warriors/rss.xml',
  'https://www.popularwoodworking.com/feed/',
  'https://www.reddit.com/r/brave_browser/.rss',
  'https://www.theguardian.com/football/arsenal/rss',
  'https://www.realclearpolitics.com/rss',
  'https://planet.mozilla.org/rss20.xml'
]

let rssCache: RssSourceContent[] | null = null

export async function getAllFeeds () {
  if (!rssCache) {
    rssCache = (await Promise.all(customFeeds.map(url => fetchRssChannel(url))))
      .filter(feed => feed !== null)
      .map((rss: RssChannel): RssSourceContent => {
        const publisherId = `rss:${rss.linkUrl}`
        const publisherName = rss.title
        console.log(`Parsing RSS Feed ${publisherName} as Publisher ID ${publisherId}`)
        return {
          publisherId,
          publisherName,
          lastUpdatedTimestamp: rss.timestamp,
          items: rss.items.map(rssItem => rssItemToFeedItem(publisherId, publisherName, rssItem))
        }
      })
  }
  return rssCache
}

export async function getAllPublishers (): Promise<BraveToday.Publisher[]> {
  const feeds = await getAllFeeds()
  return feeds.map((feed): BraveToday.Publisher => {
    return {
      publisher_id: feed.publisherId,
      publisher_name: feed.publisherName,
      category: 'rss',
      enabled: true,
      user_enabled: null
    }
  })
}

function rssItemToFeedItem (publisherId: string, publisherName: string, rss: RssItem): BraveToday.Article {
  const pubDateUTC = new Date(rss.timestamp).toUTCString()
  return {
    content_type: 'article',
    category: 'rss',
    publish_time: pubDateUTC.substr(0, pubDateUTC.length - 3),
    title: rss.title,
    url: rss.linkUrl,
    url_hash: getStringHash(rss.linkUrl),
    img: rss.imageUrl,
    publisher_id: publisherId,
    publisher_name: publisherName,
    score: getItemScore(rss.timestamp),
  }
}

function getItemScore (timestamp: number) {
  const secondsSincePublish = Math.abs((new Date().getTime() - timestamp) / 1000)
  const score = Math.log(secondsSincePublish)
  return score
}

function getStringHash (content: string) {
  let hash = 0
  for (let i = 0; i < content.length; i++) {
    const chr = content.charCodeAt(i)
    hash  = ((hash << 5) - hash) + chr
    hash |= 0
  }
  return hash.toString()
}
