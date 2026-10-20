#pragma once

#include <cstddef>
#include <stdexcept>

#include "internpath/ds/Hashing.h"
#include "internpath/ds/SinglyLinkedList.h"

namespace internpath
{
    // MEMBER B | TODO B02-B05 | Separate chaining using OUR linked list.
    template <typename Key, typename Value>
    class HashTable
    {
    public:
        struct Entry
        {
            Key key;
            Value value;
        };

    private:
        SinglyLinkedList<Entry>* buckets_ = nullptr;
        std::size_t bucketCount_ = 0;
        std::size_t size_ = 0;

        std::size_t bucketIndexFor(const Key& key, std::size_t numBuckets) const
        {
            if (numBuckets == 0)
            {
                return 0;
            }

            return static_cast<std::size_t>(hashKey(key) % numBuckets);
        }

        void rehash(std::size_t newBucketCount)
        {
            if (newBucketCount == 0)
            {
                newBucketCount = 8;
            }

            auto* newBuckets = new SinglyLinkedList<Entry>[newBucketCount];

            for (std::size_t i = 0; i < bucketCount_; i++)
            {
                auto* node = buckets_[i].firstNode();
                while (node != nullptr)
                {
                    std::size_t newIdx = bucketIndexFor(node->value.key, newBucketCount);
                    newBuckets[newIdx].pushBack(std::move(node->value));
                    node = node->next;
                }
            }

            delete[] buckets_;
            buckets_ = newBuckets;
            bucketCount_ = newBucketCount;
        }

    public:
        HashTable() : bucketCount_(8), size_(0)
        {
            buckets_ = new SinglyLinkedList<Entry>[bucketCount_];
        }

        HashTable(const HashTable&) = delete;
        HashTable& operator=(const HashTable&) = delete;

        HashTable(HashTable&& other) noexcept
            : buckets_(other.buckets_), bucketCount_(other.bucketCount_), size_(other.size_)
        {
            other.buckets_ = nullptr;
            other.bucketCount_ = 0;
            other.size_ = 0;
        }

        HashTable& operator=(HashTable&& other) noexcept
        {
            if (this != &other)
            {
                delete[] buckets_;
                buckets_ = other.buckets_;
                bucketCount_ = other.bucketCount_;
                size_ = other.size_;

                other.buckets_ = nullptr;
                other.bucketCount_ = 0;
                other.size_ = 0;
            }

            return *this;
        }

        ~HashTable()
        {
            delete[] buckets_;
        }

        std::size_t size() const
        {
            return size_;
        }

        std::size_t bucketCount() const
        {
            return bucketCount_;
        }

        bool empty() const
        {
            return size_ == 0;
        }

        double loadFactor() const
        {
            if (bucketCount_ == 0)
            {
                return 0.0;
            }

            return static_cast<double>(size_) / static_cast<double>(bucketCount_);
        }

        // TODO B02: Choose hash -> bucket -> scan chain -> insert/update.
        // Grow and rehash when load factor exceeds 0.75.
        bool insertOrAssign(const Key& key, const Value& value)
        {
            if (bucketCount_ == 0 || (static_cast<double>(size_ + 1) / static_cast<double>(bucketCount_)) > 0.75)
            {
                rehash(bucketCount_ == 0 ? 8 : bucketCount_ * 2);
            }

            std::size_t idx = bucketIndexFor(key, bucketCount_);
            auto* node = buckets_[idx].firstNode();

            while (node != nullptr)
            {
                if (node->value.key == key)
                {
                    node->value.value = value;
                    return false;
                }

                node = node->next;
            }

            buckets_[idx].pushBack(Entry{key, value});
            size_++;
            return true;
        }

        // TODO B03: Scan appropriate chain; return false if absent.
        bool tryGet(const Key& key, Value& result) const
        {
            if (bucketCount_ == 0)
            {
                return false;
            }

            std::size_t idx = bucketIndexFor(key, bucketCount_);
            auto* node = buckets_[idx].firstNode();

            while (node != nullptr)
            {
                if (node->value.key == key)
                {
                    result = node->value.value;
                    return true;
                }

                node = node->next;
            }

            return false;
        }

        bool contains(const Key& key) const
        {
            Value dummy;
            return tryGet(key, dummy);
        }

        // TODO B04: Unlink matching entry using removeIf and update count.
        bool erase(const Key& key)
        {
            if (bucketCount_ == 0)
            {
                return false;
            }

            std::size_t idx = bucketIndexFor(key, bucketCount_);
            bool removed = buckets_[idx].removeIf([&key](const Entry& entry) {
                return entry.key == key;
            });

            if (removed)
            {
                size_--;
            }

            return removed;
        }

        void clear()
        {
            for (std::size_t i = 0; i < bucketCount_; i++)
            {
                buckets_[i].clear();
            }

            size_ = 0;
        }

        // TODO B05: Validate structural invariants and element count.
        bool validate() const
        {
            std::size_t total = 0;

            for (std::size_t i = 0; i < bucketCount_; i++)
            {
                auto* node = buckets_[i].firstNode();
                while (node != nullptr)
                {
                    total++;
                    node = node->next;
                }
            }

            return total == size_;
        }
    };
}
