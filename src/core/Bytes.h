#pragma once

#include "Zlsnes.h"

namespace Bytes
{
    inline uint16_t Make16Bit(uint8_t high, uint8_t low)
    {
        return (high << 8) | low;
    }

    inline uint32_t Make24Bit(uint8_t bank, uint16_t offset)
    {
        return (bank << 16) | offset;
    }

    inline uint32_t Make24Bit(uint8_t high, uint8_t mid, uint8_t low)
    {
        return (high << 16) | (mid << 8) | low;
    }

    template <uint8_t N, typename T>
    inline uint8_t GetByte(T value)
    {
        static_assert(std::is_integral<T>::value, "value must be integral type.");
        static_assert(N < sizeof(T), "N must be less than sizeof(T)");

        // Return type is uint8_t, so there is no need to mask off other bytes.
        return value >> (8 * N);
    }

    template <uint8_t N, typename T>
    inline uint8_t GetBit(T value)
    {
        static_assert(std::is_integral<T>::value, "value must be integral type.");
        static_assert(N < sizeof(T) * 8, "N must be less than sizeof(T) * 8");

        return (value >> N) & 0x01;
    }

    template <uint8_t N, typename T>
    inline T MaskByte(T value)
    {
        static_assert(std::is_integral<T>::value, "value must be integral type.");
        static_assert(N < sizeof(T), "N must be less than sizeof(T)");

        return value & (0xFF << (N * 8));
    }
}