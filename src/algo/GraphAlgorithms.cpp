#include "internpath/algo/GraphAlgorithms.h"
#include "internpath/ds/CircularQueue.h"
#include "internpath/ds/Stack.h"

namespace internpath
{
    // TODO C04: DFS with our custom Stack and visited DynamicArray in O(V + E)
    bool isReachable(const DirectedGraph& graph,
                     std::uint32_t from, std::uint32_t target)
    {
        std::uint32_t n = graph.vertexCount();

        if (from >= n || target >= n)
        {
            return false;
        }

        if (from == target)
        {
            return true;
        }

        DynamicArray<bool> visited;
        for (std::uint32_t i = 0; i < n; i++)
        {
            visited.pushBack(false);
        }

        Stack<std::uint32_t> stack;
        stack.push(from);
        visited.at(from) = true;

        while (!stack.empty())
        {
            std::uint32_t current = 0;
            stack.pop(current);

            auto* edgeNode = graph.outgoing(current).firstNode();

            while (edgeNode != nullptr)
            {
                std::uint32_t neighbor = edgeNode->value.toIndex;

                if (neighbor == target)
                {
                    return true;
                }

                if (!visited.at(neighbor))
                {
                    visited.at(neighbor) = true;
                    stack.push(neighbor);
                }

                edgeNode = edgeNode->next;
            }
        }

        return false;
    }

    // TODO C05: Kahn's algorithm using our CircularQueue in O(V + E)
    bool topologicalOrder(const DirectedGraph& graph,
                          DynamicArray<std::uint32_t>& order)
    {
        order.clear();
        std::uint32_t n = graph.vertexCount();

        if (n == 0)
        {
            return true;
        }

        DynamicArray<std::uint32_t> inDegrees;
        for (std::uint32_t i = 0; i < n; i++)
        {
            inDegrees.pushBack(graph.inDegree(i));
        }

        CircularQueue<std::uint32_t> queue;
        for (std::uint32_t i = 0; i < n; i++)
        {
            if (inDegrees.at(i) == 0)
            {
                queue.enqueue(i);
            }
        }

        std::uint32_t processedCount = 0;

        while (!queue.empty())
        {
            std::uint32_t u = 0;
            queue.dequeue(u);
            order.pushBack(u);
            processedCount++;

            auto* edgeNode = graph.outgoing(u).firstNode();

            while (edgeNode != nullptr)
            {
                std::uint32_t v = edgeNode->value.toIndex;
                inDegrees.at(v)--;

                if (inDegrees.at(v) == 0)
                {
                    queue.enqueue(v);
                }

                edgeNode = edgeNode->next;
            }
        }

        // Return false if cycle exists (processedCount != n)
        return processedCount == n;
    }
}
