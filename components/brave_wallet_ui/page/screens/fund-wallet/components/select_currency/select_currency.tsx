// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'
import { DialogProps } from '@brave/leo/react/dialog'
import Icon from '@brave/leo/react/icon'

// Types
import { MeldFiatCurrency } from '../../../../../constants/types'

// Utils
import { getLocale } from '../../../../../../common/locale'

// Styled Components
import { Column, Row } from '../../../../../components/shared/style'
import {
  CurrencyImage,
  CurrencyName,
  CurrencyCode,
  SearchInput,
  SelectedLabel
} from './select_currency.style'
import { ContainerButton, Dialog, DialogTitle } from '../shared/style'

interface SelectCurrencyProps extends DialogProps {
  currencies: MeldFiatCurrency[]
  selectedCurrency?: MeldFiatCurrency
  onSelectCurrency: (currency: MeldFiatCurrency) => void
}

export const CurrencyListItem = ({
  currency,
  isSelected,
  onSelect
}: {
  currency: MeldFiatCurrency
  isSelected?: boolean
  onSelect: (currency: MeldFiatCurrency) => void
}) => {
  return (
    <ContainerButton onClick={() => onSelect(currency)}>
      <Row
        justifyContent='flex-start'
        gap='16px'
      >
        <CurrencyImage src={`chrome://image?${currency.symbolImageUrl}`} />
        <CurrencyName>{currency.name}</CurrencyName>
      </Row>
      <Row
        justifyContent='flex-end'
        gap='16px'
      >
        {isSelected && (
          <SelectedLabel>{getLocale('braveWalletSelected')}</SelectedLabel>
        )}
        <CurrencyCode>{currency.currencyCode}</CurrencyCode>
      </Row>
    </ContainerButton>
  )
}

export const SelectCurrency = (props: SelectCurrencyProps) => {
  const { currencies, selectedCurrency, onSelectCurrency, ...rest } = props

  // State
  const [searchText, setSearchText] = React.useState('')

  // Memos
  const searchResults = React.useMemo(() => {
    if (searchText === '') return currencies

    return currencies.filter((currency) => {
      return (
        currency?.name?.toLowerCase().includes(searchText.toLowerCase()) ||
        currency.currencyCode.toLowerCase().includes(searchText.toLowerCase())
      )
    })
  }, [currencies, searchText])

  return (
    <Dialog
      {...rest}
      showClose
      size='mobile'
    >
      <DialogTitle slot='title'>
        {getLocale('braveWalletSelectCurrency')}
      </DialogTitle>
      <Row
        padding='24px 0 0 0'
        slot='subtitle'
      >
        <SearchInput
          placeholder={getLocale('braveWalletSearchCurrency')}
          onInput={(e) => setSearchText(e.value)}
        >
          <Icon
            name='search'
            slot='left-icon'
          />
        </SearchInput>
      </Row>
      <Column
        width='100%'
        height='80vh'
        justifyContent='flex-start'
      >
        {searchResults.length === 0 ? (
          <Row
            justifyContent='center'
            alignItems='center'
          >
            {getLocale('braveWalletNoAvailableCurrencies')}
          </Row>
        ) : (
          searchResults.map((currency) => (
            <CurrencyListItem
              key={currency.currencyCode}
              currency={currency}
              onSelect={onSelectCurrency}
              isSelected={
                currency.currencyCode === selectedCurrency?.currencyCode
              }
            />
          ))
        )}
      </Column>
    </Dialog>
  )
}
