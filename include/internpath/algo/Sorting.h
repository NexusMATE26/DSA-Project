#pragma once

#include <cstddef>
#include <utility>

namespace internpath
{
    // MEMBER A | TODO A19-A21 | Comparator returns true if first goes before second.

    // TODO A19: insertionSort - stable, O(n) best, O(n^2) worst
    template <typename T, typename Compare>
    void insertionSort(T* data, std::size_t count, Compare comesBefore)
    {
        if (data == nullptr || count <= 1)
        {
            return;
        }

        for (std::size_t i = 1; i < count; i++)
        {
            T key = std::move(data[i]);
            std::size_t j = i;

            while (j > 0 && comesBefore(key, data[j - 1]))
            {
                data[j] = std::move(data[j - 1]);
                j--;
            }

            data[j] = std::move(key);
        }
    }

    namespace detail
    {
        template <typename T, typename Compare>
        void merge(T* data, std::size_t left, std::size_t mid, std::size_t right, T* temp, Compare comesBefore)
        {
            std::size_t i = left;
            std::size_t j = mid;
            std::size_t k = left;

            while (i < mid && j < right)
            {
                // Stability: if neither is strictly smaller or data[i] comesBefore data[j], preserve left
                if (comesBefore(data[j], data[i]))
                {
                    temp[k++] = std::move(data[j++]);
                }
                else
                {
                    temp[k++] = std::move(data[i++]);
                }
            }

            while (i < mid)
            {
                temp[k++] = std::move(data[i++]);
            }

            while (j < right)
            {
                temp[k++] = std::move(data[j++]);
            }

            for (std::size_t idx = left; idx < right; idx++)
            {
                data[idx] = std::move(temp[idx]);
            }
        }

        template <typename T, typename Compare>
        void mergeSortRecursive(T* data, std::size_t left, std::size_t right, T* temp, Compare comesBefore)
        {
            if (right - left <= 1)
            {
                return;
            }

            std::size_t mid = left + (right - left) / 2;
            mergeSortRecursive(data, left, mid, temp, comesBefore);
            mergeSortRecursive(data, mid, right, temp, comesBefore);
            merge(data, left, mid, right, temp, comesBefore);
        }

        template <typename T, typename Compare>
        void quickSortRecursive(T* data, std::size_t left, std::size_t right, Compare comesBefore)
        {
            if (right <= left + 1)
            {
                return;
            }

            T pivot = std::move(data[right - 1]);
            std::size_t partitionIndex = left;

            for (std::size_t i = left; i + 1 < right; i++)
            {
                if (comesBefore(data[i], pivot))
                {
                    std::swap(data[i], data[partitionIndex]);
                    partitionIndex++;
                }
            }

            data[right - 1] = std::move(data[partitionIndex]);
            data[partitionIndex] = std::move(pivot);

            quickSortRecursive(data, left, partitionIndex, comesBefore);
            quickSortRecursive(data, partitionIndex + 1, right, comesBefore);
        }
    }

    // TODO A20: mergeSort - stable O(n log n) sorting with scratch buffer
    template <typename T, typename Compare>
    void mergeSort(T* data, std::size_t count, Compare comesBefore)
    {
        if (data == nullptr || count <= 1)
        {
            return;
        }

        T* temp = new T[count];
        detail::mergeSortRecursive(data, 0, count, temp, comesBefore);
        delete[] temp;
    }

    // TODO A21: quickSort - optional in-place quicksort
    template <typename T, typename Compare>
    void quickSort(T* data, std::size_t count, Compare comesBefore)
    {
        if (data == nullptr || count <= 1)
        {
            return;
        }

        detail::quickSortRecursive(data, 0, count, comesBefore);
    }
}
