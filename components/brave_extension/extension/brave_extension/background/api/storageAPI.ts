/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import { debounce } from '../../../../../common/debounce'
import * as Shields from '../../types/state/shieldsPannelState'

const keyName = 'shields-persistent-data'

export const defaultPersistentData: Shields.State = {
  persistentData: { isFirstAccess: true },
  tabs: { },
  windows: { },
  currentWindowId: -1
}

export const loadPersistentData = (): Shields.State => {
  const dataRaw = window.localStorage.getItem(keyName)
  let state: Shields.State = defaultPersistentData
  if (dataRaw) {
    try {
      const data = JSON.parse(dataRaw)
      if (data && data.tabs && data.persistentData) {
        state = data
      }
    } catch (e) {
      console.error('[Shields] Could not parse local storage data: ', e)
    }
  }
  return state
}

export const savePersistentDataDebounced = debounce((data: Shields.State) => {
  if (data) {
    window.localStorage.setItem(keyName, JSON.stringify(data))
  }
}, 50)
