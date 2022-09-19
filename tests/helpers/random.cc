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

#include "helpers/random.h"
#include "dawn/utility/assertions.h"
#include <random>

namespace dawn {
  std::size_t tests::randomBetween(std::pair<std::size_t, std::size_t> range) noexcept {
    static auto randomDevice = std::random_device{};
    static auto seed = std::seed_seq{randomDevice(), randomDevice(), randomDevice(), randomDevice(), randomDevice()};
    static auto rng = std::mt19937_64{seed};

    return std::uniform_int_distribution{range.first, range.second}(rng);
  }

  std::size_t tests::randomBetweenButNot(std::pair<std::size_t, std::size_t> range, std::size_t exclude) noexcept {
    while (true) {
      if (auto value = randomBetween(range); value != exclude) {
        return value;
      }
    }

    DAWN_UNREACHABLE("loop cannot exit except by returning");
  }
} // namespace dawn