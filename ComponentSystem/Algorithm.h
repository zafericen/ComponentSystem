#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <cstdint>
#include <string>

#include "Core.h"

#include "Signature.h"

namespace CECS
{
    template<typename T>
    struct Hash
    {
        constexpr size_t operator()(const T& arg) const
        {
            size_t hash{ static_cast<size_t>(reinterpret_cast<std::uintptr_t>(&arg)) };
            hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
            hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
            hash = ((hash >> 16) ^ hash);
            return hash;
        }
    };

    template<>
    struct Hash<uint32_t>
    {
        constexpr uint32_t operator()(uint32_t arg) const
        {
            return arg;
        }
    };

    template<>
    struct Hash<uint16_t>
    {
        constexpr uint16_t operator()(uint16_t arg) const
        {
            return arg;
        }
    };

    template<>
    struct Hash<size_t>
    {
        constexpr size_t operator()(size_t arg) const
        {
            return arg;
        }
    };

    template<>
    struct Hash<std::string>
    {
        constexpr uint32_t operator()(const std::string& s) const
        {
            return operator()(s.c_str());
        }

        constexpr uint32_t operator()(const char* s) const
        {
            uint32_t hash{};

            for (uint32_t index{}; s[index]; ++index)
            {
                hash += (index + 1) * s[index];
            }

            return hash;
        }
    };

    template<>
    struct Hash<Signature>
    {
        size_t operator()(const Signature& signature) const
        {
            size_t out{};
            for (const auto& pair : signature.getBitsets())
            {
                out += (pair.first + 1) * pair.second.to_ullong();
            }
            return out;
        }
    };
}

#endif