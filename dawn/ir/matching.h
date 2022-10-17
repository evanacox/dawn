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

#include "../analysis/analysis_manager.h"
#include "../utility/rtti.h"
#include "./instructions.h"
#include <tuple>
#include <utility>

namespace dawn {
  namespace internal {
    template <typename... Inner> class DAWN_PUBLIC ConjunctionMatcher {
    public:
      explicit ConjunctionMatcher(Inner... pats) noexcept : pats_{std::move(pats)...} {}

      [[nodiscard]] bool matches(AnalysisManager* analysis, const Value* val) noexcept {
        return performMatch(analysis, val, std::make_index_sequence<sizeof...(Inner)>{});
      }

    private:
      template <std::size_t... Is>
      [[nodiscard]] bool performMatch(AnalysisManager* analysis,
          const Value* val,
          std::integer_sequence<std::size_t, Is...> /*unused*/) noexcept {
        return (std::get<Is>(pats_).matches(analysis, val) && ...);
      }

      std::tuple<Inner...> pats_;
    };

    template <typename... Inner> class DAWN_PUBLIC DisjunctionMatcher {
    public:
      explicit DisjunctionMatcher(Inner... pats) noexcept : pats_{std::move(pats)...} {}

      [[nodiscard]] bool matches(AnalysisManager* analysis, const Value* val) noexcept {
        return performMatch(analysis, val, std::make_index_sequence<sizeof...(Inner)>{});
      }

    private:
      template <std::size_t... Is>
      [[nodiscard]] bool performMatch(AnalysisManager* analysis,
          const Value* val,
          std::integer_sequence<std::size_t, Is...> /*unused*/) noexcept {
        return (std::get<Is>(pats_).matches(analysis, val) || ...);
      }

      std::tuple<Inner...> pats_;
    };

    template <typename T, bool Match> class DAWN_PUBLIC RealClassMatcher;

    template <typename T> class DAWN_PUBLIC RealClassMatcher<T, true> {
    public:
      explicit RealClassMatcher(const T** match) noexcept : match_{match} {}

      [[nodiscard]] bool matches(AnalysisManager* /*unused*/, const Value* val) noexcept {
        auto ptr = dyncast<const T>(val);

        if (ptr && match_ != nullptr) {
          *match_ = ptr.get();
        }

        return ptr;
      }

    private:
      const T** match_;
    };

    template <typename T> class DAWN_PUBLIC RealClassMatcher<T, false> {
    public:
      [[nodiscard]] bool matches(AnalysisManager* /*unused*/, const Value* val) noexcept {
        return isa<T>(val);
      }
    };

    template <typename T> class DAWN_PUBLIC ClassMatcher {
    public:
      [[nodiscard]] auto operator()() const noexcept {
        return RealClassMatcher<T, false>{};
      }

      [[nodiscard]] auto operator()(const T** match) const noexcept {
        return RealClassMatcher<T, true>{match};
      }
    };

    template <typename T, bool Match> class DAWN_PUBLIC RealTypeMatcher;

    template <typename T> class DAWN_PUBLIC RealTypeMatcher<T, true> {
    public:
      explicit RealTypeMatcher(const T** match) noexcept : match_{match} {}

      [[nodiscard]] bool matches(AnalysisManager* /*unused*/, const Value* val) noexcept {
        auto ptr = dyncast<const T>(val->type());

        if (ptr && match_ != nullptr) {
          *match_ = ptr.get();
        }

        return ptr;
      }

    private:
      const T** match_;
    };

    template <typename T> class DAWN_PUBLIC RealTypeMatcher<T, false> {
    public:
      [[nodiscard]] bool matches(AnalysisManager* /*unused*/, const Value* val) const noexcept {
        return isa<T>(val->type());
      }
    };

    template <typename T> class DAWN_PUBLIC IsTypeMatcher {
    public:
      [[nodiscard]] auto operator()() const noexcept {
        return RealTypeMatcher<T, false>{};
      }

      [[nodiscard]] auto operator()(const T** match) const noexcept {
        return RealTypeMatcher<T, true>{match};
      }
    };

    template <typename T, typename Inner1, typename Inner2, bool Match> class DAWN_PUBLIC RealBinopMatcher;

    template <typename T, typename Inner1, typename Inner2>
    class DAWN_PUBLIC RealBinopMatcher<T, Inner1, Inner2, true> {
    public:
      explicit RealBinopMatcher(const T** ptr, Inner1 one, Inner2 two) noexcept
          : match_{ptr},
            one_{std::move(one)},
            two_{std::move(two)} {}

      [[nodiscard]] bool matches(AnalysisManager* analysis, const Value* val) noexcept {
        if (auto ptr = dyncast<const T>(val)) {
          auto result = one_.matches(analysis, ptr->lhs()) && two_.matches(analysis, ptr->rhs());

          if (result && match_ != nullptr) {
            *match_ = ptr.get();
          }

          return result;
        }

        return false;
      }

    private:
      const T** match_;
      Inner1 one_;
      Inner2 two_;
    };

