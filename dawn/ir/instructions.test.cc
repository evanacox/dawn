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

#include "dawn/ir/ir_builder.h"
#include "helpers/implements_value_interface.h"
#include "gtest/gtest.h"

TEST(DawnIRInstructions, PhiImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main");
  builder.setInsertFn(fn);

  auto* one = builder.createBlock();
  auto* two = builder.createBlock();
  auto* three = builder.createBlock();

  builder.setInsertPoint(three);

  auto* phi1 = builder.createPhi(builder.i64Ty());
  phi1->addIncomingBlock(one, builder.constI64(3));
  phi1->addIncomingBlock(two, builder.constI64(1));

  auto* phi2 = builder.createPhi(builder.f64Ty());

  EXPECT_TRUE((absl::VerifyTypeImplementsAbslHashCorrectly({*builder.createPhi(builder.i8Ty()), *phi1, *phi2})));

  dawn::tests::implementsValueInterface({builder.createPhi(builder.i8Ty()), phi1, phi2});
}

TEST(DawnIRInstructions, PhiIncomingAddSorts) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main");
  builder.setInsertFn(fn);

  auto* one = builder.createBlock();

  auto* two = builder.createBlock();
  auto* three = builder.createBlock();

  builder.setInsertPoint(three);

  auto* phi1 = builder.createPhi(builder.i64Ty());
  phi1->addIncomingBlock(one, builder.constI64(3));
  phi1->addIncomingBlock(two, builder.constI64(1));

  auto* phi2 = builder.createPhi(builder.i64Ty());
  phi2->addIncomingBlock(two, builder.constI64(1));
  phi2->addIncomingBlock(one, builder.constI64(3));

  // shouldn't matter which order the branches were added in,
  // they should be equal and have equal hash codes. in reality this
  // means that the vec should be sorted deterministically
  EXPECT_EQ(*phi1, *phi2);
  EXPECT_EQ(absl::HashOf(*phi1), absl::HashOf(*phi2));
}

TEST(DawnIRInstructions, CallImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main");
  auto* fn2 = builder.createFunc("puts", builder.i32Ty(), {builder.ptrTy()});
  auto* fn3 = builder.createFunc("thingamabob", builder.arrayTy(builder.i8Ty(), 8), {builder.i64Ty(), builder.ptrTy()});

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  auto* call1 = builder.createCall(fn2, {builder.constNull()});
  auto* call2 = builder.createCall(fn, {});
  auto* call3 = builder.createCall(fn3, {builder.constI64(16), builder.constNull()});

  dawn::tests::implementsValueInterface({call1, call2, call3});
}

TEST(DawnIRInstructions, ICmpImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main");

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  auto* lhs = builder.constI32(16);
  auto* rhs = builder.constI32(4);
  auto* lhs2 = builder.constI32(0);

  dawn::tests::implementsValueInterface({
      builder.createICmpEQ(lhs, rhs),
      builder.createICmpNE(lhs, rhs),
      builder.createICmpSGE(lhs, rhs),
      builder.createICmpSGT(lhs, rhs),
      builder.createICmpSLE(lhs, rhs),
      builder.createICmpSLT(lhs, rhs),
      builder.createICmpUGE(lhs, rhs),
      builder.createICmpUGT(lhs, rhs),
      builder.createICmpULE(lhs, rhs),
      builder.createICmpULT(lhs, rhs),
      // same operator as other, but different operands
      builder.createICmpULT(lhs2, rhs),
  });
}

TEST(DawnIRInstructions, FCmpImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main");

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  auto* lhs = builder.constF32(1.0);
  auto* rhs = builder.constF32(0x1.544p3); // NOLINT
  auto* lhs2 = builder.constF32(0.0);

  dawn::tests::implementsValueInterface({
      builder.createFCmpOEQ(lhs, rhs),
      builder.createFCmpONE(lhs, rhs),
      builder.createFCmpOGE(lhs, rhs),
      builder.createFCmpOGT(lhs, rhs),
      builder.createFCmpOLE(lhs, rhs),
      builder.createFCmpOLT(lhs, rhs),
      builder.createFCmpORD(lhs, rhs),
      builder.createFCmpUEQ(lhs, rhs),
      builder.createFCmpUNE(lhs, rhs),
      builder.createFCmpUGE(lhs, rhs),
      builder.createFCmpUGT(lhs, rhs),
      builder.createFCmpULE(lhs, rhs),
      builder.createFCmpULT(lhs, rhs),
      builder.createFCmpUNO(lhs, rhs),
      // same operand as previous but different operands, should be unique
      builder.createFCmpUNO(lhs2, rhs),
  });
}

