#pragma once

#include <cstdint>
#include <string>
#include <type_traits>

namespace internpath
{
    // MEMBER B | TODO B01 | Handwritten hashes for integer and string keys.

    // 64-bit integer mix function (SplitMix64)
    inline std::uint64_t hashNumber(std::uint64_t key)
    {
        key ^= (key >> 30);
        key *= 0xbf58476d1ce4e5b9ULL;
        key ^= (key >> 27);
        key *= 0x94d049bb133111ebULL;
        key ^= (key >> 31);
        return key;
    }

    // FNV-1a 64-bit string hashing
    inline std::uint64_t hashText(const std::string& text)
    {
        std::uint64_t hash = 0xcbf29ce484222325ULL;

        for (char ch : text)
        {
            hash ^= static_cast<std::uint8_t>(ch);
            hash *= 0x100000001b3ULL;
        }

        return hash;
    }

    // Generic key hash dispatcher
    template <typename Key>
    inline std::uint64_t hashKey(const Key& key)
    {
        if constexpr (std::is_integral_v<Key>)
        {
            return hashNumber(static_cast<std::uint64_t>(key));
        }
        else if constexpr (std::is_same_v<Key, std::string>)
        {
            return hashText(key);
        }
        else if constexpr (std::is_pointer_v<Key>)
        {
            return hashNumber(reinterpret_cast<std::uintptr_t>(key));
        }
        else
        {
            return hashNumber(static_cast<std::uint64_t>(key));
        }
    }
}
