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

namespace dawn {
  template <typename T> class DerefHashable final {
  public:
    DerefHashable(T* object) noexcept : obj_{object} {} // NOLINT(google-explicit-constructor)

    template <typename H> friend H AbslHashValue(H state, const DerefHashable& object) {
      return H::combine(std::move(state), *object.obj_);
    }

    [[nodiscard]] operator T*() const noexcept { // NOLINT(google-explicit-constructor)
      return obj_;
    }

  private:
    T* obj_;
  };
} // namespace dawn