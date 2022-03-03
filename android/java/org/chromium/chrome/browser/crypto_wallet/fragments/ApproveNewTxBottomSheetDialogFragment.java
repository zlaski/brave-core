/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.crypto_wallet.fragments;

import android.annotation.SuppressLint;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.text.SpannableString;
import android.text.Spanned;
import android.text.method.LinkMovementMethod;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewParent;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.core.content.ContextCompat;
import androidx.viewpager.widget.ViewPager;

import com.google.android.material.tabs.TabLayout;

import org.chromium.brave_wallet.mojom.AccountInfo;
import org.chromium.brave_wallet.mojom.AssetPriceTimeframe;
import org.chromium.brave_wallet.mojom.AssetRatioService;
import org.chromium.brave_wallet.mojom.BlockchainRegistry;
import org.chromium.brave_wallet.mojom.BlockchainToken;
import org.chromium.brave_wallet.mojom.BraveWalletConstants;
import org.chromium.brave_wallet.mojom.CoinType;
import org.chromium.brave_wallet.mojom.EthereumChain;
import org.chromium.brave_wallet.mojom.JsonRpcService;
import org.chromium.brave_wallet.mojom.TransactionInfo;
import org.chromium.brave_wallet.mojom.TransactionType;
import org.chromium.brave_wallet.mojom.TxService;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.crypto_wallet.adapters.ApproveTxFragmentPageAdapter;
import org.chromium.chrome.browser.crypto_wallet.observers.ApprovedTxObserver;
import org.chromium.chrome.browser.crypto_wallet.util.TokenUtils;
import org.chromium.chrome.browser.crypto_wallet.util.TxDataHelper;
import org.chromium.chrome.browser.crypto_wallet.util.Utils;
import org.chromium.chrome.browser.crypto_wallet.util.TxDataHelper.TxDataHelperFilters;
import org.chromium.chrome.browser.util.TabUtils;
import org.chromium.ui.text.NoUnderlineClickableSpan;

import java.util.List;
import java.util.Locale;

/*
 *
 */
public class ApproveNewTxBottomSheetDialogFragment extends BraveBaseBottomSheetDialogFragment {
    public static final String TAG_FRAGMENT = ApproveNewTxBottomSheetDialogFragment.class.getName();

    private TransactionInfo mTxInfo;
    //    private String mAccountName;
//    private String mAccountAddress;
    private boolean mRejected;
    private boolean mApproved;
    private double mTotalPrice;
    private ApprovedTxObserver mApprovedTxObserver;
    private String mChainSymbol;
    private int mChainDecimals;
    private View mResetView;
    private View mApproveRejectView;
    private TxDataHelper mTxDataHelper;
    private TransactionInfo mSubmittedTx;
    private boolean shouldReusePendingTxNonce;
    private AccountInfo mAccountInfo;
    private String mAccountName;
    private boolean mShouldFindAccountInfoWithName;

    public static ApproveNewTxBottomSheetDialogFragment newInstance(
            TransactionInfo txInfo, AccountInfo accountInfo) {
        return new ApproveNewTxBottomSheetDialogFragment(txInfo, accountInfo);
    }

    public static ApproveNewTxBottomSheetDialogFragment newInstance(
            TransactionInfo txInfo, String accountName) {
        return new ApproveNewTxBottomSheetDialogFragment(txInfo, accountName);
    }

    ApproveNewTxBottomSheetDialogFragment(TransactionInfo txInfo, AccountInfo accountInfo) {
        mTxInfo = txInfo;
        mAccountInfo = accountInfo;
        mRejected = false;
        mApproved = false;
        mChainSymbol = "ETH";
        mChainDecimals = 18;
    }

    ApproveNewTxBottomSheetDialogFragment(TransactionInfo txInfo, String accountName) {
        this(txInfo, (AccountInfo) null);
        mShouldFindAccountInfoWithName = true;
        this.mAccountName = accountName;
    }

    public void setApprovedTxObserver(ApprovedTxObserver approvedTxObserver) {
        mApprovedTxObserver = approvedTxObserver;
    }

    @Override
    public String getFragmentTag() {
        return TAG_FRAGMENT;
    }

    @Override
    public void onAttach(@NonNull Context context) {
        super.onAttach(context);
        if (mShouldFindAccountInfoWithName) {
            getKeyringService().getKeyringInfo(BraveWalletConstants.DEFAULT_KEYRING_ID, keyringInfo -> {
                mAccountInfo = Utils.getAccountInfo(keyringInfo.accountInfos, mAccountName);
                mTxDataHelper = createTxHelper(mAccountInfo);
                setupInfoView();
            });
        } else {
            mTxDataHelper = createTxHelper(mAccountInfo);
            setupInfoView();
        }
    }

