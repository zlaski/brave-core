import * as React from 'react'
import { AssetOptionType, OrderTypes, SwapViewTypes, SlippagePresetObjectType, ExpirationPresetObjectType } from '../../../constants/types'
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
  orderExpiration: ExpirationPresetObjectType
  orderType: OrderTypes
  onToggleTradeType: () => void
  onFlipAssets: () => void
  onSubmitSwap: () => void
  onInputChange: (value: string, name: string) => void
  onChangeSwapView: (view: SwapViewTypes, option?: string) => void
  onSelectPresetAmount: (percent: number) => void
  onSelectExpiration: (expiration: ExpirationPresetObjectType) => void
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
    onSelectExpiration,
    onSelectSlippageTolerance,
    onFlipAssets,
    onSubmitSwap,
    onChangeSwapView
  } = props

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
        onSelectExpiration={onSelectExpiration}
        slippageTolerance={slippageTolerance}
        orderExpiration={orderExpiration}
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
