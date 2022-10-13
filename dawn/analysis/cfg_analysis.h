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

#include "../ir/basic_block.h"
#include "../ir/function.h"
#include "./analysis_passes.h"
#include "absl/container/flat_hash_map.h"
#include <span>
#include <vector>

namespace dawn {
  class BlockEdges {
  public:
    BlockEdges() = default;

    explicit BlockEdges(std::size_t expected) noexcept;

    void addDirectEdge(const BasicBlock* bb) noexcept;

    void addIndirectEdge(const BasicBlock* bb) noexcept;

    void reserveAdditional(std::size_t expected) noexcept;

    // may contain duplicates, blocks that can have direct and indirect edges with a block
    [[nodiscard]] std::span<const BasicBlock* const> allEdges() const noexcept {
      return edges_;
    }

    [[nodiscard]] std::span<const BasicBlock* const> directEdges() const noexcept {
      return {edges_.data(), indexOfDirectEdgeEnd_};
    }

    [[nodiscard]] std::span<const BasicBlock* const> indirectEdges() const noexcept {
      return {edges_.data() + indexOfDirectEdgeEnd_, edges_.data() + edges_.size()};
    }

    [[nodiscard]] bool hasDirectEdge(const BasicBlock* bb) const noexcept;

    [[nodiscard]] bool hasIndirectEdge(const BasicBlock* bb) const noexcept;

    [[nodiscard]] bool hasEdge(const BasicBlock* bb) const noexcept {
      return hasDirectEdge(bb) || hasIndirectEdge(bb);
    }

  private:
    [[nodiscard]] auto indirectEdgeBegin() const noexcept {
      return edges_.cbegin() + static_cast<std::ptrdiff_t>(indexOfDirectEdgeEnd_);
    }

    [[nodiscard]] auto indirectEdgeBegin() noexcept {
      return edges_.begin() + static_cast<std::ptrdiff_t>(indexOfDirectEdgeEnd_);
    }

    // array is effectively
    //
    //   [ direct, direct, direct, indirect, indirect ]
    //                             ^
    //                             index
    //
    // the index just needs to be the index of either the end
    // or the first non-direct edge, and our invariant is maintained.
    //
    // we also maintain that both of the two sub-arrays are sorted, allowing us
    // to binary search for elements to maintain unique-ness
    std::size_t indexOfDirectEdgeEnd_ = 0;
    std::vector<const BasicBlock*> edges_;
  };

  using BlockEdgeStorage = absl::flat_hash_map<const BasicBlock*, BlockEdges>;

  class FunctionCFGEdges final {
  public:
    FunctionCFGEdges() = default;

    explicit FunctionCFGEdges(BlockEdgeStorage successors, BlockEdgeStorage predecessors) noexcept;

    [[nodiscard]] std::span<const BasicBlock* const> directSuccessors(const BasicBlock* bb) const noexcept {
      DAWN_ASSERT(successors_.contains(bb), "cannot get successors of block from different function");

      return successors_.find(bb)->second.directEdges();
    }

    [[nodiscard]] std::span<const BasicBlock* const> directPredecessors(const BasicBlock* bb) const noexcept {
      DAWN_ASSERT(predecessors_.contains(bb), "cannot get successors of block from different function");

      return predecessors_.find(bb)->second.directEdges();
    }

    [[nodiscard]] std::span<const BasicBlock* const> indirectSuccessors(const BasicBlock* bb) const noexcept {
      DAWN_ASSERT(successors_.contains(bb), "cannot get successors of block from different function");

      return successors_.find(bb)->second.indirectEdges();
    }

    [[nodiscard]] std::span<const BasicBlock* const> indirectPredecessors(const BasicBlock* bb) const noexcept {
      DAWN_ASSERT(predecessors_.contains(bb), "cannot get successors of block from different function");

      return predecessors_.find(bb)->second.indirectEdges();
    }

    [[nodiscard]] std::span<const BasicBlock* const> allSuccessors(const BasicBlock* bb) const noexcept {
      DAWN_ASSERT(successors_.contains(bb), "cannot get successors of block from different function");

      return successors_.find(bb)->second.allEdges();
    }

    [[nodiscard]] std::span<const BasicBlock* const> allPredecessors(const BasicBlock* bb) const noexcept {
      DAWN_ASSERT(predecessors_.contains(bb), "cannot get successors of block from different function");

      return predecessors_.find(bb)->second.allEdges();
    }

    [[nodiscard]] const BlockEdges& predecessors(const BasicBlock* bb) const noexcept {
      DAWN_ASSERT(predecessors_.contains(bb), "cannot get edges of block from different function");

      return predecessors_.at(bb);
    }

    [[nodiscard]] const BlockEdges& successors(const BasicBlock* bb) const noexcept {
      DAWN_ASSERT(successors_.contains(bb), "cannot get edges of block from different function");

      return successors_.find(bb)->second;
    }

  private:
    BlockEdgeStorage successors_;
    BlockEdgeStorage predecessors_;
  };

  class CFGAnalysis final : public FunctionAnalysisPass {
  public:
    inline constexpr static Analysis kind = Analysis::cfgAnalysis;

    CFGAnalysis() noexcept : FunctionAnalysisPass(this) {}

    void run(const Function& fn, class AnalysisManager* manager) noexcept final;

    [[nodiscard]] const FunctionCFGEdges& edges(const Function& fn) const noexcept {
      return edges_.at(&fn);
    }

  private:
    absl::flat_hash_map<const Function*, FunctionCFGEdges> edges_;
  };

  [[nodiscard]] FunctionCFGEdges calculateCfgEdges(const Function& fn) noexcept;
} // namespace dawn