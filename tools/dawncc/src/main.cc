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

#include "absl/flags/parse.h"
#include "dawn/dawn.h"

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  dawn::Module mod;
  dawn::IRBuilder builder{&mod};

  auto* other = builder.createFunc("atoi", mod.i32(), {mod.ptr()});

  auto args = std::array{mod.i32(), mod.ptr()};
  auto* main = builder.createFunc("main", mod.i32(), std::span{args.data(), args.size()});
  auto* entry = builder.createBlock(main);
  builder.setInsertPoint(entry);
  auto* add = builder.createCall(other, {builder.constNull()});
  auto* sub = builder.createISub(add, builder.constI32(4));

  auto* one = builder.createBlock();
  auto* two = builder.createBlock();
  auto* merge = builder.createBlock();

  auto* cmp1 = builder.createICmpNE(add, sub);
  auto* cmp2 = builder.createICmpSGT(add, sub);
  auto* xor1 = builder.createXor(cmp1, cmp2);
  builder.createCbr(xor1, dawn::TrueBranch{one}, dawn::FalseBranch{two});

  builder.setInsertPoint(one);
  auto* one1 = builder.createPtrToInt(builder.i32Ty(), builder.constNull());
  auto* one2 = builder.createZext(builder.i64Ty(), one1);
  auto* val1 = builder.createIAdd(one2, builder.constI64(128));
  builder.createBr(merge);

  builder.setInsertPoint(two);
  auto* two1 = builder.createISub(builder.constI64(8), builder.constI64(16));
  auto* two2 = builder.createIMul(two1, builder.constI64(16));
  auto* two3 = builder.createAShr(two2, builder.constI64(3));
  auto* val2 = builder.createIAdd(two3, builder.constI64(1));
  builder.createBr(merge);

  builder.setInsertPoint(merge);
  auto* phi = builder.createPhi(builder.i64Ty());

  phi->addIncomingBlock(entry, builder.constI64(64));
  phi->addIncomingBlock(one, val1);
  phi->addIncomingBlock(two, val2);

  builder.createUnreachable();

  dawn::dumpModule(mod);

  std::cout << sizeof(dawn::Module);

  return 0;
}