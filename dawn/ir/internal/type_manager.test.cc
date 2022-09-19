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
#include "dawn/utility/rtti.h"
#include "gtest/gtest.h"
#include <algorithm>

TEST(DawnIRInternalTypeManager, PredefsDefinedProperly) {
  auto alloc = dawn::BumpAlloc{};
  auto manager = dawn::internal::TypeManager{&alloc};

  {
    EXPECT_EQ(manager.i8(), manager.i8());
    EXPECT_TRUE(dawn::isa<dawn::Int>(manager.i8()));
    EXPECT_TRUE(dawn::dyncast<dawn::Int>(manager.i8())->width() == dawn::Width::i8);
  }

  {
    EXPECT_EQ(manager.i16(), manager.i16());
    EXPECT_TRUE(dawn::isa<dawn::Int>(manager.i16()));
    EXPECT_TRUE(dawn::dyncast<dawn::Int>(manager.i16())->width() == dawn::Width::i16);
  }

  {
    EXPECT_EQ(manager.i32(), manager.i32());
    EXPECT_TRUE(dawn::isa<dawn::Int>(manager.i32()));
    EXPECT_TRUE(dawn::dyncast<dawn::Int>(manager.i32())->width() == dawn::Width::i32);
  }

  {
    EXPECT_EQ(manager.i64(), manager.i64());
    EXPECT_TRUE(dawn::isa<dawn::Int>(manager.i64()));
    EXPECT_TRUE(dawn::dyncast<dawn::Int>(manager.i64())->width() == dawn::Width::i64);
  }

  {
    EXPECT_EQ(manager.boolean(), manager.boolean());
    EXPECT_TRUE(dawn::isa<dawn::Bool>(manager.boolean()));
  }

  {
    EXPECT_EQ(manager.ptr(), manager.ptr());
    EXPECT_TRUE(dawn::isa<dawn::Ptr>(manager.ptr()));
  }

  {
    EXPECT_EQ(manager.f32(), manager.f32());
    EXPECT_TRUE(dawn::isa<dawn::Float>(manager.f32()));
    EXPECT_TRUE(dawn::dyncast<dawn::Float>(manager.f32())->width() == 32);
  }

  {
    EXPECT_EQ(manager.f64(), manager.f64());
    EXPECT_TRUE(dawn::isa<dawn::Float>(manager.f64()));
    EXPECT_TRUE(dawn::dyncast<dawn::Float>(manager.f64())->width() == 64);
  }

  {
    EXPECT_EQ(manager.emptyStruct(), manager.emptyStruct());
    EXPECT_TRUE(dawn::isa<dawn::Struct>(manager.emptyStruct()));
    EXPECT_TRUE(dawn::dyncast<dawn::Struct>(manager.emptyStruct())->fields().empty());
  }

  {
    EXPECT_EQ(manager.voidType(), manager.voidType());
    EXPECT_TRUE(dawn::isa<dawn::Void>(manager.voidType()));
  }
}

TEST(DawnIRInternalTypeManager, PredefsUniquedProperly) {
  auto alloc = dawn::BumpAlloc{};
  auto manager = dawn::internal::TypeManager{&alloc};
  auto list = std::array{manager.i8(),
      manager.i16(),
      manager.i32(),
      manager.i64(),
      manager.boolean(),
      manager.ptr(),
      manager.f32(),
      manager.f64(),
      manager.emptyStruct(),
      manager.voidType()};

  EXPECT_EQ(std::unique(list.begin(), list.end()), list.end());

  auto list2 = std::array{manager.i8(),
      manager.i8(),
      manager.i8(),
      manager.i8(),
      manager.i8(),
      manager.i8(),
      manager.i8(),
      manager.i8(),
      manager.i8(),
      manager.i8()};

  EXPECT_EQ(std::unique(list2.begin(), list2.end()), list2.begin() + 1);
}

TEST(DawnIRInternalTypeManager, PredefIntsSameAsByWidth) {
  auto alloc = dawn::BumpAlloc{};
  auto manager = dawn::internal::TypeManager{&alloc};

  EXPECT_EQ(manager.i8(), manager.intOfWidth(8));
  EXPECT_EQ(manager.i16(), manager.intOfWidth(16));
  EXPECT_EQ(manager.i32(), manager.intOfWidth(32));
  EXPECT_EQ(manager.i64(), manager.intOfWidth(64));
  EXPECT_NE(manager.i8(), manager.intOfWidth(32));
  EXPECT_NE(manager.i16(), manager.intOfWidth(32));
  EXPECT_NE(manager.i32(), manager.intOfWidth(64));
  EXPECT_NE(manager.i64(), manager.intOfWidth(32));
}

TEST(DawnIRInternalTypeManager, EquivalentArraysNotDuplicated) {
  auto alloc = dawn::BumpAlloc{};
  auto manager = dawn::internal::TypeManager{&alloc};

  auto* ty1 = manager.i32();
  auto* ty2 = manager.array(&alloc, ty1, 32);
  auto* ty3 = manager.array(&alloc, ty1, 32);
  auto* ty4 = manager.array(&alloc, ty1, 33); // NOLINT
  auto* ty5 = manager.array(&alloc, manager.i64(), 32);

  EXPECT_EQ(ty2, ty3);
  EXPECT_NE(ty2, ty4);
  EXPECT_NE(ty2, ty5);
}

TEST(DawnIRInternalTypeManager, EquivalentStructsNotDuplicated) {
  auto alloc = dawn::BumpAlloc{};
  auto manager = dawn::internal::TypeManager{&alloc};

  auto* ty1 = manager.ptr();
  auto* ty2 = manager.i64();
  auto fields = std::vector{ty1, ty2, ty2};

  auto* ty3 = manager.structure(&alloc, fields);
  auto* ty4 = manager.structure(&alloc, fields);
  fields.push_back(manager.i8());
  auto* ty5 = manager.structure(&alloc, fields);
  auto* ty6 = manager.structure(&alloc, fields);

  EXPECT_EQ(ty3, ty4);
  EXPECT_NE(ty3, ty5);
  EXPECT_EQ(ty5, ty6);
}