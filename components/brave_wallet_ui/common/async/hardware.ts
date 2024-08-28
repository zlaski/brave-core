// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import { assertNotReached } from 'chrome://resources/js/assert.js'
import {
  SignHardwareTransactionType,
  SignHardwareOperationResult,
  FetchHardwareWalletAccountsProps,
  GetAccountsHardwareOperationResult,
  SignMessageHardwareOperationResult
} from '../hardware/types'
import { StatusCodes as LedgerStatusCodes } from '@ledgerhq/errors'
import { getLocale } from '../../../common/locale'
import type WalletApiProxy from '../../common/wallet_api_proxy'
import {
  getHardwareKeyring,
  getLedgerEthereumHardwareKeyring,
  getLedgerFilecoinHardwareKeyring,
  getLedgerSolanaHardwareKeyring,
  getTrezorHardwareKeyring
} from '../api/hardware_keyrings'

import { TrezorErrorsCodes } from '../hardware/trezor/trezor-messages'
import TrezorBridgeKeyring from '../hardware/trezor/trezor_bridge_keyring'
import EthereumLedgerBridgeKeyring from '../hardware/ledgerjs/eth_ledger_bridge_keyring'
import SolanaLedgerBridgeKeyring from '../hardware/ledgerjs/sol_ledger_bridge_keyring'
import {
  BraveWallet,
  HardwareWalletResponseCodeType,
  SerializableTransactionInfo
} from '../../constants/types'
import {
  LedgerEthereumKeyring,
  LedgerFilecoinKeyring,
  LedgerSolanaKeyring
} from '../hardware/interfaces'
import { EthereumSignedTx } from '../hardware/trezor/trezor-connect-types'
import { FilSignedLotusMessage } from '../hardware/ledgerjs/ledger-messages'

export function dialogErrorFromLedgerErrorCode(
  code: string | number
): HardwareWalletResponseCodeType {
  if (code === 'TransportOpenUserCancelled') {
    return 'deviceNotConnected'
  }

  if (code === 'TransportLocked') {
    return 'deviceBusy'
  }

  if (code === LedgerStatusCodes.CONDITIONS_OF_USE_NOT_SATISFIED) {
    return 'transactionRejected'
  }

  return 'openLedgerApp'
}

export function dialogErrorFromTrezorErrorCode(
  code: TrezorErrorsCodes | string
): HardwareWalletResponseCodeType {
  if (code === TrezorErrorsCodes.CommandInProgress) {
    return 'deviceBusy'
  }
  if (code === 'Method_Interrupted') {
    return 'transactionRejected'
  }
  return 'openLedgerApp'
}

export async function signTrezorTransaction(
  apiProxy: WalletApiProxy,
  path: string,
  txInfo: Pick<SerializableTransactionInfo, 'id' | 'chainId' | 'txDataUnion'>,
  deviceKeyring: TrezorBridgeKeyring = getTrezorHardwareKeyring()
): Promise<SignHardwareTransactionType> {
  const nonce = await apiProxy.ethTxManagerProxy.getNonceForHardwareTransaction(
    txInfo.chainId,
    txInfo.id
  )
  if (!nonce || !nonce.nonce) {
    return {
      success: false,
      error: getLocale('braveWalletApproveTransactionError')
    }
  }
  if (!txInfo.txDataUnion.ethTxData1559) {
    return {
      success: false,
      error: getLocale('braveWalletApproveTransactionError')
    }
  }
  const tx = {
    ...txInfo,
    txDataUnion: {
      ...txInfo.txDataUnion,
      ethTxData1559: {
        ...txInfo.txDataUnion.ethTxData1559,
        baseData: {
          ...txInfo.txDataUnion.ethTxData1559?.baseData,
          nonce: nonce.nonce
        }
      }
    }
  } as SerializableTransactionInfo
  const signed = await deviceKeyring.signTransaction(path, tx, tx.chainId)
  if (!signed || !signed.success || !signed.payload) {
    const error = (
      signed.error ? signed.error : getLocale('braveWalletSignOnDeviceError')
    ) as string
    if (signed.code === TrezorErrorsCodes.CommandInProgress) {
      return { success: false, error: error, deviceError: 'deviceBusy' }
    }
    return { success: false, error: error }
  }
  const ethereumSignedTx = signed.payload as EthereumSignedTx
  if (!ethereumSignedTx) {
    return { success: false }
  }
  const { v, r, s } = ethereumSignedTx
  const result = await apiProxy.ethTxManagerProxy.processEthHardwareSignature(
    tx.id,
    v,
    r,
    s
  )
  if (!result.status) {
    return {
      success: false,
      error: getLocale('braveWalletProcessTransactionError')
    }
  }
  return { success: result.status }
}

