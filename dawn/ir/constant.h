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
#include "../utility/apint.h"
#include "./module.h"
#include "./types.h"
#include "./value.h"

namespace dawn {
  class DAWN_PUBLIC Constant : public Value {
  public:
    [[nodiscard]] static bool instance_of(const Value* val) {
      constexpr auto constant_start = ValueKind::const_int;
      constexpr auto constant_end = ValueKind::const_null;

      return val->kind() >= constant_start && val->kind() <= constant_end;
    }

    [[nodiscard]] bool is_null() const noexcept {
      return null_;
    }

  protected:
    template <typename T>
    constexpr explicit Constant(T* ptr, Type* ty, bool nullish) noexcept : Value(ptr, ty),
                                                                           null_{nullish} {}

  private:
    bool null_;
  };

  class DAWN_PUBLIC ConstantInt final : public Constant {
  public:
    inline constexpr static ValueKind kind = ValueKind::const_int;

    explicit ConstantInt(APInt value, Type* ty) noexcept : Constant(this, ty, value.value() == 0), value_{value} {}

  protected:
    void hash(absl::HashState state) const noexcept final;

  private:
    APInt value_;
  };
} // namespace dawn