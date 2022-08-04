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

#include "dawn/utility/apint.h"

namespace dawn {
  APInt::APInt(std::uint64_t value, Width width) noexcept : value_{} {
    constexpr auto shift_by = std::uint64_t{7} * std::uint64_t{8};
    auto raw_width = static_cast<std::uint64_t>(width);

    // shave off extra bits, so if the value is >= max of width it doesn't matter
    value_ = value & (~std::uint64_t{0} >> (64 - static_cast<int>(width)));

    // pack width into the highest bits
    value_ |= absl::MakeUint128(raw_width << shift_by, 0);
  }
} // namespace dawn
