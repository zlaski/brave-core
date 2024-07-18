// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'
import * as mojom from 'gen/brave/components/ai_chat/core/common/mojom/ai_chat.mojom.m.js'
import * as API from '../api/page_handler'

interface AIChatContext {
  VisibleConversations: mojom.Conversation[]
}

const defaultContext: AIChatContext = {
  VisibleConversations: []
}

export const AIChatReactContext =
    React.createContext<AIChatContext>(defaultContext)

export function AIChatContextProvider(props: React.PropsWithChildren<{}>) {
  const [context, setContext] = React.useState<AIChatContext>(defaultContext)

  const setPartialContext = (partialContext: Partial<AIChatContext>) => {
    setContext(value => ({
      ...value,
      ...partialContext
    }))
    console.log('AIChatContext update', partialContext)
  }

  React.useEffect(() => {
    API.Service.getVisibleConversations().then(result => {
      setPartialContext({
        VisibleConversations: result.conversations
      })
    })

    API.ServiceObserver.onConversationListChanged.addListener((conversations: mojom.Conversation[]) => {
      setPartialContext({
        VisibleConversations: conversations
      })
    })
  }, [])

  return (
    <AIChatReactContext.Provider value={context}>
      {props.children}
    </AIChatReactContext.Provider>
  )
}

export function useAIChat() {
  return React.useContext(AIChatReactContext)
}
