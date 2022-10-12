//======---------------------------------------------------------------======//
// Copyright (c) 2022 Evan Cox <evanacox00@gmail.com>.                       //
//                                                                           //
// Licensed under the Apache License, Version 2.0 (the "License");           //
// you may not use this file except in compliance with the License.          //
// You may obtain a copy of the License at                                   //
//                                                                           //
//     http://www.apache.org/licenses/LICENSE-2.0                            //
//                                                                           //
// Unless required by applicable law or agreed to in writing, software       //
// distributed under the License is distributed on an "AS IS" BASIS,         //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  //
// See the License for the specific language governing permissions and       //
// limitations under the License.                                            //
//======---------------------------------------------------------------======//

#pragma once

#include "../config.h"
#include "absl/numeric/int128.h"
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace dawn {
  enum class Width { i8 = 8, i16 = 16, i32 = 32, i64 = 64 };

  class DAWN_PUBLIC APInt {
  public:
    APInt(std::uint64_t value, Width width) noexcept;

    APInt(const APInt&) = default;

    APInt(APInt&&) = default;

    APInt& operator=(const APInt&) = default;

    APInt& operator=(APInt&&) = default;

    ~APInt() = default;

    friend bool operator==(const APInt& lhs, const APInt& rhs) noexcept {
      return lhs.width() == rhs.width() && lhs.value() == rhs.value();
    }

    template <typename H> friend H AbslHashValue(H state, const APInt& value) {
      H::combine(std::move(state), value.value_);

      return state;
    }

    [[nodiscard]] std::uint64_t width() const noexcept {
      constexpr auto shiftBy = std::uint64_t{7} * std::uint64_t{8};
      constexpr auto widthMask = std::uint64_t{0xFF} << shiftBy;

      // width is stored in the most-significant bits of `value_`
      return (absl::Uint128High64(value_) & widthMask) >> shiftBy;
    }

    [[nodiscard]] std::uint64_t value() const noexcept {
      // we get the 64-bit raw value, and then shave off any bits
      // that don't matter for an integer of our width
      auto lower = absl::Uint128Low64(value_);
      auto maskForWidth = ~std::uint64_t{0} >> (64 - width());

      return lower & maskForWidth;
    }

  private:
    // raw storage for the integer, larger than 64-bit so we can detect
    // any overflow conditions during any constant operations. we only
    // actually care about the bottom 64 bits
    //
    // we store the width in the most-significant 8 bits of the value.
    //
    // Bytes: 0xA.......CCCCCCCC
    //          |       ^
    //          |       actual 64-bit value
    //          |
    //          ^
    //          width
    absl::uint128 value_;
  };
} // namespace dawn