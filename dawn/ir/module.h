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
#include "./internal/function_manager.h"
#include "./internal/type_manager.h"

namespace dawn {
  class IRBuilder;

  class DAWN_PUBLIC Module {
  public:
    Module();

    Module(const Module&) = delete;

    Module(Module&&) = default;

    Module& operator=(const Module&) = delete;

    Module& operator=(Module&&) = default;

    ~Module() = default;

    [[nodiscard]] Type* i8() const noexcept {
      return tys_.i8();
    }

    [[nodiscard]] Type* i16() const noexcept {
      return tys_.i16();
    }

    [[nodiscard]] Type* i32() const noexcept {
      return tys_.i32();
    }

    [[nodiscard]] Type* i64() const noexcept {
      return tys_.i64();
    }

    [[nodiscard]] Type* boolean() const noexcept {
      return tys_.boolean();
    }

    [[nodiscard]] Type* ptr() const noexcept {
      return tys_.ptr();
    }

    [[nodiscard]] Type* f32() const noexcept {
      return tys_.f32();
    }

    [[nodiscard]] Type* f64() const noexcept {
      return tys_.f64();
    }

    [[nodiscard]] Type* emptyStruct() const noexcept {
      return tys_.emptyStruct();
    }

    [[nodiscard]] Type* intType(std::uint64_t width) const noexcept {
      return tys_.intOfWidth(width);
    }

    [[nodiscard]] Type* floatType(std::uint64_t width) const noexcept {
      return tys_.floatOfWidth(width);
    }

    [[nodiscard]] Type* array(Type* element, std::uint64_t length) noexcept {
      return tys_.array(&pool_, element, length);
    }

    [[nodiscard]] Type* structure(std::span<Type*> fields) noexcept {
      return tys_.structure(&pool_, fields);
    }

    [[nodiscard]] Type* voidType() const noexcept {
      return tys_.voidType();
    }

    [[nodiscard]] OptionalPtr<Function> findFunction(std::string_view name) const noexcept {
      return fns_.getFunctionIfExists(name);
    }

    [[nodiscard]] Function* createFunction(std::string name, Type* ty) noexcept {
      return fns_.create(std::move(name), ty);
    }

    [[nodiscard]] std::unique_ptr<IRBuilder> builder() noexcept;

  private:
    BumpAlloc pool_;
    internal::TypeManager tys_;
    internal::FunctionManager fns_;
  };
} // namespace dawn