    template <typename T, typename Inner1, typename Inner2>
    class DAWN_PUBLIC RealBinopMatcher<T, Inner1, Inner2, false> {
    public:
      explicit RealBinopMatcher(Inner1 one, Inner2 two) noexcept : one_{std::move(one)}, two_{std::move(two)} {}

      [[nodiscard]] bool matches(AnalysisManager* analysis, const Value* val) noexcept {
        if (auto ptr = dyncast<const T>(val)) {
          return one_.matches(analysis, ptr->lhs()) && two_.matches(analysis, ptr->rhs());
        }

        return false;
      }

    private:
      Inner1 one_;
      Inner2 two_;
    };

    template <typename T> class DAWN_PUBLIC BinopMatcher {
    public:
      [[nodiscard]] auto operator()() const noexcept {
        return RealClassMatcher<T, false>{};
      }

      [[nodiscard]] auto operator()(const T** match) const noexcept {
        return RealClassMatcher<T, true>{match};
      }

      template <typename Lhs, typename Rhs> [[nodiscard]] auto operator()(Lhs lhs, Rhs rhs) const noexcept {
        return RealBinopMatcher<T, Lhs, Rhs, false>{std::move(lhs), std::move(rhs)};
      }

      template <typename Lhs, typename Rhs>
      [[nodiscard]] auto operator()(const T** match, Lhs lhs, Rhs rhs) const noexcept {
        return RealBinopMatcher<T, Lhs, Rhs, true>{match, std::move(lhs), std::move(rhs)};
      }
    };

    template <typename T, typename Inner, bool Match> class DAWN_PUBLIC RealUnaryMatcher;

    template <typename T, typename Inner> class DAWN_PUBLIC RealUnaryMatcher<T, Inner, true> {
    public:
      explicit RealUnaryMatcher(const T** ptr, Inner one) noexcept : match_{ptr}, one_{std::move(one)} {}

      [[nodiscard]] bool matches(AnalysisManager* analysis, const Value* val) const noexcept {
        if (auto ptr = dyncast<T>(val)) {
          auto result = one_.matches(analysis, ptr->operand());

          if (result && match_ != nullptr) {
            match_ = ptr.get();
          }

          return result;
        }

        return false;
      }

    private:
      const T** match_;
      Inner one_;
    };

    template <typename T, typename Inner> class DAWN_PUBLIC RealUnaryMatcher<T, Inner, false> {
    public:
      explicit RealUnaryMatcher(Inner one) noexcept : one_{std::move(one)} {}

      [[nodiscard]] bool matches(AnalysisManager* analysis, const Value* val) const noexcept {
        if (auto ptr = dyncast<T>(val)) {
          return one_.matches(analysis, ptr->operand());
        }

        return false;
      }

    private:
      Inner one_;
    };

    template <typename T> class DAWN_PUBLIC UnaryMatcher {
    public:
      [[nodiscard]] auto operator()() const noexcept {
        return RealClassMatcher<T, false>{};
      }

      [[nodiscard]] auto operator()(const T** match) const noexcept {
        return RealClassMatcher<T, true>{match};
      }

      template <typename Lhs> [[nodiscard]] auto operator()(Lhs lhs) const noexcept {
        return RealUnaryMatcher<T, Lhs, false>{nullptr, std::move(lhs)};
      }

      template <typename Lhs, typename Rhs> [[nodiscard]] auto operator()(const T** match, Lhs lhs) const noexcept {
        return RealUnaryMatcher<T, Lhs, true>{match, std::move(lhs)};
      }
    };

    template <typename T> struct RealDecideMatcher {
      using type = internal::ClassMatcher<T>;
    };

    template <std::derived_from<BinaryInst> T> struct RealDecideMatcher<T> {
      using type = internal::BinopMatcher<T>;
    };

    template <std::derived_from<UnaryInst> T> struct RealDecideMatcher<T> {
      using type = internal::UnaryMatcher<T>;
    };

    template <typename T> using DecideMatcher = typename RealDecideMatcher<T>::type;
  } // namespace internal

  inline constexpr auto br = internal::DecideMatcher<Br>{};

  inline constexpr auto condBr = internal::DecideMatcher<CondBr>{};

  inline constexpr auto ret = internal::DecideMatcher<Ret>{};

  inline constexpr auto unreachable = internal::DecideMatcher<Unreachable>{};

  inline constexpr auto sext = internal::DecideMatcher<Sext>{};

  inline constexpr auto zext = internal::DecideMatcher<Zext>{};

  inline constexpr auto trunc = internal::DecideMatcher<Trunc>{};

  inline constexpr auto itob = internal::DecideMatcher<IToB>{};

  inline constexpr auto btoi = internal::DecideMatcher<BToI>{};

  inline constexpr auto itop = internal::DecideMatcher<IToP>{};

  inline constexpr auto ptoi = internal::DecideMatcher<PToI>{};

  inline constexpr auto sitof = internal::DecideMatcher<SIToF>{};

  inline constexpr auto uitof = internal::DecideMatcher<UIToF>{};

