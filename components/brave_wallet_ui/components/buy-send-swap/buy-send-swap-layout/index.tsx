import * as React from 'react'
import { BraveWallet, BuySendSwapTypes } from '../../../constants/types'
import { BuySendSwapOptions } from '../../../options/buy-send-swap-options'
import { reduceNetworkDisplayName } from '../../../utils/network-utils'
import Tooltip from '../buy-send-swap-tooltip'
import { getLocale } from '../../../../common/locale'
// Styled Components
import {
  StyledWrapper,
  // MainContainer,
  // MainContainerWrapper
  ButtonRow,
  TabButton,
  TabButtonText,
  ButtonDivider,
  BuyButtonIcon,
  SendButtonIcon,
  SwapButtonIcon,
  DepositButtonIcon
  // RightDivider,
  // LeftDivider
} from './style'

export interface Props {
  children?: React.ReactNode
  selectedTab: BuySendSwapTypes
  onChangeTab: (tab: BuySendSwapTypes) => () => void
  isBuyDisabled: boolean
  isSwapDisabled: boolean
  selectedNetwork: BraveWallet.NetworkInfo
}

function BuySendSwapLayout (props: Props) {
  const {
    // children,
    selectedTab,
    onChangeTab,
    isBuyDisabled,
    isSwapDisabled,
    selectedNetwork
  } = props

  const getTooltipLocaleKey = (optionId: string): string => {
    if (optionId === 'buy') {
      return 'braveWalletBuyNotSupportedTooltip'
    }

    if (optionId === 'swap') {
      return 'braveWalletSwapNotSupportedTooltip'
    }

    return ''
  }

  const getButtonIcon = (optionId: string) => {
    switch (optionId) {
      case 'buy':
        return <BuyButtonIcon />

      case 'send':
        return <SendButtonIcon />

      case 'swap':
        return <SwapButtonIcon />

      case 'deposit':
        return <DepositButtonIcon />

      default:
        return ''
    }
  }

  return (
    <StyledWrapper>
      <ButtonRow>
        {BuySendSwapOptions().map((option, index) =>
          <Tooltip
            isDisabled={isBuyDisabled && option.id === 'buy' || isSwapDisabled && option.id === 'swap'}
            key={option.id}
            text={getLocale(getTooltipLocaleKey(option.id)).replace('$1', reduceNetworkDisplayName(selectedNetwork.chainName))}
          >
            <TabButton
              isSelected={selectedTab === option.id}
              onClick={onChangeTab(option.id)}
              isDisabled={isBuyDisabled && option.id === 'buy' || isSwapDisabled && option.id === 'swap'}
              disabled={isBuyDisabled && option.id === 'buy' || isSwapDisabled && option.id === 'swap'}
            >
              {getButtonIcon(option.id)}
              <TabButtonText
                isSelected={selectedTab === option.id}
                isDisabled={isBuyDisabled && option.id === 'buy' || isSwapDisabled && option.id === 'swap'}
              >
                {option.name}
              </TabButtonText>
            </TabButton>
            {index !== BuySendSwapOptions().length - 1 && <ButtonDivider />}
          </Tooltip>
        )}
      </ButtonRow>
      {/* <MainContainerWrapper>
        <MainContainer selectedTab={selectedTab}>
          {children}
        </MainContainer>
      </MainContainerWrapper> */}
    </StyledWrapper>
  )
}

export default BuySendSwapLayout
