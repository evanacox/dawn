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

#include "./basic_block.h"
#include "./types.h"
#include <memory>
#include <string>
#include <vector>

namespace dawn {
  class Module;

  class DAWN_PUBLIC Argument final : public Value {
  public:
    inline constexpr static ValueKind kind = ValueKind::argument;

    explicit Argument(Type* ty, std::size_t offset) : Value(this, ty), offset_{offset} {}

    [[nodiscard]] std::size_t offset() const noexcept {
      return offset_;
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;

  private:
    std::size_t offset_;
  };

  class DAWN_PUBLIC Function {
  public:
    [[nodiscard]] explicit Function(Module* parent,
        std::string name,
        Type* return_ty,
        std::span<Argument> args) noexcept
        : parent_{parent},
          args_{args.begin(), args.end()},
          name_{std::move(name)},
          return_ty_{return_ty} {}

    [[nodiscard]] std::string_view name() const noexcept {
      return name_;
    }

    [[nodiscard]] OptionalPtr<const BasicBlock> entry() const noexcept {
      if (!blocks_.empty()) {
        return dawn::some(&blocks_.front());
      }

      return dawn::none<const BasicBlock>();
    }

    [[nodiscard]] OptionalPtr<BasicBlock> entry() noexcept {
      if (!blocks_.empty()) {
        return dawn::some(&blocks_.front());
      }

      return dawn::none<BasicBlock>();
    }

    BasicBlock* addBlock(BasicBlock block) noexcept {
      blocks_.push_back(std::move(block));

      return &blocks_.back();
    }

    [[nodiscard]] Type* returnTy() const noexcept {
      return return_ty_;
    }

    [[nodiscard]] std::span<Argument> args() noexcept {
      return args_;
    }

    [[nodiscard]] std::span<const Argument> args() const noexcept {
      return args_;
    }

    [[nodiscard]] std::span<BasicBlock> blocks() noexcept {
      return blocks_;
    }

    [[nodiscard]] std::span<const BasicBlock> blocks() const noexcept {
      return blocks_;
    }

    [[nodiscard]] Module* parent() const noexcept {
      return parent_;
    }

    [[nodiscard]] bool opaque() const noexcept {
      return blocks().empty();
    }

  private:
    Module* parent_;
    // small functions (functions with just entry block, two possible branches and a merge point) like wrappers,
    // straight-line functions, etc. fit in the inline buffer
    absl::InlinedVector<BasicBlock, 4> blocks_;
    absl::InlinedVector<Argument, 2> args_; // vast majority of functions have <= 2 arguments
    std::string name_;
    Type* return_ty_;
  };
} // namespace dawn