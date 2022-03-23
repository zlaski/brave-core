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
import android.text.SpannableString;
import android.text.Spanned;
import android.text.TextPaint;
import android.text.method.LinkMovementMethod;
import android.text.style.ClickableSpan;
import android.text.style.ForegroundColorSpan;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.Transformation;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import org.chromium.base.Log;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.app.BraveActivity;
import org.chromium.chrome.browser.customtabs.CustomTabActivity;
import org.chromium.chrome.browser.init.AsyncInitializationActivity;
import org.chromium.chrome.browser.preferences.BravePrefServiceBridge;
import org.chromium.chrome.browser.set_default_browser.BraveSetDefaultBrowserUtils;

import java.lang.Math;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Locale;

public class OnboardingActivity2 extends AsyncInitializationActivity {

    private View mVLeafAlignTop;
    private View mVLeafAlignBottom;
    private ImageView mIvLeafTop;
    private ImageView mIvLeafBottom;
    private ImageView mIvBrave;
    private ImageView mIvArrowDown;
    private LinearLayout mLayoutSetDefault;
    private LinearLayout mLayoutP3a;
    private TextView mTvWelcome;
    private Button mBtnPositive;
    private Button mBtnNotNow;
    private CheckBox mCheckboxP3a;

    private int mCurrentStep = -1;

    @Override
    protected void triggerLayoutInflation() {
        setContentView(R.layout.activity_onboarding2);

        initViews();
        onClickViews();
        startTimer();

        onInitialLayoutInflationComplete();
        OnboardingPrefManager.getInstance().setOnboardingShown(true);
    }

    private void initViews() {
        mIvLeafTop = findViewById(R.id.iv_leaf_top);
        mIvLeafBottom = findViewById(R.id.iv_leaf_bottom);
        mVLeafAlignTop = findViewById(R.id.view_leaf_top_align);
        mVLeafAlignBottom = findViewById(R.id.view_leaf_bottom_align);
        mIvBrave = findViewById(R.id.iv_brave);
        mIvArrowDown = findViewById(R.id.iv_arrow_down);
        mLayoutSetDefault = findViewById(R.id.layout_set_default);
        mLayoutP3a = findViewById(R.id.layout_p3a);
        mTvWelcome = findViewById(R.id.tv_welcome);
        mCheckboxP3a = findViewById(R.id.checkbox_p3a);
        mBtnPositive = findViewById(R.id.btn_positive);
        mBtnNotNow = findViewById(R.id.btn_not_now);
    }

    private void onClickViews() {

        mBtnPositive.setOnClickListener(view -> {

            if(!BraveSetDefaultBrowserUtils.isBraveSetAsDefaultBrowser(this)) {
                BraveSetDefaultBrowserUtils.showBraveSetDefaultBrowserDialog(this, false);
            } else {
                gotoSearchBox();
            }
        });

        mBtnNotNow.setOnClickListener(view -> { gotoSearchBox(); });
    }

    private void startTimer() {
        Handler handler = new Handler();
        int durationMs = 3000;
        handler.postDelayed(this::nextOnboardingStep, durationMs);
    }

    private void nextOnboardingStep() {
        mCurrentStep++;

        if (mCurrentStep == 0) {
            mTvWelcome.setVisibility(View.VISIBLE);
            //setFadeInAnimation(mTvWelcome, 500);
            startTimer();

        } else if (mCurrentStep == 1) {
            setLeafAnimation(mVLeafAlignTop, mIvLeafTop, 1.3f, 40, true);
            setLeafAnimation(mVLeafAlignBottom, mIvLeafBottom, 1.3f, 40, false);

            boolean isP3aEnabled = false;

            try {
                isP3aEnabled = BravePrefServiceBridge.getInstance().getP3AEnabled();
            } catch (Exception e) {
                Log.e("P3aOnboarding", e.getMessage());
            }

            mCheckboxP3a.setChecked(isP3aEnabled);
            mCheckboxP3a.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
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

            mTvWelcome.setVisibility(View.GONE);

            if(BraveSetDefaultBrowserUtils.isBraveSetAsDefaultBrowser(this)) {
                mBtnPositive.setText(getResources().getString(R.string.continue_text));
            } else {
                mBtnPositive.setText(getResources().getString(R.string.set_default_browser));
                mBtnNotNow.setVisibility(View.VISIBLE);
            }

            mLayoutSetDefault.setVisibility(View.VISIBLE);
            mIvArrowDown.setVisibility(View.VISIBLE);
            mLayoutP3a.setVisibility(View.VISIBLE);
        }
    }

    private void setFadeInAnimation(View view, int duration) {
        view.animate().alpha(1f).setDuration(duration).withEndAction(
                () -> view.setVisibility(View.VISIBLE));
    }

    private void setLeafAnimation(View leafAlignView, ImageView leafView, float scale,
            float leafMargin, boolean isTopLeaf) {
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
                    layoutParams.topMargin =
                            margin - (int) ((margin - layoutParams.topMargin) * interpolatedTime);
                }
                leafAlignView.setLayoutParams(layoutParams);
            }
        };
        animation.setDuration(200);
        leafAlignView.startAnimation(animation);

        leafView.animate().scaleX(scale).scaleY(scale).setDuration(200);
    }

    public void gotoSearchBox() {
        if (BraveActivity.getBraveActivity() != null) {
            BraveActivity.getBraveActivity().focusSearchBox();
        }
        finish();
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (resultCode == RESULT_OK
                && requestCode == BraveSetDefaultBrowserUtils.DEFAULT_BROWSER_ROLE_REQUEST_CODE) {
            BraveSetDefaultBrowserUtils.setBraveDefaultSuccess();
        }
        gotoSearchBox();
        super.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    public void onBackPressed() {}

    @Override
    public boolean shouldStartGpuProcess() {
        return false;
    }
}
