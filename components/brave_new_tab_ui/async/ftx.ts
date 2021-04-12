// Copyright (c) 2020 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import AsyncActionHandler from '../../common/AsyncActionHandler'
import * as Actions from '../actions/ftx_actions'
// import { ApplicationState } from '../reducers'

const handler = new AsyncActionHandler()

handler.on(Actions.startConnectToFtx.getType(), async (store, payload) => {
  chrome.ftx.getClientUrl(url => {
    window.open(url, '_self', 'noopener')
  })
})

export default handler.middleware
