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

#include "dawn/ir/internal/type_manager.h"
#include "dawn/ir/types.h"
#include "dawn/utility/assertions.h"

enum class TypeIndex { i8 = 0, i16, i32, i64, boolean, ptr, f32, f64, empty_struct, void_type };

namespace dawn::internal {
  TypeManager::TypeManager(BumpAlloc* alloc) noexcept {
    // integers
    for (auto width : {8, 16, 32, 64}) {
      insertUnique(alloc->alloc<Int>(width));
    }

    // bool
    insertUnique(alloc->alloc<Bool>());

    // ptr
    insertUnique(alloc->alloc<Ptr>());

    // floats
    for (auto width : {32, 64}) {
      insertUnique(alloc->alloc<Float>(width));
    }

    // {}
    insertUnique(alloc->alloc<Struct>(std::span<Type*>{}));

    // `void`
    insertUnique(alloc->alloc<Void>());
  }

  [[nodiscard]] Type* TypeManager::i8() const noexcept {
    return owned_[static_cast<int>(TypeIndex::i8)].get();
  }

  [[nodiscard]] Type* TypeManager::i16() const noexcept {
    return owned_[static_cast<int>(TypeIndex::i16)].get();
  }

  [[nodiscard]] Type* TypeManager::i32() const noexcept {
    return owned_[static_cast<int>(TypeIndex::i32)].get();
  }

  [[nodiscard]] Type* TypeManager::i64() const noexcept {
    return owned_[static_cast<int>(TypeIndex::i64)].get();
  }

  [[nodiscard]] Type* TypeManager::boolean() const noexcept {
    return owned_[static_cast<int>(TypeIndex::boolean)].get();
  }

  [[nodiscard]] Type* TypeManager::ptr() const noexcept {
    return owned_[static_cast<int>(TypeIndex::ptr)].get();
  }

  [[nodiscard]] Type* TypeManager::f32() const noexcept {
    return owned_[static_cast<int>(TypeIndex::f32)].get();
  }

  [[nodiscard]] Type* TypeManager::f64() const noexcept {
    return owned_[static_cast<int>(TypeIndex::f64)].get();
  }

  [[nodiscard]] Type* TypeManager::emptyStruct() const noexcept {
    return owned_[static_cast<int>(TypeIndex::empty_struct)].get();
  }

  Type* TypeManager::intOfWidth(std::uint64_t width) const noexcept {
    switch (width) {
      case 8: return i8();
      case 16: return i16();
      case 32: return i32();
      case 64: return i64();
      default: DAWN_UNREACHABLE("cannot create integer of width other than 8, 16, 32 or 64");
    }
  }

  Type* TypeManager::floatOfWidth(std::uint64_t width) const noexcept {
    switch (width) {
      case 32: return f32();
      case 64: return f64();
      default: DAWN_UNREACHABLE("cannot create float of width other than 32 or 64");
    }
  }

  Type* TypeManager::array(BumpAlloc* alloc, Type* element, std::uint64_t length) noexcept {
    auto ty = Array{element, length};
    auto iter = types_.find(&ty);

    if (iter != types_.end()) {
      return *iter;
    }

    return insertUnique(alloc->alloc<Array>(std::move(ty)));
  }

  Type* TypeManager::structure(BumpAlloc* alloc, std::span<Type*> fields) noexcept {
    auto ty = Struct{fields};
    auto iter = types_.find(&ty);

    if (iter != types_.end()) {
      return *iter;
    }

    return insertUnique(alloc->alloc<Struct>(std::move(ty)));
  }

  Type* TypeManager::insertUnique(BumpPtr<Type> ty) noexcept {
    owned_.push_back(std::move(ty));
    auto [iter, did_insert] = types_.insert(owned_.back().get());

    DAWN_ASSERT(did_insert, "tried to insertUnique with non-unique type");

    return *iter;
  }

  Type* TypeManager::voidType() const noexcept {
    return owned_[static_cast<int>(TypeIndex::void_type)].get();
  }
} // namespace dawn::internal
