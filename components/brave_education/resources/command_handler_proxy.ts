/* Copyright (c) 2024 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

import { CommandHandlerFactory, CommandHandlerRemote } from 'gen/brave/components/brave_education/common/brave_education.mojom.m.js'

let instance: CommandHandlerProxy | null = null;

export class CommandHandlerProxy {
  static getInstance (): CommandHandlerProxy {
    return instance || (instance = new CommandHandlerProxy());
  }

  handler: CommandHandlerRemote;

  constructor () {
    this.handler = new CommandHandlerRemote();

    CommandHandlerFactory.getRemote().createEducationCommandHandler(
        this.handler.$.bindNewPipeAndPassReceiver());
  }
}
