#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <vector>

#include "bench/benchmark_common.h"
#include "internpath/algo/Searching.h"
#include "internpath/ds/DeadlineAVL.h"
#include "internpath/ds/HashTable.h"

using namespace internpath;
using namespace internpath::bench;

int main()
{
    if (!checkSystemSafety())
    {
        return 1;
    }

    BenchmarkRunner runner("Searching Algorithms & Indexes");
    runner.printHeader();

    const std::vector<std::size_t> sizes = {1000, 10000, 100000, 1000000};
    std::mt19937 rng(42);

    for (std::size_t n : sizes)
    {
        std::string label = "N=" + std::to_string(n);

        // Prepare sorted array of even integers
        std::vector<int> sortedData(n);
        for (std::size_t i = 0; i < n; i++)
        {
            sortedData[i] = static_cast<int>(i * 2);
        }

        // Prepare HashTable
        HashTable<int, int> table;
        for (std::size_t i = 0; i < n; i++)
        {
            table.insertOrAssign(sortedData[i], static_cast<int>(i));
        }

        // Prepare DeadlineAVL
        DeadlineAVL avl;
        for (std::size_t i = 0; i < n; i++)
        {
            avl.insert({sortedData[i], static_cast<std::uint32_t>(i + 1)});
        }

        auto lessCmp = [](int a, int b) { return a < b; };

        // 1a. Linear Search Hits (target in middle)
        int hitTarget = sortedData[n / 2];
        runner.run("LinearSearch::hit", label, 100, 5, [&]() {
            for (int q = 0; q < 100; q++)
            {
                long long idx = linearSearch(sortedData.data(), sortedData.size(), hitTarget);
                assert(idx == static_cast<long long>(n / 2));
            }
        });

        // 1b. Linear Search Misses (target absent: O(N) full traversal)
        int missTarget = hitTarget + 1; // odd number, not present
        runner.run("LinearSearch::miss", label, 100, 5, [&]() {
            for (int q = 0; q < 100; q++)
            {
                long long idx = linearSearch(sortedData.data(), sortedData.size(), missTarget);
                assert(idx == -1);
            }
        });

        // 1c. Binary Search: lowerBound (O(log N))
        runner.run("BinarySearch::lowerBound", label, 10000, 5, [&]() {
            for (int q = 0; q < 10000; q++)
            {
                int query = static_cast<int>((rng() % n) * 2);
                std::size_t idx = lowerBound(sortedData.data(), sortedData.size(), query, lessCmp);
                assert(idx < n && sortedData[idx] == query);
            }
        });

        // 1d. HashTable Exact Lookup: Expected O(1)
        runner.run("HashTable::exact_lookup", label, 10000, 5, [&]() {
            int outVal = 0;
            for (int q = 0; q < 10000; q++)
            {
                int query = static_cast<int>((rng() % n) * 2);
                bool found = table.tryGet(query, outVal);
                assert(found);
            }
        });

        // 1e. DeadlineAVL Range Search: O(log N + K)
        DynamicArray<std::uint32_t> matched;
        runner.run("DeadlineAVL::range_query", label, 1000, 5, [&]() {
            for (int q = 0; q < 1000; q++)
            {
                matched.clear();
                int start = static_cast<int>((rng() % (n / 2)) * 2);
                int end = start + 200;
                avl.range(start, end, matched);
            }
        });

        // 1f. Differential Reference: std::lower_bound (Oracle)
        runner.run("std::lower_bound::oracle", label, 10000, 5, [&]() {
            for (int q = 0; q < 10000; q++)
            {
                int query = static_cast<int>((rng() % n) * 2);
                auto it = std::lower_bound(sortedData.begin(), sortedData.end(), query);
                assert(it != sortedData.end() && *it == query);
            }
        });
    }

    runner.saveCsv("bench_search_results.csv");
    return 0;
}
