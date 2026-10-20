#include <cassert>
#include <iostream>
#include <vector>

#include "bench/benchmark_common.h"
#include "internpath/ds/CircularQueue.h"
#include "internpath/ds/DoublyLinkedList.h"
#include "internpath/ds/DynamicArray.h"
#include "internpath/ds/SinglyLinkedList.h"
#include "internpath/ds/Stack.h"

using namespace internpath;
using namespace internpath::bench;

int main()
{
    if (!checkSystemSafety())
    {
        return 1;
    }

    BenchmarkRunner runner("Core Data Structures (Containers)");
    runner.printHeader();

    const std::vector<std::size_t> sizes = {1000, 10000, 100000, 1000000};

    // -------------------------------------------------------------
    // 1. DynamicArray Benchmarks
    // -------------------------------------------------------------
    for (std::size_t n : sizes)
    {
        std::string label = "N=" + std::to_string(n);

        // 1a. DynamicArray pushBack (Geometric Doubling)
        runner.run("DynamicArray::pushBack", label, n, 5, [&]() {
            DynamicArray<int> arr;
            for (std::size_t i = 0; i < n; i++)
            {
                arr.pushBack(static_cast<int>(i));
            }
            assert(arr.size() == n);
        });

        // 1b. DynamicArray reserve + pushBack (Pre-allocated)
        runner.run("DynamicArray::reserve_push", label, n, 5, [&]() {
            DynamicArray<int> arr;
            arr.reserve(n);
            for (std::size_t i = 0; i < n; i++)
            {
                arr.pushBack(static_cast<int>(i));
            }
            assert(arr.size() == n);
        });

        // 1c. Indexed Random Read Access
        DynamicArray<int> readArr;
        readArr.reserve(n);
        for (std::size_t i = 0; i < n; i++) readArr.pushBack(static_cast<int>(i * 3));

        runner.run("DynamicArray::index_read", label, n, 5, [&]() {
            volatile long long sum = 0;
            for (std::size_t i = 0; i < n; i++)
            {
                sum += readArr[i];
            }
        });
    }

    // -------------------------------------------------------------
    // 2. SinglyLinkedList Benchmarks
    // -------------------------------------------------------------
    for (std::size_t n : {1000, 10000, 100000})
    {
        std::string label = "N=" + std::to_string(n);

        // 2a. SinglyLinkedList Tail Append O(1)
        runner.run("SinglyLinkedList::pushBack", label, n, 5, [&]() {
            SinglyLinkedList<int> list;
            for (std::size_t i = 0; i < n; i++)
            {
                list.pushBack(static_cast<int>(i));
            }
            assert(list.size() == n);
        });

        // 2b. SinglyLinkedList removeIf Predicate
        runner.run("SinglyLinkedList::removeIf", label, 1, 5, [&]() {
            SinglyLinkedList<int> list;
            for (std::size_t i = 0; i < n; i++) list.pushBack(static_cast<int>(i));
            int target = static_cast<int>(n / 2);
            bool removed = list.removeIf([target](int val) { return val == target; });
            assert(removed && list.size() == n - 1);
        });
    }

    // -------------------------------------------------------------
    // 3. DoublyLinkedList Benchmarks (Handles vs Scans)
    // -------------------------------------------------------------
    for (std::size_t n : {1000, 10000, 100000})
    {
        std::string label = "N=" + std::to_string(n);

        // 3a. DoublyLinkedList PushBack & Handle Retention
        std::vector<DoublyLinkedList<int>::Handle> handles;
        handles.reserve(n);

        DoublyLinkedList<int> dll;
        for (std::size_t i = 0; i < n; i++)
        {
            handles.push_back(dll.pushBack(static_cast<int>(i)));
        }

        // 3b. O(1) Middle Node Unlink by Handle
        runner.run("DoublyLinkedList::erase_handle", label, 1000, 5, [&]() {
            // Unlink 1,000 arbitrary nodes by direct handle
            DoublyLinkedList<int> localDll;
            std::vector<DoublyLinkedList<int>::Handle> localHandles;
            localHandles.reserve(n);
            for (std::size_t i = 0; i < n; i++)
            {
                localHandles.push_back(localDll.pushBack(static_cast<int>(i)));
            }

            for (std::size_t k = 0; k < 1000 && k < n; k++)
            {
                localDll.erase(localHandles[k * (n / 1000)]);
            }
            assert(localDll.size() == n - (n >= 1000 ? 1000 : n));
        });

        // 3c. PopFront FIFO extraction
        runner.run("DoublyLinkedList::popFront", label, n, 5, [&]() {
            DoublyLinkedList<int> localDll;
            for (std::size_t i = 0; i < n; i++) localDll.pushBack(static_cast<int>(i));
            int val = 0;
            while (localDll.popFront(val)) {}
            assert(localDll.empty());
        });
    }

    // -------------------------------------------------------------
    // 4. Stack Benchmarks (Profile-Edit Undo LIFO)
    // -------------------------------------------------------------
    for (std::size_t n : sizes)
    {
        std::string label = "N=" + std::to_string(n);

        runner.run("Stack::push_pop_cycle", label, n, 5, [&]() {
            Stack<int> st;
            for (std::size_t i = 0; i < n; i++)
            {
                st.push(static_cast<int>(i));
            }
            int val = 0;
            for (std::size_t i = 0; i < n; i++)
            {
                bool popped = st.pop(val);
                assert(popped && val == static_cast<int>(n - 1 - i));
            }
            assert(st.empty());
        });
    }

    // -------------------------------------------------------------
    // 5. CircularQueue Benchmarks (FIFO Ring Buffer)
    // -------------------------------------------------------------
    for (std::size_t n : sizes)
    {
        std::string label = "N=" + std::to_string(n);

        // 5a. Enqueue and Dequeue batch
        runner.run("CircularQueue::enqueue_dequeue", label, n, 5, [&]() {
            CircularQueue<int> q;
            for (std::size_t i = 0; i < n; i++)
            {
                q.enqueue(static_cast<int>(i));
            }
            int val = 0;
            for (std::size_t i = 0; i < n; i++)
            {
                bool deq = q.dequeue(val);
                assert(deq && val == static_cast<int>(i));
            }
            assert(q.empty());
        });

        // 5b. Continuous Wrap-Around Ring Stress
        runner.run("CircularQueue::ring_wrap_around", label, n, 5, [&]() {
            CircularQueue<int> q;
            // Pre-fill 100 elements
            for (int i = 0; i < 100; i++) q.enqueue(i);
            int outVal = 0;
            // Continuously push and pop to force head/tail to wrap around capacity millions of times
            for (std::size_t i = 0; i < n; i++)
            {
                q.enqueue(static_cast<int>(i));
                q.dequeue(outVal);
            }
            assert(q.size() == 100);
        });
    }

    runner.saveCsv("bench_containers_results.csv");
    return 0;
}
