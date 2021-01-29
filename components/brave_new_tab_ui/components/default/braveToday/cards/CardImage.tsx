// Copyright (c) 2020 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import * as React from 'react'
import * as Card from '../cardSizes'
import * as Background from '../../../../../common/Background'

type Props = {
  imageUrl: string
  isPadded?: boolean
  list?: boolean
  isPromoted?: boolean
  onLoaded?: () => any
}

function useGetUnpaddedImage (url: string, isPadded: boolean, onLoaded?: () => any) {
  const [unpaddedUrl, setUnpaddedUrl] = React.useState('')
  const onReceiveUnpaddedUrl = (result: string) => {
    setUnpaddedUrl(result)
    window.requestAnimationFrame(() => {
      if (onLoaded) {
        onLoaded()
      }
    })
  }
  React.useEffect(() => {
    if (!url) {
      return
    }
    // Storybook method
    // @ts-ignore
    if (window.braveStorybookUnpadUrl) {
      // @ts-ignore
      window.braveStorybookUnpadUrl(paddedUrl)
      .then(onReceiveUnpaddedUrl)
      return
    }
    Background.send<BraveToday.Messages.GetImageDataResponse,
        BraveToday.Messages.GetImageDataPayload>(
      Background.MessageTypes.Today.getImageData,
      { url, isPadded }
    )
    .then(result => onReceiveUnpaddedUrl(result.dataUrl))
    .catch(err => {
      console.error(`Error getting image for ${url}.`, err)
    })

  }, [url, isPadded])
  return unpaddedUrl
}

export default function CardImage (props: Props) {
  const url = useGetUnpaddedImage(props.imageUrl, !!props.isPadded, props.onLoaded)
  const [isImageLoaded, setIsImageLoaded] = React.useState(false)
  React.useEffect(() => {
    if (url) {
      const img = new Image()
      img.addEventListener('load', () => {
        setIsImageLoaded(true)
      })
      img.src = url
    }
  }, [url])
  const Frame = props.list ? Card.ListImageFrame : Card.ImageFrame
  return (
    <Frame isImageLoaded={isImageLoaded}>
      <Card.Image isPromoted={props.isPromoted} src={url} />
    </Frame>
  )
}
