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
#include "../utility/apint.h"
#include "./types.h"
#include "./value.h"
#include "absl/container/inlined_vector.h"
#include <concepts>
#include <variant>

namespace dawn {
  class DAWN_PUBLIC Constant : public Value {
  public:
    [[nodiscard]] static bool instanceOf(const Value* val) {
      return val->kind() >= ValueKind::constBegin && val->kind() <= ValueKind::constEnd;
    }

    [[nodiscard]] bool isZeroValue() const noexcept {
      return null_;
    }

  protected:
    template <typename T>
    constexpr explicit Constant(T* ptr, Type* ty, bool nullish) noexcept : Value(ptr, ty),
                                                                           null_{nullish} {}

  private:
    bool null_;
  };

  class DAWN_PUBLIC ConstantInt final : public Constant {
  public:
    inline constexpr static ValueKind kind = ValueKind::constInt;

    explicit ConstantInt(APInt value, Type* ty) noexcept : Constant(this, ty, value.value() == 0), value_{value} {}

    [[nodiscard]] APInt value() const noexcept {
      return value_;
    }

    [[nodiscard]] std::uint64_t realValue() const noexcept {
      return value_.value();
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;

  private:
    APInt value_;
  };

  class DAWN_PUBLIC ConstantBool final : public Constant {
  public:
    inline constexpr static ValueKind kind = ValueKind::constBool;

    explicit ConstantBool(class Module* mod, bool value) noexcept;

    [[nodiscard]] bool value() const noexcept {
      return value_;
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;

  private:
    bool value_;
  };

  class DAWN_PUBLIC ConstantFloat final : public Constant {
  public:
    inline constexpr static ValueKind kind = ValueKind::constFP;

    explicit ConstantFloat(Type* ty, double value) noexcept : Constant(this, ty, value == 0.0), value_{value} {}

    [[nodiscard]] double value() const noexcept {
      return value_;
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;

  private:
    double value_;
  };

  class DAWN_PUBLIC ConstantNull final : public Constant {
  public:
    inline constexpr static ValueKind kind = ValueKind::constNull;

    explicit ConstantNull(class Module* mod) noexcept;

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC ConstantArray final : public Constant {
  public:
    inline constexpr static ValueKind kind = ValueKind::constArray;

    explicit ConstantArray(class Module* mod, std::span<Constant* const> values) noexcept;

    [[nodiscard]] std::span<Constant* const> values() const noexcept {
      return values_;
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;

  private:
    absl::InlinedVector<Constant*, 3> values_; // 4 makes `ConstantArray` more than 64 bytes per object, unacceptable
  };

  class DAWN_PUBLIC ConstantStruct final : public Constant {
  public:
    inline constexpr static ValueKind kind = ValueKind::constArray;

    explicit ConstantStruct(Type* ty, std::span<Constant* const> values) noexcept : Constant(this, ty, false) {
      DAWN_ASSERT(isa<Struct>(ty), "type must be a struct for `ConstantStruct`");

      const auto* sty = dawn::dyncastUnchecked<Struct>(ty);
      auto fields = sty->fields();

      DAWN_ASSERT(values.size() == fields.size(), "must have an initializer for every struct field");

      values_.reserve(values.size());

      for (auto i = std::size_t{0}; i < fields.size(); ++i) {
        DAWN_ASSERT(fields[i] == values[i]->type(), "initializer must be the right type for that struct element");

        values_.push_back(values[i]);
      }
    }

    [[nodiscard]] std::span<Constant* const> values() const noexcept {
      return values_;
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;

  private:
    absl::InlinedVector<Constant*, 3> values_; // 4 makes `ConstantStruct` more than 64 bytes per object, unacceptable
  };

  class DAWN_PUBLIC ConstantUndef final : public Constant {
  public:
    inline constexpr static ValueKind kind = ValueKind::constUndef;

    explicit ConstantUndef(Type* ty) noexcept : Constant(this, ty, false) {}

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;
  };

  class DAWN_PUBLIC ConstantString final : public Constant {
  public:
    inline constexpr static ValueKind kind = ValueKind::constString;

    explicit ConstantString(class Module* mod, std::string content) noexcept;

    [[nodiscard]] std::string_view stringData() const noexcept {
      return real_;
    }

    [[nodiscard]] std::span<ConstantInt* const> asValues() const noexcept {
      return chars_;
    }

  protected:
    void hash(absl::HashState state) const noexcept final;

    bool equals(const Value* val) const noexcept final;

  private:
    // no reason to try to sso this, basically every string is larger than we can
    // reasonably fit in 8-byte per char sso. the struct is also already above the size
    // of one cache line, so at this point it doesn't matter
    std::vector<ConstantInt*> chars_;
    std::string real_;
  };
} // namespace dawn