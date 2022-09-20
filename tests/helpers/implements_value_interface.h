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

#include "absl/hash/hash_testing.h"
#include "dawn/ir/constant.h"
#include "dawn/ir/instructions.h"
#include "helpers/random.h"
#include "gtest/gtest.h"
#include <concepts>
#include <cstddef>

namespace dawn::tests {
  namespace internal {
    template <typename T, typename Real> void correctlyDoesRTTI(const dawn::Value* val) noexcept {
      // Real == T => isa should be true,
      // otherwise isa should be false
      EXPECT_EQ((std::same_as<Real, T>), dawn::isa<T>(val));
    }
  } // namespace internal

  // NOLINTNEXTLINE
  template <typename T> void implementsValueInterface(std::span<const T* const> values) noexcept {
    {
      // we need to actually have values for `absl::VerifyTypeImplementsAbslHaslCorrectly`
      // because if we pass in a span of pointers, it will check that the *pointers*
      // are hashable, not the things pointed to.
      //
      // every value should be copyable anyway, this implicitly enforces that as well
      auto copy = std::vector<T>{};

      for (const auto* val : values) {
        copy.push_back(*val);
      }

      EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly(copy));
    }

#define DAWN_INTERNAL_IF_DERIVED_EXPECT_ISA(ty)                                                                        \
  do {                                                                                                                 \
    if constexpr (std::derived_from<T, ty>) {                                                                          \
      EXPECT_TRUE(dawn::isa<ty>(static_cast<const dawn::Value*>(values[i])));                                          \
    }                                                                                                                  \
  } while (false)

    for (auto i = std::size_t{0}; i < values.size(); ++i) {
      for (auto j = std::size_t{0}; j < values.size(); ++j) {
        if (i != j) {
          EXPECT_NE(*values[i], *values[j]);
        }
      }

      DAWN_INTERNAL_IF_DERIVED_EXPECT_ISA(TerminatorInst);
      DAWN_INTERNAL_IF_DERIVED_EXPECT_ISA(BinaryInst);
      DAWN_INTERNAL_IF_DERIVED_EXPECT_ISA(ConversionInst);
      DAWN_INTERNAL_IF_DERIVED_EXPECT_ISA(Instruction);
      DAWN_INTERNAL_IF_DERIVED_EXPECT_ISA(Constant);

      EXPECT_EQ(*values[i], *values[i]);
    }

#define DAWN_INTERNAL_IMPLS_VALUE_DYN_CAST(ty) internal::correctlyDoesRTTI<dawn::ty, T>(val);

    // test that for each value, it only can be dyn-casted to T and no other type
    // technically overkill, but can't hurt to check it for all of them
    for (const auto* val : values) {
      DAWN_FOR_EACH_INST(DAWN_INTERNAL_IMPLS_VALUE_DYN_CAST);
    }
  }

  template <typename T> void implementsValueInterface(std::initializer_list<const T*> values) noexcept {
    implementsValueInterface<T>({values.begin(), values.size()});
  }
} // namespace dawn::tests