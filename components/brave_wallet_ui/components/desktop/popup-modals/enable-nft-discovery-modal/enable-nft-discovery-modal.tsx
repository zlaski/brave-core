// Copyright (c) 2023 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.
import * as React from 'react'

// utils
import { getLocale } from '../../../../../common/locale'

// components
import { PopupModal } from '../..'

// styles
import {
  ButtonRow,
  CancelButton,
  ConfirmButton,
  Header,
  StyledWrapper
} from './enable-nft-discovery-modal.style'
import { NftAutoDiscoveryDescription } from '../../../shared/nft-auto-discovery-description/nft-auto-discovery-description'

interface Props {
  onConfirm: () => void
  onCancel: () => void
}

export const EnableNftDiscoveryModal = ({ onConfirm, onCancel }: Props) => {
  return (
    <PopupModal
      title=''
      width='477px'
      hideHeader
      onClose={onCancel}
    >
      <StyledWrapper>
        <Header>{getLocale('braveWalletEnableNftAutoDiscoveryModalHeader')}</Header>
        <NftAutoDiscoveryDescription />
        <ButtonRow>
          <ConfirmButton onClick={onConfirm}>{getLocale('braveWalletEnableNftAutoDiscoveryModalConfirm')}</ConfirmButton>
          <CancelButton onClick={onCancel}>{getLocale('braveWalletEnableNftAutoDiscoveryModalCancel')}</CancelButton>
        </ButtonRow>
      </StyledWrapper>
    </PopupModal>
  )
}
