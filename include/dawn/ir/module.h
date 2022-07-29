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
#include "../utility/bump_alloc.h"
#include "./internal/type_manager.h"
#include "./types.h"

namespace dawn {
  class DAWN_PUBLIC Module {
  public:
    Module();

    Module(const Module&) = delete;

    Module(Module&&) = default;

    Module& operator=(const Module&) = delete;

    Module& operator=(Module&&) = default;

    ~Module() = default;

    [[nodiscard]] Type* i8() const noexcept {
      return manager_.i8();
    }

    [[nodiscard]] Type* i16() const noexcept {
      return manager_.i16();
    }

    [[nodiscard]] Type* i32() const noexcept {
      return manager_.i32();
    }

    [[nodiscard]] Type* i64() const noexcept {
      return manager_.i64();
    }

    [[nodiscard]] Type* boolean() const noexcept {
      return manager_.boolean();
    }

    [[nodiscard]] Type* ptr() const noexcept {
      return manager_.ptr();
    }

    [[nodiscard]] Type* f32() const noexcept {
      return manager_.f32();
    }

    [[nodiscard]] Type* f64() const noexcept {
      return manager_.f64();
    }

    [[nodiscard]] Type* empty_struct() const noexcept {
      return manager_.empty_struct();
    }

    [[nodiscard]] Type* int_of_width(std::uint64_t width) const noexcept {
      return manager_.int_of_width(width);
    }

    [[nodiscard]] Type* float_of_width(std::uint64_t width) const noexcept {
      return manager_.float_of_width(width);
    }

    [[nodiscard]] Type* array(Type* element, std::uint64_t length) noexcept {
      return manager_.array(&pool_, element, length);
    }

    [[nodiscard]] Type* structure(std::span<Type*> fields) noexcept {
      return manager_.structure(&pool_, fields);
    }

  private:
    BumpAlloc pool_;
    internal::TypeManager manager_;
  };
} // namespace dawn