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
#include "./types.h"
#include "absl/hash/hash.h"

namespace dawn {
  class Module;

  enum class ValueKind {
    phiInst,
    instBegin = phiInst,
    callInst,
    selInst,
    brInst,
    terminatorsInstBegin = brInst,
    cbrInst,
    switchInst,
    retInst,
    unreachableInst,
    terminatorsInstEnd = unreachableInst,
    andInst,
    binaryInstBegin = andInst,
    orInst,
    xorInst,
    shlInst,
    lshrInst,
    ashrInst,
    iaddInst,
    isubInst,
    imulInst,
    udivInst,
    sdivInst,
    uremInst,
    sremInst,
    fnegInst,
    faddInst,
    fsubInst,
    fmulInst,
    fdivInst,
    fremInst,
    icmpInst,
    fcmpInst,
    binaryInstEnd = fcmpInst,
    allocaInst,
    loadInst,
    storeInst,
    offsetInst,
    extractInst,
    insertInst,
    elemptrInst,
    sextInst,
    conversionInstBegin = sextInst,
    zextInst,
    truncInst,
    itobInst,
    btoiInst,
    sitofInst,
    uitofInst,
    ftosiInst,
    ftouiInst,
    itopInst,
    ptoiInst,
    conversionInstEnd = ptoiInst,
    instEnd = ptoiInst,
    global,
    constInt,
    constBegin = constInt,
    constFP,
    constArray,
    constStruct,
    constNull,
    constBool,
    constUndef,
    constString,
    constEnd = constString,
    argument
  };

  class DAWN_PUBLIC Value {
  public:
    virtual ~Value() = default;

    [[nodiscard]] ValueKind kind() const noexcept {
      return kind_;
    }

    [[nodiscard]] Type* type() const noexcept {
      return ty_;
    }

    [[nodiscard]] Module* module() const noexcept {
      return ty_->module();
    }

    [[nodiscard]] friend bool operator==(const Value& val1, const Value& val2) noexcept {
      return val1.kind() == val2.kind() && val1.equals(&val2);
    }

    template <typename H> friend H AbslHashValue(H state, const Value& value) {
      value.hash(absl::HashState::Create(&state));

      return state;
    }

  protected:
    Value() = default;

    Value(const Value&) = default;

    Value(Value&&) = default;

    Value& operator=(const Value&) = default;

    Value& operator=(Value&&) = default;

    template <typename T> explicit Value(T* /*unused*/, Type* ty) noexcept : kind_{T::kind}, ty_{ty} {}

    virtual void hash(absl::HashState state) const noexcept = 0;

    // invariant: `val->kind() == this->kind()`
    [[nodiscard]] virtual bool equals(const Value* val) const noexcept = 0;

  private:
    ValueKind kind_;
    Type* ty_;
  };
} // namespace dawn