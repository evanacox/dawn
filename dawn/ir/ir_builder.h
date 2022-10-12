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

#include "../utility/bump_alloc.h"
#include "../utility/strong_types.h"
#include "./basic_block.h"
#include "./constant.h"
#include "./function.h"
#include "./instruction.h"
#include "./instructions.h"
#include "./module.h"

namespace dawn {
  class IRBuilder {
  public:
    [[nodiscard]] explicit IRBuilder(Module* mod) noexcept;

    [[nodiscard]] OptionalPtr<BasicBlock> currentBlock() noexcept {
      return currBlock_;
    }

    Function* createFunc(std::string name) noexcept {
      return createFunc(std::move(name), voidTy());
    }

    Function* createFunc(std::string name, Type* ty) noexcept {
      return createFunc(std::move(name), ty, {});
    }

    Function* createFunc(std::string name, Type* ty, std::span<Type* const> args) noexcept {
      absl::InlinedVector<Argument, 4> realArgs;

      for (auto i = std::size_t{0}; i < args.size(); ++i) {
        realArgs.emplace_back(args[i], i);
      }

      return mod_->createFunc(std::move(name), ty, realArgs);
    }

    Function* createFunc(std::string name, Type* ty, std::initializer_list<Type*> args) noexcept {
      absl::InlinedVector<Argument, 4> realArgs;

      for (auto i = std::size_t{0}; i < args.size(); ++i) {
        realArgs.emplace_back(*(args.begin() + i), i);
      }

      return mod_->createFunc(std::move(name), ty, realArgs);
    }

    [[nodiscard]] Function* createOrGetFunc(std::string name, Type* ty, std::span<Type* const> args) noexcept {
      absl::InlinedVector<Argument, 4> realArgs;

      for (auto i = std::size_t{0}; i < args.size(); ++i) {
        realArgs.emplace_back(args[i], i);
      }

      return mod_->findOrCreateFunc(std::move(name), ty, realArgs);
    }

    [[nodiscard]] OptionalPtr<Function> findFn(std::string_view name) noexcept {
      return mod_->findFunc(name);
    }

    BasicBlock* createBlock() noexcept {
      DAWN_ASSERT(curr_fn_, "must have a function to put the block in. use `setInsertFn`");

      return curr_fn_->addBlock(BasicBlock{curr_fn_.get()});
    }

    BasicBlock* createBlock(Function* fn) noexcept {
      curr_fn_ = dawn::some(fn);

      return createBlock();
    }

    void setInsertBlock(BasicBlock* block) noexcept {
      currBlock_ = dawn::some(block);
    }

    void clearInsertBlock() noexcept {
      currBlock_ = dawn::none<BasicBlock>();
    }

    void setInsertFn(Function* fn) noexcept {
      currFn_ = dawn::some(fn);
    }

    void clearInsertFn() noexcept {
      currFn_ = dawn::none<Function>();
    }

    void setInsertPoint(BasicBlock* bb) noexcept {
      currFn_ = dawn::some(bb->parent());
      currBlock_ = dawn::some(bb);
    }

    [[nodiscard]] Type* i8Ty() const noexcept {
      return mod_->i8();
    }

    [[nodiscard]] Type* i16Ty() const noexcept {
      return mod_->i16();
    }

    [[nodiscard]] Type* i32Ty() const noexcept {
      return mod_->i32();
    }

    [[nodiscard]] Type* i64Ty() const noexcept {
      return mod_->i64();
    }

    [[nodiscard]] Type* boolTy() const noexcept {
      return mod_->boolean();
    }

    [[nodiscard]] Type* ptrTy() const noexcept {
      return mod_->ptr();
    }

    [[nodiscard]] Type* f32Ty() const noexcept {
      return mod_->f32();
    }

    [[nodiscard]] Type* f64Ty() const noexcept {
      return mod_->f64();
    }

    [[nodiscard]] Type* emptyStructTy() const noexcept {
      return mod_->emptyStruct();
    }

    [[nodiscard]] Type* intTy(std::uint64_t width) const noexcept {
      return mod_->intType(width);
    }

