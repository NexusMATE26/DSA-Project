#pragma once

#include "internpath/domain/Types.h"
#include "internpath/ds/DoublyLinkedList.h"
#include "internpath/ds/HashTable.h"

namespace internpath
{
    // Member A: ranked list is frozen at close, then processed head-first.
    // Hash index permits expected O(1) lookup of a node by application ID.
    class WaitingList
    {
    private:
        DoublyLinkedList<ApplicationId> order_;
        HashTable<ApplicationId, DoublyLinkedList<ApplicationId>::Handle> handles_;

    public:
        // TODO A22-A24: append rank, withdraw by handle, promote head.
        WaitingList() = default;
        WaitingList(const WaitingList&) = delete;
        WaitingList& operator=(const WaitingList&) = delete;
        WaitingList(WaitingList&&) noexcept = default;
        WaitingList& operator=(WaitingList&&) noexcept = default;

        std::size_t size() const
        {
            return order_.size();
        }

        bool empty() const
        {
            return order_.empty();
        }

        bool contains(ApplicationId id) const
        {
            DoublyLinkedList<ApplicationId>::Handle dummy;
            return handles_.tryGet(id, dummy);
        }

        // TODO A22: append rank - add application ID to waiting list and store handle
        bool append(ApplicationId id)
        {
            DoublyLinkedList<ApplicationId>::Handle existing;
            if (handles_.tryGet(id, existing))
            {
                return false;
            }

            auto handle = order_.pushBack(id);
            handles_.insertOrAssign(id, handle);
            return true;
        }

        // TODO A23: withdraw by handle in expected O(1)
        bool withdraw(ApplicationId id)
        {
            DoublyLinkedList<ApplicationId>::Handle handle = nullptr;
            if (!handles_.tryGet(id, handle))
            {
                return false;
            }

            handles_.erase(id);
            order_.erase(handle);
            return true;
        }

        // TODO A24: promote head of waiting list
        bool promoteHead(ApplicationId& outId)
        {
            if (order_.empty())
            {
                return false;
            }

            if (order_.popFront(outId))
            {
                handles_.erase(outId);
                return true;
            }

            return false;
        }

        void clear()
        {
            order_.clear();
            handles_.clear();
        }

        DynamicArray<ApplicationId> snapshot() const
        {
            DynamicArray<ApplicationId> result;
            auto* node = order_.firstNode();
            while (node != nullptr)
            {
                result.pushBack(node->value);
                node = node->next;
            }
            return result;
        }
    };
}
