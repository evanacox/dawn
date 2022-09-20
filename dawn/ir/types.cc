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

#include "dawn/ir/types.h"
#include <typeindex>
#include <typeinfo>

namespace {
  std::vector<dawn::Type*> from_span(std::span<dawn::Type* const> view) noexcept {
    std::vector<dawn::Type*> result{view.size(), nullptr};

    std::copy(view.begin(), view.end(), result.begin());

    return result;
  }
} // namespace

namespace dawn {
  Struct::Struct(std::vector<dawn::Type*> fields) noexcept : Type(this), fields_{std::move(fields)} {}

  Struct::Struct(std::span<dawn::Type* const> fields) noexcept : Type(this), fields_{from_span(fields)} {}

  void Int::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(Int)}, width_);
  }

  bool Int::equals(const Type* ty) const noexcept {
    const auto& ty1 = *dawn::dyncastUnchecked<const Int>(ty);

    return width() == ty1.width();
  }

  void Float::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(Float)}, width_);
  }

  bool Float::equals(const Type* ty) const noexcept {
    const auto& ty1 = *dawn::dyncastUnchecked<const Float>(ty);

    return width() == ty1.width();
  }

  void Bool::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(Bool)});
  }

  bool Bool::equals(const Type* /*unused*/) const noexcept {
    return true;
  }

  void Ptr::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(Ptr)});
  }

  bool Ptr::equals(const Type* /*unused*/) const noexcept {
    return true;
  }

  void Array::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(Array)}, element_, len_);
  }

  bool Array::equals(const Type* ty) const noexcept {
    const auto& ty1 = *dawn::dyncastUnchecked<const Array>(ty);

    return len() == ty1.len() && *element() == *ty1.element();
  }

  void Struct::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(Struct)}, fields_);
  }

  bool Struct::equals(const Type* ty) const noexcept {
    const auto& ty1 = *dawn::dyncastUnchecked<const Struct>(ty);

    return std::equal(fields_.begin(),
        fields_.end(),
        ty1.fields_.begin(),
        ty1.fields_.end(),
        [](const Type* lhs, const Type* rhs) {
          return *lhs == *rhs;
        });
  }

  void Void::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(Void)});
  }

  bool Void::equals(const Type* /*unused*/) const noexcept {
    return true;
  }
} // namespace dawn
