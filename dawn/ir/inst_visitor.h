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

#include "./instructions.h"

namespace dawn {
  template <typename Derived, typename Ret = void> class InstVisitor {
  public:
    void visit(std::span<Instruction* const> instructions) noexcept(noexcept(visit(std::declval<Instruction*>()))) {
      static_cast<Derived*>(this)->visitRange(instructions);

      for (auto* inst : instructions) {
        visit(inst);
      }
    }

    void visitRange(std::span<Instruction* const> /*unused*/) noexcept(noexcept(visit(std::declval<Instruction*>()))) {}

    Ret visit(Instruction* instruction) {
      switch (instruction->kind()) {
#define DAWN_INTERNAL_INST_VISITOR_CASE(ty)                                                                            \
  case ty::kind: return static_cast<Derived*>(this)->visit##ty(dawn::dyncastUnchecked<ty>(instruction));

        DAWN_FOR_EACH_INST(DAWN_INTERNAL_INST_VISITOR_CASE)
        default: DAWN_UNREACHABLE("unknown instruction type");
      }
    }

    void visitInstruction(Instruction* /*unused*/) noexcept {}

    Ret visitTerminator(TerminatorInst* terminator) noexcept(
        noexcept(static_cast<Derived*>(this)->visitInstruction(terminator))) {
      return static_cast<Derived*>(this)->visitInstruction(terminator);
    }

    Ret visitBinary(BinaryInst* binary) noexcept(noexcept(static_cast<Derived*>(this)->visitInstruction(binary))) {
      return static_cast<Derived*>(this)->visitInstruction(binary);
    }

    Ret visitConversion(ConversionInst* conversion) noexcept(
        noexcept(static_cast<Derived*>(this)->visitInstruction(conversion))) {
      return static_cast<Derived*>(this)->visitInstruction(conversion);
    }

#define DAWN_INTERNAL_INST_VISITOR_FN(ty)                                                                              \
  Ret visit##ty(ty* inst) noexcept(noexcept(delegate(inst))) { return delegate(inst); }

    DAWN_FOR_EACH_INST(DAWN_INTERNAL_INST_VISITOR_FN)

  private:
    Ret delegate(BinaryInst* inst) noexcept(noexcept(static_cast<Derived*>(this)->visitBinary(inst))) {
      return static_cast<Derived*>(this)->visitBinary(inst);
    }

    Ret delegate(ConversionInst* inst) noexcept(noexcept(static_cast<Derived*>(this)->visitConversion(inst))) {
      return static_cast<Derived*>(this)->visitConversion(inst);
    }

    Ret delegate(TerminatorInst* inst) noexcept(noexcept(static_cast<Derived*>(this)->visitTerminator(inst))) {
      return static_cast<Derived*>(this)->visitTerminator(inst);
    }

    Ret delegate(Instruction* inst) noexcept(noexcept(static_cast<Derived*>(this)->visitInstruction(inst))) {
      return static_cast<Derived*>(this)->visitInstruction(inst);
    }
  };
} // namespace dawn