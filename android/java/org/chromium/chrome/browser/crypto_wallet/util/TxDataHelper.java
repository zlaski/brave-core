/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.crypto_wallet.util;

import org.chromium.base.Consumer;
import org.chromium.base.Predicate;
import org.chromium.brave_wallet.mojom.AccountInfo;
import org.chromium.brave_wallet.mojom.CoinType;
import org.chromium.brave_wallet.mojom.KeyringInfo;
import org.chromium.brave_wallet.mojom.TransactionInfo;
import org.chromium.brave_wallet.mojom.TransactionStatus;
import org.chromium.brave_wallet.mojom.TransactionType;
import org.chromium.brave_wallet.mojom.TxService;
import org.chromium.chrome.browser.crypto_wallet.observers.TxServiceObserver;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;

public class TxDataHelper implements TxServiceObserver {
    private TxService mTxService;
    private AccountInfo[] mAccountInfos;
    private HashMap<String, TransactionInfo[]> mTxInfos;
    private boolean mReturnAll;
    private String mFilterByContractAddress;
    private String mRopstenContractAddress;
    private volatile boolean isUpdating;
    private Predicate<TransactionInfo>[] filters;

    public TxDataHelper(TxService txService, AccountInfo[] accountInfos, boolean returnAll,
                        String filterByContractAddress) {
        assert txService != null;
        mTxService = txService;
        mAccountInfos = accountInfos;
        mFilterByContractAddress = filterByContractAddress;
        mReturnAll = returnAll;
        mTxInfos = new HashMap<String, TransactionInfo[]>();
        if (mFilterByContractAddress != null && !mFilterByContractAddress.isEmpty()) {
            mRopstenContractAddress = Utils.getRopstenContractAddress(mFilterByContractAddress);
        }
        txService.addObserver(this);
    }

    public HashMap<String, TransactionInfo[]> getTransactions() {
        return mTxInfos;
    }

    /**
     * Fetch the transactions based 'returnAll' or filters.
     * Get all transactions if 'returnAll' is true and filters are ignored
     *
     * @param runWhenDone callback method once transactions are fetched
     * @param filters     to be applied to keep the selected transactions. Note: it is ignored if `returnAll` is true
     */
    @SafeVarargs
    public final void fetchTransactions(Runnable runWhenDone, Predicate<TransactionInfo>... filters) {
        this.filters = filters;
        isUpdating = true;
        AsyncUtils.MultiResponseHandler allTxMultiResponse =
                new AsyncUtils.MultiResponseHandler(mAccountInfos.length);
        ArrayList<AsyncUtils.GetAllTransactionInfoResponseContext> allTxContexts =
                new ArrayList<>();
        for (AccountInfo accountInfo : mAccountInfos) {
            AsyncUtils.GetAllTransactionInfoResponseContext allTxContext =
                    new AsyncUtils.GetAllTransactionInfoResponseContext(
                            allTxMultiResponse.singleResponseComplete, accountInfo.name);

            allTxContexts.add(allTxContext);

            mTxService.getAllTransactionInfo(CoinType.ETH, accountInfo.address, allTxContext);
        }

        allTxMultiResponse.setWhenAllCompletedAction(() -> {
            for (AsyncUtils.GetAllTransactionInfoResponseContext allTxContext : allTxContexts) {
                ArrayList<TransactionInfo> newValue = new ArrayList<TransactionInfo>();
                for (TransactionInfo txInfo : allTxContext.txInfos) {
                    boolean isIncluded = true;
                    if (!mReturnAll) {
                        for (Predicate<TransactionInfo> transactionInfoPredicate : this.filters) {
                            isIncluded = isIncluded && transactionInfoPredicate.test(txInfo);
                        }
                    }
                    if (isIncluded) {
                        if (mFilterByContractAddress == null) {
                            // Don't filter by contract
                            newValue.add(txInfo);
                        } else if (!mFilterByContractAddress.isEmpty()) {
                            if (mFilterByContractAddress.toLowerCase(Locale.getDefault())
                                    .equals(txInfo.txDataUnion.getEthTxData1559()
                                            .baseData.to.toLowerCase(
                                                    Locale.getDefault()))) {
                                newValue.add(txInfo);
                            }
                            if (mRopstenContractAddress != null
                                    && !mRopstenContractAddress.isEmpty()
                                    && mRopstenContractAddress.toLowerCase(Locale.getDefault())
                                    .equals(txInfo.txDataUnion.getEthTxData1559()
                                            .baseData.to.toLowerCase(
                                                    Locale.getDefault()))) {
                                newValue.add(txInfo);
                            }

                        } else if (txInfo.txType != TransactionType.ERC20_APPROVE
                                && txInfo.txType != TransactionType.ERC20_TRANSFER) {
                            // Filter by ETH only
                            newValue.add(txInfo);
                        }
                    }
                }
                Collections.sort(newValue, (lhs, rhs) -> {
                    // -1 - less than, 1 - greater than, 0 - equal, all inversed for descending
                    return Long.compare(rhs.createdTime.microseconds, lhs.createdTime.microseconds);
                });
                TransactionInfo[] newArray = new TransactionInfo[newValue.size()];
                newArray = newValue.toArray(newArray);
                TransactionInfo[] value = mTxInfos.get(allTxContext.name);
                if (value == null) {
                    mTxInfos.put(allTxContext.name, newArray);
                } else {
                    TransactionInfo[] both = Arrays.copyOf(value, value.length + newArray.length);
                    System.arraycopy(newArray, 0, both, value.length, newArray.length);
                    mTxInfos.put(allTxContext.name, both);
                }
            }
            if (runWhenDone != null) {
                runWhenDone.run();
            }
            isUpdating = false;
        });
    }