    [[nodiscard]] Type* floatTy(std::uint64_t width) const noexcept {
      return mod_->floatType(width);
    }

    [[nodiscard]] Type* arrayTy(Type* element, std::uint64_t length) noexcept {
      return mod_->array(element, length);
    }

    [[nodiscard]] Type* structTy(std::span<Type* const> fields) noexcept {
      return mod_->structure(fields);
    }

    [[nodiscard]] Type* structTy(std::initializer_list<Type*> fields) noexcept {
      return structTy(std::span{fields.begin(), fields.size()});
    }

    [[nodiscard]] Type* voidTy() const noexcept {
      return mod_->voidType();
    }

    [[nodiscard]] ConstantInt* constInt(APInt value, Type* ty) noexcept {
      return mod_->constants_.createOrGet<ConstantInt>(&mod_->pool_, value, ty);
    }

    [[nodiscard]] ConstantInt* constI8(std::uint8_t value) noexcept {
      return constInt(APInt(static_cast<std::uint64_t>(value), Width::i8), i8Ty());
    }

    [[nodiscard]] ConstantInt* constI16(std::uint16_t value) noexcept {
      return constInt(APInt(static_cast<std::uint64_t>(value), Width::i16), i16Ty());
    }

    [[nodiscard]] ConstantInt* constI32(std::uint32_t value) noexcept {
      return constInt(APInt(static_cast<std::uint64_t>(value), Width::i32), i32Ty());
    }

    [[nodiscard]] ConstantInt* constI64(std::uint64_t value) noexcept {
      return constInt(APInt(value, Width::i64), i64Ty());
    }

    [[nodiscard]] ConstantBool* constBool(bool value) noexcept {
      return mod_->constants_.createOrGet<ConstantBool>(&mod_->pool_, mod_, value);
    }

    [[nodiscard]] ConstantBool* constTrue() noexcept {
      return constBool(true);
    }

    [[nodiscard]] ConstantBool* constFalse() noexcept {
      return constBool(false);
    }

    [[nodiscard]] ConstantFloat* constFloat(double value, Float* ty) noexcept {
      return mod_->constants_.createOrGet<ConstantFloat>(&mod_->pool_, ty, value);
    }

    [[nodiscard]] ConstantFloat* constF32(double value) noexcept {
      return mod_->constants_.createOrGet<ConstantFloat>(&mod_->pool_, f32Ty(), value);
    }

    [[nodiscard]] ConstantFloat* constF64(double value) noexcept {
      return mod_->constants_.createOrGet<ConstantFloat>(&mod_->pool_, f64Ty(), value);
    }

    [[nodiscard]] ConstantNull* constNull() noexcept {
      return mod_->constants_.createOrGet<ConstantNull>(&mod_->pool_, mod_);
    }

    [[nodiscard]] ConstantUndef* constUndef(Type* ofTy) noexcept {
      return mod_->constants_.createOrGet<ConstantUndef>(&mod_->pool_, ofTy);
    }

    [[nodiscard]] ConstantArray* constArray(std::span<Constant* const> vals) noexcept {
      return mod_->constants_.createOrGet<ConstantArray>(&mod_->pool_, mod_, vals);
    }

    [[nodiscard]] ConstantArray* constArray(std::initializer_list<Constant*> vals) noexcept {
      return constArray({vals.begin(), vals.size()});
    }

    [[nodiscard]] ConstantArray* constArrayFill(Constant* val, std::size_t length) noexcept;

    [[nodiscard]] ConstantStruct* constStruct(std::span<Constant* const> vals) noexcept;

    [[nodiscard]] ConstantStruct* constStruct(std::initializer_list<Constant*> vals) noexcept {
      return constStruct({vals.begin(), vals.size()});
    }

    [[nodiscard]] ConstantString* constString(std::string content) noexcept {
      return mod_->constants_.createOrGet<ConstantString>(&mod_->pool_, mod_, std::move(content));
    }

