// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'
import * as mojom from 'gen/brave/components/ai_chat/core/common/mojom/ai_chat.mojom.m.js'
import * as API from '../api/page_handler'

interface ConversationContextProps {
  conversationId: string | undefined
}

interface ConversationContext {
  conversationHistory: mojom.ConversationTurn[]
  associatedContentInfo?: mojom.SiteInfo
}

const defaultContext: ConversationContext = {
  conversationHistory: []
}

export const ConverationReactContext =
    React.createContext<ConversationContext>(defaultContext)

export function ConversationContextProvider(props: React.PropsWithChildren<ConversationContextProps>) {
  const { conversationHandler, callbackRouter } = React.useMemo(() => {
    if (props.conversationId)
      return API.getConversation(props.conversationId)
    else
      return API.getInitialConversation()
  }, [props.conversationId])

  const [context, setContext] = React.useState<ConversationContext>(defaultContext)

  const setPartialContext = (partialContext: Partial<ConversationContext>) => {
    setContext(value => ({
      ...value,
      ...partialContext
    }))
  }

  // Initialization
  React.useEffect(() => {
    async function updateHistory() {
      const { conversationHistory } = await conversationHandler.getConversationHistory()
      setPartialContext({
        conversationHistory
      })
    }

    // Bind the conversation handler
    callbackRouter.onConversationHistoryUpdate.addListener(() => {
      updateHistory()
    })

    // Initial data
    // TODO(petemill): have a single mojom call that provides the initial state
    updateHistory()
  }, [conversationHandler, callbackRouter])

  return (
    <ConverationReactContext.Provider value={context}>
      {props.children}
    </ConverationReactContext.Provider>
  )
}

export function useConversation() {
  return React.useContext(ConverationReactContext)
}
