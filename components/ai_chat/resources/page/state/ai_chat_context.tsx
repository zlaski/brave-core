// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'
import getAPI, * as mojom from '../api'
import { loadTimeData } from '$web-common/loadTimeData'

export interface AIChatContext {
  visibleConversations: mojom.Conversation[]
  hasAcceptedAgreement: boolean
  isPremiumStatusFetching: boolean
  isPremiumUser: boolean
  isPremiumUserDisconnected: boolean
  canShowPremiumPrompt?: boolean
  isMobile: boolean
  allActions: mojom.ActionGroup[]
  goPremium: () => void
  managePremium: () => void
  handleAgreeClick: () => void
  dismissPremiumPrompt: () => void
  userRefreshPremiumSession: () => void,
  uiHandler?: mojom.AIChatUIHandlerRemote
}

const defaultContext: AIChatContext = {
  visibleConversations: [],
  hasAcceptedAgreement: loadTimeData.getBoolean('hasAcceptedAgreement'),
  isPremiumStatusFetching: true,
  isPremiumUser: false,
  isPremiumUserDisconnected: false,
  canShowPremiumPrompt: undefined,
  isMobile: loadTimeData.getBoolean('isMobile'),
  allActions: [],
  goPremium: () => {},
  managePremium: () => {},
  handleAgreeClick: () => {},
  dismissPremiumPrompt: () => {},
  userRefreshPremiumSession: () => {},
}

export const AIChatReactContext =
  React.createContext<AIChatContext>(defaultContext)

export function AIChatContextProvider(props: React.PropsWithChildren<{}>) {
  const [context, setContext] = React.useState<AIChatContext>(defaultContext)

  const setPartialContext = (partialContext: Partial<AIChatContext>) => {
    setContext((value) => ({
      ...value,
      ...partialContext
    }))
    console.log('AIChatContext update', partialContext)
  }

  React.useEffect(() => {
    const { Service, Observer } = getAPI()
    async function initialize() {
      const [
        { conversations: visibleConversations },
        { actionList: allActions },
        { canShow: canShowPremiumPrompt }
      ] = await Promise.all([
        Service.getVisibleConversations(),
        Service.getActionMenuList(),
        Service.getCanShowPremiumPrompt()
      ])
      setPartialContext({
        visibleConversations,
        allActions,
        canShowPremiumPrompt
      })
    }

    async function updateCurrentPremiumStatus () {
      console.debug('Getting premium status...')
      const { status } = await getAPI().Service.getPremiumStatus()
      console.debug('got premium status: ', status)
      // setPremiumStatus(status)
      setPartialContext({
        isPremiumStatusFetching: false,
        isPremiumUser: (status !== undefined && status !== mojom.PremiumStatus.Inactive),
        isPremiumUserDisconnected: status === mojom.PremiumStatus.ActiveDisconnected
      })
    }

    initialize()
    updateCurrentPremiumStatus()

    Observer.onConversationListChanged.addListener(
      (conversations: mojom.Conversation[]) => {
        setPartialContext({
          visibleConversations: conversations
        })
      }
    )
    Observer.onAgreementAccepted.addListener(() =>
      setPartialContext({
        hasAcceptedAgreement: true
      })
    )

    // Since there is no server-side event for premium status changing,
    // we should check often. And since purchase or login is performed in
    // a separate WebContents, we can check when focus is returned here.
    window.addEventListener('focus', () => {
      updateCurrentPremiumStatus()
    })

    document.addEventListener('visibilitychange', (e) => {
      if (document.visibilityState === 'visible') {
        updateCurrentPremiumStatus()
      }
    })
  }, [])

  const { Service, UIHandler } = getAPI()

  const store: AIChatContext = {
    ...context,
    goPremium: UIHandler.goPremium,
    managePremium: UIHandler.managePremium,
    dismissPremiumPrompt: Service.dismissPremiumPrompt,
    userRefreshPremiumSession: UIHandler.refreshPremiumSession,
    handleAgreeClick: Service.markAgreementAccepted,
    uiHandler: UIHandler
  }

  return (
    <AIChatReactContext.Provider value={store}>
      {props.children}
    </AIChatReactContext.Provider>
  )
}

export function useAIChat() {
  return React.useContext(AIChatReactContext)
}
