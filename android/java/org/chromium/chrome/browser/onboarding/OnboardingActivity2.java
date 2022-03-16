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

public class OnboardingActivity2
        extends AsyncInitializationActivity implements OnboardingSiteClickListener {
    private View mVLeafAlignTop;
    private View mVLeafAlignBottom;
    private ImageView mIvLeafTop;
    private ImageView mIvLeafBottom;
    private ImageView mIvBraveTop;
    private ImageView mIvBraveBottom;
    private ImageView mIvArrowUp;
    private ImageView mIvArrowDown;
    private LinearLayout mLayoutOtherSteps;
    private LinearLayout mLayoutP3a;
    private TextView mTvWelcome;
    private TextView mTvTitle;
    private TextView mTvDesc;
    private TextView mTvP3a;
    private Button mBtnPositive;
    private Button mBtnNegative;
    private Button mBtnSkip;
    private CheckBox mCheckboxP3a;

    private int mCurrentStep = -1;

    private final String P3A_URL = "https://brave.com/p3a";

    @Override
    protected void triggerLayoutInflation() {
        setContentView(R.layout.activity_onboarding2);

        initViews();
        onClickViews();
        nextOnboardingStep();

        onInitialLayoutInflationComplete();
        OnboardingPrefManager.getInstance().setOnboardingShown(true);
    }

    private void initViews() {
        mIvLeafTop = findViewById(R.id.iv_leaf_top);
        mIvLeafBottom = findViewById(R.id.iv_leaf_bottom);
        mVLeafAlignTop = findViewById(R.id.view_leaf_top_align);
        mVLeafAlignBottom = findViewById(R.id.view_leaf_bottom_align);
        mIvBraveTop = findViewById(R.id.iv_brave_top);
        mIvBraveBottom = findViewById(R.id.iv_brave_bottom);
        mIvArrowUp = findViewById(R.id.iv_arrow_up);
        mIvArrowDown = findViewById(R.id.iv_arrow_down);
        mLayoutOtherSteps = findViewById(R.id.layout_other_steps);
        mLayoutP3a = findViewById(R.id.layout_p3a);
        mTvWelcome = findViewById(R.id.tv_welcome);
        mTvTitle = findViewById(R.id.tv_title);
        mTvDesc = findViewById(R.id.tv_desc);
        mCheckboxP3a = findViewById(R.id.checkbox_p3a);
        mTvP3a = findViewById(R.id.tv_p3a);
        mBtnPositive = findViewById(R.id.btn_positive);
        mBtnNegative = findViewById(R.id.btn_negative);
        mBtnSkip = findViewById(R.id.btn_skip);
    }

    private void onClickViews() {
        mBtnPositive.setOnClickListener(view -> {
            if (mCurrentStep == 1 && BraveSetDefaultBrowserUtils.isBraveSetAsDefaultBrowser(this)) {
                mCurrentStep++;
            } else if (mCurrentStep == 2) {
                BraveSetDefaultBrowserUtils.showBraveSetDefaultBrowserDialog(this, false);
            }
            nextOnboardingStep();
        });

        mBtnNegative.setOnClickListener(view -> { nextOnboardingStep(); });

        mBtnSkip.setOnClickListener(view -> { finish(); });
    }

    private void startTimer() {
        Handler handler = new Handler();
        int durationMs = 3000;
        handler.postDelayed(this::nextOnboardingStep, durationMs);
    }

    private void nextOnboardingStep() {
        mCurrentStep++;

        if (mCurrentStep == 0) {
            setFadeInAnimation(mTvWelcome, 500);
            setFadeInAnimation(mIvArrowDown, 500);
            startTimer();

        } else if (mCurrentStep == 1) {
            setLeafAnimation(mVLeafAlignTop, mIvLeafTop, 1.3f, 40, true);
            setLeafAnimation(mVLeafAlignBottom, mIvLeafBottom, 1.3f, 40, false);

            mTvTitle.setText(getResources().getString(R.string.privacy_onboarding));
            mTvDesc.setText(getResources().getString(R.string.privacy_desc_onboarding));
            mBtnPositive.setText(getResources().getString(R.string.lets_go));

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

            String productAnalysisString =
                    String.format(getResources().getString(R.string.p3a_onboarding_checkbox_text,
                            getResources().getString(R.string.private_product_analysis_text)));
            int productAnalysisIndex = productAnalysisString.indexOf(
                    getResources().getString(R.string.private_product_analysis_text));
            SpannableString productAnalysisTextSS = new SpannableString(productAnalysisString);

            ClickableSpan productAnalysisClickableSpan = new ClickableSpan() {
                @Override
                public void onClick(@NonNull View textView) {
                    CustomTabActivity.showInfoPage(OnboardingActivity2.this, P3A_URL);
                }
                @Override
                public void updateDrawState(@NonNull TextPaint ds) {
                    super.updateDrawState(ds);
                    ds.setUnderlineText(false);
                }
            };

            productAnalysisTextSS.setSpan(productAnalysisClickableSpan, productAnalysisIndex,
                    Math.min(productAnalysisIndex
                                    + getResources()
                                              .getString(R.string.private_product_analysis_text)
                                              .length(),
                            productAnalysisTextSS.length()),
                    Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
            productAnalysisTextSS.setSpan(
                    new ForegroundColorSpan(getResources().getColor(R.color.brave_blue_tint_color)),
                    productAnalysisIndex,
                    Math.min(productAnalysisIndex
                                    + getResources()
                                              .getString(R.string.private_product_analysis_text)
                                              .length(),
                            productAnalysisTextSS.length()),
                    Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
            mTvP3a.setMovementMethod(LinkMovementMethod.getInstance());
            mTvP3a.setText(productAnalysisTextSS);

            mIvArrowDown.setVisibility(View.GONE);
            mTvWelcome.setVisibility(View.GONE);
            mLayoutP3a.setVisibility(View.VISIBLE);
            mIvArrowDown.setVisibility(View.VISIBLE);
            mLayoutOtherSteps.setVisibility(View.VISIBLE);

        } else if (mCurrentStep == 2) {
            setLeafAnimation(mVLeafAlignTop, mIvLeafTop, 1.6f, 80, true);
            setLeafAnimation(mVLeafAlignBottom, mIvLeafBottom, 1.6f, 80, false);

            mIvArrowDown.setVisibility(View.INVISIBLE);
            mLayoutOtherSteps.setVisibility(View.INVISIBLE);
            mLayoutP3a.setVisibility(View.INVISIBLE);

            mIvBraveBottom.setImageResource(R.drawable.ic_phone_onboarding);
            mTvTitle.setText(getResources().getString(R.string.set_default_browser_title));
            mTvDesc.setText(getResources().getString(R.string.set_default_browser_text));
            mBtnPositive.setText(getResources().getString(R.string.set_brave_default_browser));
            mBtnNegative.setText(getResources().getString(R.string.not_now));

            mBtnNegative.setVisibility(View.VISIBLE);
            mBtnSkip.setVisibility(View.VISIBLE);
            mLayoutP3a.setVisibility(View.GONE);
            mIvArrowDown.setVisibility(View.VISIBLE);
            mLayoutOtherSteps.setVisibility(View.VISIBLE);

        } else if (mCurrentStep == 3) {
            setLeafAnimation(mVLeafAlignTop, mIvLeafTop, 2f, 120, true);
            setLeafAnimation(mVLeafAlignBottom, mIvLeafBottom, 2f, 120, false);

            mIvArrowDown.setVisibility(View.GONE);
            mLayoutOtherSteps.setVisibility(View.GONE);
            mIvBraveBottom.setVisibility(View.GONE);
            mLayoutP3a.setVisibility(View.GONE);
            mBtnPositive.setVisibility(View.GONE);
            mBtnNegative.setVisibility(View.GONE);

            mTvTitle.setText(getResources().getString(R.string.ready_browse));
            mTvDesc.setText(getResources().getString(R.string.select_popular_site));

            mBtnSkip.setVisibility(View.VISIBLE);
            mIvArrowUp.setVisibility(View.VISIBLE);
            mLayoutOtherSteps.setVisibility(View.VISIBLE);
            sitesList();
            mIvBraveTop.setVisibility(View.VISIBLE);
        }
    }

    private void sitesList() {
        String countryCode = Locale.getDefault().getCountry();

        ArrayList<String> sitesNameList;
        ArrayList<Integer> sitesImageList;
        ArrayList<String> sitesUrlList;

        if (countryCode.equals("CA")) {
            sitesNameList = new ArrayList<>(Arrays.asList(getResources().getString(R.string.yahoo),
                    getResources().getString(R.string.environment_canada),
                    getResources().getString(R.string.canadian_tire)));

            sitesImageList = new ArrayList<>(Arrays.asList(R.drawable.ic_yahoo,
                    R.drawable.ic_environment_canada, R.drawable.ic_canadian_tire));

            sitesUrlList = new ArrayList<>(Arrays.asList("https://www.yahoo.com/",
                    "https://www.weather.gc.ca/", "https://www.canadiantire.ca/"));

        } else if (countryCode.equals("GB")) {
            sitesNameList = new ArrayList<>(Arrays.asList(getResources().getString(R.string.bbc),
                    getResources().getString(R.string.sky),
                    getResources().getString(R.string.wired)));

            sitesImageList = new ArrayList<>(
                    Arrays.asList(R.drawable.ic_bbc, R.drawable.ic_sky, R.drawable.ic_wired));

            sitesUrlList = new ArrayList<>(Arrays.asList(
                    "https://www.bbc.co.uk/", "https://www.sky.com/", "https://www.wired.co.uk/"));

        } else if (countryCode.equals("DE")) {
            sitesNameList = new ArrayList<>(Arrays.asList(getResources().getString(R.string.yahoo),
                    getResources().getString(R.string.gmx),
                    getResources().getString(R.string.mobile)));

            sitesImageList = new ArrayList<>(
                    Arrays.asList(R.drawable.ic_yahoo, R.drawable.ic_gmx, R.drawable.ic_mobile_de));

            sitesUrlList = new ArrayList<>(Arrays.asList(
                    "https://www.yahoo.com/", "https://www.gmx.net/", "https://www.mobile.de/"));

        } else if (countryCode.equals("FR")) {
            sitesNameList = new ArrayList<>(Arrays.asList(getResources().getString(R.string.yahoo),
                    getResources().getString(R.string.les_journal),
                    getResources().getString(R.string.programme_tv)));

            sitesImageList = new ArrayList<>(Arrays.asList(
                    R.drawable.ic_yahoo, R.drawable.ic_les_journal, R.drawable.ic_programme_tv));

            sitesUrlList = new ArrayList<>(Arrays.asList("https://www.yahoo.com/",
                    "https://www.journaldesfemmes.fr/", "https://www.programme-tv.net/"));

        } else if (countryCode.equals("IN")) {
            sitesNameList =
                    new ArrayList<>(Arrays.asList(getResources().getString(R.string.hotstar),
                            getResources().getString(R.string.cricbuzz),
                            getResources().getString(R.string.flipkart)));

            sitesImageList = new ArrayList<>(Arrays.asList(
                    R.drawable.ic_hotstar, R.drawable.ic_cricbuzz, R.drawable.ic_flipkart));

            sitesUrlList = new ArrayList<>(Arrays.asList("https://www.hotstar.com/",
                    "https://www.cricbuzz.com/", "https://www.flipkart.com/"));

        } else if (countryCode.equals("AU")) {
            sitesNameList = new ArrayList<>(Arrays.asList(getResources().getString(R.string.news),
                    getResources().getString(R.string.gumtree),
                    getResources().getString(R.string.real_estate)));

            sitesImageList = new ArrayList<>(Arrays.asList(
                    R.drawable.ic_news_au, R.drawable.ic_gumtree, R.drawable.ic_realestate_au));

            sitesUrlList = new ArrayList<>(Arrays.asList("https://www.news.com.au/",
                    "https://www.gumtree.com.au/", "https://www.realestate.com.au/"));

        } else if (countryCode.equals("IE")) {
            sitesNameList = new ArrayList<>(Arrays.asList(getResources().getString(R.string.rte),
                    getResources().getString(R.string.iris_independent),
                    getResources().getString(R.string.done_deal)));

            sitesImageList = new ArrayList<>(Arrays.asList(
                    R.drawable.ic_rte, R.drawable.ic_iris_independent, R.drawable.ic_done_deal));

            sitesUrlList = new ArrayList<>(Arrays.asList("https://www.rte.ie/",
                    "https://www.independent.ie/", "https://www.donedeal.ie/"));

        } else if (countryCode.equals("JP")) {
            sitesNameList =
                    new ArrayList<>(Arrays.asList(getResources().getString(R.string.yahoo_japan),
                            getResources().getString(R.string.wired_japan),
                            getResources().getString(R.string.number_web)));

            sitesImageList = new ArrayList<>(Arrays.asList(
                    R.drawable.ic_yahoo_jp, R.drawable.ic_wired, R.drawable.ic_number_web));

            sitesUrlList = new ArrayList<>(Arrays.asList("https://m.yahoo.co.jp/",
                    "https://www.wired.jp/", "https://number.bunshun.jp/"));

        } else {
            sitesNameList = new ArrayList<>(Arrays.asList(getResources().getString(R.string.yahoo),
                    getResources().getString(R.string.wired),
                    getResources().getString(R.string.espn)));

            sitesImageList = new ArrayList<>(
                    Arrays.asList(R.drawable.ic_yahoo, R.drawable.ic_wired, R.drawable.ic_espn));

            sitesUrlList = new ArrayList<>(Arrays.asList(
                    "https://www.yahoo.com/", "https://www.wired.com/", "https://www.espn.com/"));
        }

        sitesNameList.add(getResources().getString(R.string.enter_website));
        sitesImageList.add(R.drawable.ic_search);
        sitesUrlList.add("");

        setSitesRecyclerView(sitesNameList, sitesImageList, sitesUrlList);
    }

    private void setSitesRecyclerView(ArrayList<String> sitesNameList,
            ArrayList<Integer> sitesImageList, ArrayList<String> sitesUrlList) {
        RecyclerView rvSites = findViewById(R.id.recyclerview_sites);
        rvSites.setVisibility(View.VISIBLE);
        rvSites.setLayoutManager(new LinearLayoutManager(this));
        OnboardingSitesAdapter onboardingSitesAdapter =
                new OnboardingSitesAdapter(this, sitesNameList, sitesImageList, sitesUrlList);
        rvSites.setAdapter(onboardingSitesAdapter);
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

    @Override
    public void OnOpenSite(String url) {
        if (BraveActivity.getBraveActivity() != null) {
            if (url != null && url.length() > 0) {
                BraveActivity.getBraveActivity().openNewOrSelectExistingTab(url, true);
            } else {
                BraveActivity.getBraveActivity().focusSearchBox();
            }
        }
        finish();
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (resultCode == RESULT_OK
                && requestCode == BraveSetDefaultBrowserUtils.DEFAULT_BROWSER_ROLE_REQUEST_CODE) {
            BraveSetDefaultBrowserUtils.setBraveDefaultSuccess();
        }
        super.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    public void onBackPressed() {}

    @Override
    public boolean shouldStartGpuProcess() {
        return false;
    }
}
