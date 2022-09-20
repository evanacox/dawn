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
#include <cstdint>

namespace dawn::internal {
  class DAWN_PUBLIC SourceLocation {
  public:
    // NOLINTNEXTLINE
    static constexpr SourceLocation current(const char* file = __builtin_FILE(),
        const char* func = __builtin_FUNCTION(),
        std::uint_least32_t line = __builtin_LINE()) noexcept {
      SourceLocation loc;

      loc.file_ = file;
      loc.func_ = func;
      loc.line_ = line;

      return loc;
    }

    constexpr SourceLocation() noexcept = default;

    [[nodiscard]] constexpr std::uint_least32_t line() const noexcept {
      return line_;
    }

    [[nodiscard]] constexpr const char* filename() const noexcept {
      return file_;
    }

    [[nodiscard]] constexpr const char* function() const noexcept {
      return func_;
    }

  private:
    const char* file_ = "<unknown>";
    const char* func_ = "<unknown>";
    uint_least32_t line_ = 0;
  };

  [[noreturn]] DAWN_PUBLIC void assertFail(const char* condition_string,
      const char* explanation,
      SourceLocation loc = SourceLocation::current()) noexcept;

  [[noreturn]] DAWN_PUBLIC void unreachable(const char* explanation,
      SourceLocation loc = SourceLocation::current()) noexcept;
} // namespace dawn::internal

#if defined _WIN32 || defined __CYGWIN__
#ifdef DAWN_DISABLE_ASSERTIONS
#define DAWN_ASSERT(cond, reason)                                                                                      \
  do {                                                                                                                 \
    if (!(cond)) {                                                                                                     \
      __assume(false);                                                                                                 \
    }                                                                                                                  \
  } while (false)
#define DAWN_UNREACHABLE(reason) __assume(false)
#else
#define DAWN_ASSERT(cond, reason)                                                                                      \
  do {                                                                                                                 \
    if (!(cond)) [[unlikely]] {                                                                                        \
      ::dawn::internal::assertFail(#cond, reason);                                                                     \
    }                                                                                                                  \
  } while (false)
#define DAWN_UNREACHABLE(reason) ::dawn::internal::unreachable(reason)
#endif
#else
#ifdef DAWN_DISABLE_ASSERTIONS
#define DAWN_ASSERT(cond, reason)                                                                                      \
  do {                                                                                                                 \
    if (!(cond)) [[unlikely]] {                                                                                        \
      __builtin_unreachable();                                                                                         \
    }                                                                                                                  \
  } while (false)
#define DAWN_UNREACHABLE(reason) __builtin_unreachable()
#else
#define DAWN_ASSERT(cond, reason)                                                                                      \
  do {                                                                                                                 \
    if (!(cond)) [[unlikely]] {                                                                                        \
      ::dawn::internal::assertFail((#cond), (reason));                                                                 \
    }                                                                                                                  \
  } while (false)
#define DAWN_UNREACHABLE(reason) ::dawn::internal::unreachable((reason))
#endif
#endif