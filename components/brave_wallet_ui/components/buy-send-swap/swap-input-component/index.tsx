import * as React from 'react'
import { AssetOptionType, OrderTypes } from '../../../constants/types'
import { AmountPresetOptions } from '../../../options/amount-preset-options'
import locale from '../../../constants/locale'

// Styled Components
import {
  RefreshButton,
  RefreshIcon,
  MarketLimitButton,
  FromBalanceText,
  AssetIcon,
  AssetButton,
  AssetTicker,
  CaratDownIcon,
  PresetButton,
  PresetRow,
  SelectValueText,
  SelectText,
  Input,
  Row
} from './style'

import { BubbleContainer } from '../shared-styles'

export interface Props {
  componentType: 'toAmount' | 'fromAmount' | 'toAddress' | 'buyAmount' | 'exchange' | 'selector'
  selectedAssetBalance?: number
  selectedAsset?: AssetOptionType
  selectedAssetInputAmount?: string
  inputName?: string
  orderType?: OrderTypes
  slippageTolerance?: number
  orderExpiration?: number
  onInputChange?: (value: string, name: string) => void
  onSelectPreset?: (percent: number) => void
  onToggleTradeType?: () => void
  onShowSelection?: () => void
  onRefresh?: () => void
}

function SwapInputComponent (props: Props) {
  const {
    selectedAsset,
    selectedAssetBalance,
    componentType,
    selectedAssetInputAmount,
    inputName,
    orderType,
    slippageTolerance,
    orderExpiration,
    onInputChange,
    onRefresh,
    onSelectPreset,
    onToggleTradeType,
    onShowSelection
  } = props
  const [spin, setSpin] = React.useState<number>(0)

  const refresh = () => {
    if (onRefresh) {
      onRefresh()
    }
    setSpin(1)
  }

  const setPresetValue = (percent: number) => () => {
    if (onSelectPreset) {
      onSelectPreset(percent)
    }
  }

  const getTitle = () => {
    switch (componentType) {
      case 'fromAmount':
        return locale.swapFrom
      case 'toAmount':
        if (orderType === 'market') {
          return `${locale.swapTo} (${locale.swapEstimate})`
        } else {
          return locale.swapTo
        }
      case 'buyAmount':
        return locale.buy
      case 'exchange':
        if (orderType === 'market') {
          return `${locale.swapMarket} ${locale.swapPriceIn} ${selectedAsset?.symbol}`
        } else {
          return `${locale.swapPriceIn} ${selectedAsset?.symbol}`
        }
      case 'selector':
        if (orderType === 'market') {
          return 'Slippage tolerance'
        } else {
          return 'Expires in'
        }
      case 'toAddress':
        return locale.swapTo
    }
  }

  const onInputChanged = (event: React.ChangeEvent<HTMLInputElement>) => {
    if (onInputChange) {
      onInputChange(event.target.value, event.target.name)
    }
  }

  const resetAnimation = () => {
    setSpin(0)
  }

  return (
    <BubbleContainer>
      {componentType !== 'selector' &&
        <>
          <Row>
            <FromBalanceText isExchange={componentType === 'exchange'}>{getTitle()}</FromBalanceText>
            {componentType === 'exchange' ? (
              <MarketLimitButton onClick={onToggleTradeType}>{orderType === 'market' ? locale.swapLimit : locale.swapMarket}</MarketLimitButton>
            ) : (
              <FromBalanceText>{locale.balance}: {selectedAssetBalance}</FromBalanceText>
            )
            }
          </Row>
          <Row isExchange={componentType === 'exchange'}>
            <Input
              isExchange={componentType === 'exchange'}
              type='number'
              placeholder='0.0'
              value={selectedAssetInputAmount}
              name={inputName}
              onChange={onInputChanged}
              disabled={orderType === 'market' && componentType === 'exchange' || orderType === 'limit' && componentType === 'toAmount'}
            />
            {componentType === 'exchange' && orderType === 'market' &&
              <RefreshButton onClick={refresh}>
                <RefreshIcon
                  onAnimationEnd={resetAnimation}
                  spin={spin}
                />
              </RefreshButton>
            }
            {componentType !== 'exchange' &&
              <AssetButton onClick={onShowSelection}>
                <AssetIcon icon={selectedAsset?.icon} />
                <AssetTicker>{selectedAsset?.symbol}</AssetTicker>
                <CaratDownIcon />
              </AssetButton>
            }
          </Row>
          {componentType === 'fromAmount' &&
            <PresetRow>
              {AmountPresetOptions.map((preset) =>
                <PresetButton
                  key={preset.id}
                  onClick={setPresetValue(preset.id)}
                >
                  {preset.name}
                </PresetButton>
              )}
            </PresetRow>
          }
        </>
      }
      {componentType === 'selector' &&
        <Row>
          <SelectText>{getTitle()}</SelectText>
          <AssetButton onClick={onShowSelection}>
            <SelectValueText>{orderType === 'market' ? `${slippageTolerance}%` : `${orderExpiration} days`}</SelectValueText>
            <CaratDownIcon />
          </AssetButton>
        </Row>
      }
    </BubbleContainer >
  )
}

export default SwapInputComponent
