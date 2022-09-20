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

#include "dawn/ir/ir_reader.h"
#include "absl/base/casts.h"
#include "absl/strings/ascii.h"
#include "absl/strings/charconv.h"
#include "absl/strings/str_cat.h"
#include "dawn/ir/ir_builder.h"
#include "dawn/ir/ir_writer.h"
#include "dawn/ir/module.h"
#include <charconv>

namespace {
  enum class IntegerBase { binary = 2, octal = 8, decimal = 10, hex = 16 };

  enum class Tok {
    globalName,
    blockLabel,
    valLabel,
    literalBinary,
    literalOctal,
    literalDecimal,
    literalHex,
    literalFloatDecimal,
    literalFloatScientific,
    literalFloatByteHex,
    literalFloatCHex,
    literalString,
    symbolParenOpen,
    symbolParenClose,
    symbolBraceOpen,
    symbolBraceClose,
    symbolBracketOpen,
    symbolBracketClose,
    symbolComma,
    symbolColon,
    symbolEq,
    keywordBool,
    keywordVoid,
    keywordPtr,
    keywordI8,
    keywordI16,
    keywordI32,
    keywordI64,
    keywordF32,
    keywordF64,
    keywordIf,
    keywordElse,
    keywordTrue,
    keywordFalse,
    keywordUndef,
    keywordNull,
    keywordVolatile,
    keywordDecl,
    keywordFunc,
    keywordPhi,
    keywordCall,
    keywordSel,
    keywordBr,
    keywordCbr,
    keywordRet,
    keywordUnreachable,
    keywordAnd,
    keywordOr,
    keywordXor,
    keywordShl,
    keywordLShr,
    keywordAShr,
    keywordIAdd,
    keywordISub,
    keywordIMul,
    keywordUDiv,
    keywordSDiv,
    keywordURem,
    keywordSRem,
    keywordFNeg,
    keywordFAdd,
    keywordFSub,
    keywordFMul,
    keywordFDiv,
    keywordFRem,
    keywordICmp,
    keywordFCmp,
    keywordLoad,
    keywordStore,
    keywordSext,
    keywordZext,
    keywordTrunc,
    keywordItob,
    keywordBtoi,
    keywordSitof,
    keywordUitof,
    keywordFtosi,
    keywordFtoui,
    keywordItop,
    keywordPtoi,
    keywordEq,
    keywordNe,
    keywordUgt,
    keywordUlt,
    keywordUge,
    keywordUle,
    keywordSgt,
    keywordSlt,
    keywordSge,
    keywordSle,
    keywordOrd,
    keywordUno,
    keywordOeq,
    keywordOne,
    keywordOgt,
    keywordOlt,
    keywordOge,
    keywordOle,
    keywordUeq,
    keywordUne,
  };

  struct PhiWorklistEntry {
    dawn::Phi* phi;
    std::variant<dawn::Constant*, std::string> referredValue;
    std::string referredBlockName;
    std::size_t line;
  };

  [[nodiscard]] bool validHexDigit(char ch) noexcept {
    return ('0' <= ch && ch <= '9') || ('a' <= ch && ch <= 'f') || ('A' <= ch && ch <= 'F');
  }

  class ParsingError : public std::runtime_error {
  public:
    explicit ParsingError(const std::string& value) noexcept : std::runtime_error{value} {}
  };

  struct Token {
    Tok type;
    std::string_view raw;
  };

  class Lexer {
  public:
    inline static auto lookup = absl::flat_hash_map<std::string, Tok>{
        {"(", Tok::symbolParenOpen},              //
        {")", Tok::symbolParenClose},             //
        {"[", Tok::symbolBracketOpen},            //
        {"]", Tok::symbolBracketClose},           //
        {"{", Tok::symbolBraceOpen},              //
        {"}", Tok::symbolBraceClose},             //
        {",", Tok::symbolComma},                  //
        {":", Tok::symbolColon},                  //
        {"=", Tok::symbolEq},                     //
        {"bool", Tok::keywordBool},               //
        {"ptr", Tok::keywordPtr},                 //
        {"i8", Tok::keywordI8},                   //
        {"i16", Tok::keywordI16},                 //
        {"i32", Tok::keywordI32},                 //
        {"i64", Tok::keywordI64},                 //
        {"f32", Tok::keywordF32},                 //
        {"f64", Tok::keywordF64},                 //
        {"void", Tok::keywordVoid},               //
        {"if", Tok::keywordIf},                   //
        {"else", Tok::keywordElse},               //
        {"true", Tok::keywordTrue},               //
        {"false", Tok::keywordFalse},             //
        {"undef", Tok::keywordUndef},             //
        {"null", Tok::keywordNull},               //
        {"decl", Tok::keywordDecl},               //
        {"func", Tok::keywordFunc},               //
        {"phi", Tok::keywordPhi},                 //
        {"call", Tok::keywordCall},               //
        {"sel", Tok::keywordSel},                 //
        {"br", Tok::keywordBr},                   //
        {"cbr", Tok::keywordCbr},                 //
        {"ret", Tok::keywordRet},                 //
        {"unreachable", Tok::keywordUnreachable}, //
        {"and", Tok::keywordAnd},                 //
        {"or", Tok::keywordOr},                   //
        {"xor", Tok::keywordXor},                 //
        {"shl", Tok::keywordShl},                 //
        {"lshr", Tok::keywordLShr},               //
        {"ashr", Tok::keywordAShr},               //
        {"iadd", Tok::keywordIAdd},               //
        {"isub", Tok::keywordISub},               //
        {"imul", Tok::keywordIMul},               //
        {"udiv", Tok::keywordUDiv},               //
        {"sdiv", Tok::keywordSDiv},               //
        {"urem", Tok::keywordURem},               //
        {"srem", Tok::keywordSRem},               //
        {"fneg", Tok::keywordFNeg},               //
        {"fadd", Tok::keywordFAdd},               //
        {"fsub", Tok::keywordFSub},               //
        {"fmul", Tok::keywordFMul},               //
        {"fdiv", Tok::keywordFDiv},               //
        {"frem", Tok::keywordFRem},               //
        {"icmp", Tok::keywordICmp},               //
        {"fcmp", Tok::keywordFCmp},               //
        {"load", Tok::keywordLoad},               //
        {"store", Tok::keywordStore},             //
        {"sext", Tok::keywordSext},               //
        {"zext", Tok::keywordZext},               //
        {"trunc", Tok::keywordTrunc},             //
        {"itob", Tok::keywordItob},               //
        {"btoi", Tok::keywordBtoi},               //
        {"sitof", Tok::keywordSitof},             //
        {"uitof", Tok::keywordUitof},             //
        {"ftosi", Tok::keywordFtosi},             //
        {"ftoui", Tok::keywordFtoui},             //
        {"itop", Tok::keywordItop},               //
        {"ptoi", Tok::keywordPtoi},               //
        {"eq", Tok::keywordEq},                   //
        {"ne", Tok::keywordNe},                   //
        {"ugt", Tok::keywordUgt},                 //
        {"ult", Tok::keywordUlt},                 //
        {"uge", Tok::keywordUge},                 //
        {"ule", Tok::keywordUle},                 //
        {"sgt", Tok::keywordSgt},                 //
        {"slt", Tok::keywordSlt},                 //
        {"sge", Tok::keywordSge},                 //
        {"sle", Tok::keywordSle},                 //
        {"ord", Tok::keywordOrd},                 //
        {"uno", Tok::keywordUno},                 //
        {"oeq", Tok::keywordOeq},                 //
        {"one", Tok::keywordOne},                 //
        {"ogt", Tok::keywordOgt},                 //
        {"olt", Tok::keywordOlt},                 //
        {"oge", Tok::keywordOge},                 //
        {"ole", Tok::keywordOle},                 //
        {"ueq", Tok::keywordUeq},                 //
        {"une", Tok::keywordUne},                 //
        {"volatile", Tok::keywordVolatile},       //
    };

