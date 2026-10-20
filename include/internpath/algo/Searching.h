#pragma once

#include <cstddef>

namespace internpath
{
    // MEMBER B | TODO B10-B11

    // TODO B10: linearSearch - return first matching index or -1 in O(n)
    template <typename T>
    long long linearSearch(const T* data, std::size_t count, const T& target)
    {
        if (data == nullptr)
        {
            return -1;
        }

        for (std::size_t i = 0; i < count; i++)
        {
            if (data[i] == target)
            {
                return static_cast<long long>(i);
            }
        }

        return -1;
    }

    // TODO B11: lowerBound - on sorted array, find first position not less than target in O(log n)
    template <typename T, typename Compare>
    std::size_t lowerBound(const T* data, std::size_t count,
                           const T& target, Compare comesBefore)
    {
        if (data == nullptr || count == 0)
        {
            return 0;
        }

        std::size_t left = 0;
        std::size_t right = count;

        while (left < right)
        {
            std::size_t mid = left + (right - left) / 2;

            if (comesBefore(data[mid], target))
            {
                left = mid + 1;
            }
            else
            {
                right = mid;
            }
        }

        return left;
    }
}
