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
#include "gtest/gtest.h"

TEST(DawnIRTypes, IntRTTI) {
  auto ty1 = dawn::Int{32};
  auto* ty = static_cast<dawn::Type*>(&ty1);

  EXPECT_TRUE(dawn::isa<dawn::Int>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Float>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Bool>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Ptr>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Array>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Struct>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Void>(ty));
}

TEST(DawnIRTypes, FloatRTTI) {
  auto ty1 = dawn::Float{32};
  auto* ty = static_cast<dawn::Type*>(&ty1);

  EXPECT_TRUE(dawn::isa<dawn::Float>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Int>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Bool>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Ptr>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Array>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Struct>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Void>(ty));
}

TEST(DawnIRTypes, BoolRTTI) {
  auto ty1 = dawn::Bool{};
  auto* ty = static_cast<dawn::Type*>(&ty1);

  EXPECT_TRUE(dawn::isa<dawn::Bool>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Float>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Int>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Ptr>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Array>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Struct>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Void>(ty));
}

TEST(DawnIRTypes, PtrRTTI) {
  auto ty1 = dawn::Ptr{};
  auto* ty = static_cast<dawn::Type*>(&ty1);

  EXPECT_TRUE(dawn::isa<dawn::Ptr>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Bool>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Float>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Int>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Array>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Struct>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Void>(ty));
}

TEST(DawnIRTypes, ArrayRTTI) {
  auto ty1 = dawn::Ptr{};
  auto ty2 = dawn::Array{&ty1, 64};
  auto* ty = static_cast<dawn::Type*>(&ty2);

  EXPECT_TRUE(dawn::isa<dawn::Array>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Ptr>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Bool>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Float>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Int>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Struct>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Void>(ty));
}

TEST(DawnIRTypes, StructRTTI) {
  auto ty1 = dawn::Ptr{};
  auto ty2 = dawn::Struct{{&ty1}};
  auto* ty = static_cast<dawn::Type*>(&ty2);

  EXPECT_TRUE(dawn::isa<dawn::Struct>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Array>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Ptr>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Bool>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Float>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Int>(ty));
}

TEST(DawnIRTypes, VoidRTTI) {
  auto ty1 = dawn::Void{};
  auto* ty = static_cast<dawn::Type*>(&ty1);

  EXPECT_TRUE(dawn::isa<dawn::Void>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Struct>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Array>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Ptr>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Bool>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Float>(ty));
  EXPECT_FALSE(dawn::isa<dawn::Int>(ty));
}