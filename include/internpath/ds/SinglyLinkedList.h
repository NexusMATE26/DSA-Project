#pragma once

#include <cstddef>
#include <stdexcept>

namespace internpath
{
    // MEMBER A | TODO A07-A09 | Used for hash collision chains.
    template <typename T>
    class SinglyLinkedList
    {
    public:
        struct Node
        {
            T value;
            Node* next = nullptr;
        };

    private:
        Node* head_ = nullptr;
        Node* tail_ = nullptr;
        std::size_t size_ = 0;

    public:
        SinglyLinkedList() = default;
        SinglyLinkedList(const SinglyLinkedList&) = delete;
        SinglyLinkedList& operator=(const SinglyLinkedList&) = delete;

        SinglyLinkedList(SinglyLinkedList&& other) noexcept
            : head_(other.head_), tail_(other.tail_), size_(other.size_)
        {
            other.head_ = nullptr;
            other.tail_ = nullptr;
            other.size_ = 0;
        }

        SinglyLinkedList& operator=(SinglyLinkedList&& other) noexcept
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

        ~SinglyLinkedList()
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

        Node* firstNode() const
        {
            return head_;
        }

        std::size_t size() const
        {
            return size_;
        }

        // TODO A07: pushBack - link at tail in O(1)
        void pushBack(const T& value)
        {
            Node* newNode = new Node{value, nullptr};

            if (head_ == nullptr)
            {
                head_ = newNode;
                tail_ = newNode;
            }
            else
            {
                tail_->next = newNode;
                tail_ = newNode;
            }

            size_++;
        }

        void pushBack(T&& value)
        {
            Node* newNode = new Node{std::move(value), nullptr};

            if (head_ == nullptr)
            {
                head_ = newNode;
                tail_ = newNode;
            }
            else
            {
                tail_->next = newNode;
                tail_ = newNode;
            }

            size_++;
        }

        // TODO A08: removeFirst - unlink one matching value in O(n)
        bool removeFirst(const T& value)
        {
            Node* prev = nullptr;
            Node* curr = head_;

            while (curr != nullptr)
            {
                if (curr->value == value)
                {
                    if (prev == nullptr)
                    {
                        head_ = curr->next;
                    }
                    else
                    {
                        prev->next = curr->next;
                    }

                    if (curr == tail_)
                    {
                        tail_ = prev;
                    }

                    delete curr;
                    size_--;
                    return true;
                }

                prev = curr;
                curr = curr->next;
            }

            return false;
        }

        // TODO A09: removeIf - unlink first node matching predicate
        template <typename Predicate>
        bool removeIf(Predicate predicate)
        {
            Node* prev = nullptr;
            Node* curr = head_;

            while (curr != nullptr)
            {
                if (predicate(curr->value))
                {
                    if (prev == nullptr)
                    {
                        head_ = curr->next;
                    }
                    else
                    {
                        prev->next = curr->next;
                    }

                    if (curr == tail_)
                    {
                        tail_ = prev;
                    }

                    delete curr;
                    size_--;
                    return true;
                }

                prev = curr;
                curr = curr->next;
            }

            return false;
        }
    };
}
