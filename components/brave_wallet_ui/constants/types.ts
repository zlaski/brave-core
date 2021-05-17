export interface WalletAccountType {
  id: string
  name: string
  address: string
  balance: number
  asset: string
}

export interface UserAccountType {
  id: string
  name: string
  address: string
}

export interface AssetOptionType {
  id: string
  name: string
  symbol: string
  icon: string
}

export interface RPCAssetType {
  id: string
  name: string
  symbol: string
  balance: number
}

export interface RPCTransactionType {
  assetId: string
  amount: number
  to: string
  from: string
  hash: string
}

export interface RPCResponseType {
  address: string
  assets: RPCAssetType[]
  transactions: RPCTransactionType[]
}

export type AmountPresetTypes =
  | 0.25
  | 0.50
  | 0.75
  | 1

export type PanelTypes =
  | 'main'
  | 'buy'
  | 'send'
  | 'swap'
  | 'apps'
  | 'networks'
  | 'settings'
  | 'expanded'
  | 'accounts'

export type NavTypes =
  | 'crypto'
  | 'rewards'
  | 'cards'
  | 'linked_accounts'
  | 'gemini'
  | 'creators'
  | 'lock_wallet'

export type TopTabNavTypes =
  | 'portfolio'
  | 'nfts'
  | 'invest'
  | 'lending'
  | 'apps'
  | 'accounts'

export type BuySendSwapTypes =
  | 'buy'
  | 'send'
  | 'swap'

export type ChartTimelineType =
  | '5MIN'
  | '24HRS'
  | '7Day'
  | '1Month'
  | '3Months'
  | '1Year'
  | 'AllTime'

export type OrderTypes =
  | 'market'
  | 'limit'

export type SwapViewTypes =
  | 'swap'
  | 'acounts'
  | 'networks'
  | 'assets'

export interface BuySendSwapObjectType {
  name: string
  id: BuySendSwapTypes
}

export interface TopTabNavObjectType {
  name: string
  id: TopTabNavTypes
}

export interface NavObjectType {
  name: string
  primaryIcon: string
  secondaryIcon: string
  id: NavTypes
}

export interface AppObjectType {
  name: string
  description: string
  url: string
  icon: string
}

export interface AppsListType {
  category: string
  categoryButtonText?: string
  appList: AppObjectType[]
}

export interface ChartTimelineObjectType {
  name: string
  id: ChartTimelineType
}

export interface PriceDataObjectType {
  date: string
  close: number
}

export interface WalletState {
}

export interface PanelState {
  hasInitialized: boolean
  isConnected: boolean
  connectedSiteOrigin: string
  accounts: WalletAccountType[]
}

export interface PageState {
  hasInitialized: boolean
}

export interface WalletPageState {
  wallet: WalletState
  page: PageState
}

export interface WalletPanelState {
  wallet: WalletState
  panel: PanelState
}

export interface AmountPresetObjectType {
  name: string
  id: AmountPresetTypes
}

export interface SlippagePresetObjectType {
  id: number
  slippage: number
}

export interface ExpirationPresetObjectType {
  id: number
  name: string
  expiration: number
}

export interface NetworkOptionsType {
  name: string
  id: number
  abbr: string
}
