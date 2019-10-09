// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at http://mozilla.org/MPL/2.0/.

export default function RunOrTimeout<T> (mainOp: Promise<T>, timeMs: number, onTimeout?: () => void): Promise<T> {
  return new Promise<T>((resolve, reject) => {
    const timerId = setTimeout(() => {
      reject(`Operation timeout after ${timeMs}ms.`)
    }, timeMs)
    mainOp.then((result: T) => {
      clearTimeout(timerId)
      resolve(result)
    }).catch((error?: any) => {
      clearTimeout(timerId)
      reject(error)
    })
  })
}