TEST(DawnIRInstructions, SelImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main");

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  auto* cond1 = builder.constTrue();
  auto* cond2 = builder.constFalse();

  auto* val1 = builder.constI8('3');
  auto* val2 = builder.constI8('0' + 16);

  dawn::tests::implementsValueInterface({
      builder.createSel(cond1, dawn::If{val1}, dawn::Else{val2}),
      builder.createSel(cond2, dawn::If{val1}, dawn::Else{val2}),
      builder.createSel(cond1, dawn::If{val2}, dawn::Else{val1}),
      builder.createSel(cond2, dawn::If{val2}, dawn::Else{val1}),
      builder.createSel(cond1, dawn::If{val2}, dawn::Else{val2}),
      builder.createSel(cond2, dawn::If{val2}, dawn::Else{val2}),
  });
}

TEST(DawnIRInstructions, BrImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main");

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  auto vals = std::vector<dawn::Br*>{builder.createBr(entry)};

  for (auto i = 0; i < 16; ++i) {
    auto* bb = builder.createBlock();

    // want to make sure we don't trigger any assertions that
    // are based on having only one terminator per block or whatever
    builder.setInsertPoint(bb);
    vals.push_back(builder.createBr(bb));
  }

  dawn::tests::implementsValueInterface<dawn::Br>({vals.data(), vals.size()});
}

TEST(DawnIRInstructions, CondBrImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main");
  auto* fn2 = builder.createFunc("returnsBool", builder.boolTy());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  auto* cond1 = builder.constTrue();
  auto* cond2 = builder.constFalse();
  auto* cond3 = builder.createCall(fn2, {});

  auto* bbF = builder.createBlock();
  auto vals = std::vector<dawn::CondBr*>{};

  for (auto i = 0; i < 3; ++i) {
    auto* bb = builder.createBlock();

    // want to make sure we don't trigger any assertions that
    // are based on having only one terminator per block or whatever
    builder.setInsertPoint(bb);

    dawn::Value* cond;

    switch (i) {
      case 0: cond = cond1; break;
      case 1: cond = cond2; break;
      case 2: cond = cond3; break;
      default: DAWN_UNREACHABLE("what");
    }

    auto* cbr = builder.createCbr(cond, dawn::TrueBranch{bb}, dawn::FalseBranch{bbF});

    vals.push_back(cbr);
  }

  dawn::tests::implementsValueInterface<dawn::CondBr>({vals.data(), vals.size()});
}

TEST(DawnIRInstructions, RetImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  auto* ret1 = builder.constI32(16);
  auto* ret2 = builder.constI32(0);
  auto* ret3 = builder.constI32(~std::uint32_t{0});
  auto vals = std::vector<dawn::Ret*>{};

  for (auto i = 0; i < 3; ++i) {
    // want to make sure we don't trigger any assertions that
    // are based on having only one terminator per block or whatever
    builder.setInsertPoint(builder.createBlock());

    dawn::Value* ret;

    switch (i) {
      case 0: ret = ret1; break;
      case 1: ret = ret2; break;
      case 2: ret = ret3; break;
      default: DAWN_UNREACHABLE("what");
    }

    vals.push_back(builder.createRet(ret));
  }

  builder.setInsertPoint(builder.createBlock());
  vals.push_back(builder.createRetVoid());

  dawn::tests::implementsValueInterface<dawn::Ret>({vals.data(), vals.size()});
}

TEST(DawnIRInstructions, UnreachableImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  // `unreachable` instructions are stateless, `implementsValueInterface`
  // expects each object passed to be unique (state-wise).
  //
  // we do the value interface checking ourselves
  auto* lhs = builder.createUnreachable();
  builder.setInsertPoint(builder.createBlock());
  auto* rhs = builder.createUnreachable();

  EXPECT_EQ(absl::HashOf(*lhs), absl::HashOf(*rhs));
  EXPECT_EQ(*lhs, *rhs);
}

