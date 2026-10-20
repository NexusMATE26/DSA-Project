#include <cassert>
#include <iostream>
#include <random>
#include <vector>

#include "bench/benchmark_common.h"
#include "internpath/algo/GraphAlgorithms.h"
#include "internpath/ds/DirectedGraph.h"

using namespace internpath;
using namespace internpath::bench;

int main()
{
    if (!checkSystemSafety())
    {
        return 1;
    }

    BenchmarkRunner runner("Directed Graph & Prerequisite Algorithms");
    runner.printHeader();

    const std::vector<std::size_t> vertexCounts = {100, 500, 2000, 5000};
    std::mt19937 rng(42);

    for (std::size_t v : vertexCounts)
    {
        std::string label = "V=" + std::to_string(v);
        std::size_t targetEdges = v * 3; // Average out-degree of 3

        // Build acyclic edges using tier stratification
        std::vector<std::pair<std::uint32_t, std::uint32_t>> validEdges;
        validEdges.reserve(targetEdges);

        for (std::uint32_t from = 0; from < v; from++)
        {
            // Connect only to higher indexed vertices to guarantee acyclicity
            for (int k = 0; k < 3; k++)
            {
                if (from + 1 < v)
                {
                    std::uint32_t to = from + 1 + (rng() % (v - from - 1));
                    validEdges.push_back({from, to});
                }
            }
        }

        // 1a. Graph Construction (with cycle-safety DFS checks on every edge)
        DirectedGraph graph;
        for (std::size_t i = 0; i < v; i++) graph.addVertex();

        runner.run("DirectedGraph::construction", label, validEdges.size(), 3, [&]() {
            DirectedGraph localGraph;
            for (std::size_t i = 0; i < v; i++) localGraph.addVertex();
            for (const auto& edge : validEdges)
            {
                localGraph.addEdge(edge.first, edge.second, 1);
            }
        });

        for (const auto& edge : validEdges)
        {
            graph.addEdge(edge.first, edge.second, 1);
        }

        // 1b. DFS Reachability Queries (Stack-based)
        runner.run("DFS::isReachable_queries", label, 1000, 5, [&]() {
            for (int q = 0; q < 1000; q++)
            {
                std::uint32_t from = rng() % v;
                std::uint32_t to = rng() % v;
                bool reachable = isReachable(graph, from, to);
                (void)reachable;
            }
        });

        // 1c. Cycle Rejection Test (Attempting to add back-edge v-1 -> 0)
        runner.run("DirectedGraph::cycle_rejection", label, 100, 5, [&]() {
            for (int c = 0; c < 100; c++)
            {
                if (v >= 2)
                {
                    bool added = graph.addEdge(static_cast<std::uint32_t>(v - 1), 0, 1);
                    assert(!added); // Must be strictly rejected!
                }
            }
        });

        // 1d. Kahn's Topological Sorting (CircularQueue-based)
        runner.run("Kahn::topologicalOrder", label, v, 5, [&]() {
            DynamicArray<std::uint32_t> order;
            bool valid = topologicalOrder(graph, order);
            assert(valid);
            assert(order.size() == v);

            // Invariant Verification: for all u -> v, pos(u) < pos(v)
            std::vector<std::size_t> pos(v);
            for (std::size_t i = 0; i < order.size(); i++)
            {
                pos[order[i]] = i;
            }
            for (const auto& edge : validEdges)
            {
                assert(pos[edge.first] < pos[edge.second]);
            }
        });
    }

    runner.saveCsv("bench_graph_results.csv");
    return 0;
}
