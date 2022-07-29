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
#include "./value.h"

namespace dawn {
  enum class InstKind {
    phi_inst,
    br_inst,
    cbr_inst,
    switch_inst,
    unreachable_inst,
    and_inst,
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
    ptoi_inst
  };
}