  inline constexpr auto ftosi = internal::DecideMatcher<FToSI>{};

  inline constexpr auto ftoui = internal::DecideMatcher<FToUI>{};

  inline constexpr auto bitAnd = internal::DecideMatcher<And>{};

  inline constexpr auto bitOr = internal::DecideMatcher<Or>{};

  inline constexpr auto bitXor = internal::DecideMatcher<Xor>{};

  inline constexpr auto shl = internal::DecideMatcher<Shl>{};

  inline constexpr auto lshr = internal::DecideMatcher<LShr>{};

  inline constexpr auto ashr = internal::DecideMatcher<AShr>{};

  inline constexpr auto iadd = internal::DecideMatcher<IAdd>{};

  inline constexpr auto isub = internal::DecideMatcher<ISub>{};

  inline constexpr auto imul = internal::DecideMatcher<IMul>{};

  inline constexpr auto udiv = internal::DecideMatcher<UDiv>{};

  inline constexpr auto sdiv = internal::DecideMatcher<SDiv>{};

  inline constexpr auto urem = internal::DecideMatcher<URem>{};

  inline constexpr auto srem = internal::DecideMatcher<SRem>{};

  inline constexpr auto fneg = internal::DecideMatcher<FNeg>{};

  inline constexpr auto fadd = internal::DecideMatcher<FAdd>{};

  inline constexpr auto fsub = internal::DecideMatcher<FSub>{};

  inline constexpr auto fmul = internal::DecideMatcher<FMul>{};

  inline constexpr auto fdiv = internal::DecideMatcher<FDiv>{};

  inline constexpr auto frem = internal::DecideMatcher<FRem>{};

  inline constexpr auto phi = internal::DecideMatcher<Phi>{};

  inline constexpr auto call = internal::DecideMatcher<Call>{};

  inline constexpr auto sel = internal::DecideMatcher<Sel>{};

  inline constexpr auto extract = internal::DecideMatcher<Extract>{};

  inline constexpr auto insert = internal::DecideMatcher<Insert>{};

  inline constexpr auto elemptr = internal::DecideMatcher<ElemPtr>{};

  inline constexpr auto alloc = internal::DecideMatcher<Alloca>{};

  inline constexpr auto load = internal::DecideMatcher<Load>{};

  inline constexpr auto store = internal::DecideMatcher<Store>{};

  inline constexpr auto offset = internal::DecideMatcher<Offset>{};

  inline constexpr auto icmp = internal::DecideMatcher<ICmp>{};

  inline constexpr auto fcmp = internal::DecideMatcher<FCmp>{};

  inline constexpr auto binaryInst = internal::DecideMatcher<BinaryInst>{};

  inline constexpr auto unaryInst = internal::DecideMatcher<UnaryInst>{};

  inline constexpr auto conversionInst = internal::DecideMatcher<ConversionInst>{};

  inline constexpr auto terminatorInst = internal::DecideMatcher<TerminatorInst>{};

  inline constexpr auto constInt = internal::DecideMatcher<ConstantInt>{};

  inline constexpr auto constBool = internal::DecideMatcher<ConstantBool>{};

  inline constexpr auto constFloat = internal::DecideMatcher<ConstantFloat>{};

  inline constexpr auto constNull = internal::DecideMatcher<ConstantNull>{};

  inline constexpr auto constArray = internal::DecideMatcher<ConstantArray>{};

  inline constexpr auto constString = internal::DecideMatcher<ConstantString>{};

  inline constexpr auto constNonStringArray = internal::DecideMatcher<ConstantValArray>{};

  inline constexpr auto constStruct = internal::DecideMatcher<ConstantStruct>{};

  inline constexpr auto ofIntTy = internal::IsTypeMatcher<Int>{};

  inline constexpr auto ofFloatTy = internal::IsTypeMatcher<Float>{};

  inline constexpr auto ofBoolTy = internal::IsTypeMatcher<Bool>{};

  inline constexpr auto ofPtrTy = internal::IsTypeMatcher<Ptr>{};

  inline constexpr auto ofArrayTy = internal::IsTypeMatcher<Array>{};

  inline constexpr auto ofStructTy = internal::IsTypeMatcher<Struct>{};

  template <typename Lhs, typename Rhs> [[nodiscard]] constexpr auto both(Lhs lhs, Rhs rhs) noexcept {
    return internal::ConjunctionMatcher<Lhs, Rhs>{std::move(lhs), std::move(rhs)};
  }

  template <typename... Pats> [[nodiscard]] constexpr auto oneOf(Pats... rest) noexcept {
    return internal::DisjunctionMatcher<Pats...>{std::move(rest)...};
  }

  template <typename... Pats> [[nodiscard]] constexpr auto all(Pats... rest) noexcept {
    return internal::ConjunctionMatcher<Pats...>{std::move(rest)...};
  }

  template <typename Pattern> [[nodiscard]] bool matches(const Value* val, AnalysisManager* am, Pattern pat) noexcept {
    return pat.matches(am, val);
  }
} // namespace dawn