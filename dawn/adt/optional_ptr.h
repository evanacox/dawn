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

#include "../utility/assertions.h"
#include <cstddef>

namespace dawn {
  namespace internal {
    struct OptionalNone {};
  } // namespace internal

  /// @brief Either `None` or a non-null pointer to a given `T`.
  template <typename T> class OptionalPtr {
  public:
    [[nodiscard]] explicit OptionalPtr(internal::OptionalNone /*unused*/) noexcept {}

    [[nodiscard]] explicit OptionalPtr(T* ptr) noexcept : ptr_{ptr} {
      DAWN_ASSERT(ptr != nullptr, "OptionalPtr cannot be created with `nullptr`");
    }

    [[nodiscard]] explicit OptionalPtr(std::nullptr_t /*unused*/) = delete;

    [[nodiscard]] operator bool() const noexcept { // NOLINT(google-explicit-constructor)
      return ptr_ != nullptr;
    }

    [[nodiscard]] T* operator->() const noexcept {
      DAWN_ASSERT(*this, "cannot access `null` OptionalPtr");

      return ptr_;
    }

    [[nodiscard]] T& operator*() const noexcept {
      DAWN_ASSERT(*this, "cannot access `null` OptionalPtr");

      return *ptr_;
    }

  private:
    T* ptr_ = nullptr;
  };

  template <typename T> [[nodiscard]] OptionalPtr<T> none() noexcept {
    return OptionalPtr<T>(internal::OptionalNone{});
  }

  template <typename T> [[nodiscard]] OptionalPtr<T> some(T* ptr) noexcept {
    return OptionalPtr<T>(ptr);
  }

  template <typename T> [[nodiscard]] OptionalPtr<T> maybe(T* ptr) noexcept {
    return (ptr == nullptr) ? dawn::none<T>() : dawn::some<T>(ptr);
  }
} // namespace dawn
