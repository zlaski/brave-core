import * as React from 'react'
import Header from '../select-header'
// Styled Components
import {
  SelectWrapper,
  SelectScrollContainer
} from '../shared-styles'

export interface Props {
  onBack: () => void
}

function SelectSlippage (props: Props) {
  const { onBack } = props
  return (
    <SelectWrapper>
      <Header title='Select Slippage' onBack={onBack} />
      <SelectScrollContainer />
    </SelectWrapper>
  )
}

export default SelectSlippage
