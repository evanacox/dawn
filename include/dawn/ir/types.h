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

/// @file
/// @brief Defines the C++ types that model Dawn IR types.
///
///

#pragma once

#include "../config.h"
#include "../utility/rtti.h"
#include "absl/hash/hash.h"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <memory>
#include <span>
#include <vector>

namespace dawn {
  namespace internal {
    enum class TypeKind { integer, floating_point, boolean, ptr, array, structure };
  } // namespace internal

  ///
  ///
  ///
  class DAWN_PUBLIC Type {
  public:
    virtual ~Type() = default;

    [[nodiscard]] internal::TypeKind kind() const noexcept {
      return kind_;
    }

    template <typename H> friend H AbslHashValue(H state, const Type& value) {
      value.hash(absl::HashState::Create(&state));

      return state;
    }

  protected:
    virtual void hash(absl::HashState state) const noexcept = 0;

    constexpr explicit Type(internal::TypeKind kind) noexcept : kind_{kind} {}

  private:
    internal::TypeKind kind_;
  };

  class DAWN_PUBLIC Int final : public Type {
  public:
    inline constexpr static internal::TypeKind kind = internal::TypeKind::integer;

    [[nodiscard]] constexpr explicit Int(std::uint64_t width) noexcept : Type(kind), width_{width} {
      assert("integers can only have powers of two as their size" && std::has_single_bit(width_));
      assert("integer width must be in the range [8, 64]" && 8 <= width && width <= 64);
    }

    ~Int() final = default;

    [[nodiscard]] constexpr std::uint64_t width() const noexcept {
      return width_;
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

  private:
    std::uint64_t width_;
  };

  class DAWN_PUBLIC Float final : public Type {
  public:
    inline constexpr static internal::TypeKind kind = internal::TypeKind::floating_point;

    [[nodiscard]] constexpr explicit Float(std::uint64_t width) noexcept : Type(kind), width_{width} {
      assert("only `binary32` and `binary64` IEEE floats are supported" && (width == 32 || width == 64));
    }

    ~Float() final = default;

    [[nodiscard]] constexpr std::uint64_t width() const noexcept {
      return width_;
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

  private:
    std::uint64_t width_;
  };

  class DAWN_PUBLIC Bool final : public Type {
  public:
    inline constexpr static internal::TypeKind kind = internal::TypeKind::boolean;

    [[nodiscard]] constexpr explicit Bool() noexcept : Type(kind) {}

    ~Bool() final = default;

  protected:
    void hash(absl::HashState state) const noexcept final;
  };

  class DAWN_PUBLIC Ptr final : public Type {
  public:
    inline constexpr static internal::TypeKind kind = internal::TypeKind::ptr;

    [[nodiscard]] constexpr explicit Ptr() noexcept : Type(kind) {}

    ~Ptr() final = default;

  protected:
    void hash(absl::HashState state) const noexcept final;
  };

  class DAWN_PUBLIC Array final : public Type {
  public:
    inline constexpr static internal::TypeKind kind = internal::TypeKind::array;

    [[nodiscard]] constexpr explicit Array(Type* element, std::uint64_t len) noexcept
        : Type(kind),
          element_{element},
          len_{len} {}

    ~Array() final = default;

    [[nodiscard]] constexpr Type* element() const noexcept {
      return element_;
    }

    [[nodiscard]] constexpr std::uint64_t len() const noexcept {
      return len_;
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

  private:
    Type* element_;
    std::uint64_t len_;
  };

  class DAWN_PUBLIC Struct final : public Type {
  public:
    inline constexpr static internal::TypeKind kind = internal::TypeKind::structure;

    [[nodiscard]] explicit Struct(std::vector<Type*> fields) noexcept;

    [[nodiscard]] explicit Struct(std::span<Type*> fields) noexcept;

    ~Struct() final = default;

    [[nodiscard]] std::span<Type* const> fields() const noexcept {
      return std::span{fields_.data(), fields_.size()};
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

  private:
    std::vector<Type*> fields_;
  };
} // namespace dawn