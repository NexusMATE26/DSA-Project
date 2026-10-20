#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <set>
#include <vector>

#include "bench/benchmark_common.h"
#include "internpath/ds/DeadlineAVL.h"

using namespace internpath;
using namespace internpath::bench;

int main()
{
    if (!checkSystemSafety())
    {
        return 1;
    }

    BenchmarkRunner runner("AVL Self-Balancing Tree (DeadlineAVL)");
    runner.printHeader();

    const std::vector<std::size_t> sizes = {1000, 10000, 100000, 500000};
    std::mt19937 rng(42);

    for (std::size_t n : sizes)
    {
        std::string label = "N=" + std::to_string(n);

        // Generate keys
        std::vector<DeadlineKey> randomKeys;
        randomKeys.reserve(n);
        for (std::size_t i = 0; i < n; i++)
        {
            std::int32_t deadline = 20261100 + static_cast<std::int32_t>(rng() % 365);
            randomKeys.push_back({deadline, static_cast<std::uint32_t>(i + 1)});
        }

        // 1a. Random AVL Insertions
        runner.run("DeadlineAVL::insert_random", label, n, 3, [&]() {
            DeadlineAVL avl;
            for (std::size_t i = 0; i < n; i++)
            {
                avl.insert(randomKeys[i]);
            }
            assert(avl.size() == n);
        });

        // 1b. Adversarial Sequential Insertions (Forcing continuous RR rotations)
        std::vector<DeadlineKey> seqKeys;
        seqKeys.reserve(n);
        for (std::size_t i = 0; i < n; i++)
        {
            seqKeys.push_back({static_cast<std::int32_t>(i + 1), static_cast<std::uint32_t>(i + 1)});
        }

        runner.run("DeadlineAVL::insert_adversarial", label, n, 3, [&]() {
            DeadlineAVL avl;
            for (std::size_t i = 0; i < n; i++)
            {
                avl.insert(seqKeys[i]);
            }
            assert(avl.size() == n);
        });

        // Prepopulate AVL for queries and erasures
        DeadlineAVL avl;
        for (std::size_t i = 0; i < n; i++) avl.insert(randomKeys[i]);

        // 1c. Structural Invariant Validation (BST + Balance Factor <= 1)
        runner.run("DeadlineAVL::validate", label, 1, 3, [&]() {
            bool valid = avl.validate();
            assert(valid);
        });

        // 1d. Range Queries with Subtree Pruning
        std::int32_t queryFrom = 20261110;
        std::int32_t queryTo = 20261150;
        DynamicArray<std::uint32_t> matchedIds;

        runner.run("DeadlineAVL::range_pruned", label, 100, 5, [&]() {
            for (int q = 0; q < 100; q++)
            {
                matchedIds.clear();
                avl.range(queryFrom, queryTo, matchedIds);
            }
        });

        // 1e. Linear Scan Baseline (to demonstrate O(log n + k) vs O(n))
        runner.run("LinearScan::range_baseline", label, 100, 5, [&]() {
            for (int q = 0; q < 100; q++)
            {
                std::vector<std::uint32_t> ids;
                for (std::size_t i = 0; i < n; i++)
                {
                    if (randomKeys[i].deadline >= queryFrom && randomKeys[i].deadline <= queryTo)
                    {
                        ids.push_back(randomKeys[i].internshipId);
                    }
                }
            }
        });

        // 1f. AVL Node Erasure
        runner.run("DeadlineAVL::erase", label, n / 2, 3, [&]() {
            DeadlineAVL localAvl;
            for (std::size_t i = 0; i < n; i++) localAvl.insert(randomKeys[i]);
            for (std::size_t i = 0; i < n / 2; i++)
            {
                localAvl.erase(randomKeys[i]);
            }
            assert(localAvl.size() == n - (n / 2));
            assert(localAvl.validate());
        });

        // 1g. Differential Reference: std::set (Oracle)
        runner.run("std::set::insert_oracle", label, n, 3, [&]() {
            std::set<std::pair<std::int32_t, std::uint32_t>> stdSet;
            for (std::size_t i = 0; i < n; i++)
            {
                stdSet.insert({randomKeys[i].deadline, randomKeys[i].internshipId});
            }
            assert(stdSet.size() == n);
        });
    }

    runner.saveCsv("bench_avl_results.csv");
    return 0;
}
