import styled from 'styled-components'
import { BuySendSwapTypes } from '../../../constants/types'
import { WalletButton } from '../../shared/style'
import CoinsIcon from '../../../assets/svg-icons/coins-icon-1.svg'
import SendIcon from '../../../assets/svg-icons/send-icon.svg'
import CurrencyExchangeIcon from '../../../assets/svg-icons/currency-exchange.svg'
import MoneyBagCoinIcon from '../../../assets/svg-icons/money-bag-coins-icon.svg'

interface StyleProps {
  isSelected: boolean
  selectedTab: BuySendSwapTypes
  tabID: BuySendSwapTypes
  isDisabled: boolean
}

export const StyledWrapper = styled.div`
  position: relative;
  display: flex;
  height: 100%;
  width: 100%;
  flex-direction: column;
  align-items: center;
  justify-content: center;
`

export const MainContainerWrapper = styled.div<Partial<StyleProps>>`
  display: flex;
  height: 100%;
  width: 100%;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  background-color: ${(p) => p.theme.color.background01};
  border-radius: 0px 0px 8px 8px;
`

export const MainContainer = styled.div<Partial<StyleProps>>`
  position: relative;
  display: flex;
  width: 100%;
  flex-direction: column;
  align-items: center;
  justify-content: flex-start;
  padding: 18px;
  background-color: ${(p) => p.theme.color.background02};
  border: ${(p) => `2px solid ${p.theme.color.divider01}`};
  border-radius: ${(p) =>
    p.selectedTab === 'buy' ? '0px 8px 8px 8px' : p.selectedTab === 'swap' ? '8px 0px 8px 8px' : '8px'};
`

export const ButtonRow = styled.div`
  display: flex;
  width: 100%;
  flex-direction: column;
  align-items: center;
  box-sizing: border-box;
  background: ${p => p.theme.color.background02};
  border: ${p => `2px solid ${p.theme.color.divider01}`};
  /* border-bottom-width: 1px; */
  border-radius: 8px;
  overflow: hidden;
`

export const TabButton = styled(WalletButton) <Partial<StyleProps>>`
  flex: 1;
  display: flex;
  height: 55px;
  align-items: center;
  justify-content: flex-start;
  flex-direction: row;
  gap: 10px;
  cursor: ${(p) => p.isDisabled ? 'default' : 'pointer'};
  outline: none;
  background: ${(p) => p.isSelected ? p.theme.color.divider01 : p.theme.color.background02};
  border: none;
  border-radius: 0px;
  padding: 18px;
  position: relative;
  pointer-events: ${(p) => p.disabled ? 'none' : 'auto'};
`

export const TabButtonText = styled.span<Partial<StyleProps>>`
  font-family: Poppins;
  font-size: 16px;
  line-height: 24px;
  font-weight: 600;
  -webkit-background-clip: text;
  color: ${p => p.theme.color.text02};
`

export const ButtonDivider = styled.div`
  display: flex;
  width: 100%;
  height: 0px;
  border-bottom: ${(p) => `1px solid ${p.theme.color.divider01}`};
`

export const ButtonWrapper = styled.div`
  display: flex;
  position: relative;
  width: 100%;
  height: 100%;
`

export const ButtonIcon = styled.div`
  width: 20px;
  height: 20px;
  background-color: ${(p) => p.theme.color.text02};
  mask-size: 100%;
`
export const BuyButtonIcon = styled(ButtonIcon)`
  -webkit-mask-image: url(${CoinsIcon});
  mask-image: url(${CoinsIcon});
`

export const SendButtonIcon = styled(ButtonIcon)`
  -webkit-mask-image: url(${SendIcon});
  mask-image: url(${SendIcon});
`

export const SwapButtonIcon = styled(ButtonIcon)`
  -webkit-mask-image: url(${CurrencyExchangeIcon});
  mask-image: url(${CurrencyExchangeIcon});
`

export const DepositButtonIcon = styled(ButtonIcon)`
  -webkit-mask-image: url(${MoneyBagCoinIcon});
  mask-image: url(${MoneyBagCoinIcon});
`
