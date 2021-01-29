// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

type RssMeta = {
  title: string
  feedUrl: string
  linkUrl?: string
  description?: string
  timestamp?: number
}

export type RssItem = {
  guid: string
  title: string
  linkUrl: string
  imageUrl: string | null
  timestamp: number
}


type RssContent = {
  items: RssItem[]
}

export type RssChannel = RssMeta & RssContent

const MAX_CONTENT_ITEMS_PER_FEED = 100

export default async function fetchRss (feedUrl: string): Promise<RssChannel | null> {
  const feed = await fetchRssXml(feedUrl)
  if (!feed) {
    return null
  }
  const channelElement = feed.querySelector('channel')
  if (!channelElement) {
    console.error(`No channel element ${feedUrl}`)
    return null
  }
  const items = extractFeedItems(channelElement)
  const title = getElementText(channelElement, 'title')
  const linkUrl = getElementText(channelElement, 'link') || feedUrl
  if (!title) {
    return null
  }
  return {
    title,
    linkUrl,
    feedUrl,
    items
  }
}

async function fetchRssXml (feedUrl: string) {
  const response = await fetch(feedUrl)
  if (!response.ok) {
    throw new Error(`Bad response for feed ${feedUrl}: ${response.status} ${response.statusText}`)
  }
  const feedRaw = await response.text()
  if (!feedRaw) {
    console.error(`No text data received from feed url ${feedUrl}`)
    return null
  }
  const feed = await (new window.DOMParser().parseFromString(feedRaw, "text/xml"))
  console.log(`Parsed feed document from ${feedUrl}`, feed)
  if (!feed.children.length || feed.children[0].tagName === 'html') {
    console.error(`Could not parse RSS feed from url ${feedUrl}`)
    return null
  }
  return feed as XMLDocument
}

function getElementText (element: Element, name: string, errorAdd?: string) {
  const item = element.querySelector(name)
  if (!item) {
    console.error(`Coult not get contents of element ${name} ${errorAdd}`)
    return null
  }
  return item.textContent
}

const imgSourceRegex = /<img[^>]+src="([^">]+)"/gmi;

function extractFeedItems (channelElement: Element): RssItem[] {
  const itemElements = channelElement.querySelectorAll('item')
  const items = []
  for (const itemElement of itemElements) {
    if (items.length === MAX_CONTENT_ITEMS_PER_FEED) {
      break
    }
    let guid, title, linkUrl, pubDateRaw
    let imageUrl = null
    let currentImageSize = 0
    for (const node of itemElement.children) {
      switch (node.tagName) {
        case 'guid':
          guid = node.textContent
          break
        case 'title':
          title = node.textContent
          break
        case 'link':
          linkUrl = node.textContent
          break
        case 'pubDate':
          pubDateRaw = node.textContent
          break
        case 'enclosure':
          if (node.getAttribute('type')?.startsWith('image')) {
            imageUrl = node.getAttribute('url')
          }
          break
        case 'media:content':
          const width = Number(node.getAttribute('width'))
          if ((!width && currentImageSize === 0) || (width > currentImageSize)) {
            imageUrl = node.getAttribute('url')
          }
          break
        case 'description':
          // Parse img src from html
          if (imageUrl) {
            break
          }
          const content = node.textContent
          if (!content) {
            break
          }
          // get first image source match
          const match = imgSourceRegex.exec(content)
          if (!match || !match.length || match.length < 2) {
            break
          }
          imageUrl = match[2]
          break
      }
    }
    if (!guid || !title || !linkUrl || !pubDateRaw) {
      continue
    }
    let timestamp = Date.parse(pubDateRaw)
    if (isNaN(timestamp)) {
      console.error(`Invalid date for item ${guid}: ${pubDateRaw}`)
      continue
    }
    items.push({
      guid,
      title,
      linkUrl,
      imageUrl,
      timestamp
    })
  }
  return items
}
