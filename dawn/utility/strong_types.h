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

#include <utility>

namespace dawn {
  template <typename T> struct ReplaceWith {
    template <typename... Args> explicit ReplaceWith(Args&&... args) : value{std::forward<Args>(args)...} {}

    T value;
  };

  class BasicBlock;
  class Value;

  struct TrueBranch {
    explicit TrueBranch(BasicBlock* target) noexcept : target{target} {}

    BasicBlock* target; // NOLINT
  };

  struct FalseBranch {
    explicit FalseBranch(BasicBlock* target) noexcept : target{target} {}

    BasicBlock* target; // NOLINT
  };

  struct If {
    explicit If(Value* val) noexcept : val{val} {}

    Value* val; // NOLINT
  };

  struct Else {
    explicit Else(Value* val) noexcept : val{val} {}

    Value* val; // NOLINT
  };

  struct Dest {
    explicit Dest(Value* val) noexcept : val{val} {}

    Value* val; // NOLINT
  };

  struct Source {
    explicit Source(Value* val) noexcept : val{val} {}

    Value* val; // NOLINT
  };

  struct Index {
    explicit Index(Value* val) noexcept : val{val} {}

    Value* val; // NOLINT
  };

  struct Agg {
    explicit Agg(Value* val) noexcept : val{val} {}

    Value* val; // NOLINT
  };
} // namespace dawn