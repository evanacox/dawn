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

#include "../adt/optional_ptr.h"
#include "../ir.h"
#include "absl/container/flat_hash_map.h"
#include <concepts>
#include <tuple>
#include <vector>

namespace dawn {
  namespace internal {
    template <typename T> using VMArray = std::vector<T>;

    template <typename... Ts> using VMStruct = std::tuple<Ts...>;

    class VMExpr {
    public:
    };

    template <typename T> inline constexpr bool isVmArray = false;

    template <typename T> inline constexpr bool isVmArray<VMArray<T>> = true;

    template <typename T> inline constexpr bool isVmStruct = false;

    template <typename... Ts> inline constexpr bool isVmStruct<VMStruct<Ts...>> = true;

    template <typename T> [[nodiscard]] Constant* fromRealValue(Type* ty, T value, Module* mod) noexcept {
      auto ib = IRBuilder{mod};

      if constexpr (std::same_as<bool, T>) {
        return ib.constBool(value);
      } else if constexpr (std::same_as<std::nullptr_t, T>) {
        return ib.constNull();
      } else if constexpr (std::floating_point<T>) {
        return ib.constFloat(value, dyncastUnchecked<Float>(ty));
      } else if constexpr (isVmArray<T>) {
        //
      } else if constexpr (isVmStruct<T>) {
        //
      } else if constexpr (std::is_pointer_v<T>) {
        //
      }
    }
  } // namespace internal

  class NativeFunction {
  public:
    template <typename Fn> explicit NativeFunction(Type* returnTy, Fn* fn) noexcept;

    [[nodiscard]] OptionalPtr<Constant> call(std::span<Constant* const> args) noexcept;

  private:
  };

  class VM {
  public:
    explicit VM(Module* module) : mod_{module} {}

    void cycle() noexcept;

    [[nodiscard]] OptionalPtr<Constant> resultOf(Instruction* inst) const noexcept;

  private:
    Module* mod_;
    absl::flat_hash_map<Instruction*, Constant*> results_;
  };
} // namespace dawn