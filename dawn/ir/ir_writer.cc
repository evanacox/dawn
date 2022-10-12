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

#include "dawn/ir/ir_writer.h"
#include "absl/cleanup/cleanup.h"
#include "absl/strings/str_cat.h"
#include <algorithm>
#include <charconv>
#include <iostream>
#include <sstream>

namespace {
  bool shouldBeLabeled(const dawn::Instruction* inst) noexcept {
    return !dawn::isa<dawn::Void>(inst->type());
  }
} // namespace

namespace dawn {
  IRWriter::IRWriter(const Module& mod) noexcept {
    for (const auto& [_, fn] : mod.allFunctions()) {
      labelFunction(fn.get());
    }
  }

  void IRWriter::labelFunction(const Function* fn) noexcept {
    currInst_ = 0;
    labeled_.insert(fn);

    for (const auto& arg : fn->args()) {
      instNames_[&arg] = currInst_++;
    }

    if (fn->opaque()) {
      return;
    }

    auto currBlock = 0;

    for (const auto& block : fn->blocks()) {
      auto name = std::string{};

      if (auto declaredName = block.name()) {
        name = std::string{declaredName.value()};
      } else if (&block == fn->entry().get()) {
        name = "entry";
      } else {
        name = absl::StrCat("bb", currBlock++);
      }

      bbNames_[&block] = std::move(name);

      labelBlock(&block);
    }

    if (auto entry = fn->entry()) {
      orderBlocks(fn, entry.get());
    }
  }

  void IRWriter::labelBlock(const BasicBlock* bb) noexcept {
    for (const auto* inst : bb->instructions()) {
      if (shouldBeLabeled(inst)) {
        instNames_[inst] = currInst_++;
      }
    }
  }

  void IRWriter::orderBlocks(const dawn::Function* fn, const dawn::BasicBlock* bb) noexcept {
    (void)fn;
    (void)bb;
    (void)this;

    //    DAWN_UNREACHABLE("todo");
  }

  std::string IRWriter::stringifyModule() const noexcept {
    std::string buffer;

    for (const auto* fn : labeled_) {
      dumpFunction(fn, &buffer);
      absl::StrAppend(&buffer, "\n\n");
    }

    return buffer;
  }

  std::string IRWriter::stringifySingleFn(const Function* fn) const noexcept {
    std::string buffer;

    dumpFunction(fn, &buffer);

    return buffer;
  }

  std::string IRWriter::stringifySingleBlock(const BasicBlock* bb) const noexcept {
    std::string buffer;

    dumpBlock(bb, &buffer);

    return buffer;
  }

  std::string IRWriter::stringifySingleVal(const Value* val) const noexcept {
    std::string buffer;

    dumpValue(val, &buffer);

    return buffer;
  }

  std::string IRWriter::stringifySingleTy(const Type* ty) const noexcept {
    std::string buffer;

    dumpTy(ty, &buffer);

    return buffer;
  }

  void IRWriter::dumpFunction(const dawn::Function* fn, std::string* buffer) const noexcept {
    if (fn->opaque()) {
      absl::StrAppend(buffer, "decl @", fn->name(), "(");
    } else {
      absl::StrAppend(buffer, "func @", fn->name(), "(");
    }

    auto args = fn->args();

    for (auto it = args.begin(); it != args.end(); ++it) {
      dumpTy(it->type(), buffer);

      absl::StrAppend(buffer, " $", instNames_.at(&*it));

      if (it != std::prev(args.end())) {
        absl::StrAppend(buffer, ", ");
      }
    }

    absl::StrAppend(buffer, ")");

    if (fn->opaque()) {
      return;
    }

    absl::StrAppend(buffer, " {");

    for (const auto& bb : fn->blocks()) {
      absl::StrAppend(buffer, "\n");
      dumpBlock(&bb, buffer);
    }

    absl::StrAppend(buffer, "}");
  }

  void IRWriter::dumpBlock(const dawn::BasicBlock* bb, std::string* buffer) const noexcept {
    absl::StrAppend(buffer, "%", bbNames_.at(bb), ":\n");

    for (const auto* inst : bb->instructions()) {
      dumpValue(inst, buffer);
    }
  }

