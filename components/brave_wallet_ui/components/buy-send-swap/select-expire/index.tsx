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

function SelectExpire (props: Props) {
  const { onBack } = props
  return (
    <SelectWrapper>
      <Header title='Select Expire' onBack={onBack} />
      <SelectScrollContainer />
    </SelectWrapper>
  )
}

export default SelectExpire
