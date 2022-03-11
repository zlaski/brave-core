/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.brave_stats;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.util.Pair;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import org.chromium.chrome.R;

import java.util.List;

public class BraveStatsAllTimeAdapter
        extends RecyclerView.Adapter<BraveStatsAllTimeAdapter.ViewHolder> {
    
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
        LayoutInflater inflater = LayoutInflater.from(parent.getContext());
        View view = inflater.inflate(R.layout.item_stats_all_time, parent, false);
        return new ViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull BraveStatsAllTimeAdapter.ViewHolder holder, int position) {
    
        holder.tvSite.setText(sitesList.get(position).first);
        holder.tvStatsCount.setText(String.valueOf(sitesList.get(position).second));

        if(isBlockedInfo) {
            holder.tvBlockedBy.setVisibility(View.VISIBLE);
            holder.tvShields.setVisibility(View.VISIBLE);
        }
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {
        public TextView tvSite;
        public TextView tvStatsCount;
        public TextView tvBlockedBy;
        public TextView tvShields;
        
        public ViewHolder(View itemView) {
            super(itemView);
            this.tvSite = itemView.findViewById(R.id.tv_site);
            this.tvStatsCount = itemView.findViewById(R.id.tv_stats_count);
            this.tvBlockedBy = itemView.findViewById(R.id.tv_blocked_by);
            this.tvShields = itemView.findViewById(R.id.tv_shields);
        }
    }
}
