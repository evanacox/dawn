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

#include "../utility/bump_alloc.h"
#include "./basic_block.h"
#include "./instruction.h"
#include "./instructions.h"

namespace dawn {
  class Module;

  class IRBuilder {
  public:
    [[nodiscard]] explicit IRBuilder(BumpAlloc* alloc, Module* mod) noexcept;

    [[nodiscard]] OptionalPtr<BasicBlock> insertBlock() noexcept {
      return curr_block_;
    }

    void setInsertBlock(BasicBlock* block) noexcept {
      curr_block_ = dawn::some(block);
    }

    void clearInsertBlock() noexcept {
      curr_block_ = dawn::none<BasicBlock>();
    }

    void createIAdd(Value* lhs, Value* rhs) noexcept;

  private:
    BumpAlloc* pool_;
    Module* mod_;
    OptionalPtr<BasicBlock> curr_block_;
  };
} // namespace dawn