  // NOLINTNEXTLINE(misc-no-recursion, readability-function-cognitive-complexity)
  void IRWriter::dumpRawValue(const dawn::Value* val, std::string* buffer) const noexcept {
    switch (val->kind()) {
      case ValueKind::phiInst: {
        const auto* phi = dyncastUnchecked<const Phi>(val);

        absl::StrAppend(buffer, "phi ");
        dumpTy(phi->type(), buffer);
        absl::StrAppend(buffer, " ");

        auto indentDist = std::distance(std::find(buffer->rbegin(), buffer->rend(), '\n').base(), buffer->end());
        auto incoming = phi->incoming();

        for (auto it = incoming.begin(); it != incoming.end(); ++it) {
          const auto& [bb, incomingVal] = *it;

          if (it != incoming.begin()) {
            absl::StrAppend(buffer, ",\n", std::string(static_cast<std::size_t>(indentDist), ' '));
          }

          absl::StrAppend(buffer, "[ ");
          dumpRefValue(incomingVal, buffer);
          absl::StrAppend(buffer, ", %", bbNames_.at(bb), " ]");
        }

        // we've been doing our own weird newline formatting, don't need an extra one
        return;
      }
      case ValueKind::callInst: {
        const auto* call = dyncastUnchecked<const Call>(val);
        absl::StrAppend(buffer, "call ");
        dumpTy(call->target()->returnTy(), buffer);
        absl::StrAppend(buffer, " @", call->target()->name(), "(");

        auto args = call->args();
        for (auto it = args.begin(); it != args.end(); ++it) {
          dumpTyRef(*it, buffer);

          if (it + 1 != args.end()) {
            absl::StrAppend(buffer, ", ");
          }
        }

        absl::StrAppend(buffer, ")");
        break;
      }
      case ValueKind::selInst: {
        const auto* sel = dyncastUnchecked<const Sel>(val);
        absl::StrAppend(buffer, "sel ");
        dumpTy(val->type(), buffer);
        absl::StrAppend(buffer, ", bool ");
        dumpRefValue(sel->cond(), buffer);
        absl::StrAppend(buffer, ", if ");
        dumpRefValue(sel->ifTrue(), buffer);
        absl::StrAppend(buffer, ", else ");
        dumpRefValue(sel->ifFalse(), buffer);
        break;
      }
      case ValueKind::icmpInst: {
        const auto* cmp = dyncastUnchecked<const ICmp>(val);

        absl::StrAppend(buffer, "icmp ");

        switch (cmp->order()) {
          case ICmpOrdering::eq: absl::StrAppend(buffer, "eq "); break;
          case ICmpOrdering::ne: absl::StrAppend(buffer, "ne "); break;
          case ICmpOrdering::ult: absl::StrAppend(buffer, "ult "); break;
          case ICmpOrdering::ugt: absl::StrAppend(buffer, "ugt "); break;
          case ICmpOrdering::ule: absl::StrAppend(buffer, "ule "); break;
          case ICmpOrdering::uge: absl::StrAppend(buffer, "uge "); break;
          case ICmpOrdering::slt: absl::StrAppend(buffer, "slt "); break;
          case ICmpOrdering::sgt: absl::StrAppend(buffer, "sgt "); break;
          case ICmpOrdering::sle: absl::StrAppend(buffer, "sle "); break;
          case ICmpOrdering::sge: absl::StrAppend(buffer, "sge "); break;
        }

        dumpTyRef(cmp->lhs(), buffer);
        absl::StrAppend(buffer, ", ");
        dumpRefValue(cmp->rhs(), buffer);
        break;
      }
      case ValueKind::fcmpInst: {
        const auto* cmp = dyncastUnchecked<const FCmp>(val);

        absl::StrAppend(buffer, "fcmp ");

        switch (cmp->order()) {
          case FCmpOrdering::ord: absl::StrAppend(buffer, "ord "); break;
          case FCmpOrdering::uno: absl::StrAppend(buffer, "uno "); break;
          case FCmpOrdering::oeq: absl::StrAppend(buffer, "oeq "); break;
          case FCmpOrdering::one: absl::StrAppend(buffer, "one "); break;
          case FCmpOrdering::ogt: absl::StrAppend(buffer, "ogt "); break;
          case FCmpOrdering::olt: absl::StrAppend(buffer, "olt "); break;
          case FCmpOrdering::oge: absl::StrAppend(buffer, "oge "); break;
          case FCmpOrdering::ole: absl::StrAppend(buffer, "ole "); break;
          case FCmpOrdering::ueq: absl::StrAppend(buffer, "oeq "); break;
          case FCmpOrdering::une: absl::StrAppend(buffer, "une "); break;
          case FCmpOrdering::ugt: absl::StrAppend(buffer, "ugt "); break;
          case FCmpOrdering::ult: absl::StrAppend(buffer, "ult "); break;
          case FCmpOrdering::uge: absl::StrAppend(buffer, "uge "); break;
          case FCmpOrdering::ule: absl::StrAppend(buffer, "ule "); break;
        }

        dumpTyRef(cmp->lhs(), buffer);
        absl::StrAppend(buffer, ", ");
        dumpRefValue(cmp->rhs(), buffer);
        break;
      }
      case ValueKind::brInst: {
        const auto* inst = dyncastUnchecked<const Br>(val);
        absl::StrAppend(buffer, "br %", bbNames_.at(inst->target()));
        break;
      }
      case ValueKind::cbrInst: {
        const auto* inst = dyncastUnchecked<const CondBr>(val);
        absl::StrAppend(buffer, "cbr bool ");
        dumpRefValue(inst->cond(), buffer);
        absl::StrAppend(buffer,
            ", if %",
            bbNames_.at(inst->trueBranch()),
            ", else %",
            bbNames_.at(inst->falseBranch()));
        break;
      }
      case ValueKind::retInst: {
        const auto* ret = dyncastUnchecked<const Ret>(val);
        absl::StrAppend(buffer, "ret ");

        if (auto ptr = ret->returnValue()) {
          dumpTyRef(ptr.get(), buffer);
        } else {
          absl::StrAppend(buffer, "void");
        }

        break;
      }
      case ValueKind::switchInst: break; // NOLINT
      case ValueKind::unreachableInst: absl::StrAppend(buffer, "unreachable"); break;
#define BINARY_INST(ty, name)                                                                                          \
  do {                                                                                                                 \
    const auto* inst = dyncastUnchecked<const ty>(val);                                                                \
    absl::StrAppend(buffer, (name), " ");                                                                              \
    dumpTyRef(inst->lhs(), buffer);                                                                                    \
    absl::StrAppend(buffer, ", ");                                                                                     \
    dumpRefValue(inst->rhs(), buffer);                                                                                 \
  } while (false)

      case ValueKind::andInst: BINARY_INST(And, "and"); break; // NOLINT
      case ValueKind::orInst: BINARY_INST(Or, "or"); break;
      case ValueKind::xorInst: BINARY_INST(Xor, "xor"); break;
      case ValueKind::shlInst: BINARY_INST(Shl, "shl"); break;
      case ValueKind::lshrInst: BINARY_INST(LShr, "lshr"); break;
      case ValueKind::ashrInst: BINARY_INST(AShr, "ashr"); break;
      case ValueKind::iaddInst: BINARY_INST(IAdd, "iadd"); break;
      case ValueKind::isubInst: BINARY_INST(ISub, "isub"); break;
      case ValueKind::imulInst: BINARY_INST(IMul, "imul"); break; // NOLINT
      case ValueKind::udivInst: BINARY_INST(UDiv, "udiv"); break;
      case ValueKind::sdivInst: BINARY_INST(SDiv, "sdiv"); break;
      case ValueKind::uremInst: BINARY_INST(URem, "urem"); break;
      case ValueKind::sremInst: BINARY_INST(SRem, "srem"); break;
      case ValueKind::fnegInst: BINARY_INST(FNeg, "fneg"); break;
      case ValueKind::faddInst: BINARY_INST(FAdd, "fadd"); break;
      case ValueKind::fsubInst: BINARY_INST(FSub, "fsub"); break;
      case ValueKind::fmulInst: BINARY_INST(FMul, "fmul"); break;
      case ValueKind::fdivInst: BINARY_INST(FDiv, "fdiv"); break;
      case ValueKind::fremInst: BINARY_INST(FRem, "frem"); break;
      case ValueKind::loadInst: {
        const auto* load = dyncastUnchecked<const Load>(val);
        absl::StrAppend(buffer, "load ");

        if (load->isVolatile()) {
          absl::StrAppend(buffer, "volatile ");
        }

        dumpTy(load->type(), buffer);
        absl::StrAppend(buffer, ", ptr ");
        dumpRefValue(load->target(), buffer);
        break;
      }
      case ValueKind::storeInst: {
        const auto* store = dyncastUnchecked<const Store>(val);
        absl::StrAppend(buffer, "store ");

        if (store->isVolatile()) {
          absl::StrAppend(buffer, "volatile ");
        }

        dumpTyRef(store->value(), buffer);
        absl::StrAppend(buffer, ", ");
        dumpTyRef(store->target(), buffer);
        break;
      }
      case ValueKind::allocaInst: {
        const auto* inst = dyncastUnchecked<const Alloca>(val);

        absl::StrAppend(buffer, "alloca ");
        dumpTy(inst->allocatingTy(), buffer);

        if (auto ptr = dyncast<const ConstantInt>(inst->numberOfObjects()); ptr->realValue() == 1) {
          break;
        }

        absl::StrAppend(buffer, ", ");
        dumpRefValue(inst->numberOfObjects(), buffer);
        break;
      }
      case ValueKind::offsetInst: {
        const auto* inst = dyncastUnchecked<const Offset>(val);

        absl::StrAppend(buffer, "index ");
        dumpTy(inst->offsetTy(), buffer);
        absl::StrAppend(buffer, ", ");
        dumpTyRef(inst->ptr(), buffer);
        absl::StrAppend(buffer, ", ");
        dumpTyRef(inst->offset(), buffer);
        break;
      }
      case ValueKind::extractInst: {
        const auto* inst = dyncastUnchecked<const Extract>(val);

        absl::StrAppend(buffer, "extract ");
        dumpTyRef(inst->aggregate(), buffer);
        absl::StrAppend(buffer, ", ");
        dumpTyRef(inst->index(), buffer);
        break;
      }
      case ValueKind::insertInst: {
        const auto* inst = dyncastUnchecked<const Insert>(val);

        absl::StrAppend(buffer, "insert ");
        dumpTyRef(inst->aggregate(), buffer);
        absl::StrAppend(buffer, ", ");
        dumpTyRef(inst->value(), buffer);
        absl::StrAppend(buffer, ", ");
        dumpTyRef(inst->index(), buffer);
        break;
      }
      case ValueKind::elemptrInst: {
        const auto* inst = dyncastUnchecked<const ElemPtr>(val);

        absl::StrAppend(buffer, "elemptr ");
        dumpTy(inst->aggregateTy(), buffer);
        absl::StrAppend(buffer, ", ");
        dumpTyRef(inst->ptr(), buffer);
        absl::StrAppend(buffer, ", ");
        dumpTyRef(inst->index(), buffer);
        break;
      }
#define CONVERSION_INST(Ty, name)                                                                                      \
  do {                                                                                                                 \
    const auto* inst = dyncastUnchecked<const Ty>(val);                                                                \
    absl::StrAppend(buffer, name " ");                                                                                 \
    dumpTy(inst->into(), buffer);                                                                                      \
    absl::StrAppend(buffer, ", ");                                                                                     \
    dumpTyRef(inst->from(), buffer);                                                                                   \
  } while (false)
      case ValueKind::sextInst: CONVERSION_INST(Sext, "sext"); break;
      case ValueKind::zextInst: CONVERSION_INST(Zext, "zext"); break;
      case ValueKind::truncInst: CONVERSION_INST(Trunc, "trunc"); break;
      case ValueKind::itobInst: CONVERSION_INST(IToB, "itob"); break;
      case ValueKind::btoiInst: CONVERSION_INST(BToI, "btoi"); break;
      case ValueKind::sitofInst: CONVERSION_INST(SIToF, "sitof"); break;
      case ValueKind::uitofInst: CONVERSION_INST(UIToF, "uitof"); break;
      case ValueKind::ftosiInst: CONVERSION_INST(FToSI, "ftosi"); break;
      case ValueKind::ftouiInst: CONVERSION_INST(FToUI, "ftoui"); break;
      case ValueKind::itopInst: CONVERSION_INST(IToP, "itop"); break;
      case ValueKind::ptoiInst: CONVERSION_INST(PToI, "ptoi"); break;
      case ValueKind::global: break;
      case ValueKind::constInt:
        absl::StrAppend(buffer, dyncastUnchecked<const ConstantInt>(val)->value().value()); // lol
        break;
      case ValueKind::constFP: {
        auto value = dyncastUnchecked<const ConstantFloat>(val)->value();
        // absl::StrAppend(buffer, "0xfp", absl::Hex(absl::bit_cast<std::uint64_t>(value), absl::kZeroPad16));
        absl::StrAppend(buffer, value);
        break;
      }
      case ValueKind::constArray: {
        const auto* array = dyncastUnchecked<const ConstantArray>(val);
        auto members = array->values();
        absl::StrAppend(buffer, "[");

        for (auto i = std::size_t{0}; i < members.size(); ++i) {
          dumpRefValue(members[i], buffer);

          if (i + 1 != members.size()) {
            absl::StrAppend(buffer, ", ");
          }
        }

        absl::StrAppend(buffer, "]");
        break;
      }
      case ValueKind::constStruct: {
        const auto* array = dyncastUnchecked<const ConstantStruct>(val);
        auto members = array->values();
        absl::StrAppend(buffer, "{ ");

        for (auto i = std::size_t{0}; i < members.size(); ++i) {
          dumpRefValue(members[i], buffer);

          if (i + 1 != members.size()) {
            absl::StrAppend(buffer, ", ");
          }
        }

        absl::StrAppend(buffer, " }");
        break;
      }
      case ValueKind::constNull: absl::StrAppend(buffer, "null"); break;
      case ValueKind::constBool:
        absl::StrAppend(buffer, dyncastUnchecked<const ConstantBool>(val)->value() ? "true" : "false");
        break;
      case ValueKind::constString:
        absl::StrAppend(buffer, "\"", dyncastUnchecked<const ConstantString>(val)->stringData(), "\"");
        break;
      case ValueKind::constUndef: absl::StrAppend(buffer, "undef"); break;
      case ValueKind::argument: absl::StrAppend(buffer, "$", instNames_.at(val)); break;
    }
  } // namespace dawn

