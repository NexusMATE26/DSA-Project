#include <cassert>
#include <iostream>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "bench/benchmark_common.h"
#include "internpath/ds/HashTable.h"
#include "internpath/ds/Hashing.h"

using namespace internpath;
using namespace internpath::bench;

int main()
{
    if (!checkSystemSafety())
    {
        return 1;
    }

    BenchmarkRunner runner("Hash Table & Hashing Algorithms");
    runner.printHeader();

    const std::vector<std::size_t> sizes = {1000, 10000, 100000, 500000};
    std::mt19937_64 rng(42);

    // -------------------------------------------------------------
    // 1. Raw Hash Function Throughput
    // -------------------------------------------------------------
    for (std::size_t n : {100000, 1000000})
    {
        std::string label = "N=" + std::to_string(n);

        runner.run("Hash::SplitMix64_integers", label, n, 5, [&]() {
            volatile std::uint64_t dummy = 0;
            for (std::size_t i = 0; i < n; i++)
            {
                dummy ^= hashNumber(i);
            }
        });

        std::vector<std::string> keys;
        keys.reserve(n);
        for (std::size_t i = 0; i < n; i++)
        {
            keys.push_back("candidate_skill_key_" + std::to_string(i));
        }

        runner.run("Hash::FNV1a_strings", label, n, 5, [&]() {
            volatile std::uint64_t dummy = 0;
            for (std::size_t i = 0; i < n; i++)
            {
                dummy ^= hashText(keys[i]);
            }
        });
    }

    // -------------------------------------------------------------
    // 2. HashTable Insertion & Chaining Rehash
    // -------------------------------------------------------------
    for (std::size_t n : sizes)
    {
        std::string label = "N=" + std::to_string(n);

        std::vector<std::uint32_t> keys;
        keys.reserve(n);
        // Guarantee unique keys via index-based generator with 64-bit hash dispersion
        for (std::size_t i = 0; i < n; i++)
        {
            keys.push_back(static_cast<std::uint32_t>((i + 1) * 2654435761ULL));
        }

        // Custom HashTable Insert
        HashTable<std::uint32_t, std::uint32_t> table;
        runner.run("HashTable::insertOrAssign", label, n, 3, [&]() {
            HashTable<std::uint32_t, std::uint32_t> localTable;
            for (std::size_t i = 0; i < n; i++)
            {
                localTable.insertOrAssign(keys[i], static_cast<std::uint32_t>(i));
            }
            assert(localTable.size() == n);
        });

        // Prepopulate table for search & delete benchmarks
        for (std::size_t i = 0; i < n; i++)
        {
            table.insertOrAssign(keys[i], static_cast<std::uint32_t>(i));
        }

        // 2b. Lookup Hits
        runner.run("HashTable::tryGet_hits", label, n, 5, [&]() {
            std::uint32_t outVal = 0;
            for (std::size_t i = 0; i < n; i++)
            {
                bool found = table.tryGet(keys[i], outVal);
                assert(found);
            }
        });

        // 2c. Lookup Misses
        runner.run("HashTable::tryGet_misses", label, n, 5, [&]() {
            std::uint32_t outVal = 0;
            for (std::size_t i = 0; i < n; i++)
            {
                // Odd numbers offset will likely miss
                bool found = table.tryGet(keys[i] ^ 0xFFFFFFFF, outVal);
                (void)found;
            }
        });

        // 2d. Invariant Validation
        runner.run("HashTable::validate", label, 1, 3, [&]() {
            bool valid = table.validate();
            assert(valid);
        });

        // 2e. Erase operations
        runner.run("HashTable::erase", label, n / 2, 3, [&]() {
            HashTable<std::uint32_t, std::uint32_t> localTable;
            for (std::size_t i = 0; i < n; i++) localTable.insertOrAssign(keys[i], static_cast<std::uint32_t>(i));

            for (std::size_t i = 0; i < n / 2; i++)
            {
                localTable.erase(keys[i]);
            }
            assert(localTable.size() == n - (n / 2));
        });

        // 2f. Differential Reference: std::unordered_map (Oracle)
        runner.run("std::unordered_map::insert", label, n, 3, [&]() {
            std::unordered_map<std::uint32_t, std::uint32_t> stdMap;
            for (std::size_t i = 0; i < n; i++)
            {
                stdMap[keys[i]] = static_cast<std::uint32_t>(i);
            }
            assert(stdMap.size() == n);
        });
    }

    runner.saveCsv("bench_hash_results.csv");
    return 0;
}
