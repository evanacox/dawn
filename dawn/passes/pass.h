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

#include "../analysis/analyses.h"
#include "../analysis/analysis_manager.h"
#include "../config.h"
#include "../ir/function.h"
#include "../ir/module.h"

namespace dawn {
  class DAWN_PUBLIC Pass {
  public:
    virtual ~Pass() = default;

    virtual void run(Module* mod, AnalysisManager* manager) noexcept = 0;
  };

  class DAWN_PUBLIC FunctionPass : public Pass {
  public:
    void run(Module* mod, AnalysisManager* manager) noexcept override;

    virtual void run(Function* fn, AnalysisManager* manager) noexcept = 0;
  };
} // namespace dawn