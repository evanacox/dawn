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

#include "absl/container/flat_hash_map.h"
#include "dawn/ir/basic_block.h"
#include "dawn/ir/function.h"
#include <span>
#include <vector>

namespace dawn {
  namespace internal {
    struct EdgeStorage {
      std::size_t indexOfDirectEdgeEnd;
      std::vector<const BasicBlock*> edges;
    };
  } // namespace internal

  class CfgBlockEdges {
  public:
    explicit CfgBlockEdges(absl::flat_hash_map<const BasicBlock*, internal::EdgeStorage> successors,
        absl::flat_hash_map<const BasicBlock*, internal::EdgeStorage> predecessors) noexcept;

    [[nodiscard]] std::span<const BasicBlock* const> directSuccessorsOf(const BasicBlock* bb) const noexcept {
      DAWN_ASSERT(successors_.contains(bb), "cannot get successors of block from different function");

      const auto& storage = successors_.find(bb)->second;

      return {storage.edges.data(), storage.indexOfDirectEdgeEnd};
    }

    [[nodiscard]] std::span<const BasicBlock* const> directPredecessorsOf(const BasicBlock* bb) const noexcept {
      DAWN_ASSERT(successors_.contains(bb), "cannot get successors of block from different function");

      const auto& storage = predecessors_.find(bb)->second;

      return {storage.edges.data(), storage.indexOfDirectEdgeEnd};
    }

    [[nodiscard]] std::span<const BasicBlock* const> indirectSuccessorsOf(const BasicBlock* bb) const noexcept {
      DAWN_ASSERT(successors_.contains(bb), "cannot get successors of block from different function");

      const auto& storage = successors_.find(bb)->second;

      return {storage.edges.data() + storage.indexOfDirectEdgeEnd, storage.edges.data() + storage.edges.size()};
    }

    [[nodiscard]] std::span<const BasicBlock* const> indirectPredecessorsOf(const BasicBlock* bb) const noexcept {
      DAWN_ASSERT(successors_.contains(bb), "cannot get successors of block from different function");

      const auto& storage = predecessors_.find(bb)->second;
      return {storage.edges.data() + storage.indexOfDirectEdgeEnd, storage.edges.data() + storage.edges.size()};
    }

    [[nodiscard]] std::span<const BasicBlock* const> allSuccessorsOf(const BasicBlock* bb) const noexcept {
      DAWN_ASSERT(successors_.contains(bb), "cannot get successors of block from different function");

      return successors_.find(bb)->second.edges;
    }

    [[nodiscard]] std::span<const BasicBlock* const> allPredecessorsOf(const BasicBlock* bb) const noexcept {
      DAWN_ASSERT(successors_.contains(bb), "cannot get successors of block from different function");

      return predecessors_.find(bb)->second.edges;
    }

  private:
    absl::flat_hash_map<const BasicBlock*, internal::EdgeStorage> successors_;
    absl::flat_hash_map<const BasicBlock*, internal::EdgeStorage> predecessors_;
  };

  [[nodiscard]] CfgBlockEdges calculateCfgEdges(const Function* fn) noexcept;
} // namespace dawn