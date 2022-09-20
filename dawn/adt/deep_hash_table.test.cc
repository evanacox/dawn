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

#include "dawn/adt/deep_hash_table.h"
#include "dawn/ir.h"
#include "gtest/gtest.h"
#include <memory>

TEST(DawnADTDeepHashTable, SetWorks) {
  dawn::DeepHashSet<std::unique_ptr<int>> table;

  // equivalent constants with **different addresses**
  auto one = std::make_unique<int>(1);
  auto two = std::make_unique<int>(1);
  table.insert(std::move(one));

  // can search from the unique_ptr by itself
  EXPECT_TRUE(table.contains(two));
  EXPECT_NE(table.find(two), table.end());

  // can search from pointer to equivalent object
  EXPECT_TRUE(table.contains(two.get()));
  EXPECT_NE(table.find(two.get()), table.end());
}

TEST(DawnADTDeepHashTable, MapWorks) {
  dawn::DeepHashMap<int*, double> table;

  // equivalent constants with **different addresses**
  auto* one = new int(1);
  auto* two = new int(1);
  table.emplace(one, 0.0);

  // can search from pointer to equivalent object
  EXPECT_TRUE(table.contains(two));
  EXPECT_NE(table.find(two), table.end());

  delete one;
  delete two;
}