    explicit Lexer(std::string_view source) noexcept : source_{source} {
      peek_ = nextTok();
    }

    std::optional<Token> next() {
      return std::exchange(peek_, nextTok());
    }

    [[nodiscard]] std::optional<Token> peek() const {
      return peek_;
    }

    [[nodiscard]] std::size_t line() const noexcept {
      return line_;
    }

  private:
    char eatCh() noexcept {
      auto ch = source_[curr_++];

      if (ch == '\n') {
        ++line_;
      }

      return ch;
    }

    [[nodiscard]] char peekCh() noexcept {
      return source_[curr_];
    }

    [[nodiscard]] static bool isReservedChar(char ch) noexcept {
      switch (ch) {
        case '(':
        case ')':
        case '[':
        case ']':
        case '{':
        case '}':
        case '=':
        case ':':
        case ',': return true;
        default: return false;
      }
    }

    std::optional<Token> nextTok() {
      if (curr_ >= source_.size()) {
        return std::nullopt;
      }

      auto tok = eatWholeTok();

      if (tok.starts_with('"') && tok.ends_with('"')) {
        return Token{Tok::literalString, tok};
      }

      if (std::isdigit(tok.front()) != 0) {
        if (tok.starts_with("0xfp")) {
          return Token{Tok::literalFloatByteHex, tok};
        }

        if (tok.find('.') != std::string_view::npos) {
          if (tok.find("e+") != std::string_view::npos || tok.find("e-") != std::string_view::npos) {
            return Token{Tok::literalFloatScientific, tok};
          }

          // if it's not 0xfp and still contains a p somewhere, it's the C hex-float format where that's required
          if (tok.find('p') != std::string_view::npos) {
            return Token{Tok::literalFloatCHex, tok};
          }

          return Token{Tok::literalFloatDecimal, tok};
        }

        if (tok.starts_with("0b")) {
          return Token{Tok::literalBinary, tok};
        }

        if (tok.starts_with("0o")) {
          return Token{Tok::literalOctal, tok};
        }

        // ORDER OF IFS DOES MATTER: floating-point hex literals start `0x` but also contain `.`
        if (tok.starts_with("0x")) {
          return Token{Tok::literalHex, tok};
        }

        return Token{Tok::literalDecimal, tok};
      }

      if (tok.starts_with('$')) {
        return Token{Tok::valLabel, tok};
      }

      if (tok.starts_with('%')) {
        return Token{Tok::blockLabel, tok};
      }

      if (tok.starts_with('@')) {
        return Token{Tok::globalName, tok};
      }

      try {
        return Token{lookup.at(tok), tok};
      } catch (...) {
        throw ParsingError{absl::StrCat("line ", line_, ": unexpected token '", tok, "'")};
      }
    }

    void skipWs() noexcept {
      while (absl::ascii_isspace(static_cast<unsigned char>(peekCh())) || peekCh() == ';') {
        if (peekCh() == ';') {
          while (eatCh() != '\n') {
            // ...
          }

          // \n got eaten already
        }

        while (curr_ < source_.size() && absl::ascii_isspace(static_cast<unsigned char>(peekCh()))) {
          eatCh();
        }
      }
    }

    [[nodiscard]] std::string_view eatWholeTok() {
      skipWs();

      if (isReservedChar(eatCh())) {
        return source_.substr(curr_ - 1, 1);
      }

      auto start = curr_ - 1;

      for (auto ch = peekCh(); (std::isspace(ch) == 0) && !isReservedChar(ch); ch = peekCh()) {
        eatCh();
      }

      return source_.substr(start, curr_ - start);
    }

    std::optional<Token> peek_;
    std::string_view source_;
    std::size_t curr_ = 0;
    std::size_t line_ = 1;
  };

  class Parser {
  public:
    explicit Parser(std::string_view source) : lex_{source}, mod_{std::make_unique<dawn::Module>()}, ib_{mod_.get()} {}

    std::unique_ptr<dawn::Module> parse() {
      while (auto tok = lex_.next()) {
        switch (tok->type) {
          case Tok::keywordDecl: parseDecl(); break;
          case Tok::keywordFunc: parseFunc(); break;
          default: parseError("unexpected token '", tok->raw, "', expected 'decl' or 'func'");
        }
      }

      return std::move(mod_);
    }

    void resetFnState() {
      valLookup_.clear();
      bbLookup_.clear();
      worklist_.clear();
      currentInst_ = 0;
    }

    dawn::Function* parseDecl() {
      resetFnState();

      auto* ret = parseTy();
      auto name = expectType(Tok::globalName, "function name");
      auto args = absl::InlinedVector<dawn::Type*, 8>{};
      auto argNames = absl::InlinedVector<std::string, 8>{};

      expectType(Tok::symbolParenOpen, "opening '(' for argument list");

      for (auto tok = lex_.peek(); tok && tok->type != Tok::symbolParenClose; tok = lex_.peek()) {
        auto* ty = parseTy();
        auto argName = parseValName();

        args.push_back(ty);
        argNames.push_back(std::move(argName));
      }

      auto* fn = ib_.createFunc(std::string{name.raw.substr(1)}, ret, args);
      auto* argNameIt = argNames.begin();

      for (auto& arg : fn->args()) {
        valLookup_.emplace(std::move(*argNameIt), &arg);
      }

      expectType(Tok::symbolParenClose, "closing ')' for argument list");

      return fn;
    }

