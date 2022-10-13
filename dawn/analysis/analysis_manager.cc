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

#include "dawn/analysis/analysis_manager.h"
#include "dawn/analysis/cfg_analysis.h"
#include "dawn/utility/traits.h"

namespace dawn {
  AnalysisManager::AnalysisManager(Module* mod) : mod_{mod} {
    auto initializeAnalysisValidity = [this]<typename T>() noexcept {
      valid_.emplace(std::type_index{typeid(T)}, false);
    };

    auto constructDefaultAnalysis = [this]<typename T, typename... Args>(Args&&... args) noexcept {
      analyses_[static_cast<std::size_t>(T::kind)] = std::make_unique<T>(std::forward<Args>(args)...);
    };

    auto defaultBoth = [&]<typename T>() noexcept {
      initializeAnalysisValidity.template operator()<T>();
      constructDefaultAnalysis.template operator()<T>();
    };

    forEachTy<CFGAnalysis>(defaultBoth);
  }

  AnalysisPass* AnalysisManager::lazyGetAnalysis(internal::ComputedHash pair) noexcept {
    DAWN_ASSERT(valid_.contains(pair.kind), "analysis not found in table, file a bug report");

    auto [hash, kind, analysis] = pair;
    auto valid = valid_.find(kind, hash)->second;
    auto* pass = analyses_[static_cast<std::size_t>(analysis)].get();

    // update the analysis, making it valid if it wasn't
    if (!valid) {
      pass->run(*mod_, this);
    }

    return pass;
  }

  void AnalysisManager::invalidateAnalysis(internal::ComputedHash pair) noexcept {
    DAWN_ASSERT(valid_.contains(pair.kind), "analysis not found in table, file a bug report");

    auto [hash, kind, analysis] = pair;
    auto it = valid_.find(kind, hash);

    // mark the analysis as invalidated
    it->second = false;
  }

  void AnalysisManager::registerAdditionalAnalysis(std::type_index index, std::unique_ptr<AnalysisPass> pass) noexcept {
    valid_.emplace(index, false);
    additionalAnalyses_.emplace(index, std::move(pass));
  }

  AnalysisPass* AnalysisManager::lazyGetAdditionalAnalysis(std::type_index index) noexcept {
    DAWN_ASSERT(valid_.contains(index), "analysis must be registered before use");

    auto valid = valid_.at(index);
    auto* pass = additionalAnalyses_.at(index).get();

    if (!valid) {
      pass->run(*mod_, this);
    }

    return pass;
  }

  void AnalysisManager::invalidateAdditionalAnalysis(std::type_index index) noexcept {
    DAWN_ASSERT(valid_.contains(index), "analysis must be registered before use");

    valid_.at(index) = false;
  }
} // namespace dawn
