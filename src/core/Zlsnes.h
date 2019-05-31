#pragma once

#include <stdio.h>
#include <stdint.h>

#include "Exceptions.h"
#include "Logger.h"

inline uint16_t Make16Bit(uint8_t high, uint16_t low)
{
    return (high << 8) | low;
}

inline uint32_t Make24Bit(uint8_t bank, uint16_t addr)
{
    return (bank << 16) | addr;
}

inline uint32_t Make24Bit(uint8_t high, uint8_t mid, uint8_t low)
{
    return (high << 16) | (mid << 8) | low;
}