    void parseFunc() {
      auto* fn = parseDecl();

      expectType(Tok::symbolBraceOpen, "opening '{' for function body");

      ib_.setInsertFn(fn);

      for (auto tok = lex_.peek(); tok && tok->type != Tok::symbolBraceClose; tok = lex_.peek()) {
        parseBlock();
      }

      expectType(Tok::symbolBraceClose, "closing '}' for function body");

      for (auto& item : worklist_) {
        auto bbIt = bbLookup_.find(item.referredBlockName);

        if (bbIt == bbLookup_.end()) {
          parseErrorAtLine(item.line, "basic block '", item.referredBlockName, "' not found");
        }

        if (auto* str = std::get_if<std::string>(&item.referredValue)) {
          auto it = valLookup_.find(std::string_view{*str});

          if (it == valLookup_.end()) {
            parseErrorAtLine(item.line, "value '", *str, "' not found");
          }

          item.phi->addIncomingBlock(bbIt->second, it->second);
        } else {
          auto* val = std::get<dawn::Constant*>(item.referredValue);

          item.phi->addIncomingBlock(bbIt->second, val);
        }
      }
    }

    void parseBlock() {
      auto label = expectType(Tok::blockLabel, "block label");
      auto* block = createOrGetBlock(label.raw);
      ib_.setInsertPoint(block);

      expectType(Tok::symbolColon, "':' following block label");

      for (auto tok = lex_.peek(); tok && tok->type != Tok::blockLabel && tok->type != Tok::symbolBraceClose;
           tok = lex_.peek()) {
        parseInst();
      }
    }

    [[nodiscard]] dawn::Type* parseTy() { // NOLINT(misc-no-recursion)
      auto tok = expectNext("type");

      switch (tok.type) {
        case Tok::keywordBool: return ib_.boolTy();
        case Tok::keywordVoid: return ib_.voidTy();
        case Tok::keywordI8: return ib_.i8Ty();
        case Tok::keywordI16: return ib_.i16Ty();
        case Tok::keywordI32: return ib_.i32Ty();
        case Tok::keywordI64: return ib_.i64Ty();
        case Tok::keywordF32: return ib_.f32Ty();
        case Tok::keywordF64: return ib_.f64Ty();
        case Tok::symbolBracketOpen: {
          auto* inner = parseTy();

          expectType(Tok::symbolComma, "comma");

          auto length = std::size_t{0};
          auto lit = expectType(Tok::literalDecimal, "number");
          auto [ptr, ec] = std::from_chars(lit.raw.data(), lit.raw.data() + lit.raw.size(), length);

          if (ec != std::errc{}) {
            parseError("invalid integer constant '", lit.raw, "'");
          }

          expectType(Tok::symbolBracketClose, "closing ']' for array type");

          return ib_.arrayTy(inner, length);
        }
        case Tok::symbolBraceOpen: {
          absl::InlinedVector<dawn::Type*, 8> tys;

          for (auto next = expectPeek("struct type"); next.type != Tok::symbolBraceClose;
               next = expectPeek("struct type")) {
            tys.push_back(parseTy());
          }

          return ib_.structTy(tys);
        }
        default: parseError("expected a type, got '", tok.raw, "'");
      }
    }

    [[nodiscard]] std::string parseValName() {
      auto name = expectType(Tok::valLabel, "val name");
      auto actual = name.raw.substr(1);

      if (std::all_of(actual.begin(), actual.end(), [](char ch) {
            return std::isdigit(ch) != 0;
          })) {
        auto val = std::size_t{0};
        auto [_, ec] = std::from_chars(actual.data(), actual.data() + actual.size(), val);

        if (ec != std::errc{}) {
          parseError("invalid integer label '", name.raw, "'");
        }

        if (val != currentInst_) {
          parseError("expected next numbered value to be named '", currentInst_, "' but got '", val, "'");
        }

        ++currentInst_;
      }

      return std::string{name.raw};
    }

    template <typename T> [[nodiscard]] T* typeShouldBe(dawn::Type* ty, std::string_view error) {
      if (auto ptr = dawn::dyncast<T>(ty)) {
        return ptr.get();
      }

      parseError(error, " but was '", dawn::stringifySingleTy(*mod_, ty), "'");
    }

    [[nodiscard]] dawn::ConstantInt* parseIntConstant(dawn::Type* expected,
        std ::string_view raw,
        int offset,
        IntegerBase base) {
      auto* ty = typeShouldBe<dawn::Int>(expected, "integer literal should be of integer type");
      auto val = parseRawInteger(raw.substr(static_cast<std::size_t>(offset)), ty->width(), base);

      return ib_.constInt(val, expected);
    }

    [[nodiscard]] dawn::ConstantFloat* parseFloatConstant(dawn::Type* expected,
        std::string_view raw,
        int offset,
        absl::chars_format fmt) {
      auto* ty = typeShouldBe<dawn::Float>(expected, "float literal should be of float type");
      auto value = double{};
      auto data = raw.substr(static_cast<std::size_t>(offset));
      auto [_, ec] = absl::from_chars(data.data(), data.data() + data.size(), value, fmt);

      if (ec != std::errc{}) {
        parseError("unable to parse decimal float literal '", raw, "'");
      }

      return ib_.constFloat(value, ty);
    }

    [[nodiscard]] dawn::ConstantArray* parseConstantArray(dawn::Type* expected) { // NOLINT(misc-no-recursion)
      auto* ty = typeShouldBe<dawn::Array>(expected, "array literal should be of array type");
      auto vals = absl::InlinedVector<dawn::Constant*, 16>{};

      for (auto i = std::uint64_t{0}; i < ty->len(); ++i) {
        auto* val = parseConstant(ty->element());

        if (i + 1 < ty->len()) {
          expectType(Tok::symbolComma, "',' between array literal elements");
        }

        vals.push_back(val);
      }

      expectType(Tok::symbolBracketClose, "']' after correct number of array elements");

      return ib_.constArray(vals);
    }

