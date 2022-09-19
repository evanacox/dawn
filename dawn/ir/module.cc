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

#include "dawn/ir/module.h"
#include "dawn/ir/ir_writer.h"

namespace {
  bool fnDeepEquals(const dawn::Function& lhs, const dawn::Function& rhs) noexcept {
    (void)lhs;
    (void)rhs;

    DAWN_UNREACHABLE("todo");
  }
} // namespace

namespace dawn {
  Module::Module() : tys_{&pool_} {}
} // namespace dawn

bool dawn::deepEquals(const dawn::Module& lhs, const dawn::Module& rhs) noexcept {
  auto lhsRange = internal::ReadonlyFunctionRange(&lhs.fns_);
  auto rhsRange = internal::ReadonlyFunctionRange(&rhs.fns_);

  for (auto lhsIt = lhsRange.begin(), rhsIt = rhsRange.begin(); lhsIt != lhsRange.end() && rhsIt != rhsRange.end();
       ++lhsIt, ++rhsIt) {
    if (!fnDeepEquals(*(lhsIt->second), *(rhsIt->second))) {
      return false;
    }
  }

  return true;
}