TEST(DawnIRInstructions, AllocaImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createAlloca(builder.i8Ty()),
      builder.createAlloca(builder.i8Ty(), builder.constI32(2)),
      builder.createAlloca(builder.i8Ty(), builder.constI32(3)),
      builder.createAlloca(builder.i8Ty(), builder.constI64(2)),
      builder.createAlloca(builder.i8Ty(), builder.constI32(4)),
      builder.createAlloca(builder.i16Ty()),
      builder.createAlloca(builder.i16Ty(), builder.constI32(2)),
      builder.createAlloca(builder.i16Ty(), builder.constI32(3)),
      builder.createAlloca(builder.i16Ty(), builder.constI64(2)),
      builder.createAlloca(builder.i16Ty(), builder.constI32(4)),
  });
}

TEST(DawnIRInstructions, LoadImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  auto* from1 = builder.constNull();
  auto* from2 = builder.createOffset(builder.ptrTy(), from1, dawn::Index{builder.constI32(4)});
  auto* structType = builder.structTy({builder.ptrTy(), builder.i64Ty(), builder.i64Ty()});

  dawn::tests::implementsValueInterface({
      builder.createLoad(builder.i8Ty(), from1),
      builder.createVolatileLoad(builder.i8Ty(), from1),
      builder.createLoad(builder.i32Ty(), from1),
      builder.createVolatileLoad(builder.i32Ty(), from1),
      builder.createLoad(structType, from1),
      builder.createVolatileLoad(structType, from1),
      builder.createLoad(builder.i8Ty(), from2),
      builder.createVolatileLoad(builder.i8Ty(), from2),
      builder.createLoad(builder.i32Ty(), from2),
      builder.createVolatileLoad(builder.i32Ty(), from2),
      builder.createLoad(structType, from2),
      builder.createVolatileLoad(structType, from2),
  });
}

TEST(DawnIRInstructions, StoreImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  auto* dest1 = builder.constNull();
  auto* dest2 = builder.createOffset(builder.i64Ty(), dest1, dawn::Index{builder.constI64(16)});
  auto* dest3 = builder.createOffset(builder.i8Ty(), dest2, dawn::Index{builder.constI64(512)});

  dawn::tests::implementsValueInterface({
      builder.createStore(builder.constI8(0), dawn::Dest{dest1}),
      builder.createVolatileStore(builder.constI8(0), dawn::Dest{dest1}),
      builder.createStore(builder.constI8(1), dawn::Dest{dest1}),
      builder.createVolatileStore(builder.constI8(1), dawn::Dest{dest1}),
      builder.createStore(builder.constI8(0), dawn::Dest{dest2}),
      builder.createVolatileStore(builder.constI8(0), dawn::Dest{dest2}),
      builder.createStore(builder.constI8(1), dawn::Dest{dest2}),
      builder.createVolatileStore(builder.constI8(1), dawn::Dest{dest2}),
      builder.createStore(builder.constI8(0), dawn::Dest{dest3}),
      builder.createVolatileStore(builder.constI8(0), dawn::Dest{dest3}),
      builder.createStore(builder.constI8(1), dawn::Dest{dest3}),
      builder.createVolatileStore(builder.constI8(1), dawn::Dest{dest3}),
  });
}

TEST(DawnIRInstructions, OffsetImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  auto* dest1 = builder.constNull();
  auto* dest2 = builder.createOffset(builder.i64Ty(), dest1, dawn::Index{builder.constI64(16)});
  auto* dest3 = builder.createOffset(builder.i8Ty(), dest2, dawn::Index{builder.constI64(512)});

  dawn::tests::implementsValueInterface({
      builder.createOffset(builder.i8Ty(), dest1, dawn::Index{builder.constI8(1)}),
      builder.createOffset(builder.i8Ty(), dest1, dawn::Index{builder.constI16(1)}),
      builder.createOffset(builder.i8Ty(), dest1, dawn::Index{builder.constI32(1)}),
      builder.createOffset(builder.i8Ty(), dest1, dawn::Index{builder.constI64(1)}),
      builder.createOffset(builder.i8Ty(), dest1, dawn::Index{builder.constI64(2)}),
      builder.createOffset(builder.i16Ty(), dest1, dawn::Index{builder.constI64(2)}),
      builder.createOffset(builder.i8Ty(), dest2, dawn::Index{builder.constI64(2)}),
      builder.createOffset(builder.ptrTy(), dest3, dawn::Index{builder.constI64(2)}),
  });
}

