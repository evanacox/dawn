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
#include "../utility/rtti.h"
#include "../utility/traits.h"
#include "./analysis_passes.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/inlined_vector.h"
#include <concepts>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>

namespace dawn {
  namespace internal {
    // we're relying on a bit of constant folding to happen here,
    // if it does, we get our hash calculated at compile time but can
    // still pass it into functions that is defined in a different TU
    struct ComputedHash {
      // NOLINTNEXTLINE(google-explicit-constructor)
      ComputedHash(std::type_index kind, Analysis analysis) noexcept
          : hash{absl::HashOf(kind)},
            kind{kind},
            analysis{analysis} {}

      std::size_t hash;     // NOLINT
      std::type_index kind; // NOLINT
      Analysis analysis;    // NOLINT
    };

    template <typename T>
    concept AnalysisRTTI = requires {
                             { T::kind } -> std::same_as<Analysis>;
                           };

    template <typename T>
    concept DefaultAnalysis = AnalysisRTTI<T> && std::derived_from<T, AnalysisPass> && (!std::is_abstract_v<T>);

    template <typename T>
    concept AdditionalAnalysis = (!AnalysisRTTI<T>) && std::derived_from<T, AnalysisPass> && (!std::is_abstract_v<T>);
  } // namespace internal

  class DAWN_PUBLIC AnalysisManager final {
  public:
    explicit AnalysisManager(Module* mod);

    template <internal::AdditionalAnalysis T, typename... Args> void registerAnalysis(Args&&... args) noexcept {
      auto pass = std::make_unique<T>(std::forward<Args>(args)...);
    }

    template <internal::DefaultAnalysis T> T* analysis() noexcept {
      DAWN_ASSERT(analyses_.size() <= static_cast<std::size_t>(T::kind), "");

      return dyncastUnchecked<T>(lazyGetAnalysis({std::type_index{typeid(T)}, T::kind}));
    }

    template <internal::AdditionalAnalysis T> T* analysis() noexcept {
      DAWN_ASSERT(additionalAnalyses_.contains(std::type_index{typeid(T)}),
          "the analysis must be registered beforehand");

      return reinterpret_cast<T*>(lazyGetAdditionalAnalysis(std::type_index{typeid(T)}));
    }

    template <internal::DefaultAnalysis T> void invalidate() noexcept {
      invalidateAnalysis({std::type_index{typeid(T)}, T::kind});
    }

    template <internal::AdditionalAnalysis T> void invalidate() noexcept {
      invalidateAdditionalAnalysis(std::type_index{typeid(T)});
    }

  private:
    [[nodiscard]] AnalysisPass* lazyGetAnalysis(internal::ComputedHash pair) noexcept;

    void invalidateAnalysis(internal::ComputedHash pair) noexcept;

    void registerAdditionalAnalysis(std::type_index index, std::unique_ptr<AnalysisPass>) noexcept;

    [[nodiscard]] AnalysisPass* lazyGetAdditionalAnalysis(std::type_index index) noexcept;

    void invalidateAdditionalAnalysis(std::type_index index) noexcept;

    Module* mod_;
    // while we almost certainly know exactly how many analyses we have, I want it to be possible to
    // add some dynamically if needed. In the case where we have any additional analyses,
    // we don't pay for them (besides the additional stack size for the hash table)
    std::array<std::unique_ptr<AnalysisPass>, PreservedAnalyses::numDefaultAnalyses> analyses_;
    absl::flat_hash_map<std::type_index, std::unique_ptr<AnalysisPass>> additionalAnalyses_;
    absl::flat_hash_map<std::type_index, bool> valid_;
  };
} // namespace dawn