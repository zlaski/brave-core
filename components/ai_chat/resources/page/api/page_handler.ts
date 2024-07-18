/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

// TODO(petemill): rename to AIChat
import * as AIChatUI from 'gen/brave/components/ai_chat/core/common/mojom/ai_chat.mojom.m.js'

// Provide access to all the generated types
export * from 'gen/brave/components/ai_chat/core/common/mojom/ai_chat.mojom.m.js'

interface API {
  pageHandler: AIChatUI.PageHandlerRemote
  callbackRouter: AIChatUI.ChatUIPageCallbackRouter
}

let apiInstance: API

// TODO: lazy-load
export const Service: AIChatUI.ServiceRemote = AIChatUI.Service.getRemote()
export const ServiceObserver: AIChatUI.ChatUICallbackRouter = new AIChatUI.ChatUICallbackRouter()

Service.bindUI(ServiceObserver.$.bindNewPipeAndPassRemote())

class PageHandlerAPI implements API {
  pageHandler: AIChatUI.PageHandlerRemote
  callbackRouter: AIChatUI.ChatUIPageCallbackRouter

  constructor() {
    this.pageHandler = AIChatUI.PageHandler.getRemote()
    this.callbackRouter = new AIChatUI.ChatUIPageCallbackRouter()
    this.pageHandler.setClientPage(
      this.callbackRouter.$.bindNewPipeAndPassRemote()
    )
  }
}

export function setPageHandlerAPIForTesting(instance: API) {
  apiInstance = instance
}

export default function getPageHandlerInstance() {
  if (!apiInstance) {
    apiInstance = new PageHandlerAPI()
  }
  return apiInstance
}

export function getConversation(id: string) {
  let conversationHandler: AIChatUI.ConversationHandlerRemote =
    new AIChatUI.ConversationHandlerRemote()
  let callbackRouter = new AIChatUI.ChatUIPageCallbackRouter()

  Service.bindConversation(
    id,
    conversationHandler.$.bindNewPipeAndPassReceiver(),
    callbackRouter.$.bindNewPipeAndPassRemote()
  )
  return {
    conversationHandler,
    callbackRouter
  }
}

export function getInitialConversation() {
  let conversationHandler: AIChatUI.ConversationHandlerRemote =
    new AIChatUI.ConversationHandlerRemote()
  let callbackRouter = new AIChatUI.ChatUIPageCallbackRouter()

  getPageHandlerInstance().pageHandler.bindInitialConversation(
    conversationHandler.$.bindNewPipeAndPassReceiver(),
    callbackRouter.$.bindNewPipeAndPassRemote()
  )

  return {
    conversationHandler,
    callbackRouter
  }
}
