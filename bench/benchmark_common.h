#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <psapi.h>
#else
#include <sys/resource.h>
#endif

namespace internpath::bench
{
    // ==========================================
    // System Memory & Resource Tracker
    // ==========================================
    struct ResourceSnapshot
    {
        std::size_t currentRssBytes = 0;
        std::size_t peakRssBytes = 0;
        std::size_t availableRamBytes = 0;
    };

    inline ResourceSnapshot captureResources()
    {
        ResourceSnapshot snap;
#ifdef _WIN32
        PROCESS_MEMORY_COUNTERS_EX pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc)))
        {
            snap.currentRssBytes = pmc.WorkingSetSize;
            snap.peakRssBytes = pmc.PeakWorkingSetSize;
        }

        MEMORYSTATUSEX memStatus;
        memStatus.dwLength = sizeof(memStatus);
        if (GlobalMemoryStatusEx(&memStatus))
        {
            snap.availableRamBytes = memStatus.ullAvailPhys;
        }
#else
        struct rusage usage;
        if (getrusage(RUSAGE_SELF, &usage) == 0)
        {
            snap.currentRssBytes = static_cast<std::size_t>(usage.ru_maxrss * 1024);
            snap.peakRssBytes = snap.currentRssBytes;
        }
#endif
        return snap;
    }

    inline bool checkSystemSafety(std::size_t minAvailableRamMb = 3000)
    {
        auto snap = captureResources();
        if (snap.availableRamBytes > 0 && snap.availableRamBytes < minAvailableRamMb * 1024ULL * 1024ULL)
        {
            std::cerr << "SAFETY ALERT: Available system RAM (" 
                      << (snap.availableRamBytes / (1024 * 1024)) << " MB) is below safe threshold (" 
                      << minAvailableRamMb << " MB). Aborting scaling to preserve OS stability.\n";
            return false;
        }
        return true;
    }

    // ==========================================
    // Monotonic High-Resolution Stopwatch
    // ==========================================
    class Timer
    {
    public:
        Timer() { reset(); }

        void reset()
        {
            start_ = std::chrono::steady_clock::now();
        }

        double elapsedSeconds() const
        {
            auto now = std::chrono::steady_clock::now();
            return std::chrono::duration<double>(now - start_).count();
        }

        double elapsedMilliseconds() const
        {
            return elapsedSeconds() * 1000.0;
        }

        double elapsedMicroseconds() const
        {
            return elapsedSeconds() * 1000000.0;
        }

        std::uint64_t elapsedNanoseconds() const
        {
            auto now = std::chrono::steady_clock::now();
            return std::chrono::duration_cast<std::chrono::nanoseconds>(now - start_).count();
        }

    private:
        std::chrono::steady_clock::time_point start_;
    };

    // ==========================================
    // Benchmark Result Statistics
    // ==========================================
    struct BenchmarkResult
    {
        std::string benchmarkName;
        std::string workload;
        std::size_t numOperations = 0;
        std::size_t iterations = 0;
        double totalTimeMs = 0.0;
        double minUs = 0.0;
        double maxUs = 0.0;
        double meanUs = 0.0;
        double medianUs = 0.0;
        double p95Us = 0.0;
        double p99Us = 0.0;
        double throughputOpsPerSec = 0.0;
        double memoryDeltaMb = 0.0;
        double peakRssMb = 0.0;
        bool correctnessVerified = false;
    };

    class BenchmarkRunner
    {
    public:
        explicit BenchmarkRunner(const std::string& suiteName)
            : suiteName_(suiteName)
        {
        }

        template <typename Func>
        BenchmarkResult run(
            const std::string& name,
            const std::string& workload,
            std::size_t numOperations,
            std::size_t iterations,
            Func&& func,
            bool verifyCorrectness = true,
            bool warmup = true)
        {
            BenchmarkResult res;
            res.benchmarkName = name;
            res.workload = workload;
            res.numOperations = numOperations;
            res.iterations = iterations;
            res.correctnessVerified = verifyCorrectness;

            // Optional warmup iteration
            if (warmup)
            {
                func();
            }

            auto initialSnap = captureResources();
            std::vector<double> timingsUs;
            timingsUs.reserve(iterations);

            Timer totalTimer;

            for (std::size_t i = 0; i < iterations; i++)
            {
                Timer itTimer;
                func();
                timingsUs.push_back(itTimer.elapsedMicroseconds());
            }

            res.totalTimeMs = totalTimer.elapsedMilliseconds();
            auto finalSnap = captureResources();

            std::sort(timingsUs.begin(), timingsUs.end());

            res.minUs = timingsUs.front();
            res.maxUs = timingsUs.back();
            double sum = std::accumulate(timingsUs.begin(), timingsUs.end(), 0.0);
            res.meanUs = sum / static_cast<double>(iterations);
            res.medianUs = timingsUs[iterations / 2];
            res.p95Us = timingsUs[static_cast<std::size_t>(iterations * 0.95)];
            res.p99Us = timingsUs[static_cast<std::size_t>(iterations * 0.99)];

            if (res.totalTimeMs > 0.0)
            {
                double totalSeconds = res.totalTimeMs / 1000.0;
                res.throughputOpsPerSec = static_cast<double>(numOperations * iterations) / totalSeconds;
            }

            long long memDiff = static_cast<long long>(finalSnap.peakRssBytes) - static_cast<long long>(initialSnap.peakRssBytes);
            res.memoryDeltaMb = (memDiff > 0) ? (static_cast<double>(memDiff) / (1024.0 * 1024.0)) : 0.0;
            res.peakRssMb = static_cast<double>(finalSnap.peakRssBytes) / (1024.0 * 1024.0);

            results_.push_back(res);
            printResult(res);
            return res;
        }

        void printHeader() const
        {
            std::cout << "\n========================================================================================================================\n";
            std::cout << " BENCHMARK SUITE: " << suiteName_ << "\n";
            std::cout << "========================================================================================================================\n";
            std::cout << std::left
                      << std::setw(28) << "Benchmark"
                      << std::setw(14) << "Workload"
                      << std::setw(12) << "N (Ops)"
                      << std::setw(12) << "Median (us)"
                      << std::setw(12) << "p95 (us)"
                      << std::setw(16) << "Throughput (ops/s)"
                      << std::setw(12) << "Peak RAM"
                      << std::setw(10) << "Status"
                      << "\n";
            std::cout << "------------------------------------------------------------------------------------------------------------------------\n";
        }

        void printResult(const BenchmarkResult& r) const
        {
            std::cout << std::left
                      << std::setw(28) << r.benchmarkName
                      << std::setw(14) << r.workload
                      << std::setw(12) << r.numOperations
                      << std::setw(12) << std::fixed << std::setprecision(2) << r.medianUs
                      << std::setw(12) << std::fixed << std::setprecision(2) << r.p95Us
                      << std::setw(16) << std::fixed << std::setprecision(0) << r.throughputOpsPerSec
                      << std::setw(12) << (std::to_string(static_cast<int>(r.peakRssMb)) + " MB")
                      << std::setw(10) << (r.correctnessVerified ? "VERIFIED" : "UNCHECKED")
                      << "\n";
        }

        void saveCsv(const std::string& filepath) const
        {
            std::ofstream out(filepath);
            if (!out.is_open())
            {
                std::cerr << "Warning: Could not save benchmark CSV to " << filepath << "\n";
                return;
            }

            out << "suite,benchmark,workload,num_operations,iterations,total_time_ms,min_us,max_us,mean_us,median_us,p95_us,p99_us,throughput_ops_per_sec,memory_delta_mb,peak_rss_mb,verified\n";
            for (const auto& r : results_)
            {
                out << suiteName_ << ","
                    << r.benchmarkName << ","
                    << r.workload << ","
                    << r.numOperations << ","
                    << r.iterations << ","
                    << r.totalTimeMs << ","
                    << r.minUs << ","
                    << r.maxUs << ","
                    << r.meanUs << ","
                    << r.medianUs << ","
                    << r.p95Us << ","
                    << r.p99Us << ","
                    << r.throughputOpsPerSec << ","
                    << r.memoryDeltaMb << ","
                    << r.peakRssMb << ","
                    << (r.correctnessVerified ? "true" : "false") << "\n";
            }
            std::cout << "\n✓ Benchmark results successfully exported to: " << filepath << "\n";
        }

        const std::vector<BenchmarkResult>& results() const { return results_; }

    private:
        std::string suiteName_;
        std::vector<BenchmarkResult> results_;
    };
}