TEST(DawnIRInstructions, ExtractImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  auto* agg1 =
      builder.constStruct({builder.constI8(16), builder.constNull(), builder.constArrayFill(builder.constI64(4), 16)});
  auto* agg2 = builder.constStruct({builder.constNull(), builder.constNull(), builder.constNull()});
  auto* agg3 = builder.constUndef(agg1->type());

  auto* arrayAccess = builder.createExtract(dawn::Agg{agg1}, builder.constI32(2));

  dawn::tests::implementsValueInterface({
      arrayAccess,
      builder.createExtract(dawn::Agg{agg1}, builder.constI8(0)),
      builder.createExtract(dawn::Agg{agg1}, builder.constI16(0)),
      builder.createExtract(dawn::Agg{agg1}, builder.constI32(0)),
      builder.createExtract(dawn::Agg{agg1}, builder.constI64(0)),
      builder.createExtract(dawn::Agg{agg1}, builder.constI32(1)),
      builder.createExtract(dawn::Agg{agg3}, builder.constI32(1)),
      builder.createExtract(dawn::Agg{arrayAccess}, builder.constI32(2)),
      builder.createExtract(dawn::Agg{agg2}, builder.constI32(0)),
      builder.createExtract(dawn::Agg{agg2}, builder.constI32(1)),
      builder.createExtract(dawn::Agg{agg2}, builder.constI32(2)),
  });
}

TEST(DawnIRInstructions, InsertImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  auto* agg1 =
      builder.constStruct({builder.constI8(16), builder.constNull(), builder.constArrayFill(builder.constI64(4), 16)});
  auto* agg2 = builder.constStruct({builder.constNull(), builder.constNull(), builder.constNull()});
  auto* agg3 = builder.constUndef(agg1->type());
  auto* arrayAccess = builder.createExtract(dawn::Agg{agg1}, builder.constI32(2));

  dawn::tests::implementsValueInterface({
      builder.createInsert(dawn::Agg{agg1}, dawn::Index{builder.constI8(0)}, builder.constI8(16)),
      builder.createInsert(dawn::Agg{agg1}, dawn::Index{builder.constI16(0)}, builder.constI8(16)),
      builder.createInsert(dawn::Agg{agg1}, dawn::Index{builder.constI32(0)}, builder.constI8(16)),
      builder.createInsert(dawn::Agg{agg1}, dawn::Index{builder.constI64(0)}, builder.constI8(16)),
      builder.createInsert(dawn::Agg{agg1}, dawn::Index{builder.constI64(0)}, builder.constI8(3)),
      builder.createInsert(dawn::Agg{agg1}, dawn::Index{builder.constI32(1)}, builder.constNull()),
      builder.createInsert(dawn::Agg{agg1},
          dawn::Index{builder.constI32(2)},
          builder.constUndef(builder.arrayTy(builder.i64Ty(), 4))),
      builder.createInsert(dawn::Agg{agg3}, dawn::Index{builder.constI32(1)}, builder.constNull()),
      builder.createInsert(dawn::Agg{arrayAccess}, dawn::Index{builder.constI32(2)}, builder.constI64(1)),
      builder.createInsert(dawn::Agg{agg2}, dawn::Index{builder.constI32(0)}, builder.constI64(1)),
      builder.createInsert(dawn::Agg{agg2}, dawn::Index{builder.constI32(1)}, builder.constI64(1)),
      builder.createInsert(dawn::Agg{agg2}, dawn::Index{builder.constI32(2)}, builder.constI64(1)),
      builder.createInsert(dawn::Agg{agg2}, dawn::Index{builder.constI32(3)}, builder.constI64(1)),
  });
}

