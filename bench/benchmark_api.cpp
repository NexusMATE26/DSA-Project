#include <atomic>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2def.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "bench/benchmark_common.h"

using namespace internpath::bench;

struct HttpResponse
{
    int statusCode = 0;
    std::string body;
    double latencyMs = 0.0;
    bool success = false;
};

HttpResponse sendHttpRequest(const std::string& host, int port, const std::string& path)
{
    HttpResponse resp;
    Timer timer;

#ifdef _WIN32
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        return resp;
    }
#else
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        return resp;
    }
#endif

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(static_cast<u_short>(port));
    inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr);

    if (connect(sock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0)
    {
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
        return resp;
    }

    std::string request = "GET " + path + " HTTP/1.1\r\n"
                          "Host: " + host + ":" + std::to_string(port) + "\r\n"
                          "Connection: close\r\n\r\n";

    send(sock, request.c_str(), static_cast<int>(request.length()), 0);

    char buffer[4096];
    std::string rawResponse;
    int bytesRead = 0;

    while ((bytesRead = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
        buffer[bytesRead] = '\0';
        rawResponse.append(buffer, bytesRead);
    }

#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif

    resp.latencyMs = timer.elapsedMilliseconds();

    if (!rawResponse.empty())
    {
        std::istringstream stream(rawResponse);
        std::string httpVersion;
        stream >> httpVersion >> resp.statusCode;
        resp.success = (resp.statusCode == 200);
    }

    return resp;
}

int main(int argc, char* argv[])
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    std::string host = "127.0.0.1";
    int port = 8080;
    if (argc > 1) port = std::stoi(argv[1]);

    std::cout << "\n========================================================================================\n";
    std::cout << " INTERNPATH HTTP API LOAD & CONCURRENCY BENCHMARK\n";
    std::cout << " Target: http://" << host << ":" << port << "\n";
    std::cout << "========================================================================================\n";

    // Pre-flight health check
    auto health = sendHttpRequest(host, port, "/api/health");
    if (!health.success)
    {
        std::cout << "Notice: Server is not currently reachable on http://" << host << ":" << port << "\n";
        std::cout << "Please ensure 'internpath_server.exe " << port << "' is running.\n";
#ifdef _WIN32
        WSACleanup();
#endif
        return 0;
    }
    std::cout << "✓ Server is active and reachable (Health ping: " << health.latencyMs << " ms).\n\n";

    std::vector<std::string> endpoints = {
        "/api/health",
        "/api/students",
        "/api/students/1/recommendations",
        "/api/students/1/plan/1"
    };

    const std::vector<int> concurrencyLevels = {1, 4, 8};
    const int requestsPerLevel = 200;

    for (const auto& endpoint : endpoints)
    {
        std::cout << "\nEndpoint: " << endpoint << "\n";
        std::cout << std::left
                  << std::setw(16) << "Concurrency"
                  << std::setw(12) << "Total Req"
                  << std::setw(12) << "Median (ms)"
                  << std::setw(12) << "p95 (ms)"
                  << std::setw(16) << "Req/sec"
                  << std::setw(12) << "Success Rate"
                  << "\n";
        std::cout << "----------------------------------------------------------------------------------------\n";

        for (int c : concurrencyLevels)
        {
            std::atomic<int> successCount{0};
            std::vector<double> latencies;
            std::mutex latMutex;

            Timer loadTimer;
            std::vector<std::thread> workers;

            int perThread = requestsPerLevel / c;
            for (int t = 0; t < c; t++)
            {
                workers.emplace_back([&, perThread]() {
                    std::vector<double> localLatencies;
                    for (int i = 0; i < perThread; i++)
                    {
                        auto resp = sendHttpRequest(host, port, endpoint);
                        if (resp.success)
                        {
                            successCount++;
                            localLatencies.push_back(resp.latencyMs);
                        }
                    }
                    std::lock_guard<std::mutex> lock(latMutex);
                    latencies.insert(latencies.end(), localLatencies.begin(), localLatencies.end());
                });
            }

            for (auto& w : workers) w.join();
            double totalTimeSec = loadTimer.elapsedSeconds();

            if (!latencies.empty())
            {
                std::sort(latencies.begin(), latencies.end());
                double median = latencies[latencies.size() / 2];
                double p95 = latencies[static_cast<std::size_t>(latencies.size() * 0.95)];
                double rps = static_cast<double>(successCount.load()) / totalTimeSec;
                double rate = (static_cast<double>(successCount.load()) / static_cast<double>(requestsPerLevel)) * 100.0;

                std::cout << std::left
                          << std::setw(16) << (std::to_string(c) + " threads")
                          << std::setw(12) << requestsPerLevel
                          << std::setw(12) << std::fixed << std::setprecision(2) << median
                          << std::setw(12) << std::fixed << std::setprecision(2) << p95
                          << std::setw(16) << std::fixed << std::setprecision(1) << rps
                          << std::setw(12) << (std::to_string(static_cast<int>(rate)) + "%")
                          << "\n";
            }
        }
    }

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
