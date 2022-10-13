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
#include <bitset>
#include <cstddef>

namespace dawn {
  enum class Analysis : unsigned {
    functionPassBegin = 0,
    cfgAnalysis = functionPassBegin,
    dominatorTree,
    functionPassEnd = dominatorTree,
    modulePassBegin,
    modulePassEnd,
  };

  class DAWN_PUBLIC PreservedAnalyses final {
  public:
    inline constexpr static std::size_t numDefaultAnalyses = 16;

    constexpr static PreservedAnalyses none() noexcept {
      return PreservedAnalyses(std::bitset<numDefaultAnalyses>{});
    }

    constexpr static PreservedAnalyses all() noexcept {
      return PreservedAnalyses(~std::bitset<numDefaultAnalyses>{});
    }

    template <typename T> void preserve() noexcept {
      preserved_.set(static_cast<std::size_t>(T::kind));
    }

    template <typename T> void invalidate() noexcept {
      preserved_.reset(static_cast<std::size_t>(T::kind));
    }

    template <typename T> bool isPreserved() noexcept {
      return preserved_.test(static_cast<std::size_t>(T::kind));
    }

    template <typename T> bool isInvalidated() noexcept {
      return !isPreserved<T>();
    }

  private:
    constexpr explicit PreservedAnalyses(std::bitset<numDefaultAnalyses> init) noexcept : preserved_{init} {}

    std::bitset<numDefaultAnalyses> preserved_;
  };
} // namespace dawn