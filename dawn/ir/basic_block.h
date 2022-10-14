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
#include "./internal/string_pool.h"
#include "absl/container/inlined_vector.h"
#include <span>

namespace dawn {
  class Module;
  class TerminatorInst;

  class BasicBlock {
  public:
    [[nodiscard]] explicit BasicBlock(class Function* parent,
        internal::MaybeInternedString name = internal::MaybeInternedString{nullptr}) noexcept
        : parent_{parent},
          name_{name} {}

    [[nodiscard]] class Function* parent() const noexcept {
      return parent_;
    }

    [[nodiscard]] std::span<const Instruction* const> instructions() const noexcept {
      return instructions_;
    }

    [[nodiscard]] std::span<Instruction* const> instructions() noexcept {
      return instructions_;
    }

    [[nodiscard]] const TerminatorInst* terminator() const noexcept {
      DAWN_ASSERT(isa<TerminatorInst>(instructions_.back()), "last instruction in a basic block must be a terminator");

      return dyncastUnchecked<const TerminatorInst>(instructions_.back());
    }

    [[nodiscard]] TerminatorInst* terminator() noexcept {
      DAWN_ASSERT(isa<TerminatorInst>(instructions_.back()), "last instruction in a basic block must be a terminator");

      return dyncastUnchecked<TerminatorInst>(instructions_.back());
    }

    void prepend(Instruction* inst) noexcept;

    void append(Instruction* inst) noexcept;

    void insertBefore(const Instruction* before, Instruction* toInsert) noexcept;

    void insertAfter(const Instruction* after, Instruction* toInsert) noexcept;

    [[nodiscard]] std::optional<std::string_view> name() const noexcept {
      return name_.str();
    }

    void setName(internal::MaybeInternedString name) noexcept {
      name_ = name;
    }

  private:
    // 5 makes the size be exactly 8 words, i.e. exactly one normal cache line in size.
    // on any architectures with 128byte cache line, two fit perfectly
    inline constexpr static std::size_t smallSize = 5;

    class Function* parent_;
    internal::MaybeInternedString name_;
    absl::InlinedVector<Instruction*, smallSize> instructions_;
  };
} // namespace dawn