/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/ntp_tiles/brave_popular_sites_impl.h"

namespace {

// static global variable is used because |sections()| returns const ref value.
std::map<ntp_tiles::SectionType, ntp_tiles::PopularSites::SitesVector>
    g_filtered_sections;

}  // namespace

namespace ntp_tiles {

// Only PERSONALIZED section type is included in |PopularSitesImpl::sections_|.
// See PopularSitesImpl::ParseVersion6OrAbove() or ParseVersion5().
const std::map<SectionType, PopularSitesImpl::SitesVector>&
BravePopularSitesImpl::sections() const {
  PopularSites::SitesVector filtered_sites;

  // Leave PERSONALIZED section as empty.
  g_filtered_sections[SectionType::PERSONALIZED] = filtered_sites;
  return g_filtered_sections;
}

}  // namespace ntp_tiles
