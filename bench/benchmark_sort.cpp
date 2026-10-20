#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <vector>

#include "bench/benchmark_common.h"
#include "internpath/algo/Sorting.h"

using namespace internpath;
using namespace internpath::bench;

struct ApplicantRankKey
{
    std::int32_t fitNumerator = 0;
    std::uint64_t submitSequence = 0;
    std::uint32_t id = 0;
};

int main()
{
    if (!checkSystemSafety())
    {
        return 1;
    }

    BenchmarkRunner runner("Sorting Algorithms (Merge, Insertion, Quick)");
    runner.printHeader();

    const std::vector<std::size_t> sizes = {1000, 10000, 100000, 1000000};
    std::mt19937 rng(42);

    for (std::size_t n : sizes)
    {
        std::string label = "N=" + std::to_string(n);

        std::vector<int> randomData(n);
        for (std::size_t i = 0; i < n; i++) randomData[i] = static_cast<int>(rng());

        std::vector<int> sortedData(n);
        for (std::size_t i = 0; i < n; i++) sortedData[i] = static_cast<int>(i);

        std::vector<int> reverseData(n);
        for (std::size_t i = 0; i < n; i++) reverseData[i] = static_cast<int>(n - i);

        std::vector<int> duplicateData(n);
        for (std::size_t i = 0; i < n; i++) duplicateData[i] = static_cast<int>(i % 5);

        auto lessCmp = [](int a, int b) { return a < b; };

        // 1a. Custom MergeSort on Random Data
        runner.run("MergeSort::random", label, n, 3, [&]() {
            std::vector<int> buffer = randomData;
            mergeSort(buffer.data(), buffer.size(), lessCmp);
            assert(std::is_sorted(buffer.begin(), buffer.end()));
        });

        // 1b. Custom MergeSort on Reverse Sorted Data
        runner.run("MergeSort::reverse", label, n, 3, [&]() {
            std::vector<int> buffer = reverseData;
            mergeSort(buffer.data(), buffer.size(), lessCmp);
            assert(std::is_sorted(buffer.begin(), buffer.end()));
        });

        // 1c. Custom MergeSort on Duplicate-Heavy Data
        runner.run("MergeSort::duplicate_heavy", label, n, 3, [&]() {
            std::vector<int> buffer = duplicateData;
            mergeSort(buffer.data(), buffer.size(), lessCmp);
            assert(std::is_sorted(buffer.begin(), buffer.end()));
        });

        // 1d. Custom QuickSort on Random Data
        runner.run("QuickSort::random", label, n, 3, [&]() {
            std::vector<int> buffer = randomData;
            quickSort(buffer.data(), buffer.size(), lessCmp);
            assert(std::is_sorted(buffer.begin(), buffer.end()));
        });

        // 1e. Custom InsertionSort (Only up to N=10,000 due to O(N^2))
        if (n <= 10000)
        {
            runner.run("InsertionSort::random", label, n, 3, [&]() {
                std::vector<int> buffer = randomData;
                insertionSort(buffer.data(), buffer.size(), lessCmp);
                assert(std::is_sorted(buffer.begin(), buffer.end()));
            });

            // InsertionSort on Nearly-Sorted Data (O(N) best case)
            std::vector<int> nearlySorted = sortedData;
            if (n >= 10) std::swap(nearlySorted[2], nearlySorted[3]);
            runner.run("InsertionSort::nearly_sorted", label, n, 3, [&]() {
                std::vector<int> buffer = nearlySorted;
                insertionSort(buffer.data(), buffer.size(), lessCmp);
                assert(std::is_sorted(buffer.begin(), buffer.end()));
            });
        }

        // 1f. Full InternPath Domain Applicant Stable Ranking
        std::vector<ApplicantRankKey> applicants(n);
        for (std::size_t i = 0; i < n; i++)
        {
            applicants[i] = {
                static_cast<std::int32_t>(rng() % 100),   // fitNumerator (many ties)
                static_cast<std::uint64_t>(i + 1),       // submitSequence
                static_cast<std::uint32_t>(rng() % 1000) // id
            };
        }

        auto applicantCmp = [](const ApplicantRankKey& a, const ApplicantRankKey& b) {
            if (a.fitNumerator != b.fitNumerator) return a.fitNumerator > b.fitNumerator;
            if (a.submitSequence != b.submitSequence) return a.submitSequence < b.submitSequence;
            return a.id < b.id;
        };

        runner.run("MergeSort::applicant_ranking", label, n, 3, [&]() {
            std::vector<ApplicantRankKey> buffer = applicants;
            mergeSort(buffer.data(), buffer.size(), applicantCmp);
        });

        // 1g. Differential Reference: std::stable_sort (Oracle)
        runner.run("std::stable_sort::oracle", label, n, 3, [&]() {
            std::vector<int> buffer = randomData;
            std::stable_sort(buffer.begin(), buffer.end(), lessCmp);
            assert(std::is_sorted(buffer.begin(), buffer.end()));
        });
    }

    runner.saveCsv("bench_sort_results.csv");
    return 0;
}
