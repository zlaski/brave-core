/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#define IDNToUnicodeWithAdjustmentsImpl \
  IDNToUnicodeWithAdjustmentsImpl_ChromiumImpl
#include "../../../../components/url_formatter/url_formatter.cc"
#undef IDNToUnicodeWithAdjustmentsImpl

namespace url_formatter {

namespace {

IDNConversionResult IDNToUnicodeWithAdjustmentsImpl(
    base::StringPiece host,
    base::OffsetAdjuster::Adjustments* adjustments,
    bool ignore_spoof_check_results) {
  // TODO: also check if pref is set
  if (!ignore_spoof_check_results) {
    if (adjustments)
      adjustments->clear();
    // Convert the ASCII input to a std::u16string for ICU.
    std::u16string host16;
    host16.reserve(host.length());
    host16.insert(host16.end(), host.begin(), host.end());
    IDNConversionResult result;
    result.result = host16;
    return result;
  }
  return IDNToUnicodeWithAdjustmentsImpl_ChromiumImpl(
      host, adjustments, ignore_spoof_check_results);
}

}  // namespace

}  // namespace url_formatter
