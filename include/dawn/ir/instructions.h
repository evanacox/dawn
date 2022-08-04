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

#include "../config.h"
#include "./instruction.h"
#include "./value.h"

namespace dawn {
  class DAWN_PUBLIC BinaryInst : public Instruction {
  public:
    [[nodiscard]] static bool instance_of(const Value* val) {
      return val->kind() >= ValueKind::binary_inst_begin && val->kind() <= ValueKind::binary_inst_end;
    }

    [[nodiscard]] Value* lhs() const noexcept {
      return this->operands()[0];
    }

    [[nodiscard]] Value* rhs() const noexcept {
      return this->operands()[0];
    }

  protected:
    template <typename T>
    BinaryInst(T* ptr, Type* ty, Value* lhs, Value* rhs) noexcept : Instruction(ptr, ty, {lhs, rhs}) {}
  };

  class DAWN_PUBLIC IAdd final : public BinaryInst {
  public:
    inline static constexpr ValueKind kind = ValueKind::iadd_inst;

    IAdd(Value* lhs, Value* rhs) noexcept : BinaryInst(this, lhs->type(), lhs, rhs) {
      DAWN_ASSERT(lhs->type() == rhs->type(), "`lhs` and `rhs` for `iadd` must have the same type");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;
  };

  class DAWN_PUBLIC ISub final : public BinaryInst {
  public:
    inline static constexpr ValueKind kind = ValueKind::iadd_inst;

    ISub(Value* lhs, Value* rhs) noexcept : BinaryInst(this, lhs->type(), lhs, rhs) {
      DAWN_ASSERT(lhs->type() == rhs->type(), "`lhs` and `rhs` for `iadd` must have the same type");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;
  };
} // namespace dawn
