/**
 * Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.chromium.chrome.browser.onboarding;

import static org.chromium.ui.base.ViewUtils.dpToPx;

import android.content.Intent;
import android.os.Handler;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.view.animation.Animation;
import android.view.animation.Transformation;

import org.chromium.base.Log;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.customtabs.CustomTabActivity;
import org.chromium.chrome.browser.init.AsyncInitializationActivity;
import org.chromium.chrome.browser.preferences.BravePrefServiceBridge;
import org.chromium.chrome.browser.set_default_browser.BraveSetDefaultBrowserUtils;

public class OnboardingActivity2 extends AsyncInitializationActivity {

    private View mVLeafAlignTop;
    private View mVLeafAlignBottom;
    private ImageView mIvLeafTop;
    private ImageView mIvLeafBottom;
    private ImageView mIvBrave;
    private ImageView mIvArrowDown;
    private LinearLayout mLayoutCard;
    private LinearLayout mLayoutP3aCrash;
    private LinearLayout mLayoutP3aInsights;
    private TextView mTvWelcome;
    private TextView mTvCard;
    private TextView mTvDefault;
    private Button mBtnPositive;
    private Button mBtnNegative;
    private CheckBox mCheckboxP3aCrash;
    private CheckBox mCheckboxP3aInsights;

    private int mCurrentStep = -1;

    private static final String P3A_URL = "https://brave.com/privacy/browser/#how-we-improve-brave";

    @Override
    protected void triggerLayoutInflation() {
        setContentView(R.layout.activity_onboarding2);

        initViews();
        onClickViews();
        startTimer();

        onInitialLayoutInflationComplete();
        OnboardingPrefManager.getInstance().setOnboardingSearchBoxTooltip(true);
        OnboardingPrefManager.getInstance().setOnboardingShown(true);
    }

    private void initViews() {
        mIvLeafTop = findViewById(R.id.iv_leaf_top);
        mIvLeafBottom = findViewById(R.id.iv_leaf_bottom);
        mVLeafAlignTop = findViewById(R.id.view_leaf_top_align);
        mVLeafAlignBottom = findViewById(R.id.view_leaf_bottom_align);
        mIvBrave = findViewById(R.id.iv_brave);
        mIvArrowDown = findViewById(R.id.iv_arrow_down);
        mLayoutCard = findViewById(R.id.layout_card);
        mLayoutP3aCrash = findViewById(R.id.layout_p3a_crash);
        mLayoutP3aInsights = findViewById(R.id.layout_p3a_insights);
        mTvWelcome = findViewById(R.id.tv_welcome);
        mTvCard = findViewById(R.id.tv_card);
        mTvDefault = findViewById(R.id.tv_default);
        mCheckboxP3aCrash = findViewById(R.id.checkbox_p3a_crash);
        mCheckboxP3aInsights = findViewById(R.id.checkbox_p3a_insights);
        mBtnPositive = findViewById(R.id.btn_positive);
        mBtnNegative = findViewById(R.id.btn_negative);
    }

    private void onClickViews() {
        mBtnPositive.setOnClickListener(view -> {
            if (mCurrentStep == 1 && !BraveSetDefaultBrowserUtils.isBraveSetAsDefaultBrowser(this)) {
                BraveSetDefaultBrowserUtils.setDefaultBrowser(this);
                if(!BraveSetDefaultBrowserUtils.supportsDefaultRoleManager()) {
                    nextOnboardingStep();
                }
            } else {
                nextOnboardingStep();
            }
        });

        mBtnNegative.setOnClickListener(view -> {
            if(mCurrentStep == 2) {
                CustomTabActivity.showInfoPage(this, P3A_URL);
            } else {
                nextOnboardingStep();
            }
        });
    }

    private void startTimer() {
        Handler handler = new Handler();
        int durationMs = 3000;
        handler.postDelayed(this::nextOnboardingStep, durationMs);
    }

    private void nextOnboardingStep() {
        mCurrentStep++;

        if (mCurrentStep == 0) {
            setLeafAnimation(mVLeafAlignTop, mIvLeafTop, 1f, 0, true);
            setLeafAnimation(mVLeafAlignBottom, mIvLeafBottom, 1f, 0, false);
            // mTvWelcome.setVisibility(View.VISIBLE);
            setFadeInAnimation(mTvWelcome, 200);
            mIvBrave.animate().scaleX(0.8f).scaleY(0.8f).setDuration(1000);
            startTimer();

        } else if (mCurrentStep == 1) {
            setLeafAnimation(mVLeafAlignTop, mIvLeafTop, 1.3f, 30, true);
            setLeafAnimation(mVLeafAlignBottom, mIvLeafBottom, 1.3f, 30, false);

            mTvWelcome.setVisibility(View.VISIBLE);
            // setFadeInAnimation(mTvWelcome, 500);

            if (BraveSetDefaultBrowserUtils.isBraveSetAsDefaultBrowser(this)) {
                mBtnPositive.setText(getResources().getString(R.string.continue_text));
            } else {
                mBtnPositive.setText(getResources().getString(R.string.set_default_browser));
                mBtnNegative.setText(getResources().getString(R.string.not_now));
                mBtnNegative.setVisibility(View.VISIBLE);
            }

            mTvWelcome.setVisibility(View.GONE);
            mTvCard.setText(getResources().getString(R.string.privacy_onboarding));
            mTvDefault.setText(getResources().getString(R.string.onboarding_set_default));
            mLayoutCard.setVisibility(View.VISIBLE);
            mIvArrowDown.setVisibility(View.VISIBLE);
            //mLayoutP3a.setVisibility(View.VISIBLE);
            //setFadeInAnimation(mLayoutCard, 2000);

        } else if(mCurrentStep == 2) {

            setLeafAnimation(mVLeafAlignTop, mIvLeafTop, 1.5f, 60, true);
            setLeafAnimation(mVLeafAlignBottom, mIvLeafBottom, 1.5f, 60, false);

            mLayoutCard.setVisibility(View.GONE);
            mTvDefault.setVisibility(View.GONE);
            mIvArrowDown.setVisibility(View.GONE);

            mTvCard.setText(getResources().getString(R.string.p3a_title));
            mBtnPositive.setText(getResources().getString(R.string.continue_text));
            mBtnNegative.setText(getResources().getString(R.string.learn_more_onboarding));

            boolean isP3aEnabled = false;

            try {
                isP3aEnabled = BravePrefServiceBridge.getInstance().getP3AEnabled();
            } catch (Exception e) {
                Log.e("P3aOnboarding", e.getMessage());
            }

            mCheckboxP3aCrash.setChecked(isP3aEnabled);
            mCheckboxP3aCrash.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                    try {
                        BravePrefServiceBridge.getInstance().setP3AEnabled(isChecked);
                        BravePrefServiceBridge.getInstance().setP3ANoticeAcknowledged(true);
                    } catch (Exception e) {
                        Log.e("P3aOnboarding", e.getMessage());
                    }
                }
            });

            mCheckboxP3aInsights.setChecked(isP3aEnabled);
            mCheckboxP3aInsights.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                    try {
                        BravePrefServiceBridge.getInstance().setP3AEnabled(isChecked);
                        BravePrefServiceBridge.getInstance().setP3ANoticeAcknowledged(true);
                    } catch (Exception e) {
                        Log.e("P3aOnboarding", e.getMessage());
                    }
                }
            });

            mTvCard.setVisibility(View.VISIBLE);
            mLayoutP3aCrash.setVisibility(View.VISIBLE);
            mLayoutP3aInsights.setVisibility(View.VISIBLE);
            mLayoutCard.setVisibility(View.VISIBLE);
            mIvArrowDown.setVisibility(View.VISIBLE);
        } else {
            //BraveActivity.getBraveActivity().focusSearchBox();
            finish();
        }
    }

    private void setFadeInAnimation(View view, int duration) {
        view.animate().alpha(1f).setDuration(duration).withEndAction(
                () -> view.setVisibility(View.VISIBLE));
    }

    private void setLeafAnimation(View leafAlignView, ImageView leafView, float scale,
            float leafMargin, boolean isTopLeaf) {
        if (leafMargin > 0) {
            int margin = (int) dpToPx(this, leafMargin);
            Animation animation = new Animation() {
                @Override
                protected void applyTransformation(float interpolatedTime, Transformation t) {
                    ViewGroup.MarginLayoutParams layoutParams =
                            (ViewGroup.MarginLayoutParams) leafAlignView.getLayoutParams();
                    if (isTopLeaf) {
                        layoutParams.bottomMargin = margin
                                - (int) ((margin - layoutParams.bottomMargin) * interpolatedTime);
                    } else {
                        layoutParams.topMargin = margin
                                - (int) ((margin - layoutParams.topMargin) * interpolatedTime);
                    }
                    leafAlignView.setLayoutParams(layoutParams);
                }
            };
            animation.setDuration(800);
            leafAlignView.startAnimation(animation);
        }

        leafView.animate().scaleX(scale).scaleY(scale).setDuration(800);
    }

    /*public void gotoNext(boolean isDefaultAsk) {
        if (BraveActivity.getBraveActivity() != null) {
            // BraveActivity.getBraveActivity().focusSearchBox();
            BraveActivity.getBraveActivity().onBoardingWelcomeComplete(isDefaultAsk);
        }
        finish();
    }*/

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (resultCode == RESULT_OK
                && requestCode == BraveSetDefaultBrowserUtils.DEFAULT_BROWSER_ROLE_REQUEST_CODE) {
            BraveSetDefaultBrowserUtils.setBraveDefaultSuccess();
        }
        nextOnboardingStep();
        super.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    public void onBackPressed() {}

    @Override
    public boolean shouldStartGpuProcess() {
        return false;
    }
}
