#pragma once

#include <cstdint>

#include "internpath/ds/DirectedGraph.h"
#include "internpath/ds/DynamicArray.h"

namespace internpath
{
    // MEMBER C | Implement in src/algo/GraphAlgorithms.cpp after DirectedGraph.
    bool isReachable(const DirectedGraph& graph,
                     std::uint32_t from, std::uint32_t target);

    // Return false if cycle exists; output valid topological ordering otherwise.
    bool topologicalOrder(const DirectedGraph& graph,
                          DynamicArray<std::uint32_t>& order);

    // Plan depends on reverse demand propagation with edge minimum proficiency.
    // See docs/DOMAIN_RULES.md before implementing.
}
