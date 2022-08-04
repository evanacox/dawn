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

#include "absl/hash/hash.h"
#include <cstddef>
#include <utility>

namespace dawn::internal {
  template <typename T, typename Hasher> class CachedHash {
  public:
    template <typename... Args>
    /*implicit*/ CachedHash(Args&&... args) // NOLINT(google-explicit-constructor)
        : object_{std::forward<Args>(args)...} {
      hash_ = Hasher{}(object_);
    }

    template <typename H> friend H AbslHashValue(H h, const CachedHash& c) {
      return H::combine(std::move(h), c.hash_);
    }

  private:
    std::size_t hash_;
    T object_;
  };
} // namespace dawn::internal