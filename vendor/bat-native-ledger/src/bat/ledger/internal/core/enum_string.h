/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_CORE_ENUM_STRING_H_
#define BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_CORE_ENUM_STRING_H_

#include <array>
#include <string>
#include <type_traits>

#include "base/strings/string_piece.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace ledger {

template <typename T>
class EnumString : public base::StringPiece {
  static_assert(std::is_enum_v<T>,
                "EnumString<T> is only valid for enum types");

 public:
  explicit EnumString(const std::string& str) : base::StringPiece(str) {}

  static absl::optional<T> Parse(const std::string& str) {
    return ParseEnum(EnumString(str));
  }

  template <T... values>
  absl::optional<T> Match() const {
    static const std::array<T, sizeof...(values)> val_array = {values...};
    for (auto v : val_array) {
      if (*this == StringifyEnum(v)) {
        return v;
      }
    }
    return {};
  }
};

}  // namespace ledger

#endif  // BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_CORE_ENUM_STRING_H_
