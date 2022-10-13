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
#include "../ir/module.h"
#include "./analyses.h"
#include "absl/container/inlined_vector.h"
#include <bitset>
#include <cstddef>

namespace dawn {
  class DAWN_PUBLIC AnalysisPass {
  public:
    virtual ~AnalysisPass() = default;

    [[nodiscard]] Analysis kind() const noexcept {
      return kind_;
    }

    virtual void run(const Module& mod, class AnalysisManager* manager) noexcept = 0;

  protected:
    template <typename T> explicit AnalysisPass(T* /*unused*/) noexcept : kind_{T::kind} {}

  private:
    Analysis kind_;
  };

  class DAWN_PUBLIC FunctionAnalysisPass : public AnalysisPass {
  public:
    [[nodiscard]] static bool instanceOf(const AnalysisPass& pass) noexcept {
      return pass.kind() >= Analysis::functionPassBegin && pass.kind() <= Analysis::functionPassEnd;
    }

    void run(const Module& mod, class AnalysisManager* manager) noexcept override;

    virtual void run(const Function& fn, class AnalysisManager* manager) noexcept = 0;

  protected:
    template <typename T> explicit FunctionAnalysisPass(T* ptr) noexcept : AnalysisPass{ptr} {}
  };
} // namespace dawn