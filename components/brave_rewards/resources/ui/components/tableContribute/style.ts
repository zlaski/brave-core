/* This Source Code Form is subject to the terms of the Mozilla Public
 * License. v. 2.0. If a copy of the MPL was not distributed with this file.
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import styled from 'styled-components'

export const tableContribute = styled.div`
  [class^='StyledTD'] {
    font-family: var(--brave-font-default);
  }
`

export const text = styled.div`
  font-size: 14px;
  text-align: right;
  color: #686978;
`

export const remove = styled.div`
  button {
    margin: 0 8px;
    background: none;
    border: none;
    cursor: pointer;
    width: 16px;
    height: 16px;
    color: #9E9FAB;
    padding: 0;
  }
`

export const thOther = styled.div`
  text-align: right;
`

export const thLast = styled.div`
  text-align: right;
  padding-right: 10px;
`

export const toggleWrap = styled.div`
  text-align: right;

  button {
    font-size: 13px;
    color: #4c54d2;
    text-transform: capitalize;
    background: none;
    border: none;
    padding: 0;
    cursor: pointer;
  }
`

export const link = styled.div`
  text-decoration: none;
`
