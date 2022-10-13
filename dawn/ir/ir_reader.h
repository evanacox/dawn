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

#include "./module.h"
#include <memory>
#include <ostream>
#include <variant>

namespace dawn {
  std::variant<std::unique_ptr<Module>, std::string> parseIRFromText(std::string_view source) noexcept;

  inline std::optional<std::unique_ptr<Module>> tryParseIR(std::string_view source, std::ostream* onError) noexcept {
    auto result = parseIRFromText(source);

    if (auto* err = std::get_if<std::string>(&result)) {
      *onError << err << '\n';

      return std::nullopt;
    }

    return std::make_optional(std::get<std::unique_ptr<Module>>(std::move(result)));
  }

  std::optional<std::string> parseIRIntoExistingModule(std::string_view source, Module* mod) noexcept;
} // namespace dawn