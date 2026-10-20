#pragma once

#include <cstddef>
#include <stdexcept>

namespace internpath
{
    // MEMBER A | TODO A16-A18 | FIFO for graph BFS and Kahn's algorithm.
    template <typename T>
    class CircularQueue
    {
    private:
        T* data_ = nullptr;
        std::size_t capacity_ = 0;
        std::size_t count_ = 0;
        std::size_t front_ = 0;

        // TODO A18: Implement private grow() that preserves FIFO order
        void grow()
        {
            std::size_t newCapacity = (capacity_ == 0) ? 4 : capacity_ * 2;
            T* newData = new T[newCapacity];

            for (std::size_t i = 0; i < count_; i++)
            {
                newData[i] = std::move(data_[(front_ + i) % capacity_]);
            }

            delete[] data_;
            data_ = newData;
            front_ = 0;
            capacity_ = newCapacity;
        }

    public:
        CircularQueue() = default;
        CircularQueue(const CircularQueue&) = delete;
        CircularQueue& operator=(const CircularQueue&) = delete;

        CircularQueue(CircularQueue&& other) noexcept
            : data_(other.data_), capacity_(other.capacity_), count_(other.count_), front_(other.front_)
        {
            other.data_ = nullptr;
            other.capacity_ = 0;
            other.count_ = 0;
            other.front_ = 0;
        }

        CircularQueue& operator=(CircularQueue&& other) noexcept
        {
            if (this != &other)
            {
                delete[] data_;
                data_ = other.data_;
                capacity_ = other.capacity_;
                count_ = other.count_;
                front_ = other.front_;

                other.data_ = nullptr;
                other.capacity_ = 0;
                other.count_ = 0;
                other.front_ = 0;
            }

            return *this;
        }

        ~CircularQueue()
        {
            delete[] data_;
        }

        bool empty() const
        {
            return count_ == 0;
        }

        std::size_t size() const
        {
            return count_;
        }

        std::size_t capacity() const
        {
            return capacity_;
        }

        // TODO A16: enqueue - grow if full, insert at (front_ + count_) % capacity_
        void enqueue(const T& value)
        {
            if (count_ >= capacity_)
            {
                grow();
            }

            std::size_t rear = (front_ + count_) % capacity_;
            data_[rear] = value;
            count_++;
        }

        void enqueue(T&& value)
        {
            if (count_ >= capacity_)
            {
                grow();
            }

            std::size_t rear = (front_ + count_) % capacity_;
            data_[rear] = std::move(value);
            count_++;
        }

        // TODO A17: dequeue - extract front item and advance front_ with wrap-around
        bool dequeue(T& result)
        {
            if (count_ == 0)
            {
                return false;
            }

            result = std::move(data_[front_]);
            front_ = (front_ + 1) % capacity_;
            count_--;
            return true;
        }

        void clear()
        {
            count_ = 0;
            front_ = 0;
        }
    };
}
