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

#include "dawn/utility/assertions.h"
#include "absl/strings/str_format.h"
#include <cstdlib>

namespace {
  [[noreturn]] void abort_fast() noexcept {
#if defined _WIN32 || defined __CYGWIN__
    __debugbreak();
    std::abort();
#else
    __builtin_trap();
#endif
  }
} // namespace

namespace dawn {
  void internal::
  
  assert_fail(const char* condition_string, const char* explanation, SourceLocation loc) noexcept {
    auto out = absl::StrFormat("[dawn] assertion failed: '%s'\n  explanation: '%s'\n  location: %s:%d (in %s)\n",
        condition_string,
        explanation,
        loc.file_name(),
        loc.line(),
        loc.function_name());

    std::fputs(out.c_str(), stderr);

    abort_fast();
  }

  void internal::debug_unreachable(const char* explanation, SourceLocation loc) noexcept {
    auto out = absl::StrFormat("[dawn] reached unreachable code!\n  explanation: '%s'\n  location: %s:%d (in %s)\n",
        explanation,
        loc.file_name(),
        loc.line(),
        loc.function_name());

    std::fputs(out.c_str(), stderr);

    abort_fast();
  }
} // namespace dawn