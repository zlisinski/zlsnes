#pragma once

#include <stdio.h>
#include <stdint.h>
#include <type_traits>

#include "Exceptions.h"
#include "Logger.h"

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