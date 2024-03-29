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

#include "dawn/utility/file.h"
#include <fstream>
#include <optional>

std::optional<std::string> dawn::readEntireFile(std::string_view filename) noexcept {
  auto data = std::string{};
  auto file = std::ifstream{std::string{filename}};

  if (file) {
    file.seekg(0, std::ios::end);
    auto length = static_cast<std::ptrdiff_t>(file.tellg());
    data.resize(static_cast<std::size_t>(length));
    file.seekg(0, std::ios::beg);
    file.read(data.data(), static_cast<std::ptrdiff_t>(data.size()));

    return data;
  }

  return std::nullopt;
}