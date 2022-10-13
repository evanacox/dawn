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

#include "dawn/ir/ir_builder.h"
#include "dawn/ir/ir_reader.h"
#include "dawn/ir/module.h"
#include <string_view>

namespace dawn::tests {
  // simple if-else CFG
  constexpr std::string_view sampleCfgIfElse = R"(
  func i32 @f(i32 $0) {
  %entry:
    $1 = icmp eq i32 $0, 0
    cbr bool $1, if %if.true, else %if.false

  %if.true:
    br %merge

  %if.false
    br %merge

  %merge:
    ret i32 $0
  }
  )";

  // simple looping CFG
  constexpr std::string_view sampleCfgLoop = R"--(
  func i32 @f(i32 $0) {
  %entry:
    br %loop.entry

  %loop.header:
    $1 = phi i32 [ $0, %entry ], [ $2, %loop.latch ]
    $2 = icmp eq i32 $0, $1
    cbr bool $2, if %exit, else %loop.body

  %loop.body
    br %loop.latch

  %loop.latch:
    $2 = iadd i32 $1, 1
    br %loop.header

  %exit:
    ret i32 $1
  }
  )--";

  // simple irreducible CFG
  constexpr std::string_view sampleCfgIrreducible = R"--(
  decl bool @cond()

  func i32 @f(i32 $0) {
  %entry:
    $1 = icmp eq $0, 16
    cbr bool $1, if %a, else %b

  %a:
    br %b

  %b:
    $2 = call bool @cond()
    cbr bool $2, if %a, else %exit

  %exit:
    ret i32 5
  }
  )--";

  // simple infinite loop CFG
  constexpr std::string_view sampleCfgInfinite = R"--(
  func i32 @f(i32 $0) {
  %entry:
    br %a

  %a:
    br %b

  %b:
    br %a

  %exit:
    unreachable
  }
  )--";

  [[nodiscard]] std::unique_ptr<dawn::Module> sampleIfElse() noexcept;

  [[nodiscard]] std::unique_ptr<dawn::Module> sampleLoop() noexcept;

  [[nodiscard]] std::unique_ptr<dawn::Module> sampleIrreducible() noexcept;

  [[nodiscard]] std::unique_ptr<dawn::Module> sampleInfinite() noexcept;

  [[nodiscard]] std::pair<std::unique_ptr<dawn::Module>, dawn::IRBuilder> generateTestModule() noexcept;

} // namespace dawn::tests