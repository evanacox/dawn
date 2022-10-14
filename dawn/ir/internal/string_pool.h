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

#include "absl/container/flat_hash_set.h"
#include <optional>
#include <string>
#include <string_view>

namespace dawn::internal {
  class InternedString {
  public:
    explicit InternedString(const char* str) noexcept : data_{str} {}

    [[nodiscard]] constexpr const char* data() const noexcept {
      return data_;
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept {
      return std::char_traits<char>::length(data_);
    }

    [[nodiscard]] constexpr std::string_view str() const noexcept { // NOLINT(google-explicit-constructor)
      return {data_};
    }

  private:
    const char* data_;
  };

  class MaybeInternedString {
  public:
    explicit MaybeInternedString(InternedString str) noexcept : data_{str.data()} {}

    explicit MaybeInternedString(std::nullptr_t /*unused*/) noexcept : data_{nullptr} {}

    [[nodiscard]] constexpr std::optional<const char*> data() const noexcept {
      return (data_ == nullptr) ? std::nullopt : std::make_optional(data_);
    }

    [[nodiscard]] constexpr std::optional<std::size_t> size() const noexcept {
      return (data_ == nullptr) ? std::nullopt : std::make_optional(std::char_traits<char>::length(data_));
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    [[nodiscard]] constexpr std::optional<std::string_view> str() const noexcept {
      return (data_ == nullptr) ? std::nullopt : std::make_optional(std::string_view{data_});
    }

  private:
    const char* data_;
  };

  static_assert(sizeof(InternedString) == sizeof(MaybeInternedString) && sizeof(InternedString) == sizeof(const char*));

  class StringPool {
  public:
    explicit StringPool() = default;

    [[nodiscard]] InternedString intern(std::string str) noexcept;

  private:
    // we need pointer stability since we distribute pointers into these strings.
    absl::flat_hash_set<std::unique_ptr<std::string>> cache_;
  };
} // namespace dawn::internal