    /**
     * Get a single TransactionInfo object after applying the filters or null if not found
     *
     * @param accountName of account to fetch the Transaction
     * @param consumer    to get the filtered TransactionInfo object if found, otherwise null
     * @param predicates  list of filters
     */
    @SafeVarargs
    public final void fetchTransactions(String accountName,
                                        Consumer<TransactionInfo> consumer, Predicate<TransactionInfo>... predicates) {
        fetchTransactions(() -> {
            TransactionInfo[] transactionInfos = getTransactions().get(accountName);
            TransactionInfo transactionInfoResult = null;
            if (transactionInfos != null) {
                for (TransactionInfo info : transactionInfos) {
                    boolean isValid = true;
                    for (Predicate<TransactionInfo> transactionInfoPredicate : predicates) {
                        if (!transactionInfoPredicate.test(info)) {
                            isValid = false;
                            break;
                        }
                    }
                    if (isValid) {
                        transactionInfoResult = info;
                        break;
                    }
                }
            }
            consumer.accept(transactionInfoResult);
        });
    }

    @SafeVarargs
    public final List<TransactionInfo> getFilteredTxDetailsFromLocal(String accountName, Predicate<TransactionInfo>... predicates) {
        TransactionInfo[] transactionInfos = getTransactions().get(accountName);
        if (transactionInfos == null) {
            return Collections.emptyList();
        }
        if (predicates.length == 0) {
            return Arrays.asList(transactionInfos);
        }
        List<TransactionInfo> infos = new ArrayList<>();
        for (TransactionInfo info : transactionInfos) {
            boolean isValid = true;
            for (Predicate<TransactionInfo> transactionInfoPredicate : predicates) {
                isValid = isValid && transactionInfoPredicate.test(info);
            }
            if (isValid) {
                infos.add(info);
            }
        }
        return infos;
    }

    @Override
    public void onNewUnapprovedTx(TransactionInfo txInfo) {
        fetchTransactions(null);
    }

    @Override
    public void onUnapprovedTxUpdated(TransactionInfo txInfo) {
        fetchTransactions(null);
    }

    @Override
    public void onTransactionStatusChanged(TransactionInfo txInfo) {
        fetchTransactions(null);
    }

    public final static class TxDataHelperFilters {

        public static Predicate<TransactionInfo> isSubmitted() {
            return (transactionInfo) -> transactionInfo.txStatus == TransactionStatus.SUBMITTED;
        }

        public static Predicate<TransactionInfo> isGreaterThan24Hours() {
            return (info) -> true;
//            return (info) -> (Utils.getTimeDifferenceMillisFromNow(Utils.getDateFromTimeDelta(info.submittedTime)) / (1000 * 60 * 60)) > 24;
        }

        public static Predicate<TransactionInfo> hasNonce(String nonce) {
            return (info) -> info.txDataUnion.getEthTxData1559().baseData.nonce.equals(nonce);
        }

        public static Predicate<TransactionInfo> isUnApproved() {
            return (transactionInfo) -> transactionInfo.txStatus == TransactionStatus.UNAPPROVED;
        }

        public static Predicate<TransactionInfo> notOf(Predicate<TransactionInfo> predicate) {
            return (transactionInfo) -> !predicate.test(transactionInfo);
        }

    }

}
