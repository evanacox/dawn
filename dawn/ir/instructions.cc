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

#include "dawn/ir/instructions.h"
#include "absl/hash/hash.h"
#include <typeindex>
#include <typeinfo>

namespace dawn {
  void Phi::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(Phi)}, type(), incoming_);
  }

  bool Phi::equals(const Value* other) const noexcept {
    const auto* phi = dawn::dyncastUnchecked<const Phi>(other);

    return type() == phi->type()
           && std::equal(phi->incoming_.begin(), phi->incoming_.end(), incoming_.begin(), incoming_.end());
  }

  void Call::hash(absl::HashState state) const noexcept {
    auto args = operands();

    state = absl::HashState::combine(std::move(state), std::type_index{typeid(Call)}, target());

    absl::HashState::combine_contiguous(std::move(state), args.data(), args.size());
  }

  bool Call::equals(const dawn::Value* val) const noexcept {
    const auto* other = dawn::dyncastUnchecked<const Call>(val);

    auto selfArgs = args();
    auto otherArgs = other->args();

    return other->target() == target()
           && std::equal(selfArgs.begin(), selfArgs.end(), otherArgs.begin(), otherArgs.end());
  }

  void ICmp::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(ICmp)}, order_, lhs(), rhs());
  }

  bool ICmp::equals(const dawn::Value* val) const noexcept {
    const auto* other = dawn::dyncastUnchecked<const ICmp>(val);

    return order() == other->order() && lhs() == other->lhs() && rhs() == other->rhs();
  }

  void FCmp::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(FCmp)}, order_, lhs(), rhs());
  }

  bool FCmp::equals(const dawn::Value* val) const noexcept {
    const auto* other = dawn::dyncastUnchecked<const FCmp>(val);

    return order() == other->order() && lhs() == other->lhs() && rhs() == other->rhs();
  }

  void Sel::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(Sel)}, cond(), ifTrue(), ifFalse());
  }

  bool Sel::equals(const dawn::Value* val) const noexcept {
    const auto* other = dawn::dyncastUnchecked<const Sel>(val);

    return other->cond() == cond() && other->ifTrue() == ifTrue() && other->ifFalse() == ifFalse();
  }

  void Br::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(Br)}, target());
  }

  bool Br::equals(const Value* other) const noexcept {
    const auto* branch = dawn::dyncastUnchecked<const Br>(other);

    return branch->target() == target();
  }

  void CondBr::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(CondBr)}, cond(), trueBranch(), falseBranch());
  }

  bool CondBr::equals(const Value* val) const noexcept {
    const auto* cond = dawn::dyncastUnchecked<const CondBr>(val);

    return cond->cond() == this->cond() && cond->trueBranch() == trueBranch() && cond->falseBranch() == falseBranch();
  }

  void Ret::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state),
        std::type_index{typeid(Ret)},
        returnTy(),
        returnValue() ? returnValue().get() : nullptr);
  }

  bool Ret::equals(const dawn::Value* val) const noexcept {
    return dawn::dyncastUnchecked<const Ret>(val)->returnValue() == returnValue();
  }

  void Unreachable::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(Unreachable)});
  }

  bool Unreachable::equals(const Value* /*unused*/) const noexcept {
    return true;
  }

  void Load::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(Load)}, isVolatile(), target(), type());
  }

  bool Load::equals(const Value* val) const noexcept {
    const auto* load = dawn::dyncastUnchecked<const Load>(val);

    return load->target() == target() && load->type() == type() && load->isVolatile() == isVolatile();
  }

  void Store::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(Store)}, isVolatile(), value(), target());
  }

  bool Store::equals(const Value* val) const noexcept {
    const auto* store = dawn::dyncastUnchecked<const Store>(val);

    return store->value() == value() && store->target() == target() && store->isVolatile() == isVolatile();
  }

#define BINARY_INST_IMPL(Ty)                                                                                           \
  void Ty::hash(absl::HashState state) const noexcept {                                                                \
    absl::HashState::combine(std::move(state), std::type_index{typeid(Ty)}, lhs(), rhs());                             \
  }                                                                                                                    \
                                                                                                                       \
  bool Ty::equals(const Value* val) const noexcept {                                                                   \
    const auto* inst = dawn::dyncastUnchecked<const Ty>(val);                                                          \
    return inst->lhs() == lhs() && inst->rhs() == rhs();                                                               \
  }

  DAWN_FOR_EACH_BINARY_INST(BINARY_INST_IMPL)

#define CONVERSION_INST_IMPL(Ty)                                                                                       \
  void Ty::hash(absl::HashState state) const noexcept {                                                                \
    absl::HashState::combine(std::move(state), std::type_index{typeid(Ty)}, type(), operands()[0]);                    \
  }                                                                                                                    \
  bool Ty::equals(const Value* val) const noexcept {                                                                   \
    const auto* ptr = dawn::dyncastUnchecked<const Ty>(val);                                                           \
    return ptr->type() == type() && ptr->operands()[0] == operands()[0];                                               \
  }

  DAWN_FOR_EACH_CONVERSION_INST(CONVERSION_INST_IMPL)

  void Offset::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(Offset)}, offsetTy(), ptr(), offset());
  }

  bool Offset::equals(const Value* val) const noexcept {
    const auto* other = dyncastUnchecked<const Offset>(val);

    return offsetTy() == other->offsetTy() && ptr() == other->ptr() && offset() == other->offset();
  }

  void Extract::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(Extract)}, type(), aggregate(), index());
  }

  bool Extract::equals(const Value* val) const noexcept {
    const auto* other = dyncastUnchecked<const Extract>(val);

    return aggregate() == other->aggregate() && index() == other->index();
  }

  void Insert::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(Insert)}, type(), aggregate(), index(), value());
  }

  bool Insert::equals(const Value* val) const noexcept {
    const auto* other = dyncastUnchecked<const Insert>(val);

    return aggregate() == other->aggregate() && index() == other->index() && value() == other->value();
  }

  void Alloca::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(Alloca)}, numberOfObjects(), allocatingTy());
  }

  bool Alloca::equals(const Value* val) const noexcept {
    const auto* other = dyncastUnchecked<const Alloca>(val);

    return numberOfObjects() == other->numberOfObjects() && allocatingTy() == other->allocatingTy();
  }

  void ElemPtr::hash(absl::HashState state) const noexcept {
    absl::HashState::combine(std::move(state), std::type_index{typeid(ElemPtr)}, type(), aggregateTy(), ptr(), index());
  }

  bool ElemPtr::equals(const Value* val) const noexcept {
    const auto* other = dyncastUnchecked<const ElemPtr>(val);

    return type() == other->type() && aggregateTy() == other->aggregateTy() && ptr() == other->ptr()
           && index() == other->index();
  }
} // namespace dawn
