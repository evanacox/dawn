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

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "dawn/ir/ir_reader.h"
#include "dawn/ir/ir_writer.h"
#include "dawn/utility/file.h"

ABSL_FLAG(std::string, name, "", "the filename to read");

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  auto data = dawn::readEntireFile(absl::GetFlag(FLAGS_name));
  auto mod = dawn::parseIRFromText(data.value_or(""));

  if (auto* err = std::get_if<std::string>(&mod)) {
    std::cerr << *err << '\n';
  } else {
    auto ptr = std::move(std::get<std::unique_ptr<dawn::Module>>(mod));

    dawn::dumpModule(*ptr);
  }
}