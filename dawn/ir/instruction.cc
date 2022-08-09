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

#include "dawn/ir/instruction.h"

namespace dawn {
  std::size_t Instruction::useCount(const Value* value) const noexcept {
    return std::count(operands_.begin(), operands_.end(), value);
  }

  bool Instruction::uses(const Value* value) const noexcept {
    return std::find(operands_.begin(), operands_.end(), value) != operands_.end();
  }

  void Instruction::replaceIfUsed(const Value* to_replace, Value* replace_with) noexcept {
    std::replace_if(
        operands_.begin(),
        operands_.end(),
        [to_replace](Value* ptr) {
          return ptr == to_replace;
        },
        replace_with);
  }
} // namespace dawn
