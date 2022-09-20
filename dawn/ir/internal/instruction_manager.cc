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

#include "dawn/ir/internal/instruction_manager.h"
#include "absl/container/flat_hash_map.h"

namespace dawn::internal {
  absl::flat_hash_map<Instruction*, std::size_t> InstructionManager::useCountOfEveryInst() const noexcept {
    auto counts = absl::flat_hash_map<Instruction*, std::size_t>{};

    for (const auto& inst : instructions_) {
      for (auto* operand : inst->operands()) {
        if (auto ptr = dawn::dyncast<Instruction>(operand)) {
          ++counts[ptr.get()];
        }
      }
    }

    return counts;
  }

  std::size_t InstructionManager::useCount(Instruction* inst) const noexcept {
    auto count = std::size_t{0};

    for (const auto& instruction : instructions_) {
      count += instruction->useCount(inst);
    }

    return count;
  }
} // namespace dawn::internal