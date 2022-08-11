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

#include "dawn/ir/internal/function_manager.h"
#include "dawn/utility/assertions.h"

namespace dawn::internal {
  OptionalPtr<Function> FunctionManager::getFunctionIfExists(std::string_view name) const noexcept {
    auto it = functions_.find(name);

    return it == functions_.end() ? dawn::none<Function>() : dawn::some(it->second.get());
  }

  bool FunctionManager::contains(std::string_view name) const noexcept {
    return functions_.contains(name);
  }

  Function* FunctionManager::create(std::string name, Type* ty) noexcept {
    DAWN_ASSERT(!contains(name), "cannot create two functions with the same name");

    return functions_.emplace(name, std::make_unique<Function>(name, ty)).first->second.get();
  }
} // namespace dawn::internal