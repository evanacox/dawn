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

#include <cassert>

#if defined _WIN32 || defined __CYGWIN__
#ifdef DAWN_BUILDING_LIBRARY
#define DAWN_PUBLIC __declspec(dllexport)
#else
#define DAWN_PUBLIC __declspec(dllimport)
#endif
#else
#ifdef DAWN_BUILDING_LIBRARY
#define DAWN_PUBLIC __attribute__((visibility("default")))
#else
#define DAWN_PUBLIC
#endif
#endif

namespace dawn {}