    private TxDataHelper createTxHelper(AccountInfo accountInfo) {
        return new TxDataHelper(getTxService(), new AccountInfo[]{accountInfo}, false, null);
    }

    @Override
    public void onDismiss(DialogInterface dialog) {
        super.onDismiss(dialog);
        if ((mRejected || mApproved) && mApprovedTxObserver != null) {
            mApprovedTxObserver.OnTxApprovedRejected(mApproved, mAccountInfo.name, mTxInfo.id);
        }
    }

    @Override
    public void setupDialog(@NonNull Dialog dialog, int style) {
        super.setupDialog(dialog, style);

        @SuppressLint("InflateParams") final View view =
                LayoutInflater.from(getContext()).inflate(R.layout.approve_new_tx_bottom_sheet, null);
        mResetView = view.findViewById(R.id.approve_new_tx_reset_include);
        mApproveRejectView = view.findViewById(R.id.approve_new_tx_approve_reject_root);
        //showTransactionBacklogInfoView();

        dialog.setContentView(view);
        ViewParent parent = view.getParent();
        ((View) parent).getLayoutParams().height = ViewGroup.LayoutParams.WRAP_CONTENT;
        initApproveRejectTxState(view);
    }

    private void setupInfoView() {
        mTxDataHelper.fetchTransactions(mAccountInfo.name, transactionInfo -> {
            if (transactionInfo != null) {
                // showTransactionBacklogInfoView();
                List<TransactionInfo> duplicateNonceTx = mTxDataHelper.getFilteredTxDetailsFromLocal(mAccountInfo.name,
                        TxDataHelperFilters.hasNonce(transactionInfo.txDataUnion.getEthTxData1559().baseData.nonce),
                        TxDataHelperFilters.notOf(TxDataHelperFilters.isUnApproved()));
                if (duplicateNonceTx.size() >= 2) {
                    showTransactionBacklogInfoView();
                } else {
                    mSubmittedTx = transactionInfo;
                    showResetNonceView();
                }

                // found a pending transaction to replace
//                getEthTxService().setNonceForUnapprovedTransaction(mTxInfo.id, transactionInfo.txData.baseData.nonce, arg1 -> {
//                    if (arg1){
//
//                    }else {
//                        // unable to set nonce
//
//                    }
//                });
            }
        }, TxDataHelperFilters.isGreaterThan24Hours(), TxDataHelperFilters.isSubmitted());
        // Todo fetch pending transactions for the account
        // setup reset button actions
        // fetch the nonce of the first submitted transaction
        // use the nonce for the new transaction if exist
    }

