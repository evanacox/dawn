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
#include "../utility/assertions.h"
#include "../utility/strong_types.h"
#include "./basic_block.h"
#include "./instruction.h"
#include "./module.h"
#include "./value.h"
#include <utility>
#include <vector>

namespace dawn {
  class DAWN_PUBLIC BinaryInst : public Instruction {
  public:
    [[nodiscard]] static bool instanceOf(const Value* val) {
      return val->kind() >= ValueKind::binaryInstBegin && val->kind() <= ValueKind::binaryInstEnd;
    }

    [[nodiscard]] const Value* lhs() const noexcept {
      return this->operands()[0];
    }

    [[nodiscard]] const Value* rhs() const noexcept {
      return this->operands()[1];
    }

    [[nodiscard]] Value* lhs() noexcept {
      return this->operands()[0];
    }

    [[nodiscard]] Value* rhs() noexcept {
      return this->operands()[1];
    }

  protected:
    template <typename T>
    BinaryInst(T* ptr, Type* ty, Value* lhs, Value* rhs) noexcept : Instruction(ptr, ty, {lhs, rhs}) {}
  };

  class DAWN_PUBLIC TerminatorInst : public Instruction {
  public:
    [[nodiscard]] static bool instanceOf(const Value* val) {
      return val->kind() >= ValueKind::terminatorsInstBegin && val->kind() <= ValueKind::terminatorsInstEnd;
    }

    [[nodiscard]] bool canBranchTo(BasicBlock* value) const noexcept {
      return std::find(references_.begin(), references_.end(), value) != references_.end();
    }

    void replaceBranchTarget(const BasicBlock* old_target, ReplaceWith<BasicBlock*> new_target) noexcept {
      std::replace(references_.begin(), references_.end(), const_cast<BasicBlock*>(old_target), new_target.value);
    }

  protected:
    template <typename T>
    TerminatorInst(T* ptr,
        BasicBlock* any,
        std::initializer_list<BasicBlock*> blocks,
        std::initializer_list<Value*> values) noexcept
        : Instruction(ptr, any->parent()->parent()->voidType(), values),
          references_{blocks} {}

    [[nodiscard]] std::span<BasicBlock* const> refs() const noexcept {
      return references_;
    }

  private:
    absl::InlinedVector<BasicBlock*, 2> references_;
  };

  class DAWN_PUBLIC ConversionInst : public Instruction {
  public:
    [[nodiscard]] static bool instanceOf(const Value* val) {
      return val->kind() >= ValueKind::conversionInstBegin && val->kind() <= ValueKind::conversionInstEnd;
    }

    [[nodiscard]] Type* into() const noexcept {
      return type();
    }

    [[nodiscard]] Type* fromTy() const noexcept {
      return from()->type();
    }

    [[nodiscard]] const Value* from() const noexcept {
      return this->operands()[0];
    }

    [[nodiscard]] Value* from() noexcept {
      return this->operands()[0];
    }

  protected:
    template <typename T> ConversionInst(T* ptr, Type* ty, Value* lhs) noexcept : Instruction(ptr, ty, {lhs}) {}
  };

  class DAWN_PUBLIC Phi final : public Instruction {
  public:
    inline static constexpr ValueKind kind = ValueKind::phiInst;

    explicit Phi(Type* ty) noexcept : Instruction(this, ty, {}) {}

    explicit Phi(Type* ty, std::initializer_list<std::pair<BasicBlock*, Value*>> incoming) noexcept : Phi(ty) {
      for (const auto& [bb, val] : incoming) {
        addIncomingBlock(bb, val);
      }
    }

    void addIncomingBlock(BasicBlock* from, Value* value_from) noexcept {
      addOperand(value_from);
      incoming_.push_back(from);

      // need to make sure that two phis with the same incoming/value pairs but
      // different insertion orders are equivalent. to maintain this, we sort after each insertion
      //
      // we can't just sort both arrays directly though, they need to be sorted the same way.
      absl::InlinedVector<std::pair<BasicBlock*, Value*>, 16> pairs;
      pairs.reserve(incoming_.size());

      for (auto i = std::size_t{0}; i < incoming_.size(); ++i) {
        pairs.emplace_back(incoming_[i], operands()[i]);
      }

      std::sort(pairs.begin(), pairs.end());

      for (auto i = std::size_t{0}; i < pairs.size(); ++i) {
        auto [bb, val] = pairs[i];

        incoming_[i] = bb;
        operandsRaw()[i] = val;
      }
    }

