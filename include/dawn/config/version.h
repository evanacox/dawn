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

#include <string_view>

namespace qz {
  /// A semver version
  struct Version {
    int major;
    int minor;
    int patch;
  };

  /// Gets a human-readable semver version string for the library
  ///
  /// \return A human-readable version string
  std::string_view version_string() noexcept;

  /// Gets a machine-readable semver version for the library
  ///
  /// \return A semver version
  Version version() noexcept;

  /// Checks whether the library was built in optimized mode.
  ///
  /// \return Whether or not the library was built in an optimized mode
  bool optimized() noexcept;
} // namespace qz