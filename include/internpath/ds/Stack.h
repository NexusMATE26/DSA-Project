#pragma once

#include <stdexcept>
#include "internpath/ds/DynamicArray.h"

namespace internpath
{
    // MEMBER A | TODO A13-A15 | Profile-edit undo (LIFO).
    template <typename T>
    class Stack
    {
    private:
        DynamicArray<T> data_;

    public:
        bool empty() const
        {
            return data_.empty();
        }

        std::size_t size() const
        {
            return data_.size();
        }

        // TODO A13: push - append to underlying DynamicArray
        void push(const T& value)
        {
            data_.pushBack(value);
        }

        void push(T&& value)
        {
            data_.pushBack(std::move(value));
        }

        // TODO A14: pop - return false if empty, otherwise extract last element
        bool pop(T& result)
        {
            if (data_.empty())
            {
                return false;
            }

            result = data_.at(data_.size() - 1);
            data_.eraseAt(data_.size() - 1);
            return true;
        }

        // TODO A15: top - read last element without removing it
        const T& top() const
        {
            if (data_.empty())
            {
                throw std::out_of_range("Stack::top() called on empty stack");
            }

            return data_.at(data_.size() - 1);
        }

        T& top()
        {
            if (data_.empty())
            {
                throw std::out_of_range("Stack::top() called on empty stack");
            }

            return data_.at(data_.size() - 1);
        }

        void clear()
        {
            data_.clear();
        }
    };
}
