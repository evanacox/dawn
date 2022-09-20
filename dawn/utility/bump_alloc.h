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
#include "absl/memory/memory.h"
#include <array>
#include <concepts>
#include <cstddef>
#include <memory>
#include <vector>

namespace dawn {
  namespace internal {
    template <typename T> struct InPlaceDeleter {
      InPlaceDeleter() = default;

      template <typename U>
      InPlaceDeleter(const InPlaceDeleter<U>& /*unused*/) noexcept // NOLINT(google-explicit-constructor)
        requires std::convertible_to<U*, T*>
      {}

      void operator()(T* ptr) const noexcept {
        std::destroy_at(ptr);
      }
    };
  } // namespace internal

  template <typename T> using BumpPtr = std::unique_ptr<T, internal::InPlaceDeleter<T>>;

  class BumpAlloc {
    inline constexpr static std::size_t chunkSize = 4096;

    // we need to create the chunks aligned to max_align_t alignment so any objects
    // can be created at offset 0. at other offsets we handle alignment ourselves, but that
    // relies on offset 0 being properly aligned for any object
    inline constexpr static std::align_val_t alignment = std::align_val_t{alignof(std::max_align_t)};

    using Chunk = std::array<std::byte, chunkSize>;

  public:
    BumpAlloc() = default;

    BumpAlloc(const BumpAlloc&) = delete;

    BumpAlloc(BumpAlloc&&) = default;

    BumpAlloc& operator=(const BumpAlloc&) = delete;

    BumpAlloc& operator=(BumpAlloc&&) = default;

    ~BumpAlloc() {
      for (auto* chunk : chunks_) {
        // memory is allocated with custom alignment, we need to dealloc it the same way.
        // the chunks are trivial, and thus we have no need for calling destructor
        ::operator delete(static_cast<void*>(chunk), alignment);
      }
    }

    template <typename T, typename... Args>
    [[nodiscard]] BumpPtr<T> alloc(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
      // ensure alignment is kept correct for T
      offset_ += alignof(T) - (offset_ % alignof(T));

      if (offset_ + sizeof(T) >= BumpAlloc::chunkSize || chunks_.empty()) {
        // while `new (alignment) Chunk` exists, that causes a compile error under MSVC as of Sept. 20 2022.
        // See https://developercommunity.visualstudio.com/t/using-c17-new-stdalign-val-tn-syntax-results-in-er/528320
        auto* storage = ::operator new(sizeof(Chunk), alignment);
        auto* chunk = std::construct_at(static_cast<Chunk*>(storage));

        chunks_.push_back(chunk);
        offset_ = 0;
      }

      auto& back = *chunks_.back();
      auto* ptr = reinterpret_cast<T*>(back.data() + offset_);

      // sizeof(T) space is now taken up after we save the address for the new object
      offset_ += sizeof(T);

      return BumpPtr<T>(std::construct_at(ptr, std::forward<Args>(args)...));
    }

  private:
    std::vector<Chunk*> chunks_; // chunks MUST have fixed addresses
    std::size_t offset_;
  };
} // namespace dawn