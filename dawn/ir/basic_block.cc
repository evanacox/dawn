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

#include "dawn/ir/basic_block.h"
#include "dawn/ir/instructions.h"

namespace dawn {
  void BasicBlock::prepend(Instruction* inst) noexcept {
    instructions_.insert(instructions_.begin(), inst);
  }

  void BasicBlock::append(Instruction* inst) noexcept {
    instructions_.push_back(inst);
  }

  void BasicBlock::insertBefore(const Instruction* before, Instruction* toInsert) noexcept {
    // NOLINTNEXTLINE(readability-qualified-auto)
    auto it = std::find(instructions_.begin(), instructions_.end(), before);

    instructions_.insert(it, toInsert);
  }

  void BasicBlock::insertAfter(const Instruction* after, Instruction* toInsert) noexcept {
    // NOLINTNEXTLINE(readability-qualified-auto)
    auto it = std::find(instructions_.begin(), instructions_.end(), after);

    instructions_.insert(it + 1, toInsert);
  }
} // namespace dawn