    void replaceBlockRef(const BasicBlock* old_target, ReplaceWith<BasicBlock*> new_target) noexcept {
      std::replace(incoming_.begin(), incoming_.end(), const_cast<BasicBlock*>(old_target), new_target.value);
    }

    [[nodiscard]] std::vector<std::pair<const BasicBlock*, const Value*>> incoming() const noexcept {
      std::vector<std::pair<const BasicBlock*, const Value*>> out;

      const auto* bbs = incoming_.begin();
      auto vals = operands().begin();

      for (; bbs != incoming_.end(); ++bbs, ++vals) {
        out.emplace_back(*bbs, *vals);
      }

      return out;
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* other) const noexcept final;

  private:
    absl::InlinedVector<BasicBlock*, 3> incoming_;
  };

  class DAWN_PUBLIC Call final : public Instruction {
  public:
    inline constexpr static ValueKind kind = ValueKind::callInst;

    explicit Call(Function* target, std::span<Value* const> args)
        : Instruction(this, target->returnTy(), args),
          target_{target} {
      auto targetArgs = target_->args();

      DAWN_ASSERT(targetArgs.size() == args.size(), "must have right number of arguments for call");

      for (auto i = std::size_t{0}; i < args.size(); ++i) {
        DAWN_ASSERT(args[i]->type() == targetArgs[i].type(), "argument must be the right type");
      }
    }

    [[nodiscard]] Function* target() const noexcept {
      return target_;
    }

    [[nodiscard]] std::span<const Value* const> args() const noexcept {
      return operands();
    }

    [[nodiscard]] std::span<Value* const> args() noexcept {
      return operands();
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;

  private:
    Function* target_;
  };

  class DAWN_PUBLIC ICmp final : public BinaryInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::icmpInst;

    explicit ICmp(Module* mod, ICmpOrdering order, Value* lhs, Value* rhs) noexcept
        : BinaryInst(this, mod->boolean(), lhs, rhs),
          order_{order} {
      DAWN_ASSERT(lhs->type() == rhs->type(), "`lhs` and `rhs` for `icmp` must have the same type");
      DAWN_ASSERT(isa<Int>(lhs->type()) || isa<Bool>(lhs->type()), "`icmp` operands must be integers or booleans!");
    }

    [[nodiscard]] ICmpOrdering order() const noexcept {
      return order_;
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;

  private:
    ICmpOrdering order_;
  };

  class DAWN_PUBLIC FCmp final : public BinaryInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::fcmpInst;

    explicit FCmp(Module* mod, FCmpOrdering order, Value* lhs, Value* rhs) noexcept
        : BinaryInst(this, mod->boolean(), lhs, rhs),
          order_{order} {
      DAWN_ASSERT(lhs->type() == rhs->type(), "`lhs` and `rhs` for `fcmp` must have the same type");
      DAWN_ASSERT(isa<Float>(lhs->type()), "`fcmp` operands must have floating-point arguments!");
    }

    [[nodiscard]] FCmpOrdering order() const noexcept {
      return order_;
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;

  private:
    FCmpOrdering order_;
  };

  class DAWN_PUBLIC Sel final : public Instruction {
  public:
    inline constexpr static ValueKind kind = ValueKind::selInst;

    explicit Sel(Value* cond, If lhs, Else rhs) noexcept
        : Instruction(this, lhs.val->type(), {cond, lhs.val, rhs.val}) {
      DAWN_ASSERT(isa<Bool>(cond->type()), "`sel` condition must be boolean!");
      DAWN_ASSERT(lhs.val->type() == rhs.val->type(), "`lhs` and `rhs` for `sel` must have the same type");
    }

    [[nodiscard]] const Value* cond() const noexcept {
      return operands()[0];
    }

    [[nodiscard]] Value* cond() noexcept {
      return operands()[0];
    }