    private void showTransactionBacklogInfoView() {
        setResetViewVisibility(true);
        mResetView.findViewById(R.id.reset_tx_cb).setVisibility(View.GONE);
        mResetView.findViewById(R.id.reset_tx_tv_duplicate_tx_no_action_desc).setVisibility(View.VISIBLE);
        // Redefining reset action button text and intended action
        Button btnResetTxNonce = mResetView.findViewById(R.id.reset_tx_btn_reset);
        btnResetTxNonce.setText(R.string.dialog_positive_button);
        btnResetTxNonce.setOnClickListener(v -> {
            setResetViewVisibility(false);
        });

        TextView tvLearnMore = mResetView.findViewById(R.id.reset_tx_tv_learn_more);
        String learnMoreTxt = getString(R.string.brave_wallet_sheet_reset_tx_learn_more);
        NoUnderlineClickableSpan span = new NoUnderlineClickableSpan(
                getResources(), R.color.brave_action_color, (textView) -> {
            TabUtils.openUrlInNewTab(false, Utils.RESET_BACKLOG_TRANSACTION_NONCE_LEARN_MORE);
            TabUtils.bringChromeTabbedActivityToTheTop(requireActivity());
        });
        SpannableString learnMoreSS = Utils.createSpannableString(
                learnMoreTxt, learnMoreTxt, span, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        tvLearnMore.setMovementMethod(LinkMovementMethod.getInstance());
        tvLearnMore.setText(learnMoreSS);
    }


    private void showResetNonceView() {
        setResetViewVisibility(true);
        CheckBox resetTxCheckBox = mResetView.findViewById(R.id.reset_tx_cb);
        Button btnResetTxNonce = mResetView.findViewById(R.id.reset_tx_btn_reset);
        TextView tvLearnMore = mResetView.findViewById(R.id.reset_tx_tv_learn_more);
        String learnMoreTxt = getString(R.string.brave_wallet_sheet_reset_tx_learn_more);
        NoUnderlineClickableSpan span = new NoUnderlineClickableSpan(
                getResources(), R.color.brave_action_color, (textView) -> {
            TabUtils.openUrlInNewTab(false, Utils.RESET_BACKLOG_TRANSACTION_NONCE_LEARN_MORE);
            TabUtils.bringChromeTabbedActivityToTheTop(requireActivity());
        });

        SpannableString learnMoreSS = Utils.createSpannableString(
                learnMoreTxt, learnMoreTxt, span, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        tvLearnMore.setMovementMethod(LinkMovementMethod.getInstance());
        tvLearnMore.setText(learnMoreSS);
        resetTxCheckBox.setOnCheckedChangeListener((buttonView, isChecked) -> {
            shouldReusePendingTxNonce = isChecked;
            if (isChecked) {
                btnResetTxNonce.setBackgroundTintList(ContextCompat.getColorStateList(requireContext(), R.color.brave_wallet_day_night_error_border));
                btnResetTxNonce.setText(R.string.brave_wallet_sheet_approve_reset_nonce);
            } else {
                btnResetTxNonce.setBackgroundTintList(null);
                btnResetTxNonce.setText(R.string.continue_text);
            }
        });
        btnResetTxNonce.setOnClickListener(v -> {
            if (shouldReusePendingTxNonce && mSubmittedTx != null) {
                getEthTxManagerProxy().setNonceForUnapprovedTransaction(mTxInfo.id, mSubmittedTx.txDataUnion.getEthTxData1559().baseData.nonce, status -> {
                    if (status) {
                        setResetViewVisibility(false);
                    } else {
                        // unable to set nonce
                    }
                    // allow to continue despite of reused old nonce
//                        mResetView.setVisibility(View.GONE);
                });
            } else {
                setResetViewVisibility(false);
            }
        });
    }

    private void setResetViewVisibility(boolean isResetVisible) {
        mResetView.setVisibility(isResetVisible ? View.VISIBLE : View.GONE);
        mApproveRejectView.setVisibility(isResetVisible ? View.GONE : View.VISIBLE);
    }

    private void initApproveRejectTxState(View view) {
        JsonRpcService jsonRpcService = getJsonRpcService();
        assert jsonRpcService != null;
        jsonRpcService.getChainId(chainId -> {
            jsonRpcService.getAllNetworks(chains -> {
                EthereumChain[] customNetworks = Utils.getCustomNetworks(chains);
                TextView networkName = view.findViewById(R.id.network_name);
                networkName.setText(
                        Utils.getNetworkText(getActivity(), chainId, customNetworks).toString());
                String chainSymbol = "ETH";
                int chainDecimals = 18;
                for (EthereumChain chain : chains) {
                    if (chainId.equals(chain.chainId)) {
                        if (Utils.isCustomNetwork(chainId)) {
                            chainSymbol = chain.symbol;
                            chainDecimals = chain.decimals;
                            break;
                        }
                    }
                }
                mChainSymbol = chainSymbol;
                mChainDecimals = chainDecimals;
                TextView txType = view.findViewById(R.id.tx_type);
                txType.setText(getResources().getString(R.string.send));
                if (mTxInfo.txType == TransactionType.ERC20_TRANSFER
                        || mTxInfo.txType == TransactionType.ERC20_APPROVE) {
                    BlockchainRegistry blockchainRegistry = getBlockchainRegistry();
                    assert blockchainRegistry != null;
                    TokenUtils.getAllTokensFiltered(
                            getBraveWalletService(), blockchainRegistry, chainId, tokens -> {
                                for (BlockchainToken token : tokens) {
                                    // Replace USDC and DAI contract addresses for Ropsten network
                                    token.contractAddress = Utils.getContractAddress(
                                            chainId, token.symbol, token.contractAddress);
                                    String symbol = token.symbol;
                                    int decimals = token.decimals;
                                    if (mTxInfo.txType == TransactionType.ERC20_APPROVE) {
                                        txType.setText(String.format(
                                                getResources().getString(R.string.activate_erc20),
                                                symbol));
                                        symbol = mChainSymbol;
                                        decimals = mChainDecimals;
                                    }
                                    if (token.contractAddress.toLowerCase(Locale.getDefault())
                                            .equals(mTxInfo.txDataUnion.getEthTxData1559()
                                                    .baseData.to.toLowerCase(
                                                            Locale.getDefault()))) {
                                        fillAssetDependentControls(symbol, view, decimals);
                                        break;
                                    }
                                }
                            });
                } else {
                    if (mTxInfo.txDataUnion.getEthTxData1559()
                            .baseData.to.toLowerCase(Locale.getDefault())
                            .equals(Utils.SWAP_EXCHANGE_PROXY.toLowerCase(
                                    Locale.getDefault()))) {
                        txType.setText(getResources().getString(R.string.swap));
                    }
                    fillAssetDependentControls(mChainSymbol, view, mChainDecimals);
                }
            });
        });
        ImageView icon = (ImageView) view.findViewById(R.id.account_picture);
        Utils.setBlockiesBitmapResource(getExecutor(), getHandler(), icon, mTxInfo.fromAddress, true);

        Button reject = view.findViewById(R.id.reject);
        reject.setOnClickListener(v -> rejectTransaction(true));

        Button approve = view.findViewById(R.id.approve);
        approve.setOnClickListener(v -> approveTransaction());
    }

    private void fillAssetDependentControls(String asset, View view, int decimals) {
        String valueToConvert = mTxInfo.txDataUnion.getEthTxData1559().baseData.value;
        String to = mTxInfo.txDataUnion.getEthTxData1559().baseData.to;
        if (mTxInfo.txType == TransactionType.ERC20_TRANSFER && mTxInfo.txArgs.length > 1) {
            valueToConvert = mTxInfo.txArgs[1];
            to = mTxInfo.txArgs[0];
        }
        TextView fromTo = view.findViewById(R.id.from_to);
        fromTo.setText(String.format(getResources().getString(R.string.crypto_wallet_from_to),
                mAccountInfo.name, Utils.stripAccountAddress(to)));
        TextView amountAsset = view.findViewById(R.id.amount_asset);
        amountAsset.setText(
                String.format(getResources().getString(R.string.crypto_wallet_amount_asset),
                        String.format(Locale.getDefault(), "%.4f",
                                Utils.fromHexWei(valueToConvert, decimals)),
                        asset));
        AssetRatioService assetRatioService = getAssetRatioService();
        assert assetRatioService != null;
        String[] assets = {asset.toLowerCase(Locale.getDefault())};
        String[] toCurr = {"usd"};
        assetRatioService.getPrice(
                assets, toCurr, AssetPriceTimeframe.LIVE, (success, values) -> {
                    String valueFiat = "0";
                    if (values.length != 0) {
                        valueFiat = values[0].price;
                    }
                    String valueAsset = mTxInfo.txDataUnion.getEthTxData1559().baseData.value;
                    if (mTxInfo.txType == TransactionType.ERC20_TRANSFER
                            && mTxInfo.txArgs.length > 1) {
                        valueAsset = mTxInfo.txArgs[1];
                    }
                    double value = Utils.fromHexWei(valueAsset, decimals);
                    double price = Double.valueOf(valueFiat);
                    mTotalPrice = value * price;
                    TextView amountFiat = view.findViewById(R.id.amount_fiat);
                    amountFiat.setText(String.format(
                            getResources().getString(R.string.crypto_wallet_amount_fiat),
                            String.format(Locale.getDefault(), "%.2f", mTotalPrice)));
                    ViewPager viewPager = view.findViewById(R.id.navigation_view_pager);
                    ApproveTxFragmentPageAdapter adapter = new ApproveTxFragmentPageAdapter(
                            getChildFragmentManager(), mTxInfo, asset, decimals, mChainSymbol,
                            mChainDecimals, mTotalPrice, getActivity());
                    viewPager.setAdapter(adapter);
                    viewPager.setOffscreenPageLimit(adapter.getCount() - 1);
                    TabLayout tabLayout = view.findViewById(R.id.tabs);
                    tabLayout.setupWithViewPager(viewPager);
                });
    }

    private void rejectTransaction(boolean dismiss) {
        TxService txService = getTxService();
        if (txService == null) {
            return;
        }
        txService.rejectTransaction(CoinType.ETH, mTxInfo.id, success -> {
            assert success;
            if (!success || !dismiss) {
                return;
            }
            mRejected = true;
            dismiss();
        });
    }

    private void approveTransaction() {
        TxService txService = getTxService();
        if (txService == null) {
            return;
        }
        txService.approveTransaction(CoinType.ETH, mTxInfo.id, (success, error, errorMessage) -> {
           assert success;
            Utils.warnWhenError(ApproveTxBottomSheetDialogFragment.TAG_FRAGMENT,
                    "approveTransaction", error, errorMessage);
            if (!success) {
                return;
            }
            mApproved = true;
            dismiss();
        });
    }
}