TEST(DawnIRInstructions, ElemPtrImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  auto* agg1 =
      builder.constStruct({builder.constI8(16), builder.constNull(), builder.constArrayFill(builder.constI64(4), 16)});
  auto* agg2 = builder.constStruct({builder.constI8(0), builder.constI8(16)});
  auto* arrayAccess = builder.createExtract(dawn::Agg{agg1}, builder.constI32(2));

  dawn::tests::implementsValueInterface({
      builder.createElemPtr(agg1->type(), builder.constNull(), dawn::Index{builder.constI8(0)}),
      builder.createElemPtr(agg1->type(), builder.constNull(), dawn::Index{builder.constI16(0)}),
      builder.createElemPtr(agg1->type(), builder.constNull(), dawn::Index{builder.constI32(0)}),
      builder.createElemPtr(agg1->type(), builder.constNull(), dawn::Index{builder.constI64(0)}),
      builder.createElemPtr(agg1->type(), builder.constNull(), dawn::Index{builder.constI32(1)}),
      builder.createElemPtr(agg1->type(),
          builder.createOffset(builder.i8Ty(), builder.constNull(), dawn::Index{builder.constI32(1)}),
          dawn::Index{builder.constI32(1)}),
      builder.createElemPtr(agg2->type(), builder.constNull(), dawn::Index{builder.constI32(0)}),
      builder.createElemPtr(arrayAccess->type(), builder.constNull(), dawn::Index{builder.constI32(2)}),
  });
}

TEST(DawnIRInstructions, SextImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createSext(builder.i64Ty(), builder.constI32(0)),
      builder.createSext(builder.i64Ty(), builder.constI32(1)),
      builder.createSext(builder.i32Ty(), builder.constI16(0)),
      builder.createSext(builder.i32Ty(), builder.constI16(1)),
      builder.createSext(builder.i16Ty(), builder.constI8(0)),
      builder.createSext(builder.i16Ty(), builder.constI8(1)),
  });
}

TEST(DawnIRInstructions, ZextImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createZext(builder.i64Ty(), builder.constI32(0)),
      builder.createZext(builder.i64Ty(), builder.constI32(1)),
      builder.createZext(builder.i32Ty(), builder.constI16(0)),
      builder.createZext(builder.i32Ty(), builder.constI16(1)),
      builder.createZext(builder.i16Ty(), builder.constI8(0)),
      builder.createZext(builder.i16Ty(), builder.constI8(1)),
  });
}

TEST(DawnIRInstructions, TruncImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createTrunc(builder.i32Ty(), builder.constI64(0)),
      builder.createTrunc(builder.i32Ty(), builder.constI64(1)),
      builder.createTrunc(builder.i32Ty(), builder.constI16(0)),
      builder.createTrunc(builder.i32Ty(), builder.constI16(1)),
      builder.createTrunc(builder.i16Ty(), builder.constI8(0)),
      builder.createTrunc(builder.i16Ty(), builder.constI8(1)),
  });
}

TEST(DawnIRInstructions, PtoiImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  auto* ptr1 = builder.constNull();
  auto* ptr2 = builder.createOffset(builder.i8Ty(), ptr1, dawn::Index{builder.constI64(512)});

  dawn::tests::implementsValueInterface({
      builder.createPtrToInt(builder.i8Ty(), ptr1),
      builder.createPtrToInt(builder.i16Ty(), ptr1),
      builder.createPtrToInt(builder.i32Ty(), ptr1),
      builder.createPtrToInt(builder.i64Ty(), ptr1),
      builder.createPtrToInt(builder.i8Ty(), ptr2),
      builder.createPtrToInt(builder.i16Ty(), ptr2),
      builder.createPtrToInt(builder.i32Ty(), ptr2),
      builder.createPtrToInt(builder.i64Ty(), ptr2),
  });
}

TEST(DawnIRInstructions, ItopImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createIntToPtr(builder.constI8(0)),
      builder.createIntToPtr(builder.constI8(1)),
      builder.createIntToPtr(builder.constI16(0)),
      builder.createIntToPtr(builder.constI16(1)),
      builder.createIntToPtr(builder.constI32(0)),
      builder.createIntToPtr(builder.constI32(1)),
      builder.createIntToPtr(builder.constI64(0)),
      builder.createIntToPtr(builder.constI64(1)),
  });
}

