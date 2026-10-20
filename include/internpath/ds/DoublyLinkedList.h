#pragma once

#include <cstddef>
#include <stdexcept>

namespace internpath
{
    // MEMBER A | TODO A10-A12 | Ranked waiting list, frozen at round close.
    template <typename T>
    class DoublyLinkedList
    {
    public:
        struct Node
        {
            T value;
            Node* previous = nullptr;
            Node* next = nullptr;
        };

        using Handle = Node*;

    private:
        Node* head_ = nullptr;
        Node* tail_ = nullptr;
        std::size_t size_ = 0;

    public:
        DoublyLinkedList() = default;
        DoublyLinkedList(const DoublyLinkedList&) = delete;
        DoublyLinkedList& operator=(const DoublyLinkedList&) = delete;

        DoublyLinkedList(DoublyLinkedList&& other) noexcept
            : head_(other.head_), tail_(other.tail_), size_(other.size_)
        {
            other.head_ = nullptr;
            other.tail_ = nullptr;
            other.size_ = 0;
        }

        DoublyLinkedList& operator=(DoublyLinkedList&& other) noexcept
        {
            if (this != &other)
            {
                clear();
                head_ = other.head_;
                tail_ = other.tail_;
                size_ = other.size_;

                other.head_ = nullptr;
                other.tail_ = nullptr;
                other.size_ = 0;
            }

            return *this;
        }

        ~DoublyLinkedList()
        {
            clear();
        }

        void clear()
        {
            Node* current = head_;

            while (current != nullptr)
            {
                Node* next = current->next;
                delete current;
                current = next;
            }

            head_ = nullptr;
            tail_ = nullptr;
            size_ = 0;
        }

        bool empty() const
        {
            return size_ == 0;
        }

        std::size_t size() const
        {
            return size_;
        }

        Node* firstNode() const
        {
            return head_;
        }

        // TODO A10: Append new node and return its stable handle
        Handle pushBack(const T& value)
        {
            Node* newNode = new Node{value, tail_, nullptr};

            if (tail_ != nullptr)
            {
                tail_->next = newNode;
            }
            else
            {
                head_ = newNode;
            }

            tail_ = newNode;
            size_++;
            return newNode;
        }

        Handle pushBack(T&& value)
        {
            Node* newNode = new Node{std::move(value), tail_, nullptr};

            if (tail_ != nullptr)
            {
                tail_->next = newNode;
            }
            else
            {
                head_ = newNode;
            }

            tail_ = newNode;
            size_++;
            return newNode;
        }

        // TODO A11: Unlink a VALID owned node via previous and next in O(1)
        void erase(Handle handle)
        {
            if (handle == nullptr)
            {
                return;
            }

            if (handle->previous != nullptr)
            {
                handle->previous->next = handle->next;
            }
            else
            {
                head_ = handle->next;
            }

            if (handle->next != nullptr)
            {
                handle->next->previous = handle->previous;
            }
            else
            {
                tail_ = handle->previous;
            }

            delete handle;
            size_--;
        }

        // TODO A12: Return false if empty; otherwise copy head value and erase in O(1)
        bool popFront(T& removedValue)
        {
            if (head_ == nullptr)
            {
                return false;
            }

            removedValue = head_->value;
            erase(head_);
            return true;
        }
    };
}