  void IRWriter::dumpValue(const dawn::Value* val, std::string* buffer) const noexcept { // NOLINT(misc-no-recursion)
    absl::StrAppend(buffer, "    ");

    if (auto it = instNames_.find(val); it != instNames_.end()) {
      absl::StrAppend(buffer, "$", it->second, " = ");
    }

    dumpRawValue(val, buffer);
    absl::StrAppend(buffer, "\n");
  } // namespace dawn

  void IRWriter::dumpRefValue(const dawn::Value* val, std::string* buffer) const noexcept { // NOLINT(misc-no-recursion)
    if (isa<Instruction>(val) || isa<Argument>(val)) {
      absl::StrAppend(buffer, "$", instNames_.at(val));
    } else if (isa<Constant>(val)) {
      dumpRawValue(val, buffer);
    } else {
      DAWN_ASSERT(dawn::isa<ConstantString>(val), "");
      DAWN_UNREACHABLE("todo: implement globals");
    }
  }

  void IRWriter::dumpTy(const Type* ty, std::string* buffer) const noexcept { // NOLINT(misc-no-recursion)
    using internal::TypeKind;

    switch (ty->kind()) {
      case TypeKind::integer: absl::StrAppend(buffer, "i", dyncastUnchecked<const Int>(ty)->width()); break;
      case TypeKind::floatingPoint: absl::StrAppend(buffer, "f", dyncastUnchecked<const Float>(ty)->width()); break;
      case TypeKind::boolean: absl::StrAppend(buffer, "bool"); break;
      case TypeKind::ptr: absl::StrAppend(buffer, "ptr"); break;
      case TypeKind::array: {
        const auto* arr = dyncastUnchecked<const Array>(ty);
        absl::StrAppend(buffer, "[");
        dumpTy(arr->element(), buffer);
        absl::StrAppend(buffer, "; ", arr->len(), "]");
        break;
      }
      case TypeKind::structure: {
        absl::StrAppend(buffer, "{ ");

        for (const auto* member : dyncastUnchecked<const Struct>(ty)->fields()) {
          dumpTy(member, buffer);
          absl::StrAppend(buffer, " ");
        }

        absl::StrAppend(buffer, "}");
        break;
      }
      case TypeKind::voidUnit: absl::StrAppend(buffer, "void"); break;
    }
  }