TEST(DawnIRInstructions, BtoiImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createBoolToInt(builder.i8Ty(), builder.constTrue()),
      builder.createBoolToInt(builder.i8Ty(), builder.constFalse()),
      builder.createBoolToInt(builder.i16Ty(), builder.constTrue()),
      builder.createBoolToInt(builder.i16Ty(), builder.constFalse()),
      builder.createBoolToInt(builder.i32Ty(), builder.constTrue()),
      builder.createBoolToInt(builder.i32Ty(), builder.constFalse()),
      builder.createBoolToInt(builder.i64Ty(), builder.constTrue()),
      builder.createBoolToInt(builder.i64Ty(), builder.constFalse()),
  });
}

TEST(DawnIRInstructions, ItobImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createIntToBool(builder.constI8(0)),
      builder.createIntToBool(builder.constI8(1)),
      builder.createIntToBool(builder.constI16(0)),
      builder.createIntToBool(builder.constI16(1)),
      builder.createIntToBool(builder.constI32(0)),
      builder.createIntToBool(builder.constI32(1)),
      builder.createIntToBool(builder.constI64(0)),
      builder.createIntToBool(builder.constI64(1)),
  });
}

TEST(DawnIRInstructions, SitofImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createSIntToFloat(builder.f32Ty(), builder.constI8(0)),
      builder.createSIntToFloat(builder.f32Ty(), builder.constI8(1)),
      builder.createSIntToFloat(builder.f64Ty(), builder.constI8(0)),
      builder.createSIntToFloat(builder.f64Ty(), builder.constI8(1)),
      builder.createSIntToFloat(builder.f32Ty(), builder.constI32(0)),
      builder.createSIntToFloat(builder.f32Ty(), builder.constI32(1)),
      builder.createSIntToFloat(builder.f64Ty(), builder.constI32(0)),
      builder.createSIntToFloat(builder.f64Ty(), builder.constI32(1)),
  });
}

TEST(DawnIRInstructions, UitofImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createUIntToFloat(builder.f32Ty(), builder.constI8(0)),
      builder.createUIntToFloat(builder.f32Ty(), builder.constI8(1)),
      builder.createUIntToFloat(builder.f64Ty(), builder.constI8(0)),
      builder.createUIntToFloat(builder.f64Ty(), builder.constI8(1)),
      builder.createUIntToFloat(builder.f32Ty(), builder.constI32(0)),
      builder.createUIntToFloat(builder.f32Ty(), builder.constI32(1)),
      builder.createUIntToFloat(builder.f64Ty(), builder.constI32(0)),
      builder.createUIntToFloat(builder.f64Ty(), builder.constI32(1)),
  });
}

TEST(DawnIRInstructions, FtosiImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createFloatToSInt(builder.i8Ty(), builder.constF32(0.0)),
      builder.createFloatToSInt(builder.i8Ty(), builder.constF32(0.1)),      // NOLINT
      builder.createFloatToSInt(builder.i8Ty(), builder.constF32(3.141592)), // NOLINT
      builder.createFloatToSInt(builder.i8Ty(), builder.constF64(0.0)),
      builder.createFloatToSInt(builder.i8Ty(), builder.constF64(0.1)),      // NOLINT
      builder.createFloatToSInt(builder.i8Ty(), builder.constF64(3.141592)), // NOLINT
      builder.createFloatToSInt(builder.i32Ty(), builder.constF32(0.0)),
      builder.createFloatToSInt(builder.i32Ty(), builder.constF32(0.1)),      // NOLINT
      builder.createFloatToSInt(builder.i32Ty(), builder.constF32(3.141592)), // NOLINT
      builder.createFloatToSInt(builder.i32Ty(), builder.constF64(0.0)),
      builder.createFloatToSInt(builder.i32Ty(), builder.constF64(0.1)),      // NOLINT
      builder.createFloatToSInt(builder.i32Ty(), builder.constF64(3.141592)), // NOLINT
  });
}

