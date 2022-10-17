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

#include "dawn/ir/ir_builder.h"
#include "absl/container/inlined_vector.h"
#include "dawn/ir/module.h"

namespace dawn {
  IRBuilder::IRBuilder(Module* mod) noexcept : pool_{&mod->pool_}, mod_{mod} {}

  ConstantStruct* IRBuilder::constStruct(std::span<Constant* const> vals) noexcept {
    auto vec = absl::InlinedVector<Type*, 16>{};
    vec.reserve(vals.size());

    for (const auto* val : vals) {
      vec.push_back(val->type());
    }

    return mod_->constants_.createOrGet<ConstantStruct>(&mod_->pool_, structTy(vec), vals);
  }

  Instruction* IRBuilder::insertInst(BumpPtr<Instruction> inst) noexcept {
    DAWN_ASSERT(currentBlock(), "must have a block to insert into with `IRBuilder::insertInst`");

    currBlock_->append(inst.get());
    mod_->instructions_.insert(std::move(inst));

    return currBlock_->instructions().back();
  }

  ConstantValArray* IRBuilder::constArrayFill(Constant* val, std::size_t length) noexcept {
    auto vec = absl::InlinedVector<Constant*, 128>{length, val};

    return constArray(vec);
  }
} // namespace dawn