    template <typename T, typename... Args> T* createRawInstruction(Args... args) noexcept {
      // if no current block exists, we simply don't create the instruction
      if (auto block = currentBlock()) {
        auto inst = pool_->alloc<T>(std::forward<Args>(args)...);
        auto inserted = insertInst(std::move(inst));

        return dawn::dyncastUnchecked<T>(inserted);
      }

      return nullptr;
    }

    Phi* createPhi(Type* ty) noexcept {
      return createRawInstruction<Phi>(ty);
    }

    Call* createCall(Function* fn, std::span<Value* const> args) noexcept {
      return createRawInstruction<Call>(fn, args);
    }

    Call* createCall(Function* fn, std::initializer_list<Value*> args) noexcept {
      return createCall(fn, std::span{args.begin(), args.size()});
    }

    ICmp* createICmp(ICmpOrdering order, Value* lhs, Value* rhs) noexcept {
      return createRawInstruction<ICmp>(mod_, order, lhs, rhs);
    }

    ICmp* createICmpEQ(Value* lhs, Value* rhs) noexcept {
      return createICmp(ICmpOrdering::eq, lhs, rhs);
    }

    ICmp* createICmpNE(Value* lhs, Value* rhs) noexcept {
      return createICmp(ICmpOrdering::ne, lhs, rhs);
    }

    ICmp* createICmpSLT(Value* lhs, Value* rhs) noexcept {
      return createICmp(ICmpOrdering::slt, lhs, rhs);
    }

    ICmp* createICmpSGT(Value* lhs, Value* rhs) noexcept {
      return createICmp(ICmpOrdering::sgt, lhs, rhs);
    }

    ICmp* createICmpSLE(Value* lhs, Value* rhs) noexcept {
      return createICmp(ICmpOrdering::sle, lhs, rhs);
    }

    ICmp* createICmpSGE(Value* lhs, Value* rhs) noexcept {
      return createICmp(ICmpOrdering::sge, lhs, rhs);
    }

    ICmp* createICmpULT(Value* lhs, Value* rhs) noexcept {
      return createICmp(ICmpOrdering::ult, lhs, rhs);
    }

    ICmp* createICmpUGT(Value* lhs, Value* rhs) noexcept {
      return createICmp(ICmpOrdering::ugt, lhs, rhs);
    }

    ICmp* createICmpULE(Value* lhs, Value* rhs) noexcept {
      return createICmp(ICmpOrdering::ule, lhs, rhs);
    }

    ICmp* createICmpUGE(Value* lhs, Value* rhs) noexcept {
      return createICmp(ICmpOrdering::uge, lhs, rhs);
    }

    FCmp* createFCmp(FCmpOrdering order, Value* lhs, Value* rhs) noexcept {
      return createRawInstruction<FCmp>(mod_, order, lhs, rhs);
    }

    FCmp* createFCmpORD(Value* lhs, Value* rhs) noexcept {
      return createFCmp(FCmpOrdering::ord, lhs, rhs);
    }

    FCmp* createFCmpUNO(Value* lhs, Value* rhs) noexcept {
      return createFCmp(FCmpOrdering::uno, lhs, rhs);
    }

    FCmp* createFCmpOEQ(Value* lhs, Value* rhs) noexcept {
      return createFCmp(FCmpOrdering::oeq, lhs, rhs);
    }

    FCmp* createFCmpONE(Value* lhs, Value* rhs) noexcept {
      return createFCmp(FCmpOrdering::one, lhs, rhs);
    }

    FCmp* createFCmpOGT(Value* lhs, Value* rhs) noexcept {
      return createFCmp(FCmpOrdering::ogt, lhs, rhs);
    }

    FCmp* createFCmpOLT(Value* lhs, Value* rhs) noexcept {
      return createFCmp(FCmpOrdering::olt, lhs, rhs);
    }

    FCmp* createFCmpOGE(Value* lhs, Value* rhs) noexcept {
      return createFCmp(FCmpOrdering::oge, lhs, rhs);
    }

    FCmp* createFCmpOLE(Value* lhs, Value* rhs) noexcept {
      return createFCmp(FCmpOrdering::ole, lhs, rhs);
    }

