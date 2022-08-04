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

#include "dawn/utility/apint.h"
#include "gtest/gtest.h"
#include <limits>

TEST(DawnUtilityAPInt, APIntNativeWidthWorks) {
  auto integer = dawn::APInt(std::numeric_limits<std::uint64_t>::max(), dawn::Width::i64);

  EXPECT_EQ(integer.width(), 64);
  EXPECT_EQ(integer.value(), std::numeric_limits<std::uint64_t>::max());
}

TEST(DawnUtilityAPInt, APIntSmallerWidthWorks) {
  auto integer = dawn::APInt(3, dawn::Width::i8);

  EXPECT_EQ(integer.width(), 8);
  EXPECT_EQ(integer.value(), 3);
}

TEST(DawnUtilityAPInt, APIntMasksOffExtraBits) {
  auto integer = dawn::APInt(std::numeric_limits<std::uint64_t>::max(), dawn::Width::i8);

  EXPECT_EQ(integer.width(), 8);
  EXPECT_EQ(integer.value(), std::numeric_limits<std::uint8_t>::max());
}