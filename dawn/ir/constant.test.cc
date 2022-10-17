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

TEST(DawnIRConstants, ConstantIntImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  dawn::tests::implementsValueInterface({
      builder.constI8(0),
      builder.constI16(0),
      builder.constI32(0),
      builder.constI64(0),
      builder.constI8(1),
      builder.constI16(1),
      builder.constI32(1),
      builder.constI64(1),
  });
}

TEST(DawnIRConstants, ConstantBoolImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  dawn::tests::implementsValueInterface({
      builder.constTrue(),
      builder.constFalse(),
  });
}

TEST(DawnIRConstants, ConstantFloatImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  dawn::tests::implementsValueInterface({
      builder.constF32(0.0),
      builder.constF64(0.0),
      builder.constF32(1.0),
      builder.constF64(1.0),
  });
}

TEST(DawnIRConstants, ConstantNullImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  dawn::tests::implementsValueInterface({builder.constNull()});
}

TEST(DawnIRConstants, ConstantStructImplementsValue) {
  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  dawn::tests::implementsValueInterface({
      builder.constStruct({}),
      builder.constStruct({builder.constI8(0)}),
      builder.constStruct({builder.constI16(0)}),
      builder.constStruct({builder.constFalse()}),
      builder.constStruct({builder.constI8(1)}),
      builder.constStruct({builder.constI8(0), builder.constI8(0)}),
      builder.constStruct({builder.constStruct({})}),
      builder.constStruct({builder.constNull()}),
  });
}