export async function signLedgerEthereumTransaction(
  apiProxy: WalletApiProxy,
  path: string,
  txInfo: Pick<SerializableTransactionInfo, 'id' | 'chainId'>,
  coin: BraveWallet.CoinType,
  deviceKeyring: LedgerEthereumKeyring = getLedgerEthereumHardwareKeyring()
): Promise<SignHardwareOperationResult> {
  const nonce = await apiProxy.ethTxManagerProxy.getNonceForHardwareTransaction(
    txInfo.chainId,
    txInfo.id
  )
  if (!nonce || !nonce.nonce) {
    return {
      success: false,
      error: getLocale('braveWalletApproveTransactionError')
    }
  }
  const { hexMessage } =
    await apiProxy.ethTxManagerProxy.getEthTransactionMessageToSign(txInfo.id)
  if (!hexMessage) {
    return {
      success: false,
      error: getLocale('braveWalletNoMessageToSignError')
    }
  }

  const signed = await deviceKeyring.signTransaction(path, hexMessage)

  if (!signed || !signed.success || !signed.payload) {
    const error = signed?.error ?? getLocale('braveWalletSignOnDeviceError')
    const code = signed?.code ?? ''
    return { success: false, error: error, code: code }
  }
  const { v, r, s } = signed.payload as EthereumSignedTx
  const result = await apiProxy.ethTxManagerProxy.processEthHardwareSignature(
    txInfo.id,
    '0x' + v,
    '0x' + r,
    '0x' + s
  )
  if (!result || !result.status) {
    return {
      success: false,
      error: getLocale('braveWalletProcessTransactionError')
    }
  }
  return { success: result.status }
}

export async function signLedgerFilecoinTransaction(
  apiProxy: WalletApiProxy,
  txInfo: Pick<SerializableTransactionInfo, 'id' | 'chainId'>,
  coin: BraveWallet.CoinType,
  deviceKeyring: LedgerFilecoinKeyring = getLedgerFilecoinHardwareKeyring()
): Promise<SignHardwareOperationResult> {
  const { jsonMessage } =
    await apiProxy.filTxManagerProxy.getFilTransactionMessageToSign(txInfo.id)
  if (!jsonMessage) {
    return {
      success: false,
      error: getLocale('braveWalletNoMessageToSignError')
    }
  }

  const signed = await deviceKeyring.signTransaction(jsonMessage)
  if (!signed || !signed.success || !signed.payload) {
    const error = signed?.error ?? getLocale('braveWalletSignOnDeviceError')
    const code = signed?.code ?? ''
    return { success: false, error: error, code: code }
  }
  const signedMessage = signed.payload as FilSignedLotusMessage
  if (!signedMessage) {
    return { success: false }
  }

  const result = await apiProxy.filTxManagerProxy.processFilHardwareSignature(
    txInfo.id,
    JSON.stringify(signedMessage)
  )
  if (!result || !result.status) {
    return {
      success: false,
      error: getLocale('braveWalletProcessTransactionError')
    }
  }
  return { success: result.status }
}

export async function signLedgerSolanaTransaction(
  apiProxy: WalletApiProxy,
  path: string,
  txInfo: Pick<SerializableTransactionInfo, 'id' | 'chainId'>,
  coin: BraveWallet.CoinType,
  deviceKeyring: LedgerSolanaKeyring = getLedgerSolanaHardwareKeyring()
): Promise<SignHardwareOperationResult> {
  const { message } =
    await apiProxy.solanaTxManagerProxy.getSolTransactionMessageToSign(
      txInfo.id
    )
  if (!message) {
    return {
      success: false,
      error: getLocale('braveWalletNoMessageToSignError')
    }
  }
  const signed = await deviceKeyring.signTransaction(path, Buffer.from(message))
  if (!signed || !signed.success || !signed.payload) {
    const error = signed?.error ?? getLocale('braveWalletSignOnDeviceError')
    const code = signed?.code ?? ''
    return { success: false, error: error, code: code }
  }

  const signedMessage = signed.payload as Buffer
  if (!signedMessage) {
    return { success: false }
  }

  const result =
    await apiProxy.solanaTxManagerProxy.processSolanaHardwareSignature(
      txInfo.id,
      { signatureBytes: [...signedMessage] }
    )
  if (!result || !result.status) {
    return {
      success: false,
      error: getLocale('braveWalletProcessTransactionError')
    }
  }

  return { success: result.status }
}