    [[nodiscard]] dawn::ConstantStruct* parseConstantStruct(dawn::Type* expected) { // NOLINT(misc-no-recursion)
      auto* ty = typeShouldBe<dawn::Struct>(expected, "array literal should be of array type");
      auto vals = absl::InlinedVector<dawn::Constant*, 16>{};
      auto fields = ty->fields();

      for (auto i = std::uint64_t{0}; i < fields.size(); ++i) {
        auto* val = parseConstant(fields[i]);

        if (i + 1 < fields.size()) {
          expectType(Tok::symbolComma, "',' between struct literal elements");
        }

        vals.push_back(val);
      }

      expectType(Tok::symbolBraceClose, "'}' after correct number of struct constant elements");

      return ib_.constStruct(vals);
    }

    [[nodiscard]] char parseSingleChar(const char*& it, const char* end) {
      auto next = [this, &it, &end]() -> char {
        ++it;

        if (it == end) {
          parseError("unexpected end of string literal, expected escape sequence after '\'");
        }

        return *it;
      };

      if (*it == '\\') {
        auto first = next();

        switch (first) {
          case '\'': return '\'';
          case '"': return '"';
          case '?': return '\?';
          case '\\': return '\\';
          case 'a': return '\a';
          case 'b': return '\b';
          case 'f': return '\f';
          case 'n': return '\n';
          case 'r': return '\r';
          case 't': return '\t';
          case 'v': return '\v';
          case '0': return '\0';
          case 'x': {
            auto arr = std::array{next(), next()};
            auto out = std::uint8_t{};

            if (!validHexDigit(arr[0]) || !validHexDigit(arr[1])) {
              parseError("invalid hex escape sequence '", arr.data(), "'");
            }

            // parse it as a hex literal
            std::from_chars(arr.data(), arr.data() + arr.size(), out, 16);

            return static_cast<char>(out);
          }
          default: parseError("unknown escape sequence '\\", std::string_view{&first, 1}, "'");
        }
      }

      return *it;
    }

    [[nodiscard]] dawn::Constant* parseConstant(dawn::Type* expected) { // NOLINT(misc-no-recursion)
      auto tok = expectNext("a constant");

      switch (tok.type) {
        case Tok::literalBinary: return parseIntConstant(expected, tok.raw, 2, IntegerBase::binary);
        case Tok::literalOctal: return parseIntConstant(expected, tok.raw, 2, IntegerBase::octal);
        case Tok::literalDecimal: return parseIntConstant(expected, tok.raw, 0, IntegerBase::decimal);
        case Tok::literalHex: return parseIntConstant(expected, tok.raw, 2, IntegerBase::hex);
        case Tok::literalFloatDecimal: return parseFloatConstant(expected, tok.raw, 0, absl::chars_format::fixed);
        case Tok::literalFloatScientific:
          return parseFloatConstant(expected, tok.raw, 2, absl::chars_format::scientific);
        case Tok::literalFloatByteHex: {
          // we parse an "integer" from the hex literal, and then bit_cast into a double for the constant.
          // these byte hex values are basically just N-bit hex literals that we treat as floats instead
          auto* ty = typeShouldBe<dawn::Float>(expected, "float literal should be of float type");
          auto val = parseRawInteger(tok.raw.substr(4), static_cast<dawn::Width>(ty->width()), IntegerBase::hex);
          return ib_.constFloat(absl::bit_cast<double>(val.value()), ty);
        }
        case Tok::literalFloatCHex: return parseFloatConstant(expected, tok.raw, 2, absl::chars_format::hex);
        case Tok::keywordTrue: return ib_.constTrue();
        case Tok::keywordFalse: return ib_.constFalse();
        case Tok::keywordNull: return ib_.constNull();
        case Tok::keywordUndef: return ib_.constUndef(expected);
        case Tok::symbolBraceOpen: return parseConstantStruct(expected);
        case Tok::symbolBracketOpen: return parseConstantArray(expected);
        case Tok::literalString: {
          auto final = std::string{};
          const auto* end = tok.raw.data() + tok.raw.size() - 1;

          for (const auto* it = tok.raw.data() + 1; it != end; ++it) {
            final += parseSingleChar(it, end);
          }

          return ib_.constString(std::move(final));
        }
        default: parseError("unexpected token '", tok.raw, "', expected constant");
      }
    }

    [[nodiscard]] dawn::Value* parseRefValue(dawn::Type* expected) { // NOLINT(misc-no-recursion)
      auto tok = lex_.peek();

      if (tok && tok->type == Tok::valLabel) {
        if (auto it = valLookup_.find(tok->raw); it != valLookup_.end()) {
          lex_.next(); // consume tok, we only did .peek() earlier

          expectToBeTy(expected, it->second, "value");

          return it->second;
        }
      }

      return parseConstant(expected);
    }

    [[nodiscard]] std::pair<dawn::Type*, dawn::Value*> parseTyValPair() {
      auto* ty = parseTy();
      auto* val = parseRefValue(ty);

      return {ty, val};
    }

    [[nodiscard]] std::pair<dawn::Value*, dawn::Value*> parseBinopOperands() {
      auto [ty, lhs] = parseTyValPair();
      expectType(Tok::symbolComma, "comma between binary instruction operands");
      auto* rhs = parseRefValue(ty);

      return {lhs, rhs};
    }

    PhiWorklistEntry parsePhiIncoming(dawn::Phi* phi) {
      expectType(Tok::symbolBracketOpen, "incoming branch for 'phi'");
      auto nameOrValue = std::variant<dawn::Constant*, std::string>{};

      // the reason this isn't just a parseRefValue call is because we need to put any name references
      // on a worklist and resolve them at the end of parsing the function. `phi`s are allowed to accept values
      // that are defined after the `phi` itself, e.g. this code for a loop:
      //
      // %entry:
      //     br %bb0
      //
      // %bb0:
      //     $0 = phi i32 [ 0, %entry ], [ $2, %bb1 ]
      //     $1 = icmp eq i32 $0, 128
      //     cbr bool $1, if %bb2, else %bb1
      //
      // %bb1:
      //     call void @whatever(i32 $0)
      //     $2 = iadd i32 $0, 1
      //     br %bb0
      if (auto tok = lex_.peek(); tok && tok->type == Tok::valLabel) {
        nameOrValue = parseValName();
      } else {
        nameOrValue = parseConstant(phi->type());
      }

      expectType(Tok::symbolComma, "comma between value and label");
      auto block = expectType(Tok::blockLabel, "incoming block name");

      return PhiWorklistEntry{phi, std::move(nameOrValue), std::string{block.raw}, lex_.line()};
    }

