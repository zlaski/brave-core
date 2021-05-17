import * as React from 'react'
import { AssetOptionType, OrderTypes, SwapViewTypes } from '../../../constants/types'
import { NavButton } from '../../extension'
import SwapInputComponent from '../swap-input-component'
// Styled Components
import {
  StyledWrapper,
  ArrowDownIcon,
  ArrowButton
} from './style'

export interface Props {
  toAsset: AssetOptionType
  fromAsset: AssetOptionType
  fromAmount: string
  toAmount: string
  exchangeRate: string
  slippageTolerance: number
  orderExpiration: number
  orderType: OrderTypes
  onToggleTradeType: () => void
  onFlipAssets: () => void
  onSubmitSwap: () => void
  onSelectPreset: (percent: number) => void
  onInputChange: (value: string, name: string) => void
  onChangeSwapView: (view: SwapViewTypes, option?: string) => void
}

function Swap (props: Props) {
  const {
    toAsset,
    fromAsset,
    fromAmount,
    toAmount,
    orderType,
    exchangeRate,
    slippageTolerance,
    orderExpiration,
    onToggleTradeType,
    onInputChange,
    onSelectPreset,
    onFlipAssets,
    onSubmitSwap,
    onChangeSwapView
  } = props

  const onShowEpirationOrSlippage = () => {
    if (orderType === 'market') {
      onChangeSwapView('slippage')
    } else {
      onChangeSwapView('expiration')
    }
  }

  const onShowAssetTo = () => {
    onChangeSwapView('assets', 'to')
  }

  const onShowAssetFrom = () => {
    onChangeSwapView('assets', 'from')
  }

  return (
    <StyledWrapper>
      <SwapInputComponent
        componentType='fromAmount'
        onSelectPreset={onSelectPreset}
        onInputChange={onInputChange}
        selectedAssetInputAmount={fromAmount}
        inputName='from'
        selectedAssetBalance={6}
        selectedAsset={fromAsset}
        onShowSelection={onShowAssetFrom}
      />
      <ArrowButton onClick={onFlipAssets}>
        <ArrowDownIcon />
      </ArrowButton>
      <SwapInputComponent
        componentType='toAmount'
        orderType={orderType}
        onInputChange={onInputChange}
        selectedAssetInputAmount={toAmount}
        inputName='to'
        selectedAssetBalance={1.2832}
        selectedAsset={toAsset}
        onShowSelection={onShowAssetTo}
      />
      <SwapInputComponent
        componentType='exchange'
        orderType={orderType}
        onToggleTradeType={onToggleTradeType}
        onInputChange={onInputChange}
        selectedAssetInputAmount={exchangeRate}
        inputName='rate'
        selectedAsset={fromAsset}
      />
      <SwapInputComponent
        componentType='selector'
        orderType={orderType}
        slippageTolerance={slippageTolerance}
        orderExpiration={orderExpiration}
        onShowSelection={onShowEpirationOrSlippage}
      />
      <NavButton
        disabled={false}
        buttonType='primary'
        text='Swap'
        onSubmit={onSubmitSwap}
      />
    </StyledWrapper>
  )
}

export default Swap
