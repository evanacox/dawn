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
#include "dawn/ir/ir_builder.h"
#include "dawn/ir/ir_reader.h"
#include "helpers/sample_programs.h"
#include "gtest/gtest.h"

namespace {
  bool containsUniqueBlock(std::span<const dawn::BasicBlock* const> blocks, const dawn::BasicBlock* bb) noexcept {
    return std::count(blocks.begin(), blocks.end(), bb) == 1;
  }

  const dawn::BasicBlock* addDirect(dawn::BlockEdges* edges, dawn::IRBuilder* builder) noexcept {
    const auto* bb = builder->createBlock();

    edges->addDirectEdge(bb);

    return bb;
  }

  const dawn::BasicBlock* addIndirect(dawn::BlockEdges* edges, dawn::IRBuilder* builder) noexcept {
    const auto* bb = builder->createBlock();

    edges->addIndirectEdge(bb);

    return bb;
  }
} // namespace

// NOLINTNEXTLINE
TEST(DawnAnalysisCFG, BlockEdgeWorks) {
  auto [mod, builder] = dawn::tests::generateTestModule();

  {
    auto edges = dawn::BlockEdges{};

    const auto* bb1 = addDirect(&edges, &builder);
    const auto* bb2 = addDirect(&edges, &builder);

    auto direct = edges.directEdges();
    auto indirect = edges.indirectEdges();

    EXPECT_EQ(direct.size(), 2);
    EXPECT_TRUE(indirect.empty());

    EXPECT_EQ(std::count(direct.begin(), direct.end(), bb1), 1);
    EXPECT_EQ(std::count(direct.begin(), direct.end(), bb2), 1);
  }

  {
    auto edges = dawn::BlockEdges{};

    const auto* bb1 = addDirect(&edges, &builder);
    const auto* bb2 = addDirect(&edges, &builder);
    const auto* bb3 = addIndirect(&edges, &builder);
    edges.addIndirectEdge(bb3);
    edges.addDirectEdge(bb1);

    auto direct = edges.directEdges();
    auto indirect = edges.indirectEdges();

    EXPECT_EQ(direct.size(), 2);
    EXPECT_EQ(indirect.size(), 1);

    EXPECT_EQ(std::count(direct.begin(), direct.end(), bb1), 1);
    EXPECT_EQ(std::count(direct.begin(), direct.end(), bb2), 1);
    EXPECT_EQ(std::count(indirect.begin(), indirect.end(), bb3), 1);
  }

  {
    auto edges = dawn::BlockEdges{};

    const auto* bb1 = addDirect(&edges, &builder);
    const auto* bb2 = addDirect(&edges, &builder);
    edges.addDirectEdge(bb1);

    auto direct = edges.directEdges();
    auto indirect = edges.indirectEdges();

    EXPECT_EQ(direct.size(), 2);
    EXPECT_TRUE(indirect.empty());

    EXPECT_EQ(std::count(direct.begin(), direct.end(), bb1), 1);
    EXPECT_EQ(std::count(direct.begin(), direct.end(), bb2), 1);
  }

  {
    auto edges = dawn::BlockEdges{};

    const auto* bb1 = addDirect(&edges, &builder);
    const auto* bb2 = addDirect(&edges, &builder);
    const auto* bb3 = addIndirect(&edges, &builder);
    edges.addDirectEdge(bb1);
    edges.addIndirectEdge(bb3);

    auto direct = edges.directEdges();
    auto indirect = edges.indirectEdges();

    EXPECT_EQ(direct.size(), 2);
    EXPECT_EQ(indirect.size(), 1);

    EXPECT_EQ(std::count(direct.begin(), direct.end(), bb1), 1);
    EXPECT_EQ(std::count(direct.begin(), direct.end(), bb2), 1);
  }

  {
    auto edges = dawn::BlockEdges{};

    const auto* bb1 = addDirect(&edges, &builder);
    const auto* bb2 = addDirect(&edges, &builder);
    const auto* bb3 = addIndirect(&edges, &builder);
    const auto* bb4 = addIndirect(&edges, &builder);
    const auto* bb5 = addIndirect(&edges, &builder);
    const auto* bb6 = addDirect(&edges, &builder);

    edges.addIndirectEdge(bb5); // shouldn't be added again, set-like invariant should be maintained
    edges.addIndirectEdge(bb5);
    edges.addIndirectEdge(bb6); // already a direct edge
    edges.addIndirectEdge(bb6);
    edges.addDirectEdge(bb6); // shouldn't be added again, set-like invariant should be maintained
    edges.addDirectEdge(bb6);
    edges.addDirectEdge(bb5); // bb5 is already an indirect edge
    edges.addDirectEdge(bb5);

    auto all = edges.allEdges();
    auto direct = edges.directEdges();
    auto indirect = edges.indirectEdges();

    EXPECT_EQ(all.size(), 8);
    // both of these should exist as both direct and indirect edges, but only once each
    EXPECT_EQ(std::count(all.begin(), all.end(), bb6), 2);
    EXPECT_EQ(std::count(all.begin(), all.end(), bb5), 2);

    EXPECT_EQ(direct.size(), 4);
    EXPECT_EQ(std::count(direct.begin(), direct.end(), bb1), 1);
    EXPECT_EQ(std::count(direct.begin(), direct.end(), bb2), 1);
    EXPECT_EQ(std::count(direct.begin(), direct.end(), bb5), 1);
    EXPECT_EQ(std::count(direct.begin(), direct.end(), bb6), 1);

    EXPECT_EQ(indirect.size(), 4);
    EXPECT_EQ(std::count(indirect.begin(), indirect.end(), bb3), 1);
    EXPECT_EQ(std::count(indirect.begin(), indirect.end(), bb4), 1);
    EXPECT_EQ(std::count(indirect.begin(), indirect.end(), bb5), 1);
    EXPECT_EQ(std::count(indirect.begin(), indirect.end(), bb6), 1);
  }
}

