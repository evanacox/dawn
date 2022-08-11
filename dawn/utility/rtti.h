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

#include "../adt/optional_ptr.h"
#include <concepts>

namespace dawn {
  namespace internal {
    template <typename Base, typename Derived>
    concept RTTIInstanceOf = requires(const Base* base, const Derived* derived) {
                               { Derived::instanceOf(base) } -> std::convertible_to<bool>;
                             };

    template <typename Base, typename Derived>
    concept RTTIAssociatedKey = requires(const Base* base, const Derived* derived) {
                                  { Derived::kind == base->kind() } -> std::convertible_to<bool>;
                                };

    template <typename Base, typename Derived>
    concept RTTICompatible = std::derived_from<Derived, Base>
                             && (internal::RTTIInstanceOf<Base, Derived> || internal::RTTIAssociatedKey<Base, Derived>);
  } // namespace internal

  template <typename Derived, typename Base>
  [[nodiscard]] bool isa(Base* ptr) noexcept
    requires internal::RTTICompatible<Base, Derived>
  {
    static_assert(!std::same_as<Derived, Base>, "should not be doing `isa` on type check known at compile time");

    if constexpr (internal::RTTIInstanceOf<Base, Derived>) {
      return Derived::instanceOf(ptr);
    } else {
      return Derived::kind == ptr->kind();
    }
  }

  template <typename Derived, typename Base>
  [[nodiscard]] OptionalPtr<Derived> dyncast(Base* ptr) noexcept
    requires internal::RTTICompatible<Base, Derived>
  {
    return dawn::isa<Derived>(ptr) ? dawn::some(static_cast<Derived*>(ptr)) : dawn::none<Derived>();
  }

  template <typename Derived, typename Base>
  [[nodiscard]] Derived* dyncastUnchecked(Base* ptr) noexcept
    requires internal::RTTICompatible<Base, Derived>
  {
    DAWN_ASSERT(dawn::isa<Derived>(ptr), "dawn::dyncastUnchecked: was not an instance of Derived");

    return static_cast<Derived*>(ptr);
  }
} // namespace dawn
