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
#include "../utility/assertions.h"
#include "./basic_block.h"
#include "./instruction.h"
#include "./module.h"
#include "./value.h"
#include <utility>
#include <vector>

namespace dawn {
  class DAWN_PUBLIC Phi final : public Instruction {
  public:
    inline static constexpr ValueKind kind = ValueKind::phi_inst;

    [[nodiscard]] explicit Phi(Type* ty) noexcept : Instruction(this, ty, {}) {}

    void addIncomingBlock(BasicBlock* from, Value* value_from) noexcept {
      incoming_.emplace_back(from, value_from);
    }

    [[nodiscard]] std::span<const std::pair<BasicBlock*, Value*>> incoming() const noexcept {
      return incoming_;
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

  private:
    std::vector<std::pair<BasicBlock*, Value*>> incoming_;
  };

  class DAWN_PUBLIC TerminatorInst : public Instruction {
  public:
    [[nodiscard]] static bool instanceOf(const Value* val) {
      return val->kind() >= ValueKind::terminators_inst_begin && val->kind() <= ValueKind::terminators_inst_end;
    }

    [[nodiscard]] bool references(BasicBlock* value) const noexcept;

    void replaceReferencedBlock(BasicBlock* to_replace, BasicBlock* replace_with) noexcept;

  protected:
    template <typename T>
    TerminatorInst(T* ptr,
        BasicBlock* any,
        std::initializer_list<BasicBlock*> blocks,
        std::initializer_list<Value*> values) noexcept
        : Instruction(ptr, any->parent()->voidType(), values),
          references_{blocks} {}

    [[nodiscard]] std::span<BasicBlock* const> refs() const noexcept {
      return references_;
    }

  private:
    absl::InlinedVector<BasicBlock*, 2> references_;
  };

  class DAWN_PUBLIC Br final : public TerminatorInst {
  public:
    inline static constexpr ValueKind kind = ValueKind::br_inst;

    [[nodiscard]] explicit Br(BasicBlock* target) noexcept : TerminatorInst(this, target, {target}, {}) {}

    [[nodiscard]] BasicBlock* target() const noexcept {
      return this->refs()[0];
    }

  protected:
    void hash(absl::HashState state) const noexcept final;
  };

  class DAWN_PUBLIC CondBr final : public TerminatorInst {
  public:
    inline static constexpr ValueKind kind = ValueKind::cbr_inst;

    [[nodiscard]] explicit CondBr(BasicBlock* if_true, Value* cond, BasicBlock* if_false) noexcept
        : TerminatorInst(this, if_true, {if_true, if_false}, {cond}) {}

    [[nodiscard]] Value* cond() const noexcept {
      return this->operands()[0];
    }

    [[nodiscard]] BasicBlock* trueBranch() const noexcept {
      return this->refs()[0];
    }

    [[nodiscard]] BasicBlock* falseBranch() const noexcept {
      return this->refs()[1];
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

  private:
    BasicBlock* target_;
  };

  class DAWN_PUBLIC BinaryInst : public Instruction {
  public:
    [[nodiscard]] static bool instanceOf(const Value* val) {
      return val->kind() >= ValueKind::binary_inst_begin && val->kind() <= ValueKind::binary_inst_end;
    }

    [[nodiscard]] const Value* lhs() const noexcept {
      return this->operands()[0];
    }

    [[nodiscard]] const Value* rhs() const noexcept {
      return this->operands()[0];
    }

    [[nodiscard]] Value* lhs() noexcept {
      return this->operands()[0];
    }

    [[nodiscard]] Value* rhs() noexcept {
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
