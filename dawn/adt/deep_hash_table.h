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

#include "../ir/value.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/hash/hash.h"
#include <concepts>
#include <utility>

namespace dawn {
  namespace internal {
    template <typename T> struct DerefAbslHasher {
      constexpr std::size_t operator()(const T& value) const noexcept {
        return absl::HashOf(*value);
      }
    };

    template <typename T, typename D> struct DerefAbslHasher<std::unique_ptr<T, D>> {
      using is_transparent = void;

      constexpr std::size_t operator()(const std::unique_ptr<T, D>& value) const noexcept {
        return absl::HashOf(*value);
      }

      constexpr std::size_t operator()(const T* value) const noexcept {
        return absl::HashOf(*value);
      }
    };

    template <typename T> struct DerefAbslHasher<std::shared_ptr<T>> {
      using is_transparent = void;

      constexpr std::size_t operator()(const std::shared_ptr<T>& value) const noexcept {
        return absl::HashOf(*value);
      }

      constexpr std::size_t operator()(const T* value) const noexcept {
        return absl::HashOf(*value);
      }
    };

    template <typename T> struct DerefEquality {
      constexpr bool operator()(const T& lhs, const T& rhs) const {
        return *lhs == *rhs;
      }
    };

    template <typename T, typename D> struct DerefEquality<std::unique_ptr<T, D>> {
      using is_transparent = void;

      constexpr std::size_t operator()(const std::unique_ptr<T, D>& lhs,
          const std::unique_ptr<T, D>& rhs) const noexcept {
        return *lhs == *rhs;
      }

      constexpr bool operator()(const T* lhs, const std::unique_ptr<T, D>& rhs) const {
        return *lhs == *rhs;
      }

      constexpr bool operator()(const std::unique_ptr<T, D>& lhs, const T* rhs) const {
        return *lhs == *rhs;
      }

      constexpr bool operator()(const T* lhs, const T* rhs) const {
        return *lhs == *rhs;
      }
    };

    template <typename T> struct DerefEquality<std::shared_ptr<T>> {
      using is_transparent = void;

      constexpr std::size_t operator()(const std::shared_ptr<T>& lhs, const std::shared_ptr<T>& rhs) const noexcept {
        return *lhs == *rhs;
      }

      constexpr bool operator()(const T* lhs, const std::shared_ptr<T>& rhs) const {
        return *lhs == *rhs;
      }

      constexpr bool operator()(const std::shared_ptr<T>& lhs, const T* rhs) const {
        return *lhs == *rhs;
      }

      constexpr bool operator()(const T* lhs, const T* rhs) const {
        return *lhs == *rhs;
      }
    };
  } // namespace internal

  template <typename K, typename V>
  using DeepHashMap = absl::flat_hash_map<K, V, internal::DerefAbslHasher<K>, internal::DerefEquality<K>>;

  template <typename V>
  using DeepHashSet = absl::flat_hash_set<V, internal::DerefAbslHasher<V>, internal::DerefEquality<V>>;
} // namespace dawn
