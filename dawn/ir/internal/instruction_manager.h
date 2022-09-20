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

#include "../../utility/assertions.h"
#include "../../utility/bump_alloc.h"
#include "../instruction.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"

namespace dawn::internal {
  class InstructionManager {
  public:
    void insert(BumpPtr<Instruction> ptr) noexcept {
      instructions_.insert(std::move(ptr));
    }

    void remove(Instruction* to_remove) noexcept {
      DAWN_ASSERT(instructions_.contains(to_remove), "cannot remove non-existent instruction");

      instructions_.erase(to_remove);
    }

    [[nodiscard]] std::size_t useCount(Instruction* inst) const noexcept;

    [[nodiscard]] absl::flat_hash_map<Instruction*, std::size_t> useCountOfEveryInst() const noexcept;

  private:
    absl::flat_hash_set<BumpPtr<Instruction>> instructions_;
  };
} // namespace dawn::internal