export async function signMessageWithHardwareKeyring(
  vendor: BraveWallet.HardwareVendor,
  path: string,
  messageData: Omit<BraveWallet.SignMessageRequest, 'originInfo'>
): Promise<SignMessageHardwareOperationResult> {
  const deviceKeyring = getHardwareKeyring(vendor, messageData.coin)
  const signTypedData = messageData.signData.ethSignTypedData
  const standardSignData = messageData.signData.ethStandardSignData
  if (deviceKeyring instanceof EthereumLedgerBridgeKeyring) {
    if (signTypedData) {
      if (!signTypedData.domainHash || !signTypedData.primaryHash) {
        return {
          success: false,
          error: getLocale('braveWalletUnknownInternalError')
        }
      }
      return deviceKeyring.signEip712Message(
        path,
        signTypedData.domainHash,
        signTypedData.primaryHash
      )
    }
    if (!standardSignData) {
      return {
        success: false,
        error: getLocale('braveWalletUnknownInternalError')
      }
    }
    return deviceKeyring.signPersonalMessage(path, standardSignData.message)
  } else if (deviceKeyring instanceof TrezorBridgeKeyring) {
    if (signTypedData) {
      if (!signTypedData.domainHash || !signTypedData.primaryHash) {
        return {
          success: false,
          error: getLocale('braveWalletUnknownInternalError')
        }
      }
      return deviceKeyring.signEip712Message(
        path,
        signTypedData.domainHash,
        signTypedData.primaryHash
      )
    }
    if (!standardSignData) {
      return {
        success: false,
        error: getLocale('braveWalletUnknownInternalError')
      }
    }
    return deviceKeyring.signPersonalMessage(path, standardSignData.message)
  } else if (deviceKeyring instanceof SolanaLedgerBridgeKeyring) {
    // Not supported yet, see
    // https://github.com/solana-labs/solana/issues/21366.
    return {
      success: false,
      error: getLocale('braveWalletHardwareOperationUnsupportedError')
    }
  }
  return { success: false, error: getLocale('braveWalletUnknownKeyringError') }
}

export async function signSolTransactionWithHardwareKeyring(
  vendor: BraveWallet.HardwareVendor,
  path: string,
  message: Buffer,
  onAuthorized?: () => void
): Promise<SignMessageHardwareOperationResult> {
  const deviceKeyring = getHardwareKeyring(
    vendor,
    BraveWallet.CoinType.SOL,
    onAuthorized
  )

  if (deviceKeyring instanceof SolanaLedgerBridgeKeyring) {
    return deviceKeyring.signTransaction(path, message)
  }

  assertNotReached(`Unsupported  vendor ${vendor}`)
}

export async function cancelHardwareOperation(
  vendor: BraveWallet.HardwareVendor,
  coin: BraveWallet.CoinType
) {
  const deviceKeyring = getHardwareKeyring(vendor, coin)
  if (
    deviceKeyring instanceof EthereumLedgerBridgeKeyring ||
    deviceKeyring instanceof TrezorBridgeKeyring ||
    deviceKeyring instanceof SolanaLedgerBridgeKeyring
  ) {
    return deviceKeyring.cancelOperation()
  }
}

export const loadAccountsFromDevice = async (
  opts: FetchHardwareWalletAccountsProps
): Promise<GetAccountsHardwareOperationResult> => {
  const keyring = getHardwareKeyring(
    opts.scheme.vendor,
    opts.scheme.coin,
    opts.onAuthorized
  )
  return await keyring.getAccounts(opts.startIndex, opts.count, opts.scheme)
}