TEST(DawnIRInstructions, FtouiImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createFloatToUInt(builder.i8Ty(), builder.constF32(0.0)),
      builder.createFloatToUInt(builder.i8Ty(), builder.constF32(0.1)),      // NOLINT
      builder.createFloatToUInt(builder.i8Ty(), builder.constF32(3.141592)), // NOLINT
      builder.createFloatToUInt(builder.i8Ty(), builder.constF64(0.0)),
      builder.createFloatToUInt(builder.i8Ty(), builder.constF64(0.1)),      // NOLINT
      builder.createFloatToUInt(builder.i8Ty(), builder.constF64(3.141592)), // NOLINT
      builder.createFloatToUInt(builder.i32Ty(), builder.constF32(0.0)),
      builder.createFloatToUInt(builder.i32Ty(), builder.constF32(0.1)),      // NOLINT
      builder.createFloatToUInt(builder.i32Ty(), builder.constF32(3.141592)), // NOLINT
      builder.createFloatToUInt(builder.i32Ty(), builder.constF64(0.0)),
      builder.createFloatToUInt(builder.i32Ty(), builder.constF64(0.1)),      // NOLINT
      builder.createFloatToUInt(builder.i32Ty(), builder.constF64(3.141592)), // NOLINT
  });
}

TEST(DawnIRInstructions, AndImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createAnd(builder.constFalse(), builder.constTrue()),
      builder.createAnd(builder.constFalse(), builder.constFalse()),
      builder.createAnd(builder.constI8(0), builder.constI8(0)),
      builder.createAnd(builder.constI8(0), builder.constI8(1)),
      builder.createAnd(builder.constI32(0), builder.constI32(0)),
      builder.createAnd(builder.constI32(1), builder.constI32(1)),
  });
}

TEST(DawnIRInstructions, OrImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createOr(builder.constFalse(), builder.constTrue()),
      builder.createOr(builder.constFalse(), builder.constFalse()),
      builder.createOr(builder.constI8(0), builder.constI8(0)),
      builder.createOr(builder.constI8(0), builder.constI8(1)),
      builder.createOr(builder.constI32(0), builder.constI32(0)),
      builder.createOr(builder.constI32(1), builder.constI32(1)),
  });
}

TEST(DawnIRInstructions, XorImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createXor(builder.constFalse(), builder.constTrue()),
      builder.createXor(builder.constFalse(), builder.constFalse()),
      builder.createXor(builder.constI8(0), builder.constI8(0)),
      builder.createXor(builder.constI8(0), builder.constI8(1)),
      builder.createXor(builder.constI32(0), builder.constI32(0)),
      builder.createXor(builder.constI32(1), builder.constI32(1)),
  });
}

TEST(DawnIRInstructions, ShlImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createShl(builder.constI8(0), builder.constI8(0)),
      builder.createShl(builder.constI8(0), builder.constI8(1)),
      builder.createShl(builder.constI32(0), builder.constI32(0)),
      builder.createShl(builder.constI32(1), builder.constI32(1)),
  });
}

TEST(DawnIRInstructions, LShrImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createLShr(builder.constI8(0), builder.constI8(0)),
      builder.createLShr(builder.constI8(0), builder.constI8(1)),
      builder.createLShr(builder.constI32(0), builder.constI32(0)),
      builder.createLShr(builder.constI32(1), builder.constI32(1)),
  });
}

TEST(DawnIRInstructions, AShrImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createAShr(builder.constI8(0), builder.constI8(0)),
      builder.createAShr(builder.constI8(0), builder.constI8(1)),
      builder.createAShr(builder.constI32(0), builder.constI32(0)),
      builder.createAShr(builder.constI32(1), builder.constI32(1)),
  });
}

TEST(DawnIRInstructions, IAddImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createIAdd(builder.constI8(0), builder.constI8(0)),
      builder.createIAdd(builder.constI8(0), builder.constI8(1)),
      builder.createIAdd(builder.constI32(0), builder.constI32(0)),
      builder.createIAdd(builder.constI32(1), builder.constI32(1)),
  });
}

TEST(DawnIRInstructions, ISubImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createISub(builder.constI8(0), builder.constI8(0)),
      builder.createISub(builder.constI8(0), builder.constI8(1)),
      builder.createISub(builder.constI32(0), builder.constI32(0)),
      builder.createISub(builder.constI32(1), builder.constI32(1)),
  });
}

