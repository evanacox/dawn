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
#include "../utility/apint.h"
#include "../utility/assertions.h"
#include "../utility/rtti.h"
#include "absl/hash/hash.h"
#include <algorithm>
#include <bit>
#include <cstddef>
#include <memory>
#include <span>
#include <vector>

namespace dawn {
  class Module;

  namespace internal {
    enum class TypeKind { integer, floatingPoint, boolean, ptr, array, structure, voidUnit };
  } // namespace internal

  class DAWN_PUBLIC Type {
  public:
    virtual ~Type() = default;

    [[nodiscard]] internal::TypeKind kind() const noexcept {

      return kind_;
    }

    [[nodiscard]] Module* module() const noexcept {
      return mod_;
    }

    template <typename H> friend H AbslHashValue(H state, const Type& value) {
      value.hash(absl::HashState::Create(&state));

      return state;
    }

    [[nodiscard]] friend bool operator==(const Type& ty1, const Type& ty2) noexcept {
      return ty1.kind() == ty2.kind() && ty1.equals(&ty2);
    }

  protected:
    template <typename T> constexpr explicit Type(T* /*unused*/) noexcept : kind_{T::kind} {}

    virtual void hash(absl::HashState state) const noexcept = 0;

    // invariant: `ty->kind() == this->kind()`
    [[nodiscard]] virtual bool equals(const Type* ty) const noexcept = 0;

  private:
    Module* mod_;
    internal::TypeKind kind_;
  };

  class DAWN_PUBLIC Int final : public Type {
  public:
    inline constexpr static internal::TypeKind kind = internal::TypeKind::integer;

    [[nodiscard]] constexpr explicit Int(std::uint64_t width) noexcept : Type(this), width_{width} {
      DAWN_ASSERT(std::has_single_bit(width_), "integers can only have powers of two as their size");
      DAWN_ASSERT(width >= 8 && width <= 64, "integer width must be in the range [8, 64]");
    }

    [[nodiscard]] constexpr std::uint64_t rawWidth() const noexcept {
      return width_;
    }

    [[nodiscard]] constexpr Width width() const noexcept {
      return static_cast<Width>(width_);
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Type* ty) const noexcept final;

  private:
    std::uint64_t width_;
  };

  class DAWN_PUBLIC Float final : public Type {
  public:
    inline constexpr static internal::TypeKind kind = internal::TypeKind::floatingPoint;

    [[nodiscard]] constexpr explicit Float(std::uint64_t width) noexcept : Type(this), width_{width} {
      DAWN_ASSERT(width == 32 || width == 64, "only `binary32` and `binary64` IEEE floats are supported");
    }

    [[nodiscard]] constexpr std::uint64_t width() const noexcept {
      return width_;
    }

  protected:
    void hash(absl::HashState state) const noexcept final;
    bool equals(const Type* ty) const noexcept final;

  private:
    std::uint64_t width_;
  };

  class DAWN_PUBLIC Bool final : public Type {
  public:
    inline constexpr static internal::TypeKind kind = internal::TypeKind::boolean;

    [[nodiscard]] constexpr explicit Bool() noexcept : Type(this) {}

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Type* ty) const noexcept final;
  };

  class DAWN_PUBLIC Ptr final : public Type {
  public:
    inline constexpr static internal::TypeKind kind = internal::TypeKind::ptr;

    [[nodiscard]] constexpr explicit Ptr() noexcept : Type(this) {}

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Type* ty) const noexcept final;
  };

  class DAWN_PUBLIC Array final : public Type {
  public:
    inline constexpr static internal::TypeKind kind = internal::TypeKind::array;

    [[nodiscard]] constexpr explicit Array(Type* element, std::uint64_t len) noexcept
        : Type(this),
          element_{element},
          len_{len} {}

    [[nodiscard]] constexpr Type* element() const noexcept {
      return element_;
    }

    [[nodiscard]] constexpr std::uint64_t len() const noexcept {
      return len_;
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Type* ty) const noexcept final;

  private:
    Type* element_;
    std::uint64_t len_;
  };

  class DAWN_PUBLIC Struct final : public Type {
  public:
    inline constexpr static internal::TypeKind kind = internal::TypeKind::structure;

    [[nodiscard]] explicit Struct(std::vector<Type*> fields) noexcept;

    [[nodiscard]] explicit Struct(std::span<Type* const> fields) noexcept;

    [[nodiscard]] std::span<Type* const> fields() const noexcept {
      return std::span{fields_.data(), fields_.size()};
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Type* ty) const noexcept final;

  private:
    std::vector<Type*> fields_;
  };

  class DAWN_PUBLIC Void final : public Type {
  public:
    inline constexpr static internal::TypeKind kind = internal::TypeKind::voidUnit;

    [[nodiscard]] constexpr Void() noexcept : Type(this) {}

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Type* ty) const noexcept final;
  };
} // namespace dawn