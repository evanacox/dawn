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

#include "dawn/analysis/cfg_analysis.h"
#include "dawn/ir/instructions.h"
#include "dawn/utility/assertions.h"
#include <algorithm>

namespace {
  dawn::BlockEdges calculateDirectSuccessors(const dawn::BasicBlock& block) noexcept {
    const auto* val = block.terminator();

    dawn::BlockEdges edges{val->possibleBranchTargets().size()};

    // extremely simple approach. look at the terminator of the block, and check
    // which blocks that terminator can branch to. all of those are possible direct successors
    for (const auto* bb : val->possibleBranchTargets()) {
      edges.addDirectEdge(bb);
    }

    return edges;
  }

  dawn::BlockEdges calculateDirectPredecessors(const dawn::BasicBlock& block,
      const dawn::BlockEdgeStorage& allDirectSuccessors) noexcept {
    auto edges = dawn::BlockEdges{};

    // simple algorithm. to find the direct preds of `block`, look in each block in the CFG,
    // and check if that block is a direct successor of `block`. if it is, we've found a predecessor
    for (const auto& [bb, successors] : allDirectSuccessors) {
      if (successors.hasDirectEdge(&block)) {
        edges.addDirectEdge(bb);
      }
    }

    return edges;
  }

  // NOLINTNEXTLINE(misc-no-recursion)
  void calculateIndirectSuccessors(const dawn::BasicBlock* bb,
      dawn::BlockEdgeStorage* all,
      std::vector<const dawn::BasicBlock*>* addTo,
      absl::flat_hash_set<const dawn::BasicBlock*>* seen) noexcept {
    // prevent getting stuck in infinite recursion when we have a cycle
    if (!seen->insert(bb).second) {
      return;
    }

    for (const auto* successor : all->at(bb).directEdges()) {
      for (const auto* pred : *addTo) {
        all->at(pred).addIndirectEdge(successor);
      }

      addTo->push_back(bb);

      calculateIndirectSuccessors(successor, all, addTo, seen);

      addTo->pop_back();
    }
  }

  void calculateAllIndirectSuccessors(std::span<const dawn::BasicBlock> blocks, dawn::BlockEdgeStorage* all) noexcept {
    if (blocks.empty()) {
      return;
    }

    const auto* root = &blocks.front();
    auto addTo = std::vector<const dawn::BasicBlock*>{};
    auto seen = absl::flat_hash_set<const dawn::BasicBlock*>{};

    calculateIndirectSuccessors(root, all, &addTo, &seen);
  }

  void calculateAllIndirectPredecessors(std::span<const dawn::BasicBlock> blocks,
      const dawn::BlockEdgeStorage& allSuccessors,
      dawn::BlockEdgeStorage* allPredecessors) noexcept {
    for (const auto& [bb, edges] : allSuccessors) {
      for (const auto& block : blocks) {
        if (edges.hasIndirectEdge(&block)) {
          allPredecessors->at(&block).addIndirectEdge(bb);
        }
      }
    }
  }

  [[nodiscard]] dawn::FunctionCFGEdges calculateBlockEdges(const dawn::Function& fn) noexcept {
    dawn::BlockEdgeStorage successors;
    dawn::BlockEdgeStorage predecessors;

    for (const auto& bb : fn.blocks()) {
      successors.emplace(&bb, calculateDirectSuccessors(bb));
    }

    for (const auto& bb : fn.blocks()) {
      predecessors.emplace(&bb, calculateDirectPredecessors(bb, successors));
    }

    // right now, successors only has direct edges
    calculateAllIndirectSuccessors(fn.blocks(), &successors);

    // now that we have all indirect successors calculated, we
    // can calculate predecessors trivially
    calculateAllIndirectPredecessors(fn.blocks(), successors, &predecessors);

    return dawn::FunctionCFGEdges{std::move(successors), std::move(predecessors)};
  }
} // namespace

namespace dawn {
  BlockEdges::BlockEdges(std::size_t expected) noexcept {
    reserveAdditional(expected);
  }

  void BlockEdges::addDirectEdge(const dawn::BasicBlock* bb) noexcept {
    // we try to act like a set here, only unique direct/indirect edges
    if (hasDirectEdge(bb)) {
      return;
    }

    // insert and maintain our sorted requirement
    edges_.insert(std::upper_bound(edges_.begin(), indirectEdgeBegin(), bb), bb);

    ++indexOfDirectEdgeEnd_;
  }

  void BlockEdges::addIndirectEdge(const dawn::BasicBlock* bb) noexcept {
    if (hasIndirectEdge(bb)) {
      return;
    }

    // no need to update the direct edges index, it's still accurate if we append
    // an indirect edge no matter what the value of it is.
    edges_.insert(std::upper_bound(indirectEdgeBegin(), edges_.end(), bb), bb);
  }

  void BlockEdges::reserveAdditional(std::size_t expected) noexcept {
    edges_.reserve(edges_.size() + expected);
  }

  bool BlockEdges::hasDirectEdge(const dawn::BasicBlock* bb) const noexcept {
    auto range = directEdges();

    DAWN_ASSERT(std::is_sorted(range.begin(), range.end()), "edges should be sorted");

    return std::binary_search(range.begin(), range.end(), bb);
  }

  bool BlockEdges::hasIndirectEdge(const dawn::BasicBlock* bb) const noexcept {
    auto range = indirectEdges();

    DAWN_ASSERT(std::is_sorted(range.begin(), range.end()), "edges should be sorted");

    return std::binary_search(range.begin(), range.end(), bb);
  }

  FunctionCFGEdges::FunctionCFGEdges(absl::flat_hash_map<const BasicBlock*, BlockEdges> successors,
      absl::flat_hash_map<const BasicBlock*, BlockEdges> predecessors) noexcept
      : successors_{std::move(successors)},
        predecessors_{std::move(predecessors)} {}

  void CFGAnalysis::run(const Function& fn, AnalysisManager* /*unused*/) noexcept {
    edges_[&fn] = calculateCfgEdges(fn);
  }
} // namespace dawn

dawn::FunctionCFGEdges dawn::calculateCfgEdges(const dawn::Function& fn) noexcept {
  return calculateBlockEdges(fn);
}