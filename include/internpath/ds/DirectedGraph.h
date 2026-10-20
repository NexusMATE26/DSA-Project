#pragma once

#include <cstdint>
#include <stdexcept>

#include "internpath/ds/DynamicArray.h"
#include "internpath/ds/SinglyLinkedList.h"

namespace internpath
{
    class DirectedGraph;
    bool isReachable(const DirectedGraph& graph, std::uint32_t from, std::uint32_t target);

    struct PrerequisiteEdge
    {
        std::uint32_t toIndex = 0;
        std::uint8_t minimumLevel = 1;
    };

    // MEMBER C | TODO C01-C03 | Skill A -> B means A is prerequisite of B.
    class DirectedGraph
    {
    private:
        // One custom linked adjacency list per skill index.
        DynamicArray<SinglyLinkedList<PrerequisiteEdge>> outgoing_;
        DynamicArray<SinglyLinkedList<PrerequisiteEdge>> incoming_;
        std::uint32_t count_ = 0;

    public:
        DirectedGraph() = default;

        std::uint32_t vertexCount() const
        {
            return count_;
        }

        // TODO C01: Add one empty outgoing and incoming adjacency list.
        std::uint32_t addVertex()
        {
            outgoing_.pushBack(SinglyLinkedList<PrerequisiteEdge>{});
            incoming_.pushBack(SinglyLinkedList<PrerequisiteEdge>{});
            std::uint32_t newIndex = count_;
            count_++;
            return newIndex;
        }

        // TODO C02: Validate, reject duplicate/self-loop/cycle, add to both lists.
        bool addEdge(std::uint32_t from, std::uint32_t to, std::uint8_t minLevel)
        {
            if (from >= count_ || to >= count_ || from == to || minLevel < 1 || minLevel > 3)
            {
                return false;
            }

            // Check for duplicate edge
            auto* node = outgoing_.at(from).firstNode();
            while (node != nullptr)
            {
                if (node->value.toIndex == to)
                {
                    return false;
                }

                node = node->next;
            }

            // Check cycle: if 'to' can already reach 'from', adding from -> to causes a cycle
            if (isReachable(*this, to, from))
            {
                return false;
            }

            outgoing_.at(from).pushBack(PrerequisiteEdge{to, minLevel});
            incoming_.at(to).pushBack(PrerequisiteEdge{from, minLevel});
            return true;
        }

        // TODO C03: Safe read-only adjacency traversal API
        const SinglyLinkedList<PrerequisiteEdge>& outgoing(std::uint32_t vertex) const
        {
            return outgoing_.at(vertex);
        }

        const SinglyLinkedList<PrerequisiteEdge>& incoming(std::uint32_t vertex) const
        {
            return incoming_.at(vertex);
        }

        std::uint32_t inDegree(std::uint32_t vertex) const
        {
            return static_cast<std::uint32_t>(incoming_.at(vertex).size());
        }

        std::uint32_t outDegree(std::uint32_t vertex) const
        {
            return static_cast<std::uint32_t>(outgoing_.at(vertex).size());
        }
    };
}
