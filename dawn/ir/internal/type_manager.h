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

#include "../../adt/deep_hash_table.h"
#include "../../utility/bump_alloc.h"
#include "../types.h"
#include <span>

namespace dawn::internal {
  class TypeManager final {
  public:
    explicit TypeManager(BumpAlloc* alloc) noexcept;

    TypeManager(const TypeManager&) = delete;

    TypeManager(TypeManager&&) = default;

    TypeManager& operator=(const TypeManager&) = delete;

    TypeManager& operator=(TypeManager&&) noexcept = default;

    ~TypeManager() = default;

    [[nodiscard]] Type* i8() const noexcept;

    [[nodiscard]] Type* i16() const noexcept;

    [[nodiscard]] Type* i32() const noexcept;

    [[nodiscard]] Type* i64() const noexcept;

    [[nodiscard]] Type* boolean() const noexcept;

    [[nodiscard]] Type* ptr() const noexcept;

    [[nodiscard]] Type* f32() const noexcept;

    [[nodiscard]] Type* f64() const noexcept;

    [[nodiscard]] Type* emptyStruct() const noexcept;

    [[nodiscard]] Type* voidType() const noexcept;

    [[nodiscard]] Type* intOfWidth(std::uint64_t width) const noexcept;

    [[nodiscard]] Type* floatOfWidth(std::uint64_t width) const noexcept;

    [[nodiscard]] Type* array(BumpAlloc* alloc, Type* element, std::uint64_t length) noexcept;

    [[nodiscard]] Type* structure(BumpAlloc* alloc, std::span<Type* const> fields) noexcept;

  private:
    Type* insertUnique(BumpPtr<Type> ty) noexcept;

    // some types live at fixed offsets, its faster for them to live in the array normally
    std::vector<BumpPtr<Type>> owned_;
    dawn::DeepHashSet<Type*> types_;
  };
} // namespace dawn::internal