    [[nodiscard]] const Value* ifTrue() const noexcept {
      return operands()[1];
    }

    [[nodiscard]] Value* ifTrue() noexcept {
      return operands()[1];
    }

    [[nodiscard]] const Value* ifFalse() const noexcept {
      return operands()[2];
    }

    [[nodiscard]] Value* ifFalse() noexcept {
      return operands()[2];
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC Br final : public TerminatorInst {
  public:
    inline static constexpr ValueKind kind = ValueKind::brInst;

    explicit Br(BasicBlock* target) noexcept : TerminatorInst(this, target, {target}, {}) {}

    [[nodiscard]] BasicBlock* target() const noexcept {
      return this->refs()[0];
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* other) const noexcept final;
  };

  class DAWN_PUBLIC CondBr final : public TerminatorInst {
  public:
    inline static constexpr ValueKind kind = ValueKind::cbrInst;

    explicit CondBr(Value* cond, TrueBranch true_branch, FalseBranch false_branch) noexcept
        : TerminatorInst(this, true_branch.target, {true_branch.target, false_branch.target}, {cond}) {}

    [[nodiscard]] const Value* cond() const noexcept {
      return this->operands()[0];
    }

    [[nodiscard]] Value* cond() noexcept {
      return this->operands()[0];
    }

    [[nodiscard]] const BasicBlock* trueBranch() const noexcept {
      return this->refs()[0];
    }

    [[nodiscard]] BasicBlock* trueBranch() noexcept {
      return this->refs()[0];
    }

    [[nodiscard]] const BasicBlock* falseBranch() const noexcept {
      return this->refs()[1];
    }

    [[nodiscard]] BasicBlock* falseBranch() noexcept {
      return this->refs()[1];
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC Ret final : public TerminatorInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::retInst;

    explicit Ret(BasicBlock* parent, Value* return_value) : TerminatorInst(this, parent, {}, {return_value}) {}

    explicit Ret(BasicBlock* parent) : TerminatorInst(this, parent, {}, {}) {}

    [[nodiscard]] OptionalPtr<const Value> returnValue() const noexcept {
      if (operands().empty()) {
        return dawn::none<const Value>();
      }

      return dawn::some(operands()[0]);
    }

    [[nodiscard]] Type* returnTy() const noexcept {
      if (operands().empty()) {
        return type(); // always `void`
      }

      return operands()[0]->type();
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC Unreachable final : public TerminatorInst {
  public:
    inline static constexpr ValueKind kind = ValueKind::unreachableInst;

    explicit Unreachable(BasicBlock* any) noexcept : TerminatorInst(this, any, {}, {}) {}

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC Alloca final : public Instruction {
  public:
    inline static constexpr ValueKind kind = ValueKind::allocaInst;

    explicit Alloca(class Module* mod, Type* toAlloc, Value* numObjects) noexcept
        : Instruction(this, mod->ptr(), {numObjects}),
          ty_{toAlloc} {
      DAWN_ASSERT(isa<Int>(numObjects->type()), "'alloca' number of objects must be integer");
    }

    [[nodiscard]] const Value* numberOfObjects() const noexcept {
      return operands()[0];
    }

    [[nodiscard]] Value* numberOfObjects() noexcept {
      return operands()[0];
    }

    [[nodiscard]] Type* allocatingTy() const noexcept {
      return ty_;
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;

  private:
    Type* ty_;
  };

  class DAWN_PUBLIC Load final : public Instruction {
  public:
    inline static constexpr ValueKind kind = ValueKind::loadInst;

    explicit Load(Type* ty, Value* target, bool isVolatile) noexcept
        : Instruction(this, ty, {target}),
          isVolatile_{isVolatile} {
      DAWN_ASSERT(isa<Ptr>(target->type()), "cannot load from non-`ptr` value");
    }

    [[nodiscard]] const Value* target() const noexcept {
      return this->operands()[0];
    }

    [[nodiscard]] Value* target() noexcept {
      return this->operands()[0];
    }

    [[nodiscard]] bool isVolatile() const noexcept {
      return isVolatile_;
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;

  private:
    bool isVolatile_;
  };

  class DAWN_PUBLIC Store final : public Instruction {
  public:
    inline static constexpr ValueKind kind = ValueKind::storeInst;

    explicit Store(BasicBlock* parent, Value* value, Value* target, bool isVolatile) noexcept
        : Instruction(this, parent->parent()->parent()->voidType(), {value, target}),
          isVolatile_{isVolatile} {
      DAWN_ASSERT(isa<Ptr>(target->type()), "cannot store to non-`ptr` value");
    }

    [[nodiscard]] const Value* value() const noexcept {
      return this->operands()[0];
    }

    [[nodiscard]] Value* value() noexcept {
      return this->operands()[0];
    }

    [[nodiscard]] Value* target() noexcept {
      return this->operands()[1];
    }

    [[nodiscard]] const Value* target() const noexcept {
      return this->operands()[1];
    }

    [[nodiscard]] bool isVolatile() const noexcept {
      return isVolatile_;
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;

  private:
    bool isVolatile_;
  };

  class DAWN_PUBLIC Offset final : public Instruction {
  public:
    inline constexpr static ValueKind kind = ValueKind::offsetInst;

    explicit Offset(Type* ty, Value* ptr, Value* offset) noexcept
        : Instruction(this, ptr->type(), {ptr, offset}),
          offsetTy_{ty} {
      DAWN_ASSERT(isa<Int>(offset->type()), "`index` second operand must be an integer!");
      DAWN_ASSERT(isa<Ptr>(ptr->type()), "`index` first operand must be a pointer!");
    }

    [[nodiscard]] Type* offsetTy() const noexcept {
      return offsetTy_;
    }

    [[nodiscard]] Value* ptr() noexcept {
      return this->operands()[0];
    }

    [[nodiscard]] const Value* ptr() const noexcept {
      return this->operands()[0];
    }

    [[nodiscard]] Value* offset() noexcept {
      return this->operands()[1];
    }

    [[nodiscard]] const Value* offset() const noexcept {
      return this->operands()[1];
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;

  private:
    Type* offsetTy_;
  };

  namespace internal {
    // NOLINTNEXTLINE
    [[nodiscard]] inline Type* aggregateExtractTy(Type* aggTy, Value* offset) noexcept {
      DAWN_ASSERT(isa<Int>(offset->type()), "aggregate instruction index must be an integer");

      if (auto arrTy = dyncast<Array>(aggTy)) {
        return arrTy->element();
      }

      DAWN_ASSERT(isa<Struct>(aggTy), "aggregate instruction only operates on arrays or structures");
      DAWN_ASSERT(isa<ConstantInt>(offset), "index must be a constant integer when given a structure");

      auto index = dyncastUnchecked<ConstantInt>(offset)->realValue();
      auto* structTy = dyncastUnchecked<Struct>(aggTy);

      return structTy->fields()[index];
    }

    // NOLINTNEXTLINE
    [[nodiscard]] inline bool checkWithinBounds(Value* aggregate, Value* offset) noexcept {
      if (auto structTy = dyncast<Struct>(aggregate->type())) {
        auto* index = dyncastUnchecked<ConstantInt>(offset);

        return index->realValue() <= structTy->fields().size();
      }

      if (auto arrayTy = dyncast<Array>(aggregate->type())) {
        if (auto index = dyncast<ConstantInt>(offset)) {
          return index->realValue() <= arrayTy->len();
        }
      }

      return true;
    }
  } // namespace internal

  class DAWN_PUBLIC Extract final : public Instruction {
  public:
    inline constexpr static ValueKind kind = ValueKind::extractInst;

    explicit Extract(dawn::Agg aggregate, dawn::Value* index) noexcept
        : Instruction(this, internal::aggregateExtractTy(aggregate.val->type(), index), {aggregate.val, index}) {
      DAWN_ASSERT(internal::checkWithinBounds(aggregate.val, index), "'extract' index must be within bounds");
    }

    [[nodiscard]] Value* aggregate() noexcept {
      return operands()[0];
    }

    [[nodiscard]] const Value* aggregate() const noexcept {
      return operands()[0];
    }

    [[nodiscard]] Value* index() noexcept {
      return operands()[1];
    }

    [[nodiscard]] const Value* index() const noexcept {
      return operands()[1];
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC Insert final : public Instruction {
  public:
    inline constexpr static ValueKind kind = ValueKind::insertInst;

    explicit Insert(dawn::Agg aggregate, dawn::Index index, Value* value) noexcept
        : Instruction(this, aggregate.val->type(), {aggregate.val, index.val, value}) {
      DAWN_ASSERT(internal::checkWithinBounds(aggregate.val, index.val), "'insert' index must be within bounds");
    }

    [[nodiscard]] Value* aggregate() noexcept {
      return operands()[0];
    }

    [[nodiscard]] const Value* aggregate() const noexcept {
      return operands()[0];
    }

    [[nodiscard]] Value* index() noexcept {
      return operands()[1];
    }

    [[nodiscard]] const Value* index() const noexcept {
      return operands()[1];
    }

    [[nodiscard]] Value* value() noexcept {
      return operands()[2];
    }

    [[nodiscard]] const Value* value() const noexcept {
      return operands()[2];
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC ElemPtr final : public Instruction {
  public:
    inline constexpr static ValueKind kind = ValueKind::elemptrInst;

    explicit ElemPtr(Type* aggTy, Value* ptr, dawn::Index index) noexcept
        : Instruction(this, internal::aggregateExtractTy(aggTy, index.val), {ptr, index.val}),
          aggTy_{aggTy} {}

    [[nodiscard]] Type* aggregateTy() const noexcept {
      return aggTy_;
    }

    [[nodiscard]] Value* ptr() noexcept {
      return operands()[0];
    }

    [[nodiscard]] const Value* ptr() const noexcept {
      return operands()[0];
    }

    [[nodiscard]] Value* index() noexcept {
      return operands()[1];
    }

    [[nodiscard]] const Value* index() const noexcept {
      return operands()[1];
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;

  private:
    Type* aggTy_;
  };

  class DAWN_PUBLIC Sext final : public ConversionInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::sextInst;

    explicit Sext(Type* into, Value* from) noexcept : ConversionInst(this, into, from) {
      DAWN_ASSERT(isa<Int>(into), "`sext` result type must be integral!");
      DAWN_ASSERT(isa<Int>(from->type()), "`sext` operand must be an integer!");
      DAWN_ASSERT(dyncastUnchecked<Int>(from->type())->rawWidth() < dyncastUnchecked<Int>(into)->rawWidth(),
          "`sext` must increase width, cannot shrink or leave the same");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC Zext final : public ConversionInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::zextInst;

    explicit Zext(Type* into, Value* from) noexcept : ConversionInst(this, into, from) {
      DAWN_ASSERT(isa<Int>(into), "`zext` result type must be integral!");
      DAWN_ASSERT(isa<Int>(from->type()), "`zext` operand must be an integer!");
      DAWN_ASSERT(dyncastUnchecked<Int>(from->type())->rawWidth() < dyncastUnchecked<Int>(into)->rawWidth(),
          "`zext` must increase width, cannot shrink or leave the same");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC Trunc final : public ConversionInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::truncInst;

    explicit Trunc(Type* into, Value* from) noexcept : ConversionInst(this, into, from) {
      DAWN_ASSERT(isa<Int>(into), "`trunc` result type must be integral!");
      DAWN_ASSERT(isa<Int>(from->type()), "`trunc` operand must be an integer!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC PToI final : public ConversionInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::ptoiInst;

    explicit PToI(Type* into, Value* from) noexcept : ConversionInst(this, into, from) {
      DAWN_ASSERT(isa<Int>(into), "`ptoi` result type must be integral!");
      DAWN_ASSERT(isa<Ptr>(from->type()), "`ptoi` operand must be a pointer!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC IToP final : public ConversionInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::itopInst;

    explicit IToP(class Module* mod, Value* from) noexcept : ConversionInst(this, mod->ptr(), from) {
      DAWN_ASSERT(isa<Int>(from->type()), "`itop` operand must be an integer!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC BToI final : public ConversionInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::btoiInst;

    explicit BToI(Type* into, Value* from) noexcept : ConversionInst(this, into, from) {
      DAWN_ASSERT(isa<Bool>(from->type()), "`btoi` operand must be a boolean!");
      DAWN_ASSERT(isa<Int>(into), "`itop` result type must be an integer!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC IToB final : public ConversionInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::itobInst;

    explicit IToB(class Module* mod, Value* from) noexcept : ConversionInst(this, mod->boolean(), from) {
      DAWN_ASSERT(isa<Int>(from->type()), "`itob` operand must be an integer!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC FToSI final : public ConversionInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::ftosiInst;

    explicit FToSI(Type* into, Value* from) noexcept : ConversionInst(this, into, from) {
      DAWN_ASSERT(isa<Float>(from->type()), "`ftosi` operand must be a float!");
      DAWN_ASSERT(isa<Int>(into), "`ftosi` result type must be an integer!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC FToUI final : public ConversionInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::ftouiInst;

    explicit FToUI(Type* into, Value* from) noexcept : ConversionInst(this, into, from) {
      DAWN_ASSERT(isa<Float>(from->type()), "`ftoui` operand must be a float!");
      DAWN_ASSERT(isa<Int>(into), "`ftoui` result type must be an integer!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC SIToF final : public ConversionInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::sitofInst;

    explicit SIToF(Type* into, Value* from) noexcept : ConversionInst(this, into, from) {
      DAWN_ASSERT(isa<Int>(from->type()), "`sitof` operand must be an integer!");
      DAWN_ASSERT(isa<Float>(into), "`sitof` type must be a floating-point type!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC UIToF final : public ConversionInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::uitofInst;

    explicit UIToF(Type* into, Value* from) noexcept : ConversionInst(this, into, from) {
      DAWN_ASSERT(isa<Int>(from->type()), "`uitof` operand must be an integer!");
      DAWN_ASSERT(isa<Float>(into), "`uitof` type must be a floating-point type!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC And final : public BinaryInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::andInst;

    explicit And(Value* lhs, Value* rhs) noexcept : BinaryInst(this, lhs->type(), lhs, rhs) {
      DAWN_ASSERT(lhs->type() == rhs->type(), "`lhs` and `rhs` for `and` must have the same type");
      DAWN_ASSERT(isa<Int>(lhs->type()) || isa<Bool>(lhs->type()), "`and` operands must be integers or booleans!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC Or final : public BinaryInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::orInst;

    explicit Or(Value* lhs, Value* rhs) noexcept : BinaryInst(this, lhs->type(), lhs, rhs) {
      DAWN_ASSERT(lhs->type() == rhs->type(), "`lhs` and `rhs` for `or` must have the same type");
      DAWN_ASSERT(isa<Int>(lhs->type()) || isa<Bool>(lhs->type()), "`or` operands must be integers or booleans!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC Xor final : public BinaryInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::xorInst;

    explicit Xor(Value* lhs, Value* rhs) noexcept : BinaryInst(this, lhs->type(), lhs, rhs) {
      DAWN_ASSERT(lhs->type() == rhs->type(), "`lhs` and `rhs` for `xor` must have the same type");
      DAWN_ASSERT(isa<Int>(lhs->type()) || isa<Bool>(lhs->type()), "`xor` operands must be integers or booleans!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC Shl final : public BinaryInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::shlInst;

    explicit Shl(Value* lhs, Value* rhs) noexcept : BinaryInst(this, lhs->type(), lhs, rhs) {
      DAWN_ASSERT(lhs->type() == rhs->type(), "`lhs` and `rhs` for `shl` must have the same type");
      DAWN_ASSERT(isa<Int>(lhs->type()), "`shl` operands must be integral!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC LShr final : public BinaryInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::lshrInst;

    explicit LShr(Value* lhs, Value* rhs) noexcept : BinaryInst(this, lhs->type(), lhs, rhs) {
      DAWN_ASSERT(lhs->type() == rhs->type(), "`lhs` and `rhs` for `lshr` must have the same type");
      DAWN_ASSERT(isa<Int>(lhs->type()), "`lshr` operands must be integral!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC AShr final : public BinaryInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::ashrInst;

    explicit AShr(Value* lhs, Value* rhs) noexcept : BinaryInst(this, lhs->type(), lhs, rhs) {
      DAWN_ASSERT(lhs->type() == rhs->type(), "`lhs` and `rhs` for `ashr` must have the same type");
      DAWN_ASSERT(isa<Int>(lhs->type()), "`ashr` operands must be integral!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC IAdd final : public BinaryInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::iaddInst;

    explicit IAdd(Value* lhs, Value* rhs) noexcept : BinaryInst(this, lhs->type(), lhs, rhs) {
      DAWN_ASSERT(lhs->type() == rhs->type(), "`lhs` and `rhs` for `iadd` must have the same type");
      DAWN_ASSERT(isa<Int>(lhs->type()), "`iadd` operands must be integral!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC ISub final : public BinaryInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::isubInst;

    explicit ISub(Value* lhs, Value* rhs) noexcept : BinaryInst(this, lhs->type(), lhs, rhs) {
      DAWN_ASSERT(lhs->type() == rhs->type(), "`lhs` and `rhs` for `isub` must have the same type");
      DAWN_ASSERT(isa<Int>(lhs->type()), "`isub` operands must be integral!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC IMul final : public BinaryInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::imulInst;

    explicit IMul(Value* lhs, Value* rhs) noexcept : BinaryInst(this, lhs->type(), lhs, rhs) {
      DAWN_ASSERT(lhs->type() == rhs->type(), "`lhs` and `rhs` for `imul` must have the same type");
      DAWN_ASSERT(isa<Int>(lhs->type()), "`imul` operands must be integral!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC UDiv final : public BinaryInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::udivInst;

    explicit UDiv(Value* lhs, Value* rhs) noexcept : BinaryInst(this, lhs->type(), lhs, rhs) {
      DAWN_ASSERT(lhs->type() == rhs->type(), "`lhs` and `rhs` for `udiv` must have the same type");
      DAWN_ASSERT(isa<Int>(lhs->type()), "`udiv` operands must be integral!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC SDiv final : public BinaryInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::sdivInst;

    explicit SDiv(Value* lhs, Value* rhs) noexcept : BinaryInst(this, lhs->type(), lhs, rhs) {
      DAWN_ASSERT(lhs->type() == rhs->type(), "`lhs` and `rhs` for `sdiv` must have the same type");
      DAWN_ASSERT(isa<Int>(lhs->type()), "`sdiv` operands must be integral!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC URem final : public BinaryInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::uremInst;

    explicit URem(Value* lhs, Value* rhs) noexcept : BinaryInst(this, lhs->type(), lhs, rhs) {
      DAWN_ASSERT(lhs->type() == rhs->type(), "`lhs` and `rhs` for `urem` must have the same type");
      DAWN_ASSERT(isa<Int>(lhs->type()), "`urem` operands must be integral!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC SRem final : public BinaryInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::sremInst;

    explicit SRem(Value* lhs, Value* rhs) noexcept : BinaryInst(this, lhs->type(), lhs, rhs) {
      DAWN_ASSERT(lhs->type() == rhs->type(), "`lhs` and `rhs` for `srem` must have the same type");
      DAWN_ASSERT(isa<Int>(lhs->type()), "`srem` operands must be integral!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC FNeg final : public BinaryInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::fnegInst;

    explicit FNeg(Value* lhs, Value* rhs) noexcept : BinaryInst(this, lhs->type(), lhs, rhs) {
      DAWN_ASSERT(lhs->type() == rhs->type(), "`lhs` and `rhs` for `fneg` must have the same type");
      DAWN_ASSERT(isa<Float>(lhs->type()), "`fneg` operands must be floating-point!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC FAdd final : public BinaryInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::faddInst;

    explicit FAdd(Value* lhs, Value* rhs) noexcept : BinaryInst(this, lhs->type(), lhs, rhs) {
      DAWN_ASSERT(lhs->type() == rhs->type(), "`lhs` and `rhs` for `fadd` must have the same type");
      DAWN_ASSERT(isa<Float>(lhs->type()), "`fadd` operands must be floating-point!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC FSub final : public BinaryInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::fsubInst;

    explicit FSub(Value* lhs, Value* rhs) noexcept : BinaryInst(this, lhs->type(), lhs, rhs) {
      DAWN_ASSERT(lhs->type() == rhs->type(), "`lhs` and `rhs` for `fsub` must have the same type");
      DAWN_ASSERT(isa<Float>(lhs->type()), "`fsub` operands must be floating-point!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC FMul final : public BinaryInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::fmulInst;

    explicit FMul(Value* lhs, Value* rhs) noexcept : BinaryInst(this, lhs->type(), lhs, rhs) {
      DAWN_ASSERT(lhs->type() == rhs->type(), "`lhs` and `rhs` for `fmul` must have the same type");
      DAWN_ASSERT(isa<Float>(lhs->type()), "`fmul` operands must be floating-point!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC FDiv final : public BinaryInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::fdivInst;

    explicit FDiv(Value* lhs, Value* rhs) noexcept : BinaryInst(this, lhs->type(), lhs, rhs) {
      DAWN_ASSERT(lhs->type() == rhs->type(), "`lhs` and `rhs` for `fdiv` must have the same type");
      DAWN_ASSERT(isa<Float>(lhs->type()), "`fdiv` operands must be floating-point!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC FRem final : public BinaryInst {
  public:
    inline constexpr static ValueKind kind = ValueKind::fremInst;

    explicit FRem(Value* lhs, Value* rhs) noexcept : BinaryInst(this, lhs->type(), lhs, rhs) {
      DAWN_ASSERT(lhs->type() == rhs->type(), "`lhs` and `rhs` for `frem` must have the same type");
      DAWN_ASSERT(isa<Float>(lhs->type()), "`frem` operands must be floating-point!");
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

#define DAWN_FOR_EACH_CONVERSION_INST(MACRO)                                                                           \
  MACRO(Sext);                                                                                                         \
  MACRO(Zext);                                                                                                         \
  MACRO(Trunc);                                                                                                        \
  MACRO(IToB);                                                                                                         \
  MACRO(BToI);                                                                                                         \
  MACRO(IToP);                                                                                                         \
  MACRO(PToI);                                                                                                         \
  MACRO(SIToF);                                                                                                        \
  MACRO(UIToF);                                                                                                        \
  MACRO(FToSI);                                                                                                        \
  MACRO(FToUI)

#define DAWN_FOR_EACH_BINARY_INST(MACRO)                                                                               \
  MACRO(And);                                                                                                          \
  MACRO(Or);                                                                                                           \
  MACRO(Xor);                                                                                                          \
  MACRO(Shl);                                                                                                          \
  MACRO(LShr);                                                                                                         \
  MACRO(AShr);                                                                                                         \
  MACRO(IAdd);                                                                                                         \
  MACRO(ISub);                                                                                                         \
  MACRO(IMul);                                                                                                         \
  MACRO(UDiv);                                                                                                         \
  MACRO(SDiv);                                                                                                         \
  MACRO(URem);                                                                                                         \
  MACRO(SRem);                                                                                                         \
  MACRO(FNeg);                                                                                                         \
  MACRO(FAdd);                                                                                                         \
  MACRO(FSub);                                                                                                         \
  MACRO(FMul);                                                                                                         \
  MACRO(FDiv);                                                                                                         \
  MACRO(FRem)

#define DAWN_FOR_EACH_INST(MACRO)                                                                                      \
  MACRO(Phi);                                                                                                          \
  MACRO(Call);                                                                                                         \
  MACRO(ICmp);                                                                                                         \
  MACRO(FCmp);                                                                                                         \
  MACRO(Sel);                                                                                                          \
  MACRO(Br);                                                                                                           \
  MACRO(CondBr);                                                                                                       \
  MACRO(Ret);                                                                                                          \
  MACRO(Unreachable);                                                                                                  \
  MACRO(Alloca);                                                                                                       \
  MACRO(Load);                                                                                                         \
  MACRO(Store);                                                                                                        \
  MACRO(Extract);                                                                                                      \
  MACRO(Insert);                                                                                                       \
  MACRO(ElemPtr);                                                                                                      \
  MACRO(Store);                                                                                                        \
  DAWN_FOR_EACH_CONVERSION_INST(MACRO);                                                                                \
  DAWN_FOR_EACH_BINARY_INST(MACRO)
} // namespace dawn
