import * as React from 'react'
import { UserAccountType } from '../../../constants/types'
import { SearchBar, SelectAccount } from '../../shared'
import Header from '../../buy-send-swap/select-header'
import locale from '../../../constants/locale'
// Styled Components
import {
  SelectWrapper,
  SelectScrollSearchContainer
} from '../../buy-send-swap/shared-styles'

export interface Props {
  accounts: UserAccountType[]
  onSelectAccount: (account: UserAccountType) => () => void
  onBack: () => void
}

function SelectAccountWithHeader (props: Props) {
  const { accounts, onSelectAccount, onBack } = props
  return (
    <SelectWrapper>
      <Header title={locale.selectAccount} onBack={onBack} />
      <SearchBar placeholder={locale.searchAccount} />
      <SelectScrollSearchContainer>
        <SelectAccount
          accounts={accounts}
          onSelectAccount={onSelectAccount}
        />
      </SelectScrollSearchContainer>
    </SelectWrapper>
  )
}

export default SelectAccountWithHeader
