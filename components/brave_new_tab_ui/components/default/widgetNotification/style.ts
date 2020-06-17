// Copyright (c) 2020 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import styled from 'brave-ui/theme'

export const StyledWrapper = styled<{}, 'div'>('div')`
  padding: 25px;
  padding-top: 40px;
  background: white;
  max-height: 280px;
  width: 284px;
  overflow: hidden;
  font-family: ${p => p.theme.fontFamily.body};
  border-radius: 6px;
  position: absolute;
  margin-left: -310px;

  &:before {
    content: "";
    position: absolute;
    top: 100%;
    left: 20px;
    width: 0;
    border-top: 20px solid black;
    border-left: 20px solid transparent;
    border-right: 20px solid transparent;
  }
`

export const StyledInfoArea = styled<{}, 'div'>('div')`
  padding: 15px;
  color: #000;
  border-radius: 6px;
  border: 1px solid #d3d3d3;
  border-bottom: none;
`

export const StyledTitle = styled<{}, 'div'>('div')`
  font-weight: 600;
  font-size: 14px;
`

export const StyledIcon = styled<{}, 'div'>('div')`
  display: inline-block;
  margin-right: 10px;
  vertical-align: middle;
`

export const StyledCopy = styled<{}, 'p'>('p')`

`

export const StyledActions = styled<{}, 'div'>('div')`
  padding: 15px 15px 0px 15px;
  text-align: center;
  color: #000;
`

export const StyledAddButton = styled<{}, 'button'>('button')`
  font-size: 13px;
  color: white;
  font-weight: 600;
  padding: 10px 30px;
  border-radius: 100px;
  background: #FB542B;
  border: none;
  cursor: pointer;
`

export const StyledDismiss = styled<{}, 'span'>('span')`
  font-weight: bold;
  font-size: 13px;
  display: block;
  cursor:pointer;
  margin-top: 15px;
`

export const StyledPointer = styled<{}, 'div'>('div')`
  width: 0;
  height: 0;
  border-top: 50px solid transparent;
  border-left: 100px solid white;
  border-bottom: 50px solid transparent;
`

export const StyledIconButton = styled<{}, 'div'>('div')`
  display: inline-block;
  width: 18px;
  height: 18px;
  vertical-align: top;
  margin-right: 4px;
`

export const StyledNewCard = styled<{}, 'div'>('div')`
  border-top-left-radius: 5px;
  border-bottom-right-radius: 6px;
  font-weight: 600;
  font-size: 14px;
  background: linear-gradient(168.67deg,#4C54D2 0%,#BF14A2 56.25%,#F73A1C 100%);
  top: 0;
  left: 0;
  position: absolute;
  padding: 3px 8px;
  color: white;
`
