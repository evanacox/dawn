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

#include "dawn/utility/version.h"
#include "dawn/version_config.h" // configured by CMake
#include <charconv>
#include <string_view>

namespace {
  int major() noexcept {
#ifdef DAWN_VERSION_MAJOR
    constexpr auto data = std::string_view{DAWN_VERSION_MAJOR};
    auto result = 0;
    std::from_chars(data.data(), data.data() + data.size(), result);

    return result;
#else
    return 0;
#endif
  }

  int minor() noexcept {
#ifdef DAWN_VERSION_MINOR
    constexpr auto data = std::string_view{DAWN_VERSION_MINOR};
    auto result = 0;
    std::from_chars(data.data(), data.data() + data.size(), result);

    return result;
#else
    return 0;
#endif
  }

  int patch() noexcept {
#ifdef DAWN_VERSION_PATCH
    constexpr auto data = std::string_view{DAWN_VERSION_PATCH};
    auto result = 0;
    std::from_chars(data.data(), data.data() + data.size(), result);

    return result;
#else
    return 0;
#endif
  }
} // namespace

dawn::Version dawn::version() noexcept {
  static auto maj = major();
  static auto min = minor();
  static auto pat = patch();

  return dawn::Version{maj, min, pat};
}