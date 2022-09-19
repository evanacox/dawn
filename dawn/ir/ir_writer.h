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

#include "./basic_block.h"
#include "./constant.h"
#include "./instructions.h"
#include "./module.h"
#include "absl/container/flat_hash_map.h"
#include <string>

namespace dawn {
  class IRWriter {
  public:
    explicit IRWriter(const Module& mod) noexcept;

    [[nodiscard]] std::string stringifyModule() const noexcept;

    [[nodiscard]] std::string stringifySingleFn(const Function* fn) const noexcept;

    [[nodiscard]] std::string stringifySingleVal(const Value* val) const noexcept;

    [[nodiscard]] std::string stringifySingleTy(const Type* ty) const noexcept;

    [[nodiscard]] std::string stringifySingleBlock(const BasicBlock* bb) const noexcept;

  private:
    void labelFunction(const Function* fn) noexcept;

    void orderBlocks(const Function* fn, const BasicBlock* bb) noexcept;

    void labelBlock(const BasicBlock* bb) noexcept;

    void dumpFunction(const Function* fn, std::string* buffer) const noexcept;

    void dumpBlock(const BasicBlock* bb, std::string* buffer) const noexcept;

    void dumpValue(const Value* val, std::string* buffer) const noexcept;

    void dumpRefValue(const Value* val, std::string* buffer) const noexcept;

    void dumpRawValue(const Value* val, std::string* buffer) const noexcept;

    void dumpTy(const Type* ty, std::string* buffer) const noexcept;

    void dumpTyRef(const Value* val, std::string* buffer) const noexcept;

    absl::flat_hash_set<const Function*> labeled_;
    absl::flat_hash_map<const BasicBlock*, std::string> bbNames_;
    absl::flat_hash_map<const Value*, std::size_t> instNames_;
    absl::flat_hash_map<const Type*, std::string> namedTys_;
    absl::flat_hash_map<const Function*, std::vector<const BasicBlock*>> printOrder_;
    std::size_t currInst_ = 0;
  };

  [[nodiscard]] std::string stringifyModule(const Module& mod) noexcept;

  [[nodiscard]] std::string stringifySingleFn(const Module& mod, const Function* fn) noexcept;

  [[nodiscard]] std::string stringifySingleVal(const Module& mod, const Value* val) noexcept;

  [[nodiscard]] std::string stringifySingleTy(const Module& mod, const Type* ty) noexcept;

  void dumpModule(const Module& mod) noexcept;

  void dumpSingleFn(const Module& mod, const Function* fn) noexcept;

  void dumpSingleVal(const Module& mod, const Value* val) noexcept;

  void dumpSingleTy(const Module& mod, const Type* ty) noexcept;
} // namespace dawn