    void parseInst() { // NOLINT(readability-function-cognitive-complexity)
      auto name = std::string{};

      if (auto tok = lex_.peek(); tok && tok->type == Tok::valLabel) {
        name = parseValName();

        expectType(Tok::symbolEq, "'=' after value name");
      }

      auto* value = static_cast<dawn::Value*>(nullptr);
      auto tok = expectNext("instruction");

#define DAWN_PARSE_INT_BOOL_BINOP(ty)                                                                                  \
  do {                                                                                                                 \
    value = parseSimpleBinary<dawn::ty, dawn::Int, dawn::Bool>("integral or 'bool'", &dawn::IRBuilder::create##ty);    \
  } while (false)

#define DAWN_PARSE_INT_BINOP(ty)                                                                                       \
  do {                                                                                                                 \
    value = parseSimpleBinary<dawn::ty, dawn::Int>("integral", &dawn::IRBuilder::create##ty);                          \
  } while (false)

#define DAWN_PARSE_FLOAT_BINOP(ty)                                                                                     \
  do {                                                                                                                 \
    value = parseSimpleBinary<dawn::ty, dawn::Float>("floating-point", &dawn::IRBuilder::create##ty);                  \
  } while (false)

#define DAWN_PARSE_CONVERSION(name, message, ...)                                                                      \
  do {                                                                                                                 \
    using T = decltype(std::declval<dawn::IRBuilder>().create##name(std::declval<dawn::Type*>(),                       \
        std::declval<dawn::Value*>()));                                                                                \
    value = parseSimpleConversion<std::remove_pointer_t<T>, __VA_ARGS__>((message), &dawn::IRBuilder::create##name);   \
  } while (false)

#define DAWN_PARSE_STATIC_CONVERSION(name, message, ...)                                                               \
  do {                                                                                                                 \
    using T = decltype(std::declval<dawn::IRBuilder>().create##name(std::declval<dawn::Value*>()));                    \
    value = parseStaticConversion<std::remove_pointer_t<T>, __VA_ARGS__>((message), &dawn::IRBuilder::create##name);   \
  } while (false)

      switch (tok.type) {
        case Tok::keywordPhi: value = parsePhi(); break;
        case Tok::keywordCall: value = parseCall(); break;
        case Tok::keywordSel: value = parseSel(); break;
        case Tok::keywordBr: value = parseBr(); break;
        case Tok::keywordCbr: value = parseCbr(); break;
        case Tok::keywordRet: value = parseRet(); break;
        case Tok::keywordUnreachable: value = parseUnreachable(); break;
        case Tok::keywordAnd: DAWN_PARSE_INT_BOOL_BINOP(And); break;
        case Tok::keywordOr: DAWN_PARSE_INT_BOOL_BINOP(Or); break;
        case Tok::keywordXor: DAWN_PARSE_INT_BOOL_BINOP(Xor); break;
        case Tok::keywordShl: DAWN_PARSE_INT_BOOL_BINOP(Shl); break;
        case Tok::keywordLShr: DAWN_PARSE_INT_BOOL_BINOP(LShr); break;
        case Tok::keywordAShr: DAWN_PARSE_INT_BOOL_BINOP(AShr); break;
        case Tok::keywordIAdd: DAWN_PARSE_INT_BINOP(IAdd); break;
        case Tok::keywordISub: DAWN_PARSE_INT_BINOP(ISub); break;
        case Tok::keywordIMul: DAWN_PARSE_INT_BINOP(IMul); break;
        case Tok::keywordUDiv: DAWN_PARSE_INT_BINOP(UDiv); break;
        case Tok::keywordSDiv: DAWN_PARSE_INT_BINOP(SDiv); break;
        case Tok::keywordURem: DAWN_PARSE_INT_BINOP(URem); break;
        case Tok::keywordSRem: DAWN_PARSE_INT_BINOP(SRem); break;
        case Tok::keywordFNeg: DAWN_PARSE_FLOAT_BINOP(FNeg); break;
        case Tok::keywordFAdd: DAWN_PARSE_FLOAT_BINOP(FAdd); break;
        case Tok::keywordFSub: DAWN_PARSE_FLOAT_BINOP(FSub); break;
        case Tok::keywordFMul: DAWN_PARSE_FLOAT_BINOP(FMul); break;
        case Tok::keywordFDiv: DAWN_PARSE_FLOAT_BINOP(FDiv); break;
        case Tok::keywordFRem: DAWN_PARSE_FLOAT_BINOP(FRem); break;
        case Tok::keywordICmp: value = parseICmp(); break;
        case Tok::keywordFCmp: value = parseFCmp(); break;
        case Tok::keywordLoad: value = parseLoad(); break;
        case Tok::keywordStore: value = parseStore(); break;
        case Tok::keywordSext: DAWN_PARSE_CONVERSION(Sext, "integral", dawn::Int); break;
        case Tok::keywordZext: DAWN_PARSE_CONVERSION(Zext, "integral", dawn::Int); break;
        case Tok::keywordTrunc: DAWN_PARSE_CONVERSION(Trunc, "integral", dawn::Int); break;
        case Tok::keywordItob: DAWN_PARSE_STATIC_CONVERSION(IntToBool, "integral", dawn::Int); break;
        case Tok::keywordBtoi: DAWN_PARSE_CONVERSION(BoolToInt, "'bool'", dawn::Bool); break;
        case Tok::keywordSitof: DAWN_PARSE_CONVERSION(SIntToFloat, "floating-point", dawn::Float); break;
        case Tok::keywordUitof: DAWN_PARSE_CONVERSION(UIntToFloat, "floating-point", dawn::Float); break;
        case Tok::keywordFtosi: DAWN_PARSE_CONVERSION(FloatToSInt, "integral", dawn::Int); break;
        case Tok::keywordFtoui: DAWN_PARSE_CONVERSION(FloatToUInt, "integral", dawn::Int); break;
        case Tok::keywordItop: DAWN_PARSE_STATIC_CONVERSION(IntToPtr, "integral", dawn::Int); break;
        case Tok::keywordPtoi: DAWN_PARSE_CONVERSION(FloatToUInt, "integral", dawn::Int); break;
        default: parseError("expected instruction name but got '", tok.raw, "' instead");
      }

      if (!name.empty()) {
        DAWN_ASSERT(value != nullptr, "should have gotten an instruction");

        valLookup_.emplace(std::move(name), value);
      } else if (!dawn::isa<dawn::Call>(value) && !dawn::isa<dawn::Store>(value)
                 && !dawn::isa<dawn::TerminatorInst>(value)) {
        parseError("expected instruction to be labeled");
      }
    }

    template <typename T> using IRBuilderBinaryCreateMethod = T* (dawn::IRBuilder::*)(dawn::Value*, dawn::Value*);

    template <typename T, typename... Ts>
    [[nodiscard]] T* parseSimpleBinary(std::string_view expectedTys, IRBuilderBinaryCreateMethod<T> method) {
      auto [lhs, rhs] = parseBinopOperands();

      if (!(dawn::isa<Ts>(lhs->type()) || ...)) {
        parseError("expected operand type to be ",
            expectedTys,
            " but got '",
            dawn::stringifySingleTy(*mod_, lhs->type()),
            "'");
      }

      return (ib_.*method)(lhs, rhs);
    }

    template <typename... Ts>
    [[nodiscard]] std::pair<dawn::Type*, dawn::Value*> parseConversionPattern(std::string_view expectedTys) {
      auto* ty = parseTy();
      expectType(Tok::symbolComma, "comma between conversion output and input");
      auto [_, val] = parseTyValPair();

      if (!(dawn::isa<Ts>(ty) || ...)) {
        parseError("expected operand type to be ", expectedTys, " but got '", dawn::stringifySingleTy(*mod_, ty), "'");
      }

      return {ty, val};
    }

    template <typename T> using IRBuilderConversionCreate = T* (dawn::IRBuilder::*)(dawn::Type*, dawn::Value*);

    template <typename T, typename... Ts>
    [[nodiscard]] T* parseSimpleConversion(std::string_view expectedTys, IRBuilderConversionCreate<T> method) {
      auto [ty, val] = parseConversionPattern<Ts...>(expectedTys);

      return (ib_.*method)(ty, val);
    }

    template <typename T> using IRBuilderStaticConversionCreate = T* (dawn::IRBuilder::*)(dawn::Value*);

    template <typename T, typename... Ts>
    [[nodiscard]] T* parseStaticConversion(std::string_view expectedTys, IRBuilderStaticConversionCreate<T> method) {
      auto [_, val] = parseConversionPattern<Ts...>(expectedTys);

      return (ib_.*method)(val);
    }

    [[nodiscard]] dawn::Value* parsePhi() {
      auto* inst = ib_.createPhi(parseTy());

      worklist_.push_back(parsePhiIncoming(inst));

      for (auto tok = lex_.peek(); tok && tok->type == Tok::symbolComma; tok = lex_.peek()) {
        expectType(Tok::symbolComma, "',' before next 'phi' incoming branch");

        worklist_.push_back(parsePhiIncoming(inst));
      }

      return inst;
    }

    [[nodiscard]] dawn::Value* parseCall() {
      auto args = absl::InlinedVector<dawn::Value*, 8>{};
      auto argTypes = absl::InlinedVector<dawn::Type*, 8>{};
      auto* retTy = parseTy();
      auto callee = expectType(Tok::globalName, "name of function to call");

      expectType(Tok::symbolParenOpen, "'(' before argument list");

      for (auto tok = lex_.peek(); tok && tok->type != Tok::symbolParenClose; tok = lex_.peek()) {
        auto [ty, val] = parseTyValPair();

        if (val->type() != ty) {
          parseError("type mismatch with value, expected type '",
              dawn::stringifySingleTy(*mod_, ty),
              "' for argument but got '",
              dawn::stringifySingleTy(*mod_, val->type()),
              "'");
        }

        args.emplace_back(val);

        if (lex_.peek() && lex_.peek()->type != Tok::symbolParenClose) {
          expectType(Tok::symbolComma, "expected ',' between arguments");
        }
      }

      expectType(Tok::symbolParenClose, "')' after argument list");

      for (auto* val : args) {
        argTypes.push_back(val->type());
      }

      auto* fn = ib_.createOrGetFunc(std::string{callee.raw.substr(1)}, retTy, argTypes);
      auto fnArgs = fn->args();

      if (fn->returnTy() != retTy) {
        parseError("attempted to call function '", callee.raw, "' with the wrong return type");
      }

      if (args.size() != fnArgs.size()) {
        parseError("attempted to call function '", callee.raw, "' with the wrong number of arguments");
      }

      for (auto i = std::size_t{0}; i < args.size(); ++i) {
        if (args[i]->type() != fnArgs[i].type()) {
          parseError("attempted to call function '", callee.raw, "' with arguments of the wrong type");
        }
      }

      return ib_.createCall(ib_.createOrGetFunc(std::string{callee.raw}, retTy, argTypes), args);
    }

    [[nodiscard]] dawn::Value* parseSel() {
      auto* ty = parseTy();
      expectType(Tok::symbolComma, "comma after 'sel' type");
      auto* cond = parseBoolCondition("'sel' condition");

      expectType(Tok::symbolComma, "comma after 'sel' condition");
      expectType(Tok::keywordIf, "'if' after 'sel' condition");
      auto* ifTrue = parseRefValue(ty);

      expectType(Tok::symbolComma, "comma after 'sel' 'if'");
      expectType(Tok::keywordElse, "'else' after 'sel' 'if'");
      auto* ifFalse = parseRefValue(ty);

      return ib_.createSel(cond, dawn::If{ifTrue}, dawn::Else{ifFalse});
    }

    [[nodiscard]] dawn::Value* parseBr() {
      auto label = expectType(Tok::blockLabel, "block label");

      return ib_.createBr(createOrGetBlock(label.raw));
    }

    [[nodiscard]] dawn::Value* parseCbr() {
      auto* cond = parseBoolCondition("'cbr' condition");

      expectType(Tok::symbolComma, "comma after 'cbr' condition");
      expectType(Tok::keywordIf, "'if' for 'cbr'");
      auto ifLabel = expectType(Tok::blockLabel, "block label for 'if'");

      expectType(Tok::symbolComma, "comma after 'cbr' 'if'");
      expectType(Tok::keywordElse, "'else' for 'cbr'");
      auto elseLabel = expectType(Tok::blockLabel, "block label for 'else'");

      return ib_.createCbr(cond,
          dawn::TrueBranch{createOrGetBlock(ifLabel.raw)},
          dawn::FalseBranch{createOrGetBlock(elseLabel.raw)});
    }

    [[nodiscard]] dawn::Value* parseRet() {
      auto* ty = parseTy();

      if (ty == ib_.voidTy()) {
        return ib_.createRetVoid();
      }

      return ib_.createRet(parseRefValue(ty));
    }

    [[nodiscard]] dawn::Value* parseUnreachable() {
      return ib_.createUnreachable();
    }

    [[nodiscard]] dawn::Value* parseICmp() {
      auto next = expectNext("should have opcode after 'icmp'");
      auto opcode = dawn::ICmpOrdering{};

      switch (next.type) {
        case Tok::keywordEq: opcode = dawn::ICmpOrdering::eq; break;
        case Tok::keywordNe: opcode = dawn::ICmpOrdering::ne; break;
        case Tok::keywordUgt: opcode = dawn::ICmpOrdering::ugt; break;
        case Tok::keywordUlt: opcode = dawn::ICmpOrdering::ult; break;
        case Tok::keywordUge: opcode = dawn::ICmpOrdering::uge; break;
        case Tok::keywordUle: opcode = dawn::ICmpOrdering::ule; break;
        case Tok::keywordSgt: opcode = dawn::ICmpOrdering::sgt; break;
        case Tok::keywordSlt: opcode = dawn::ICmpOrdering::slt; break;
        case Tok::keywordSge: opcode = dawn::ICmpOrdering::sge; break;
        case Tok::keywordSle: opcode = dawn::ICmpOrdering::sle; break;
        default: parseError("expected 'icmp' opcode, but got '", next.raw, "'");
      }

      auto [lhs, rhs] = parseBinopOperands();

      if (!dawn::isa<dawn::Int>(lhs->type()) && !dawn::isa<dawn::Bool>(lhs->type())) {
        parseError("expected 'icmp' operands to be integers or 'bool' values but got '",
            dawn::stringifySingleTy(*mod_, lhs->type()),
            "'");
      }

      return ib_.createICmp(opcode, lhs, rhs);
    }

    [[nodiscard]] dawn::Value* parseFCmp() {
      auto next = expectNext("should have opcode after 'fcmp'");
      auto opcode = dawn::FCmpOrdering{};

      switch (next.type) {
        case Tok::keywordOrd: opcode = dawn::FCmpOrdering::ord; break;
        case Tok::keywordUno: opcode = dawn::FCmpOrdering::uno; break;
        case Tok::keywordOeq: opcode = dawn::FCmpOrdering::oeq; break;
        case Tok::keywordOne: opcode = dawn::FCmpOrdering::one; break;
        case Tok::keywordOgt: opcode = dawn::FCmpOrdering::ogt; break;
        case Tok::keywordOlt: opcode = dawn::FCmpOrdering::olt; break;
        case Tok::keywordOge: opcode = dawn::FCmpOrdering::oge; break;
        case Tok::keywordOle: opcode = dawn::FCmpOrdering::ole; break;
        case Tok::keywordUeq: opcode = dawn::FCmpOrdering::ueq; break;
        case Tok::keywordUne: opcode = dawn::FCmpOrdering::une; break;
        case Tok::keywordUgt: opcode = dawn::FCmpOrdering::ugt; break;
        case Tok::keywordUlt: opcode = dawn::FCmpOrdering::ult; break;
        case Tok::keywordUge: opcode = dawn::FCmpOrdering::uge; break;
        case Tok::keywordUle: opcode = dawn::FCmpOrdering::ule; break;
        default: parseError("expected 'fcmp' opcode, but got '", next.raw, "'");
      }

      auto [lhs, rhs] = parseBinopOperands();

      if (!dawn::isa<dawn::Float>(lhs->type())) {
        parseError("expected 'fcmp' operands to be of floating-point types, but got '",
            dawn::stringifySingleTy(*mod_, lhs->type()),
            "'");
      }

      return ib_.createFCmp(opcode, lhs, rhs);
    }

    [[nodiscard]] bool volatileOrComma() {
      auto isVolatile = expectPeek("token after instruction");

      if (isVolatile.type == Tok::keywordVolatile) {
        expectType(Tok::keywordVolatile, "'volatile'");
      }

      expectType(Tok::symbolComma, "comma after 'volatile'");

      return isVolatile.type == Tok::keywordVolatile;
    }

    [[nodiscard]] dawn::Value* parseLoad() {
      auto isVolatile = volatileOrComma();
      auto* ty = parseTy();

      expectType(Tok::symbolComma, "comma after 'load' type");
      auto [ptrTy, val] = parseTyValPair();

      if (!dawn::isa<dawn::Ptr>(ptrTy)) {
        parseError("can only load from 'ptr' operand");
      }

      return ib_.createLoad(ty, val, isVolatile);
    }

    [[nodiscard]] dawn::Value* parseStore() {
      auto isVolatile = volatileOrComma();
      auto [storeTy, storeVal] = parseTyValPair();

      expectType(Tok::symbolComma, "comma after 'store' operand");
      auto [ptrTy, address] = parseTyValPair();

      if (!dawn::isa<dawn::Ptr>(ptrTy)) {
        parseError("cannot load from non-'ptr' value");
      }

      return ib_.createStore(storeVal, dawn::Dest{address}, isVolatile);
    }

    [[nodiscard]] Token expectNext(std::string_view expected) {
      auto tok = lex_.next();

      if (!tok) {
        parseError("expected a ", expected, ", but got eof");
      }

      return tok.value();
    }

    [[nodiscard]] Token expectPeek(std::string_view expected) {
      auto tok = lex_.peek();

      if (!tok) {
        parseError("expected a ", expected, ", but got eof");
      }

      return tok.value();
    }

    Token expectType(Tok tok, std::string_view expected) {
      auto single = expectNext(expected);

      if (single.type != tok) {
        parseError("expected a ", expected, ", but got '", single.raw, "' instead");
      }

      return single;
    }

    Token expectPeekType(Tok tok, std::string_view expected) {
      auto single = expectPeek(expected);

      if (single.type != tok) {
        parseError("expected a ", expected, ", but got '", single.raw, "' instead");
      }

      return single;
    }

    [[nodiscard]] dawn::BasicBlock* createOrGetBlock(std::string_view name) noexcept {
      if (auto it = bbLookup_.find(name); it != bbLookup_.end()) {
        return it->second;
      }

      return bbLookup_.emplace(std::string{name}, ib_.createBlock()).first->second;
    }

    [[nodiscard]] dawn::Value* parseBoolCondition(std::string_view name) {
      auto [ty, val] = parseTyValPair();

      expectToBeTy(ib_.boolTy(), val, name);

      return val;
    }

    void expectToBeTy(dawn::Type* expected, dawn::Value* val, std::string_view name) {
      if (val->type() != expected) {
        parseError(name,
            " value expected to be of type '",
            dawn::stringifySingleTy(*mod_, expected),
            "' but got '",
            dawn::stringifySingleTy(*mod_, val->type()));
      }
    }

    [[nodiscard]] dawn::APInt parseRawInteger(std::string_view raw, dawn::Width width, IntegerBase base) {
      // we need to ensure that the `-` is properly accounted for, even though we want the value as unsigned
      // in the compiler. so, we need to parse as signed and then bit-cast to unsigned if needed
      auto value = (raw.starts_with('-')) //
                       ? fromCharsGeneric<std::ptrdiff_t>(raw, base)
                       : fromCharsGeneric<std::size_t>(raw, base);

      return {value, width};
    }

    template <typename T> [[nodiscard]] std::size_t fromCharsGeneric(std::string_view raw, IntegerBase base) {
      auto value = T{0};
      auto [ptr, ec] = std::from_chars(raw.data(), raw.data() + raw.size(), value, static_cast<int>(base));

      if (ec != std::errc{}) {
        parseError("unable to parse integer '", raw, "'. integer literal values must fit within 64 bits");
      }

      return absl::bit_cast<std::size_t>(value);
    }

    template <typename... Args> [[noreturn]] void parseError(Args&&... args) {
      parseErrorAtLine(lex_.line(), std::forward<Args>(args)...);
    }

    template <typename... Args> [[noreturn]] static void parseErrorAtLine(std::size_t line, Args&&... args) {
      throw ParsingError{absl::StrCat("line ", line, ": ", std::forward<Args>(args)...)};
    }

  private:
    Lexer lex_;
    std::unique_ptr<dawn::Module> mod_;
    dawn::IRBuilder ib_;
    std::vector<PhiWorklistEntry> worklist_;
    absl::flat_hash_map<std::string, dawn::BasicBlock*> bbLookup_;
    absl::flat_hash_map<std::string, dawn::Value*> valLookup_;
    std::size_t currentInst_ = 0;
  }; // namespace

  absl::flat_hash_map<Tok, std::string> reverseLookup(const absl::flat_hash_map<std::string, Tok>& from) {
    auto res = absl::flat_hash_map<Tok, std::string>{};

    for (const auto& [k, v] : from) {
      res.emplace(v, k);
    }

    return res;
  }

  std::string_view tokTypeToStr(Tok ty) noexcept {
    static auto lookup = reverseLookup(Lexer::lookup);

    // intentionally did **not** do `default` here, so I get an error if I
    // add another token type
    switch (ty) {
      case Tok::globalName: return "global-name";
      case Tok::blockLabel: return "block-label";
      case Tok::valLabel: return "val-label";
      case Tok::literalBinary: return "binary-lit";
      case Tok::literalOctal: return "octal-lit";
      case Tok::literalDecimal: return "decimal-lit";
      case Tok::literalHex: return "hex-lit";
      case Tok::literalFloatDecimal: return "float-lit";
      case Tok::literalFloatScientific: return "scientific-float-lit";
      case Tok::literalFloatByteHex: return "byte-float-lit";
      case Tok::literalFloatCHex: return "hex-float-lit";
      case Tok::literalString: return "string-lit";
      case Tok::symbolParenOpen:
      case Tok::symbolParenClose:
      case Tok::symbolBraceOpen:
      case Tok::symbolBraceClose:
      case Tok::symbolBracketOpen:
      case Tok::symbolBracketClose:
      case Tok::symbolComma:
      case Tok::symbolColon:
      case Tok::symbolEq:
      case Tok::keywordBool:
      case Tok::keywordVoid:
      case Tok::keywordPtr:
      case Tok::keywordI8:
      case Tok::keywordI16:
      case Tok::keywordI32:
      case Tok::keywordI64:
      case Tok::keywordF32:
      case Tok::keywordF64:
      case Tok::keywordIf:
      case Tok::keywordElse:
      case Tok::keywordTrue:
      case Tok::keywordFalse:
      case Tok::keywordUndef:
      case Tok::keywordNull:
      case Tok::keywordVolatile:
      case Tok::keywordDecl:
      case Tok::keywordFunc:
      case Tok::keywordPhi:
      case Tok::keywordCall:
      case Tok::keywordSel:
      case Tok::keywordBr:
      case Tok::keywordCbr:
      case Tok::keywordRet:
      case Tok::keywordUnreachable:
      case Tok::keywordAnd:
      case Tok::keywordOr:
      case Tok::keywordXor:
      case Tok::keywordShl:
      case Tok::keywordLShr:
      case Tok::keywordAShr:
      case Tok::keywordIAdd:
      case Tok::keywordISub:
      case Tok::keywordIMul:
      case Tok::keywordUDiv:
      case Tok::keywordSDiv:
      case Tok::keywordURem:
      case Tok::keywordSRem:
      case Tok::keywordFNeg:
      case Tok::keywordFAdd:
      case Tok::keywordFSub:
      case Tok::keywordFMul:
      case Tok::keywordFDiv:
      case Tok::keywordFRem:
      case Tok::keywordICmp:
      case Tok::keywordFCmp:
      case Tok::keywordLoad:
      case Tok::keywordStore:
      case Tok::keywordSext:
      case Tok::keywordZext:
      case Tok::keywordTrunc:
      case Tok::keywordItob:
      case Tok::keywordBtoi:
      case Tok::keywordSitof:
      case Tok::keywordUitof:
      case Tok::keywordFtosi:
      case Tok::keywordFtoui:
      case Tok::keywordItop:
      case Tok::keywordPtoi:
      case Tok::keywordEq:
      case Tok::keywordNe:
      case Tok::keywordUgt:
      case Tok::keywordUlt:
      case Tok::keywordUge:
      case Tok::keywordUle:
      case Tok::keywordSgt:
      case Tok::keywordSlt:
      case Tok::keywordSge:
      case Tok::keywordSle:
      case Tok::keywordOrd:
      case Tok::keywordUno:
      case Tok::keywordOeq:
      case Tok::keywordOne:
      case Tok::keywordOgt:
      case Tok::keywordOlt:
      case Tok::keywordOge:
      case Tok::keywordOle:
      case Tok::keywordUeq:
      case Tok::keywordUne: return lookup.at(ty);
    }
  }

  // here to be called while debugging
  [[maybe_unused]] void dumpToks(std::string_view source) noexcept {
    auto lex = Lexer(source);

    while (auto tok = lex.next()) {
      std::cout << "[ '" << tok->raw << "', `" << tokTypeToStr(tok->type) << "` ]\n";
    }
  }
} // namespace

std::variant<std::unique_ptr<dawn::Module>, std::string> dawn::parseIRFromText(std::string_view source) noexcept {
  try {
    return Parser(source).parse();
  } catch (const ParsingError& err) {
    return std::string{err.what()};
  }
}
