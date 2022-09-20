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

#include "dawn/ir/constant.h"
#include "dawn/ir/ir_builder.h"
#include "dawn/ir/module.h"
#include <typeindex>
#include <typeinfo>

namespace dawn {
  void ConstantInt::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(ConstantInt)}, type(), value_);
  }

  bool ConstantInt::equals(const Value* val) const noexcept {
    return type() == val->type() && dawn::dyncastUnchecked<const ConstantInt>(val)->value() == value();
  }

  ConstantBool::ConstantBool(Module* mod, bool value) noexcept
      : Constant(this, mod->boolean(), !value),
        value_{value} {}

  void ConstantBool::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index(typeid(ConstantBool)), value_);
  }

  bool ConstantBool::equals(const Value* val) const noexcept {
    return dawn::dyncastUnchecked<const ConstantBool>(val)->value() == value();
  }

  void ConstantFloat::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(ConstantFloat)}, type(), value_);
  }

  bool ConstantFloat::equals(const dawn::Value* val) const noexcept {
    return type() == val->type() && dawn::dyncastUnchecked<const ConstantFloat>(val)->value() == value();
  }

  ConstantNull::ConstantNull(Module* mod) noexcept : Constant(this, mod->ptr(), true) {}

  void ConstantNull::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(ConstantNull)});
  }

  bool ConstantNull::equals(const dawn::Value* /*unused*/) const noexcept {
    return true;
  }

  ConstantArray::ConstantArray(class Module* mod, std::span<Constant* const> values) noexcept
      : Constant(this, mod->array(values[0]->type(), values.size()), false) {
    auto haveSameTy = [first = values[0]](const Constant* val) {
      return val->type() == first->type();
    };

    DAWN_ASSERT(!values.empty(), "cannot create empty array literal");
    DAWN_ASSERT(std::all_of(values.begin(), values.end(), haveSameTy),
        "all array literal elements must have the same type!");
  }

  void ConstantArray::hash(absl::HashState state) const noexcept {
    auto values = this->values();

    state = absl::HashState::combine(std::move(state), std::type_index{typeid(ConstantArray)}, type());

    absl::HashState::combine_contiguous(std::move(state), values.data(), values.size());
  }

  bool ConstantArray::equals(const Value* val) const noexcept {
    const auto* other = dyncastUnchecked<const ConstantArray>(val);
    auto otherVals = other->values();
    auto selfVals = values();

    return type() == other->type() && std::equal(selfVals.begin(), selfVals.end(), otherVals.begin(), otherVals.end());
  }

  void ConstantStruct::hash(absl::HashState state) const noexcept {
    auto values = this->values();

    state = absl::HashState::combine(std::move(state), std::type_index{typeid(ConstantStruct)}, type());

    absl::HashState::combine_contiguous(std::move(state), values.data(), values.size());
  }

  bool ConstantStruct::equals(const Value* val) const noexcept {
    const auto* other = dyncastUnchecked<const ConstantStruct>(val);
    auto otherVals = other->values();
    auto selfVals = values();

    return type() == other->type() && std::equal(selfVals.begin(), selfVals.end(), otherVals.begin(), otherVals.end());
  }

  void ConstantUndef::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(ConstantUndef)}, type());
  }

  bool ConstantUndef::equals(const Value* val) const noexcept {
    return type() == val->type();
  }

  ConstantString::ConstantString(class Module* mod, std::string content) noexcept
      : Constant(this, mod->array(mod->i8(), content.size()), false),
        real_{std::move(content)} {
    chars_.reserve(real_.size());
    auto builder = dawn::IRBuilder{mod};

    for (auto ch : real_) {
      chars_.push_back(builder.constI8(static_cast<std::uint8_t>(ch)));
    }
  }

  void ConstantString::hash(absl::HashState state) const noexcept {
    auto values = asValues();

    state = absl::HashState::combine(std::move(state), std::type_index{typeid(ConstantString)}, type(), stringData());

    absl::HashState::combine_contiguous(std::move(state), values.data(), values.size());
  }

  bool ConstantString::equals(const Value* val) const noexcept {
    const auto* other = dyncastUnchecked<const ConstantString>(val);

    return stringData() == other->stringData();
  }
} // namespace dawn