import * as React from 'react'
import SelectAssetItem from '../select-asset-item'
import { AssetOptionType } from '../../../constants/types'
import { SearchBar } from '../../shared'
import Header from '../select-header'
import locale from '../../../constants/locale'
// Styled Components
import {
  SelectWrapper,
  SelectScrollSearchContainer
} from '../shared-styles'

export interface Props {
  assets: AssetOptionType[]
  onSelectAsset: (asset: AssetOptionType) => () => void
  onBack: () => void
}

function SelectAsset (props: Props) {
  const { assets, onBack, onSelectAsset } = props
  return (
    <SelectWrapper>
      <Header title={locale.selectAsset} onBack={onBack} />
      <SearchBar placeholder={locale.searchAsset} />
      <SelectScrollSearchContainer>
        {assets.map((asset) => <SelectAssetItem key={asset.id} asset={asset} onSelectAsset={onSelectAsset(asset)} />)}
      </SelectScrollSearchContainer>
    </SelectWrapper>
  )
}

export default SelectAsset