  void IRWriter::dumpTyRef(const Value* val, std::string* buffer) const noexcept { // NOLINT(misc-no-recursion)
    dumpTy(val->type(), buffer);
    absl::StrAppend(buffer, " ");
    dumpRefValue(val, buffer);
  }
} // namespace dawn

std::string dawn::stringifyModule(const dawn::Module& mod) noexcept {
  return IRWriter(mod).stringifyModule();
}

std::string dawn::stringifySingleFn(const dawn::Module& mod, const dawn::Function* fn) noexcept {
  return IRWriter(mod).stringifySingleFn(fn);
}

std::string dawn::stringifySingleVal(const dawn::Module& mod, const dawn::Value* val) noexcept {
  return IRWriter(mod).stringifySingleVal(val);
}

std::string dawn::stringifySingleTy(const dawn::Module& mod, const dawn::Type* ty) noexcept {
  return IRWriter(mod).stringifySingleTy(ty);
}

void dawn::dumpModule(const dawn::Module& mod) noexcept {
  std::cout << stringifyModule(mod);
}

void dawn::dumpSingleFn(const dawn::Module& mod, const Function* fn) noexcept {
  std::cout << stringifySingleFn(mod, fn);
}

void dawn::dumpSingleVal(const dawn::Module& mod, const dawn::Value* val) noexcept {
  std::cout << stringifySingleVal(mod, val);
}

void dawn::dumpSingleTy(const dawn::Module& mod, const dawn::Type* ty) noexcept {
  std::cout << stringifySingleTy(mod, ty);
}