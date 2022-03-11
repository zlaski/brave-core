/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.onboarding;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import org.chromium.chrome.R;

import java.util.List;

public class OnboardingSitesAdapter
        extends RecyclerView.Adapter<OnboardingSitesAdapter.ViewHolder> {
    private OnboardingSiteClickListener onboardingSiteClickListener;
    private Context context;
    private List<String> sitesNameList;
    private List<Integer> sitesImageList;
    private List<String> sitesUrlList;

    public OnboardingSitesAdapter(OnboardingSiteClickListener onboardingSiteClickListener,
            List<String> sitesNameList, List<Integer> sitesImageList, List<String> sitesUrlList) {
        this.onboardingSiteClickListener = onboardingSiteClickListener;
        this.sitesNameList = sitesNameList;
        this.sitesImageList = sitesImageList;
        this.sitesUrlList = sitesUrlList;
    }

    @Override
    public int getItemCount() {
        return sitesNameList.size();
    }

    @Override
    public @NonNull OnboardingSitesAdapter.ViewHolder onCreateViewHolder(
            ViewGroup parent, int viewType) {
        context = parent.getContext();
        LayoutInflater inflater = LayoutInflater.from(context);
        View view = inflater.inflate(R.layout.item_onboarding_site, parent, false);
        return new ViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull OnboardingSitesAdapter.ViewHolder holder, int position) {
        holder.tvSite.setText(sitesNameList.get(position));
        holder.ivSite.setImageResource(sitesImageList.get(position));

        holder.itemView.setOnClickListener(
                view -> { onboardingSiteClickListener.OnOpenSite(sitesUrlList.get(position)); });
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {
        public ImageView ivSite;
        public ImageView ivForward;
        public TextView tvSite;

        public ViewHolder(View itemView) {
            super(itemView);
            this.ivSite = itemView.findViewById(R.id.iv_site);
            this.ivForward = itemView.findViewById(R.id.iv_forward);
            this.tvSite = itemView.findViewById(R.id.tv_site);
        }
    }
}
