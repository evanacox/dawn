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
  std::vector<dawn::Type*> from_span(std::span<dawn::Type*> view) noexcept {
    std::vector<dawn::Type*> result{view.size(), nullptr};

    std::copy(view.begin(), view.end(), result.begin());

    return result;
  }
} // namespace

namespace dawn {
  Struct::Struct(std::vector<dawn::Type*> fields) noexcept : Type(Struct::kind), fields_{std::move(fields)} {}

  Struct::Struct(std::span<dawn::Type*> fields) noexcept : Type(Struct::kind), fields_{from_span(fields)} {}

  void Int::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(Int)}, width_);
  }

  void Float::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(Float)}, width_);
  }

  void Bool::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(Bool)});
  }

  void Ptr::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(Ptr)});
  }

  void Array::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(Array)}, element_, len_);
  }

  void Struct::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(Struct)}, fields_);
  }
} // namespace dawn