TEST(DawnAnalysisCFG, CfgEdgeAnalysisCorrectWithSampleIfElse) {
  auto module = dawn::tests::sampleIfElse();
  auto builder = dawn::IRBuilder{module.get()};
  auto* fn = module->findFunc("f").get();
  builder.setInsertFn(fn);

  auto* entry = builder.findBlockWithName("entry").value_or(nullptr);
  auto* ifTrue = builder.findBlockWithName("if.true").value_or(nullptr);
  auto* ifFalse = builder.findBlockWithName("if.false").value_or(nullptr);
  auto* merge = builder.findBlockWithName("merge").value_or(nullptr);

  auto edges = dawn::calculateCfgEdges(*fn);

  // %entry
  {
    auto directPreds = edges.directPredecessors(entry);
    auto indirectPreds = edges.indirectPredecessors(entry);

    EXPECT_TRUE(directPreds.empty());
    EXPECT_TRUE(indirectPreds.empty());

    auto directSuccessors = edges.directSuccessors(entry);
    auto indirectSuccessors = edges.indirectSuccessors(entry);
    const auto& successors = edges.successors(entry);

    EXPECT_EQ(directSuccessors.size(), 2);
    EXPECT_TRUE(successors.hasDirectEdge(ifTrue));
    EXPECT_TRUE(successors.hasDirectEdge(ifFalse));
    EXPECT_TRUE(containsUniqueBlock(directSuccessors, ifTrue));
    EXPECT_TRUE(containsUniqueBlock(directSuccessors, ifFalse));

    EXPECT_EQ(indirectSuccessors.size(), 1);
    EXPECT_TRUE(successors.hasIndirectEdge(merge));
    EXPECT_TRUE(containsUniqueBlock(indirectSuccessors, merge));
  }

  // %if.false
  {
    auto directPreds = edges.directPredecessors(ifTrue);
    auto indirectPreds = edges.indirectPredecessors(ifTrue);
    const auto& preds = edges.predecessors(ifTrue);

    EXPECT_EQ(directPreds.size(), 1);
    EXPECT_TRUE(indirectPreds.empty());
    EXPECT_TRUE(preds.hasDirectEdge(entry));
    EXPECT_TRUE(containsUniqueBlock(directPreds, entry));

    auto directSuccessors = edges.directSuccessors(ifTrue);
    auto indirectSuccessors = edges.indirectSuccessors(ifTrue);
    const auto& successors = edges.successors(ifTrue);

    EXPECT_EQ(directSuccessors.size(), 1);
    EXPECT_TRUE(successors.hasDirectEdge(merge));
    EXPECT_TRUE(containsUniqueBlock(directSuccessors, merge));
    EXPECT_TRUE(indirectSuccessors.empty());
  }

  // %if.true
  {
    auto directPreds = edges.directPredecessors(ifFalse);
    auto indirectPreds = edges.indirectPredecessors(ifFalse);
    const auto& preds = edges.predecessors(ifFalse);

    EXPECT_EQ(directPreds.size(), 1);
    EXPECT_TRUE(indirectPreds.empty());
    EXPECT_TRUE(preds.hasDirectEdge(entry));
    EXPECT_TRUE(containsUniqueBlock(directPreds, entry));

    auto directSuccessors = edges.directSuccessors(ifFalse);
    auto indirectSuccessors = edges.indirectSuccessors(ifFalse);
    const auto& successors = edges.successors(ifFalse);

    EXPECT_EQ(directSuccessors.size(), 1);
    EXPECT_TRUE(successors.hasDirectEdge(merge));
    EXPECT_TRUE(containsUniqueBlock(directSuccessors, merge));
    EXPECT_TRUE(indirectSuccessors.empty());
  }

  // %merge
  {
    auto directPreds = edges.directPredecessors(merge);
    auto indirectPreds = edges.indirectPredecessors(merge);
    const auto& preds = edges.predecessors(merge);

    EXPECT_EQ(directPreds.size(), 2);
    EXPECT_EQ(indirectPreds.size(), 1);
    EXPECT_TRUE(preds.hasDirectEdge(ifTrue));
    EXPECT_TRUE(preds.hasDirectEdge(ifFalse));
    EXPECT_TRUE(preds.hasIndirectEdge(entry));
    EXPECT_TRUE(containsUniqueBlock(directPreds, ifTrue));
    EXPECT_TRUE(containsUniqueBlock(directPreds, ifFalse));
    EXPECT_TRUE(containsUniqueBlock(indirectPreds, entry));

    auto directSuccessors = edges.directSuccessors(merge);
    auto indirectSuccessors = edges.indirectSuccessors(merge);

    EXPECT_TRUE(directSuccessors.empty());
    EXPECT_TRUE(indirectSuccessors.empty());
  }
}

TEST(DawnAnalysisCFG, CfgEdgeAnalysisCorrectWithSampleLoops) {
  //
}