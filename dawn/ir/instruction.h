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
#include "./value.h"
#include "absl/container/inlined_vector.h"
#include <initializer_list>
#include <span>

namespace dawn {
  class Instruction : public Value {
  public:
    [[nodiscard]] static bool instanceOf(const Value* val) {
      return val->kind() >= ValueKind::inst_begin && val->kind() <= ValueKind::inst_end;
    }

    [[nodiscard]] std::span<const Value* const> operands() const noexcept {
      return {operands_.data(), operands_.size()};
    }

    [[nodiscard]] std::span<Value* const> operands() noexcept {
      return {operands_.data(), operands_.size()};
    }

    [[nodiscard]] std::size_t useCount(const Value* value) const noexcept;

    [[nodiscard]] bool uses(const Value* value) const noexcept;

    void replaceIfUsed(const Value* to_replace, Value* replace_with) noexcept;

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

  private:
    absl::InlinedVector<Value*, 3> operands_;
  };
} // namespace dawn