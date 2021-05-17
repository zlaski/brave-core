import * as React from 'react'
import { UserAccountType, AssetOptionType, NetworkOptionsType, OrderTypes, SwapViewTypes, SlippagePresetObjectType } from '../../constants/types'
import { AssetOptions } from '../../options/asset-options'
import { NetworkOptions } from '../../options/network-options'
import { SlippagePresetOptions } from '../../options/slippage-preset-options'
import {
  Header,
  SelectAccount,
  SelectAsset,
  SelectNetwork,
  SelectExpire,
  Swap
} from '../../components/buy-send-swap'

export interface Props {
  accounts: UserAccountType[]
}

function SwapTab (props: Props) {
  const { accounts } = props
  const [swapView, setSwapView] = React.useState<SwapViewTypes>('swap')
  const [selectedAccount, setSelectedAccount] = React.useState<UserAccountType>(accounts[0])
  const [selectedNetwork, setSelectedNetwork] = React.useState<NetworkOptionsType>(NetworkOptions[0])
  const [isSelectingAsset, setIsSelectingAsset] = React.useState<string>('')
  const [fromAsset, setFromAsset] = React.useState<AssetOptionType>(AssetOptions[0])
  const [toAsset, setToAsset] = React.useState<AssetOptionType>(AssetOptions[1])
  const [fromAmount, setFromAmount] = React.useState('')
  const [toAmount, setToAmount] = React.useState('')
  const [exchangeRate, setExchangeRate] = React.useState('0.0027533')
  const [orderType, setOrderType] = React.useState<OrderTypes>('market')
  const [slippageTolerance, setSlippageTolerance] = React.useState<SlippagePresetObjectType>(SlippagePresetOptions[0])
  const [orderExpiration] = React.useState<number>(7)

  const calculateToAmount = (amount: number, market: boolean) => {
    if (market) {
      const calculated = Number(amount) / Number(exchangeRate)
      setToAmount(calculated.toString())
    } else {
      const calculated = Number(fromAmount) / Number(amount)
      setToAmount(calculated.toString())
    }
  }

  const onSelectPresetAmount = (percent: number) => {
    const amount = 1.2832 * percent
    setFromAmount(amount.toString())
    calculateToAmount(amount, true)
  }

  const onChangeSwapView = (view: SwapViewTypes, option?: 'to' | 'from') => {
    if (option) {
      setIsSelectingAsset(option)
      setSwapView(view)
    } else {
      setSwapView(view)
    }
  }

  const onSelectNetwork = (network: NetworkOptionsType) => () => {
    setSelectedNetwork(network)
    setSwapView('swap')
  }

  const onSelectAccount = (account: UserAccountType) => () => {
    setSelectedAccount(account)
    setSwapView('swap')
  }

  const onSelectSlippageTolerance = (slippage: SlippagePresetObjectType) => {
    setSlippageTolerance(slippage)
  }

  const onSelectAsset = (asset: AssetOptionType) => () => {
    if (isSelectingAsset === 'from') {
      setFromAsset(asset)
      setSwapView('swap')
    } else {
      setToAsset(asset)
      setSwapView('swap')
    }
  }

  const toggleTradeType = () => {
    if (orderType === 'market') {
      setOrderType('limit')
    } else {
      setOrderType('market')
      setExchangeRate('0.0027533')
      calculateToAmount(Number('0.0027533'), false)
    }
  }

  const onInputChange = (value: string, name: string) => {
    if (name === 'to') {
      setToAmount(value)
    }
    if (name === 'from') {
      setFromAmount(value)
      calculateToAmount(Number(value), true)
    }
    if (name === 'rate') {
      setExchangeRate(value)
      calculateToAmount(Number(value), false)
    }
  }

  const goBack = () => {
    setSwapView('swap')
  }

  const flipAssets = () => {
    setFromAsset(toAsset)
    setToAsset(fromAsset)
  }

  const onSubmit = () => {
    alert('Submit the Swap')
  }

  return (
    <>
      {swapView === 'swap' &&
        <>
          <Header
            selectedAccount={selectedAccount}
            selectedNetwork={selectedNetwork}
            onChangeSwapView={onChangeSwapView}
          />
          <Swap
            toAsset={toAsset}
            fromAsset={fromAsset}
            toAmount={toAmount}
            fromAmount={fromAmount}
            orderType={orderType}
            exchangeRate={exchangeRate}
            slippageTolerance={slippageTolerance}
            orderExpiration={orderExpiration}
            onInputChange={onInputChange}
            onFlipAssets={flipAssets}
            onSubmitSwap={onSubmit}
            onSelectPresetAmount={onSelectPresetAmount}
            onSelectSlippageTolerance={onSelectSlippageTolerance}
            onChangeSwapView={onChangeSwapView}
            onToggleTradeType={toggleTradeType}
          />
        </>
      }
      {swapView === 'acounts' &&
        <SelectAccount
          accounts={accounts}
          onSelectAccount={onSelectAccount}
          onBack={goBack}
        />
      }
      {swapView === 'assets' &&
        <SelectAsset
          assets={AssetOptions}
          onSelectAsset={onSelectAsset}
          onBack={goBack}
        />
      }
      {swapView === 'networks' &&
        <SelectNetwork
          networks={NetworkOptions}
          onSelectNetwork={onSelectNetwork}
          onBack={goBack}
        />
      }
      {swapView === 'expiration' &&
        <SelectExpire
          onBack={goBack}
        />
      }
    </>
  )
}

export default SwapTab
