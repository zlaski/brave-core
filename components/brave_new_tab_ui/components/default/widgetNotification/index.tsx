/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/. */

import * as React from 'react'
import config from './data'
import {
  StyledWrapper, StyledInfoArea, StyledTitle, StyledCopy, StyledIcon, StyledAddButton, StyledActions, StyledDismiss, StyledIconButton, StyledNewCard
} from './style'
import { PlusIcon } from 'brave-ui/components/icons'


interface Props {
  id: string
  onAddWidget: () => void
  onDismissWidget: () => void
}

export default class WidgetNotification extends React.PureComponent<Props, {}> {

  render () {
    const notificationInfo = config[this.props.id]
  
    return (
      <StyledWrapper>
        <StyledNewCard>
          {'NEW CARD'}
        </StyledNewCard>
        <StyledInfoArea>
          <StyledTitle>
            <StyledIcon>
              <notificationInfo.icon/>
            </StyledIcon>
            {notificationInfo.title}
          </StyledTitle>
          <StyledCopy>
            {notificationInfo.copy}
          </StyledCopy>
        </StyledInfoArea>
        <StyledActions>
          <StyledAddButton onClick={this.props.onAddWidget}>
            <StyledIconButton>
              <PlusIcon />
            </StyledIconButton>
            {`Add ${notificationInfo.title}`}
          </StyledAddButton>
          <StyledDismiss onClick={this.props.onDismissWidget}>
            {`Dismiss`}
          </StyledDismiss>
        </StyledActions>
      </StyledWrapper>
    )
  }
}
