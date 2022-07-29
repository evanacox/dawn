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
#include <array>
#include <concepts>
#include <cstddef>
#include <memory>
#include <vector>

namespace dawn {
  namespace internal {
    template <typename T> class InPlaceDeleter {
    public:
      constexpr InPlaceDeleter() = default;

      template <typename U>
      constexpr InPlaceDeleter(const InPlaceDeleter<U>& /*unused*/) noexcept // NOLINT(google-explicit-constructor)
        requires std::convertible_to<U*, T*>
      {}

      constexpr void operator()(T* ptr) noexcept {
        std::destroy_at(ptr);
      }
    };
  } // namespace internal

  template <typename T> using BumpPtr = std::unique_ptr<T, internal::InPlaceDeleter<T>>;

  class BumpAlloc {
    inline constexpr static std::size_t chunk_size = 4096;

    using Chunk = std::array<std::byte, chunk_size>;

  public:
    BumpAlloc() = default;

    BumpAlloc(const BumpAlloc&) = delete;

    BumpAlloc(BumpAlloc&&) = default;

    BumpAlloc& operator=(const BumpAlloc&) = delete;

    BumpAlloc& operator=(BumpAlloc&&) = default;

    template <typename T, typename... Args>
    [[nodiscard]] BumpPtr<T> alloc(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
      if (offset_ + sizeof(T) >= BumpAlloc::chunk_size) {
        chunks_.push_back(std::make_unique<Chunk>());
        offset_ = 0;
      }

      auto& back = *chunks_.back();
      auto* ptr = reinterpret_cast<T*>(back.data() + offset_);
      offset_ += sizeof(T);

      return BumpPtr<T>(std::construct_at(ptr, std::forward<Args>(args)...));
    }

  private:
    std::vector<std::unique_ptr<Chunk>> chunks_; // chunks MUST have fixed addresses
    std::size_t offset_;
  };
} // namespace dawn