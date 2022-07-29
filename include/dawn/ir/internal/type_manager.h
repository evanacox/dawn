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

#include "../../utility/bump_alloc.h"
#include "../types.h"
#include "absl/container/flat_hash_set.h"

namespace dawn::internal {
  class TypeRef final {
  public:
    TypeRef(Type* ty) noexcept : ty_{ty} {} // NOLINT(google-explicit-constructor)

    template <typename H> friend H AbslHashValue(H state, const TypeRef& ty) {
      return H::combine(std::move(state), *ty.ty_);
    }

    [[nodiscard]] operator Type*() const noexcept { // NOLINT(google-explicit-constructor)
      return ty_;
    }

  private:
    Type* ty_;
  };

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

    [[nodiscard]] Type* empty_struct() const noexcept;

    [[nodiscard]] Type* int_of_width(std::uint64_t width) const noexcept;

    [[nodiscard]] Type* float_of_width(std::uint64_t width) const noexcept;

    [[nodiscard]] Type* array(BumpAlloc* alloc, Type* element, std::uint64_t length) noexcept;

    [[nodiscard]] Type* structure(BumpAlloc* alloc, std::span<Type*> fields) noexcept;

  private:
    Type* insert_unique(BumpPtr<Type> ty) noexcept;

    absl::flat_hash_set<TypeRef> types_;
    std::vector<BumpPtr<Type>> owned_;
  };
} // namespace dawn::internal