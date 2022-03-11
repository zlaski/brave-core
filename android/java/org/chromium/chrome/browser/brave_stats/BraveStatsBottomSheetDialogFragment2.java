/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.brave_stats;

import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Button;
import android.widget.ScrollView;
import androidx.core.widget.NestedScrollView;
import android.util.Pair;
import android.os.Handler;
import android.os.Looper;
import android.text.Spanned;
import android.widget.LinearLayout;
import androidx.core.content.ContextCompat;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import androidx.recyclerview.widget.DividerItemDecoration;

import androidx.annotation.Nullable;

import com.google.android.material.bottomsheet.BottomSheetDialogFragment;

import org.chromium.chrome.browser.brave_stats.BraveStatsUtil;
import org.chromium.chrome.browser.local_database.BraveStatsTable;
import org.chromium.chrome.browser.local_database.DatabaseHelper;
import org.chromium.chrome.browser.BraveRewardsHelper;
import org.chromium.chrome.browser.onboarding.OnboardingPrefManager;
import org.chromium.chrome.browser.notifications.retention.RetentionNotificationUtil;
import org.chromium.chrome.R;
import org.chromium.base.Log;

import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class BraveStatsBottomSheetDialogFragment2 extends BottomSheetDialogFragment {

    private ImageView mIvBack;
    private ImageView mIvClose;
    private ImageView mIvNotificationClose;
    private TextView mTvTitle;
    private TextView mTvEmpty;
    private TextView mTvLastWeekTracker;
    private TextView mTvLastWeekWebsite;
    private TextView mTvAllTimeTracker;
    private TextView mTvAllTimeWebsite;
    private TextView mTvTrackerAds;
    private TextView mTvWebsite;
    private TextView mTvTrackerWebsiteInfo;
    private Button mBtnNotification;
    private Button mBtnAllTime;
    private ScrollView mScrollView;
    private View mLayoutNotification;
    private LinearLayout mLayoutTrackerWebsite;
    private RecyclerView mAllTimeRecyclerView;

    private List<Pair<String, Integer>> mAllStatsList;
    private List<Pair<String, Integer>> mAllSitesList;
    private DatabaseHelper mDatabaseHelper = DatabaseHelper.getInstance();
    private static final int LAST_WEEK = -7;

    private final ExecutorService mExecutor = Executors.newSingleThreadExecutor();
    private final Handler mHandler = new Handler(Looper.getMainLooper());

    private enum ALL_TIMES_TYPES {
        TRACKERS_ADS,
        WEBSITE
    }

    private ALL_TIMES_TYPES mAllTimesListTypes = ALL_TIMES_TYPES.TRACKERS_ADS;

    public static BraveStatsBottomSheetDialogFragment2 newInstance() {
        return new BraveStatsBottomSheetDialogFragment2();
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //getActivity().setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
    }

    @Override
    public View onCreateView(
            LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(
                R.layout.fragment_brave_stats_bottom_sheet, container, false);
    }

    @Override
    public void onViewCreated(View view, Bundle savedInstanceState) {

        mIvBack = view.findViewById(R.id.iv_back);
        mIvClose = view.findViewById(R.id.iv_close);
        mTvTitle = view.findViewById(R.id.tv_title);
        mTvEmpty = view.findViewById(R.id.tv_empty);
        mTvLastWeekTracker = view.findViewById(R.id.tv_last_week_tracker);
        mTvLastWeekWebsite = view.findViewById(R.id.tv_last_week_website);
        mTvAllTimeTracker = view.findViewById(R.id.tv_all_time_tracker);
        mTvAllTimeWebsite = view.findViewById(R.id.tv_all_time_website);
        mLayoutNotification = view.findViewById(R.id.layout_notification);
        mIvNotificationClose = view.findViewById(R.id.iv_notification_close);
        mBtnNotification = view.findViewById(R.id.btn_notification);
        mBtnAllTime = view.findViewById(R.id.btn_all_time);
        mScrollView = view.findViewById(R.id.scrollview);
        mLayoutTrackerWebsite = view.findViewById(R.id.layout_tracker_website);
        mTvTrackerAds = view.findViewById(R.id.tv_tracker_ads);
        mTvWebsite = view.findViewById(R.id.tv_website);
        mTvTrackerWebsiteInfo = view.findViewById(R.id.tv_tracker_website_info);
        mAllTimeRecyclerView = view.findViewById(R.id.all_time_recyclerview);

        onClickViews();
        setData();
    }

    private void onClickViews() {

        mIvClose.setOnClickListener(view -> { dismiss(); });
        
        mIvNotificationClose.setOnClickListener(view -> { 

            OnboardingPrefManager.getInstance().setBraveStatsNotificationShown(true);
            mLayoutNotification.setVisibility(View.GONE);
        });

        mBtnNotification.setOnClickListener(view -> {

            OnboardingPrefManager.getInstance().setBraveStatsNotificationShown(true);
            OnboardingPrefManager.getInstance().setBraveStatsNotificationEnabled(true);
            RetentionNotificationUtil.scheduleNotificationForEverySunday(getActivity(), RetentionNotificationUtil.EVERY_SUNDAY);
            mLayoutNotification.setVisibility(View.GONE);
        });

        mIvBack.setOnClickListener(view -> {
            
            mTvTitle.setText(getResources().getString(R.string.privacy_report));
            mIvBack.setVisibility(View.GONE);
            mScrollView.setVisibility(View.VISIBLE);
            mLayoutTrackerWebsite.setVisibility(View.GONE);
            mTvTrackerWebsiteInfo.setVisibility(View.GONE);
            mAllTimeRecyclerView.setVisibility(View.GONE);
        });

        mBtnAllTime.setOnClickListener(view -> {

            mTvTitle.setText(getResources().getString(R.string.all_time_lists_title));
            mIvBack.setVisibility(View.VISIBLE);
            mScrollView.setVisibility(View.GONE);
            mLayoutNotification.setVisibility(View.GONE);

            mLayoutTrackerWebsite.setVisibility(View.VISIBLE);
            mTvTrackerWebsiteInfo.setVisibility(View.VISIBLE);
            mAllTimeRecyclerView.setVisibility(View.VISIBLE);
            
            mTvTrackerWebsiteInfo.setText(getResources().getString(R.string.trackers_ads_info));

            setAllTimesList(ALL_TIMES_TYPES.TRACKERS_ADS);
        
        });

        mTvTrackerAds.setOnClickListener(view -> {

            setAllTimesList(ALL_TIMES_TYPES.TRACKERS_ADS);
        });

        mTvWebsite.setOnClickListener(view -> {

            setAllTimesList(ALL_TIMES_TYPES.WEBSITE);
        });
    }

    private void setAllTimesList(ALL_TIMES_TYPES allTimesListTypes) {

        mAllTimesListTypes = allTimesListTypes;

        if(mAllTimesListTypes == ALL_TIMES_TYPES.TRACKERS_ADS) {
            
            mTvTrackerWebsiteInfo.setText(getResources().getString(R.string.trackers_ads_info));
            mTvTrackerAds.setBackgroundResource(R.drawable.brave_stats_tracker_website_selected);
            mTvWebsite.setBackgroundResource(0);

            setAllTimesRecyclerView(mAllSitesList, true);

        } else if(mAllTimesListTypes == ALL_TIMES_TYPES.WEBSITE) {

            mTvTrackerWebsiteInfo.setText(getResources().getString(R.string.websites_info));
            mTvWebsite.setBackgroundResource(R.drawable.brave_stats_tracker_website_selected);
            mTvTrackerAds.setBackgroundResource(0);

            setAllTimesRecyclerView(mAllStatsList, false);     
        }
    }

    private void setAllTimesRecyclerView(List<Pair<String, Integer>> sitesList, boolean isBlockedInfo) {

        Log.e("tapan","sitesList:"+sitesList.size());
        mAllTimeRecyclerView.setLayoutManager(new LinearLayoutManager(getActivity()));
        BraveStatsAllTimeAdapter braveStatsAllTimeAdapter =
                new BraveStatsAllTimeAdapter(sitesList, isBlockedInfo);
        mAllTimeRecyclerView.setAdapter(braveStatsAllTimeAdapter);
        mAllTimeRecyclerView.addItemDecoration(new BraveStatsDividerItemDecoration(getActivity()));
    }

    private void setData() {

        mExecutor.execute(() -> {

            List<Pair<String, Integer>> lastWeekStatsList = mDatabaseHelper
                    .getStatsWithDate(BraveStatsUtil.getCalculatedDate("yyyy-MM-dd", LAST_WEEK),
                            BraveStatsUtil.getCalculatedDate("yyyy-MM-dd", 0));

            List<Pair<String, Integer>> lastWeekSitesList = mDatabaseHelper
                    .getSitesWithDate(BraveStatsUtil.getCalculatedDate("yyyy-MM-dd", LAST_WEEK),
                            BraveStatsUtil.getCalculatedDate("yyyy-MM-dd", 0));

            mAllStatsList = mDatabaseHelper.getStats();
            mAllSitesList = mDatabaseHelper.getSites();

            mHandler.post(() -> {
                updateUI(lastWeekStatsList, lastWeekSitesList);
            });
        });
    }

    private void updateUI(List<Pair<String, Integer>> lastWeekStatsList, 
        List<Pair<String, Integer>> lastWeekSitesList) {

        mScrollView.setVisibility(View.VISIBLE);

        if(lastWeekSitesList.isEmpty()) {

            mTvLastWeekTracker.setText(getResources().getString(R.string.no_data_text));

        } else {

            String lastWeekTracker = "<b>" + lastWeekSitesList.get(0).first + "</b> "
                    + getResources().getString(R.string.creepiest_tracker_data);

            lastWeekTracker = lastWeekTracker + " <b>" + lastWeekSitesList.get(0).second + "</b>";
            
            if(lastWeekSitesList.get(0).second>1) {
                lastWeekTracker = lastWeekTracker + " " + getResources().getString(R.string.sites);
            } else {
                lastWeekTracker = lastWeekTracker + " " + getResources().getString(R.string.site);
            }

            mTvLastWeekTracker.setText(BraveRewardsHelper.spannedFromHtmlString(lastWeekTracker));
            mTvLastWeekTracker.setTextColor(ContextCompat.getColor(getActivity(), R.color.brave_stats_text_color));
        }

        if(lastWeekStatsList.isEmpty()) {

            mTvLastWeekWebsite.setText(getResources().getString(R.string.no_data_text));

        } else {

            String lastWeekWebsite = "<b>" + lastWeekStatsList.get(0).first + "</b> "
                    + getResources().getString(R.string.creepiest_website_data);

            lastWeekWebsite = lastWeekWebsite + " <b>" + lastWeekStatsList.get(0).second + "</b>";

            if(lastWeekStatsList.get(0).second>1) {
                lastWeekWebsite = lastWeekWebsite + " " + getResources().getString(R.string.creepiest_website_trackers_blocked);
            } else {
                lastWeekWebsite = lastWeekWebsite + " " + getResources().getString(R.string.creepiest_website_tracker_blocked);
            }

            mTvLastWeekWebsite.setText(BraveRewardsHelper.spannedFromHtmlString(lastWeekWebsite));
            mTvLastWeekWebsite.setTextColor(ContextCompat.getColor(getActivity(), R.color.brave_stats_text_color));
        }

        if(mAllSitesList.isEmpty()) {

            mTvAllTimeTracker.setText(getResources().getString(R.string.no_data_text));

        } else {
            
            String allTimeTracker = mAllSitesList.get(0).first + "\n" + getResources().getQuantityString(
                    R.plurals.site_count, mAllSitesList.get(0).second,
                    mAllSitesList.get(0).second);
            mTvAllTimeTracker.setText(allTimeTracker);
            mTvAllTimeTracker.setTextColor(ContextCompat.getColor(getActivity(), R.color.brave_stats_text_color));
        }

        if(mAllStatsList.isEmpty()) {

            mTvAllTimeWebsite.setText(getResources().getString(R.string.no_data_text));

        } else {

            String allTimeWebsite = mAllStatsList.get(0).first + "\n" + getResources().getQuantityString(
                    R.plurals.tracker_count, mAllStatsList.get(0).second,
                    mAllStatsList.get(0).second);
            mTvAllTimeWebsite.setText(allTimeWebsite);
            mTvAllTimeWebsite.setTextColor(ContextCompat.getColor(getActivity(), R.color.brave_stats_text_color));
        }

        if(mAllStatsList.isEmpty() && mAllSitesList.isEmpty()) {
            mTvEmpty.setVisibility(View.VISIBLE);
        } else {
            mBtnAllTime.setVisibility(View.VISIBLE);
        }

        if(!OnboardingPrefManager.getInstance().isBraveStatsNotificationShown() && !OnboardingPrefManager.getInstance().isBraveStatsNotificationEnabled()) {
            mLayoutNotification.setVisibility(View.VISIBLE);
        }
    }
}
