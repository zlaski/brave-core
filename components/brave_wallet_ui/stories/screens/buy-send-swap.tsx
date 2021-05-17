import * as React from 'react'
import { BuySendSwapTypes, UserAccountType } from '../../constants/types'
import Swap from './swap-tab'
import {
  Layout
} from '../../components/buy-send-swap'

export interface Props {
  accounts: UserAccountType[]
}

function BuySendSwap (props: Props) {
  const { accounts } = props
  const [selectedTab, setSelectedTab] = React.useState<BuySendSwapTypes>('swap')

  const changeTab = (tab: BuySendSwapTypes) => () => {
    setSelectedTab(tab)
  }

  return (
    <Layout selectedTab={selectedTab} onChangeTab={changeTab}>
      { selectedTab === 'swap' &&
        <Swap accounts={accounts} />
      }
    </Layout>
  )
}

export default BuySendSwap
