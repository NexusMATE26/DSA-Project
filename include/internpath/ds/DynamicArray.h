#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>

namespace internpath
{
    // MEMBER A | TODO A01-A06 | Dynamic array (no std::vector)
    template <typename T>
    class DynamicArray
    {
    private:
        T* data_ = nullptr;
        std::size_t size_ = 0;
        std::size_t capacity_ = 0;

    public:
        DynamicArray() = default;

        ~DynamicArray()
        {
            delete[] data_;
        }

        // Rule of Five (TODO A06)
        DynamicArray(const DynamicArray& other)
            : size_(other.size_), capacity_(other.capacity_)
        {
            if (capacity_ > 0)
            {
                data_ = new T[capacity_];
                for (std::size_t i = 0; i < size_; i++)
                {
                    data_[i] = other.data_[i];
                }
            }
        }

        DynamicArray& operator=(const DynamicArray& other)
        {
            if (this != &other)
            {
                T* newData = nullptr;
                if (other.capacity_ > 0)
                {
                    newData = new T[other.capacity_];
                    for (std::size_t i = 0; i < other.size_; i++)
                    {
                        newData[i] = other.data_[i];
                    }
                }

                delete[] data_;
                data_ = newData;
                size_ = other.size_;
                capacity_ = other.capacity_;
            }

            return *this;
        }

        DynamicArray(DynamicArray&& other) noexcept
            : data_(other.data_), size_(other.size_), capacity_(other.capacity_)
        {
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }

        DynamicArray& operator=(DynamicArray&& other) noexcept
        {
            if (this != &other)
            {
                delete[] data_;
                data_ = other.data_;
                size_ = other.size_;
                capacity_ = other.capacity_;

                other.data_ = nullptr;
                other.size_ = 0;
                other.capacity_ = 0;
            }

            return *this;
        }

        std::size_t size() const
        {
            return size_;
        }

        std::size_t capacity() const
        {
            return capacity_;
        }

        bool empty() const
        {
            return size_ == 0;
        }

        T* data()
        {
            return data_;
        }

        const T* data() const
        {
            return data_;
        }

        // TODO A01: reserve capacity without changing size
        void reserve(std::size_t newCapacity)
        {
            if (newCapacity <= capacity_)
            {
                return;
            }

            T* newData = new T[newCapacity];
            for (std::size_t i = 0; i < size_; i++)
            {
                newData[i] = std::move(data_[i]);
            }

            delete[] data_;
            data_ = newData;
            capacity_ = newCapacity;
        }

        // TODO A02: pushBack (amortized O(1), doubling capacity starting at 2)
        void pushBack(const T& value)
        {
            if (size_ >= capacity_)
            {
                std::size_t newCapacity = (capacity_ == 0) ? 2 : capacity_ * 2;
                reserve(newCapacity);
            }

            data_[size_] = value;
            size_++;
        }

        void pushBack(T&& value)
        {
            if (size_ >= capacity_)
            {
                std::size_t newCapacity = (capacity_ == 0) ? 2 : capacity_ * 2;
                reserve(newCapacity);
            }

            data_[size_] = std::move(value);
            size_++;
        }

        // TODO A03: indexed access with bounds check
        T& at(std::size_t index)
        {
            if (index >= size_)
            {
                throw std::out_of_range("DynamicArray index out of range");
            }

            return data_[index];
        }

        const T& at(std::size_t index) const
        {
            if (index >= size_)
            {
                throw std::out_of_range("DynamicArray index out of range");
            }

            return data_[index];
        }

        T& operator[](std::size_t index)
        {
            return data_[index];
        }

        const T& operator[](std::size_t index) const
        {
            return data_[index];
        }

        // TODO A04: eraseAt shifting elements left
        void eraseAt(std::size_t index)
        {
            if (index >= size_)
            {
                throw std::out_of_range("DynamicArray erase index out of range");
            }

            for (std::size_t i = index; i + 1 < size_; i++)
            {
                data_[i] = std::move(data_[i + 1]);
            }

            size_--;
        }

        // TODO A05: clear logical size while retaining allocated capacity
        void clear()
        {
            size_ = 0;
        }
    };
}
