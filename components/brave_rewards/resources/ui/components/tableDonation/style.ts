/* This Source Code Form is subject to the terms of the Mozilla Public
 * License. v. 2.0. If a copy of the MPL was not distributed with this file.
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import styled from 'styled-components'

export const tableDonation = styled.div`
  [class^='StyledTD'] {
    font-family: var(--brave-font-default);
  }
`

export const remove = styled.span`
  button {
    font-size: 14px;
    line-height: 0;
    height: 20px;
    color: #9E9FAB;
    padding: 0;
    border: none;
    background: none;
    cursor: pointer;
    display: inline-block;
  }
`

export const removeIcon = styled.span`
  vertical-align: middle;
  color: #9E9FAB;
  width: 12px;
  height: 12px;
  display: inline-block;
  margin-right: 4px;
`

export const type = styled.div`
  font-size: 14px;
  font-weight: 600;
  color: #686978;
`

export const date = styled.div`
  font-size: 14px;
  line-height: 1;
  margin-top: 3px;
  color: #b8b9c4;
`

export const tipDate = styled.div`
  font-size: 14px;
  line-height: 1;
  margin-top: 0px;
  color: #b8b9c4;
`

export const toggleWrap = styled.div`
  text-align: right;

  button {
    font-size: 13px;
    color: #4c54d2;
    padding: 0;
    border: none;
    background: none;
    cursor: pointer;
  }
`

export const link = styled.div`
  text-decoration: none;
`
