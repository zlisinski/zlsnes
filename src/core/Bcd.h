#pragma once

#include "Zlsnes.h"

namespace Bcd
{
    // The Add and Subtract were copied, with some modification, from http://homepage.divms.uiowa.edu/~jones/bcd/bcd.html#packed
    // I won't pretend to understand all the math.

    inline uint32_t Add(uint32_t a, uint32_t b)
    {
        uint32_t t1 = a + 0x6666;
        uint32_t t2 = t1 + b;
        uint32_t t3 = t1 ^ b;
        uint32_t t4 = t2 ^ t3;
        uint32_t t5 = ~t4 & 0x11110;
        uint32_t t6 = (t5 >> 2) | (t5 >> 3);
        return t2 - t6;
    }

    // This returns 0x0001nnnn when there is no carry/borrow and 0x0000nnnn when there is.
    // I don't feel like messing with it, so deal with it ;)
    inline uint32_t Subtract(uint16_t a, uint16_t b)
    {
        uint32_t t1 = 0x9999 - b;
        uint32_t t2 = Add(t1, 1);
        return Add(t2, a);
    }

    inline uint32_t ToBcd(uint32_t num)
    {
        uint32_t result = 0;
        uint32_t shift = 0;

        while (num > 0)
        {
            result |= (num % 10) << (shift * 4);
            num /= 10;
            shift++;
        }

        return result;
    }
}