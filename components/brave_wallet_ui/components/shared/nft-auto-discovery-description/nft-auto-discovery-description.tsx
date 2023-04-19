// Copyright (c) 2023 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.
import * as React from 'react'

// utils
import { getLocale, splitStringForTag } from '../../../../common/locale'

// styles
import { Description, Underline, Link } from './nft-auto-discovery-description.style'

const LEARN_MORE_LINK = 'https://github.com/brave/brave-browser/wiki/NFT-Discovery'

export const NftAutoDiscoveryDescription = () => {
  const { beforeTag, duringTag, afterTag } = splitStringForTag(getLocale('braveWalletEnableNftAutoDiscoveryModalDescription'), 1)
  const { beforeTag: beforeLink, duringTag: learnMore } = splitStringForTag(afterTag || '', 3)

  return (
    <Description>
      {beforeTag}
      <Underline>{duringTag}</Underline>
      {beforeLink}
      <Link target='_blank' rel='noreferrer' href={LEARN_MORE_LINK}>{learnMore}</Link>
    </Description>
  )
}
