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

#include "../../adt/optional_ptr.h"
#include "../function.h"
#include "absl/container/flat_hash_map.h"

namespace dawn::internal {
  class FunctionManager {
  public:
    [[nodiscard]] OptionalPtr<Function> getFunctionIfExists(std::string_view name) const noexcept;

    [[nodiscard]] bool contains(std::string_view name) const noexcept;

    [[nodiscard]] Function* create(std::string name, Type* ty) noexcept;

  private:
    absl::flat_hash_map<std::string, std::unique_ptr<Function>> functions_;
  };
} // namespace dawn::internal