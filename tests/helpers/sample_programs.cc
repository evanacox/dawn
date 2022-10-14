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

#include "./sample_programs.h"
#include <iostream>

namespace dawn {
  std::pair<std::unique_ptr<dawn::Module>, dawn::IRBuilder> tests::generateTestModule() noexcept {
    auto module = std::make_unique<dawn::Module>();
    auto builder = dawn::IRBuilder{module.get()};
    auto* main = builder.createFunc("main", builder.i32Ty());
    builder.setInsertFn(main);

    return {std::move(module), builder};
  }

  std::unique_ptr<dawn::Module> tests::sampleIfElse() noexcept {
    return tryParseIR(sampleCfgIfElse, &std::cerr).value(); // NOLINT
  }

  std::unique_ptr<dawn::Module> tests::sampleLoop() noexcept {
    return tryParseIR(sampleCfgLoop, &std::cerr).value(); // NOLINT
  }

  std::unique_ptr<dawn::Module> tests::sampleIrreducible() noexcept {
    return tryParseIR(sampleCfgIrreducible, &std::cerr).value(); // NOLINT
  }

  std::unique_ptr<dawn::Module> tests::sampleInfinite() noexcept {
    return tryParseIR(sampleCfgInfinite, &std::cerr).value(); // NOLINT
  }
} // namespace dawn
