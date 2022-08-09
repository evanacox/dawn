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
    phi_inst,
    inst_begin = phi_inst,
    terminators_inst_begin = phi_inst,
    br_inst,
    cbr_inst,
    switch_inst,
    unreachable_inst,
    terminators_inst_end = unreachable_inst,
    and_inst,
    binary_inst_begin = and_inst,
    or_inst,
    xor_inst,
    shl_inst,
    lshr_inst,
    ashr_inst,
    iadd_inst,
    isub_inst,
    imul_inst,
    udiv_inst,
    sdiv_inst,
    urem_inst,
    srem_inst,
    fneg_inst,
    fadd_inst,
    fsub_inst,
    fmul_inst,
    fdiv_inst,
    frem_inst,
    binary_inst_end = frem_inst,
    load_inst,
    store_inst,
    offset_inst,
    extract_inst,
    insert_inst,
    ptrinto_inst,
    itob_inst,
    btoi_inst,
    itof_inst,
    ftoi_inst,
    itop_inst,
    ptoi_inst,
    inst_end = ptoi_inst,
    global,
    const_int,
    const_begin = const_int,
    const_fp,
    const_arr,
    const_struct,
    const_null,
    const_end = const_null
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

  private:
    ValueKind kind_;
    Type* ty_;
  };
} // namespace dawn