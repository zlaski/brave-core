import * as React from 'react'
import { AssetOptionType, OrderTypes, SwapViewTypes, SlippagePresetObjectType } from '../../../constants/types'
// import { SlippagePresetOptions } from '../../../options/slippage-preset-options'
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
  slippageTolerance: SlippagePresetObjectType
  orderExpiration: number
  orderType: OrderTypes
  onToggleTradeType: () => void
  onFlipAssets: () => void
  onSubmitSwap: () => void
  onInputChange: (value: string, name: string) => void
  onChangeSwapView: (view: SwapViewTypes, option?: string) => void
  onSelectPresetAmount: (percent: number) => void
  onSelectSlippageTolerance: (slippage: SlippagePresetObjectType) => void
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
    onSelectPresetAmount,
    onSelectSlippageTolerance,
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
        onSelectPresetAmount={onSelectPresetAmount}
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
        onSelectSlippageTolerance={onSelectSlippageTolerance}
        slippageTolerance={slippageTolerance}
        orderExpiration={orderExpiration}
        onShowSelection={onShowEpirationOrSlippage}
      />
      {/* <div>{SlippagePresetOptions.map((slippage) => <span>{slippage.slippage}</span>)}</div> */}
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
