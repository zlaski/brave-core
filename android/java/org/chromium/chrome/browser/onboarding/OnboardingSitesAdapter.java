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

public class OnboardingSitesAdapter extends RecyclerView.Adapter<OnboardingSitesAdapter.ViewHolder> {

	private enum Sites {
        YAHOO,
        CNN,
        FOX_NEWS,
        ENTER_WEBSITE;
    }

    private OnboardingSiteClickListener onboardingSiteClickListener;
    private Context context;

    public OnboardingSitesAdapter(OnboardingSiteClickListener onboardingSiteClickListener) {
        this.onboardingSiteClickListener = onboardingSiteClickListener;
    }

	@Override
    public int getItemCount() {
        return Sites.values().length;
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

        Sites site = Sites.values()[position];
        if(site == Sites.YAHOO) {

            holder.tvSite.setText(context.getResources().getString(R.string.yahoo));
            holder.ivSite.setImageResource(R.drawable.ic_yahoo);

        } else if(site == Sites.CNN) {

            holder.tvSite.setText(context.getResources().getString(R.string.cnn));
            holder.ivSite.setImageResource(R.drawable.ic_cnn);

        } else if(site == Sites.FOX_NEWS) {

            holder.tvSite.setText(context.getResources().getString(R.string.fox_news));
            holder.ivSite.setImageResource(R.drawable.ic_fox_news);

        } else if(site == Sites.ENTER_WEBSITE) {

            holder.tvSite.setText(context.getResources().getString(R.string.enter_website));
            holder.ivSite.setImageResource(R.drawable.ic_search);
        }

        holder.itemView.setOnClickListener(view ->  {

            if(site == Sites.ENTER_WEBSITE) {
                onboardingSiteClickListener.OnOpenSite("");
            } else {
                onboardingSiteClickListener.OnOpenSite("https://www.yahoo.com");
            }
        });
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
