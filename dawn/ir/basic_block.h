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

#include "./instruction.h"
#include "absl/container/inlined_vector.h"
#include <span>

namespace dawn {
  class Module;

  class BasicBlock {
  public:
    [[nodiscard]] explicit BasicBlock(Module* parent) noexcept : parent_{parent} {}

    [[nodiscard]] Module* parent() const noexcept {
      return parent_;
    }

    [[nodiscard]] std::span<const Instruction* const> instructions() const noexcept {
      return instructions_;
    }

    [[nodiscard]] std::span<Instruction* const> instructions() noexcept {
      return instructions_;
    }

    void prepend(Instruction* inst) noexcept;

    void append(Instruction* inst) noexcept;

    void insertBefore(const Instruction* before, Instruction* to_insert) noexcept;

    void insertAfter(const Instruction* after, Instruction* to_insert) noexcept;

  private:
    // 6 makes the size be exactly 8 words, i.e. exactly one normal cache line in size.
    // on any architectures with 128byte cache line, two fit perfectly
    inline constexpr static std::size_t small_size = 6;

    Module* parent_;
    absl::InlinedVector<Instruction*, small_size> instructions_;
  };
} // namespace dawn