    FCmp* createFCmpUEQ(Value* lhs, Value* rhs) noexcept {
      return createFCmp(FCmpOrdering::ueq, lhs, rhs);
    }

    FCmp* createFCmpUNE(Value* lhs, Value* rhs) noexcept {
      return createFCmp(FCmpOrdering::une, lhs, rhs);
    }

    FCmp* createFCmpUGT(Value* lhs, Value* rhs) noexcept {
      return createFCmp(FCmpOrdering::ugt, lhs, rhs);
    }

    FCmp* createFCmpULT(Value* lhs, Value* rhs) noexcept {
      return createFCmp(FCmpOrdering::ult, lhs, rhs);
    }

    FCmp* createFCmpUGE(Value* lhs, Value* rhs) noexcept {
      return createFCmp(FCmpOrdering::uge, lhs, rhs);
    }

    FCmp* createFCmpULE(Value* lhs, Value* rhs) noexcept {
      return createFCmp(FCmpOrdering::ule, lhs, rhs);
    }

    Sel* createSel(Value* cond, If lhs, Else rhs) noexcept {
      return createRawInstruction<Sel>(cond, lhs, rhs);
    }

    Br* createBr(BasicBlock* target) noexcept {
      return createRawInstruction<Br>(target);
    }

    CondBr* createCbr(dawn::Value* cond, TrueBranch true_branch, FalseBranch false_branch) noexcept {
      return createRawInstruction<CondBr>(cond, true_branch, false_branch);
    }

    Ret* createRet(Value* val) noexcept {
      return createRawInstruction<Ret>(currBlock_.get(), val);
    }

    Ret* createRetVoid() noexcept {
      return createRawInstruction<Ret>(currBlock_.get());
    }

    Unreachable* createUnreachable() noexcept {
      return createRawInstruction<Unreachable>(currBlock_.get());
    }

    Alloca* createAlloca(Type* ty) noexcept {
      return createAlloca(ty, constI64(1));
    }

    Alloca* createAlloca(Type* ty, Value* num) noexcept {
      return createRawInstruction<Alloca>(mod_, ty, num);
    }

    Load* createLoad(Type* ty, Value* from) noexcept {
      return createLoad(ty, from, false);
    }

    Load* createLoad(Type* ty, Value* from, bool isVolatile) noexcept {
      return createRawInstruction<Load>(ty, from, isVolatile);
    }

    Load* createVolatileLoad(Type* ty, Value* from) noexcept {
      return createLoad(ty, from, true);
    }

    Store* createStore(Value* val, Dest dest) noexcept {
      return createStore(val, dest, false);
    }

    Store* createStore(Value* val, Dest dest, bool isVolatile) noexcept {
      return createRawInstruction<Store>(currentBlock().get(), val, dest.val, isVolatile);
    }

    Store* createVolatileStore(Value* val, Dest dest) noexcept {
      return createStore(val, dest, true);
    }

    Offset* createOffset(Type* ty, Value* ptr, Index index) noexcept {
      return createRawInstruction<Offset>(ty, ptr, index.val);
    }

    Extract* createExtract(Agg aggregate, Value* index) noexcept {
      return createRawInstruction<Extract>(aggregate, index);
    }

    Insert* createInsert(Agg aggregate, Index index, Value* val) noexcept {
      return createRawInstruction<Insert>(aggregate, index, val);
    }

    ElemPtr* createElemPtr(Type* aggTy, Value* ptr, Index index) noexcept {
      return createRawInstruction<ElemPtr>(aggTy, ptr, index);
    }

    Sext* createSext(Type* into, Value* from) noexcept {
      return createRawInstruction<Sext>(into, from);
    }

    Zext* createZext(Type* into, Value* from) noexcept {
      return createRawInstruction<Zext>(into, from);
    }

    Trunc* createTrunc(Type* into, Value* from) noexcept {
      return createRawInstruction<Trunc>(into, from);
    }

    IToB* createIntToBool(Value* from) noexcept {
      return createRawInstruction<IToB>(mod_, from);
    }