TEST(DawnIRInstructions, IMulImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createIMul(builder.constI8(0), builder.constI8(0)),
      builder.createIMul(builder.constI8(0), builder.constI8(1)),
      builder.createIMul(builder.constI32(0), builder.constI32(0)),
      builder.createIMul(builder.constI32(1), builder.constI32(1)),
  });
}

TEST(DawnIRInstructions, UDivImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createUDiv(builder.constI8(0), builder.constI8(0)),
      builder.createUDiv(builder.constI8(0), builder.constI8(1)),
      builder.createUDiv(builder.constI32(0), builder.constI32(0)),
      builder.createUDiv(builder.constI32(1), builder.constI32(1)),
  });
}

TEST(DawnIRInstructions, SDivImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createSDiv(builder.constI8(0), builder.constI8(0)),
      builder.createSDiv(builder.constI8(0), builder.constI8(1)),
      builder.createSDiv(builder.constI32(0), builder.constI32(0)),
      builder.createSDiv(builder.constI32(1), builder.constI32(1)),
  });
}

TEST(DawnIRInstructions, URemImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createURem(builder.constI8(0), builder.constI8(0)),
      builder.createURem(builder.constI8(0), builder.constI8(1)),
      builder.createURem(builder.constI32(0), builder.constI32(0)),
      builder.createURem(builder.constI32(1), builder.constI32(1)),
  });
}

TEST(DawnIRInstructions, SRemImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createSRem(builder.constI8(0), builder.constI8(0)),
      builder.createSRem(builder.constI8(0), builder.constI8(1)),
      builder.createSRem(builder.constI32(0), builder.constI32(0)),
      builder.createSRem(builder.constI32(1), builder.constI32(1)),
  });
}

TEST(DawnIRInstructions, FNegImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createFNeg(builder.constF32(0.0), builder.constF32(0.0)),
      builder.createFNeg(builder.constF32(0.0), builder.constF32(1.0)),
      builder.createFNeg(builder.constF64(0.0), builder.constF64(0)),
      builder.createFNeg(builder.constF64(1.0), builder.constF64(1.0)),
  });
}

TEST(DawnIRInstructions, FAddImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createFAdd(builder.constF32(0.0), builder.constF32(0.0)),
      builder.createFAdd(builder.constF32(0.0), builder.constF32(1.0)),
      builder.createFAdd(builder.constF64(0.0), builder.constF64(0)),
      builder.createFAdd(builder.constF64(1.0), builder.constF64(1.0)),
  });
}

TEST(DawnIRInstructions, FSubImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createFSub(builder.constF32(0.0), builder.constF32(0.0)),
      builder.createFSub(builder.constF32(0.0), builder.constF32(1.0)),
      builder.createFSub(builder.constF64(0.0), builder.constF64(0)),
      builder.createFSub(builder.constF64(1.0), builder.constF64(1.0)),
  });
}

TEST(DawnIRInstructions, FMulImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createFMul(builder.constF32(0.0), builder.constF32(0.0)),
      builder.createFMul(builder.constF32(0.0), builder.constF32(1.0)),
      builder.createFMul(builder.constF64(0.0), builder.constF64(0)),
      builder.createFMul(builder.constF64(1.0), builder.constF64(1.0)),
  });
}

TEST(DawnIRInstructions, FDivImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createFDiv(builder.constF32(0.0), builder.constF32(0.0)),
      builder.createFDiv(builder.constF32(0.0), builder.constF32(1.0)),
      builder.createFDiv(builder.constF64(0.0), builder.constF64(0)),
      builder.createFDiv(builder.constF64(1.0), builder.constF64(1.0)),
  });
}

TEST(DawnIRInstructions, FRemImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* fn = builder.createFunc("main", builder.i32Ty());

  builder.setInsertFn(fn);
  auto* entry = builder.createBlock();
  builder.setInsertPoint(entry);

  dawn::tests::implementsValueInterface({
      builder.createFRem(builder.constF32(0.0), builder.constF32(0.0)),
      builder.createFRem(builder.constF32(0.0), builder.constF32(1.0)),
      builder.createFRem(builder.constF64(0.0), builder.constF64(0)),
      builder.createFRem(builder.constF64(1.0), builder.constF64(1.0)),
  });
}