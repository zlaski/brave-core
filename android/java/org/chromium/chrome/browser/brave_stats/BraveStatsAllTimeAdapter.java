/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.brave_stats;

import android.content.Context;
import android.util.Pair;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import com.bumptech.glide.Glide;

import org.chromium.chrome.R;

import java.util.List;

public class BraveStatsAllTimeAdapter
        extends RecyclerView.Adapter<BraveStatsAllTimeAdapter.ViewHolder> {
    private Context context;
    private List<Pair<String, Integer>> sitesList;
    private boolean isBlockedInfo;

    public BraveStatsAllTimeAdapter(List<Pair<String, Integer>> sitesList, boolean isBlockedInfo) {
        this.sitesList = sitesList;
        this.isBlockedInfo = isBlockedInfo;
    }

    @Override
    public int getItemCount() {
        return sitesList.size();
    }

    @Override
    public @NonNull BraveStatsAllTimeAdapter.ViewHolder onCreateViewHolder(
            ViewGroup parent, int viewType) {
        context = parent.getContext();
        LayoutInflater inflater = LayoutInflater.from(parent.getContext());
        View view = inflater.inflate(R.layout.item_stats_all_time, parent, false);
        return new ViewHolder(view);
    }

    @Override
    public void onBindViewHolder(
            @NonNull BraveStatsAllTimeAdapter.ViewHolder holder, int position) {
        Pair<String, Integer> site = sitesList.get(position);
        holder.tvSite.setText(site.first);
        holder.tvStatsCount.setText(String.valueOf(site.second));

        if (isBlockedInfo) {
            holder.tvBlockedBy.setVisibility(View.VISIBLE);
            holder.tvShields.setVisibility(View.VISIBLE);
            holder.ivSite.setVisibility(View.GONE);
        } else {
            holder.tvBlockedBy.setVisibility(View.GONE);
            holder.tvShields.setVisibility(View.GONE);
            holder.ivSite.setVisibility(View.VISIBLE);

            Glide.with(context)
                    .load("https://www.google.com/s2/favicons?sz=48&domain_url=" + site.first)
                    .into(holder.ivSite);
        }
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {
        public ImageView ivSite;
        public TextView tvSite;
        public TextView tvStatsCount;
        public TextView tvBlockedBy;
        public TextView tvShields;

        public ViewHolder(View itemView) {
            super(itemView);
            this.ivSite = itemView.findViewById(R.id.iv_site);
            this.tvSite = itemView.findViewById(R.id.tv_site);
            this.tvStatsCount = itemView.findViewById(R.id.tv_stats_count);
            this.tvBlockedBy = itemView.findViewById(R.id.tv_blocked_by);
            this.tvShields = itemView.findViewById(R.id.tv_shields);
        }
    }
}
