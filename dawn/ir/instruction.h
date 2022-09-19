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
#include "../utility/strong_types.h"
#include "./value.h"
#include "absl/container/inlined_vector.h"
#include <initializer_list>
#include <span>

namespace dawn {
  enum class ICmpOrdering { eq, ne, ult, ugt, ule, uge, slt, sgt, sle, sge };

  enum class FCmpOrdering { ord, uno, oeq, one, ogt, olt, oge, ole, ueq, une, ugt, ult, uge, ule };

  class Instruction : public Value {
  public:
    [[nodiscard]] static bool instanceOf(const Value* val) {
      return val->kind() >= ValueKind::instBegin && val->kind() <= ValueKind::instEnd;
    }

    [[nodiscard]] std::span<const Value* const> operands() const noexcept {
      return {operands_.data(), operands_.size()};
    }

    [[nodiscard]] std::span<Value* const> operands() noexcept {
      return {operands_.data(), operands_.size()};
    }

    [[nodiscard]] std::size_t useCount(const Value* value) const noexcept;

    [[nodiscard]] bool uses(const Value* value) const noexcept;

    void replaceOperandWith(const Value* old_operand, ReplaceWith<Value*> new_operand) noexcept;

  protected:
    Instruction() = default;

    Instruction(const Instruction&) = default;

    Instruction(Instruction&&) = default;

    Instruction& operator=(const Instruction&) = default;

    Instruction& operator=(Instruction&&) = default;

    template <typename T>
    Instruction(T* ptr, Type* ty, std::initializer_list<Value*> operands) noexcept
        : Value(ptr, ty),
          operands_{operands} {}

    template <typename T>
    Instruction(T* ptr, Type* ty, std::span<Value* const> operands) noexcept
        : Value(ptr, ty),
          operands_{operands.begin(), operands.end()} {}

    void addOperand(Value* operand) noexcept {
      operands_.push_back(operand);
    }

    [[nodiscard]] std::span<Value*> operandsRaw() noexcept {
      return operands_;
    }

  private:
    absl::InlinedVector<Value*, 3> operands_;
  };
} // namespace dawn