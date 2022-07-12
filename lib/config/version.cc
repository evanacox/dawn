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

#include "dawn/config/version.h"
#include <absl/strings/str_split.h>
#include <charconv>

#define STRINGIFY2(value) #value
#define STRINGIFY(value) STRINGIFY2(value)

namespace {
  inline constexpr std::string_view version_str = STRINGIFY(DAWN_VERSION_STRING);

  qz::Version generate_version() noexcept {
    std::vector<std::string_view> split = absl::StrSplit(version_str, '.');
    qz::Version ver;

    // none of these should fail
    std::from_chars(split[0].begin(), split[0].end(), ver.major);
    std::from_chars(split[1].begin(), split[1].end(), ver.minor);
    std::from_chars(split[2].begin(), split[2].end(), ver.patch);

    return ver;
  }
} // namespace

std::string_view qz::version_string() noexcept {
  return version_str;
}

qz::Version qz::version() noexcept {
  static qz::Version version = generate_version();

  return version;
}

bool qz::optimized() noexcept {
#ifdef QUARTZ_OPTIMIZED_BUILD
  return true;
#else
  return false;
#endif
}