    BToI* createBoolToInt(Type* into, Value* from) noexcept {
      return createRawInstruction<BToI>(into, from);
    }

    IToP* createIntToPtr(Value* from) noexcept {
      return createRawInstruction<IToP>(mod_, from);
    }

    PToI* createPtrToInt(Type* into, Value* from) noexcept {
      return createRawInstruction<PToI>(into, from);
    }

    SIToF* createSIntToFloat(Type* into, Value* from) noexcept {
      return createRawInstruction<SIToF>(into, from);
    }

    UIToF* createUIntToFloat(Type* into, Value* from) noexcept {
      return createRawInstruction<UIToF>(into, from);
    }

    FToSI* createFloatToSInt(Type* into, Value* from) noexcept {
      return createRawInstruction<FToSI>(into, from);
    }

    FToUI* createFloatToUInt(Type* into, Value* from) noexcept {
      return createRawInstruction<FToUI>(into, from);
    }

    And* createAnd(Value* lhs, Value* rhs) noexcept {
      return createRawInstruction<And>(lhs, rhs);
    }

    Or* createOr(Value* lhs, Value* rhs) noexcept {
      return createRawInstruction<Or>(lhs, rhs);
    }

    Xor* createXor(Value* lhs, Value* rhs) noexcept {
      return createRawInstruction<Xor>(lhs, rhs);
    }

    Shl* createShl(Value* lhs, Value* rhs) noexcept {
      return createRawInstruction<Shl>(lhs, rhs);
    }

    LShr* createLShr(Value* lhs, Value* rhs) noexcept {
      return createRawInstruction<LShr>(lhs, rhs);
    }

    AShr* createAShr(Value* lhs, Value* rhs) noexcept {
      return createRawInstruction<AShr>(lhs, rhs);
    }

    IAdd* createIAdd(Value* lhs, Value* rhs) noexcept {
      return createRawInstruction<IAdd>(lhs, rhs);
    }

    ISub* createISub(Value* lhs, Value* rhs) noexcept {
      return createRawInstruction<ISub>(lhs, rhs);
    }

    IMul* createIMul(Value* lhs, Value* rhs) noexcept {
      return createRawInstruction<IMul>(lhs, rhs);
    }

    UDiv* createUDiv(Value* lhs, Value* rhs) noexcept {
      return createRawInstruction<UDiv>(lhs, rhs);
    }

    SDiv* createSDiv(Value* lhs, Value* rhs) noexcept {
      return createRawInstruction<SDiv>(lhs, rhs);
    }

    URem* createURem(Value* lhs, Value* rhs) noexcept {
      return createRawInstruction<URem>(lhs, rhs);
    }

    SRem* createSRem(Value* lhs, Value* rhs) noexcept {
      return createRawInstruction<SRem>(lhs, rhs);
    }

    FNeg* createFNeg(Value* lhs, Value* rhs) noexcept {
      return createRawInstruction<FNeg>(lhs, rhs);
    }

    FAdd* createFAdd(Value* lhs, Value* rhs) noexcept {
      return createRawInstruction<FAdd>(lhs, rhs);
    }

    FSub* createFSub(Value* lhs, Value* rhs) noexcept {
      return createRawInstruction<FSub>(lhs, rhs);
    }

    FMul* createFMul(Value* lhs, Value* rhs) noexcept {
      return createRawInstruction<FMul>(lhs, rhs);
    }

    FDiv* createFDiv(Value* lhs, Value* rhs) noexcept {
      return createRawInstruction<FDiv>(lhs, rhs);
    }

    FRem* createFRem(Value* lhs, Value* rhs) noexcept {
      return createRawInstruction<FRem>(lhs, rhs);
    }

  private:
    Instruction* insertInst(BumpPtr<Instruction> inst) noexcept;

    BumpAlloc* pool_;
    Module* mod_;
    OptionalPtr<Function> currFn_ = dawn::none<Function>();
    OptionalPtr<BasicBlock> currBlock_ = dawn::none<BasicBlock>();
  };
} // namespace dawn