#ifndef ZLSNES_CORE_BGR555_H
#define ZLSNES_CORE_BGR555_H


#include "Zlsnes.h"
#include "Bytes.h"


struct Bgr555
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    inline Bgr555() : red(0), green(0), blue(0) {}

    inline Bgr555(uint16_t color) :
        red(color & 0x1F),
        green((color >> 5) & 0x1F),
        blue((color >> 10) & 0x1F)
    {}

    inline Bgr555(const Bgr555 &other)
    {
        // memcpy can copy in 1 instruction vs 3 when copying each field separately.
        memcpy(this, &other, sizeof(Bgr555));
    }

    inline Bgr555 &operator=(const Bgr555 &other)
    {
        memcpy(this, &other, sizeof(Bgr555));
        return *this;
    }

    inline uint16_t ToUint16()
    {
        return (blue << 10) | (green << 5) | red;
    }

    inline uint32_t ToARGB888(uint8_t brightness)
    {
        uint8_t r = (red << 3) | ((red >> 2) & 0x07);
        uint8_t g = (green << 3) | ((green >> 2) & 0x07);
        uint8_t b = (blue << 3) | ((blue >> 2) & 0x07);

        return ((brightness * 17) << 24) | Bytes::Make24Bit(r, g, b);
    }

    inline void Add(Bgr555 other, bool halve)
    {
        blue = (blue + other.blue) >> halve;
        green = (green + other.green) >> halve;
        red = (red + other.red) >> halve;

        if (blue > 31)
            blue = 31;
        if (green > 31)
            green = 31;
        if (red > 31)
            red = 31;
    }

    inline void Subtract(Bgr555 other, bool halve)
    {
        blue = (blue - other.blue) >> halve;
        green = (green - other.green) >> halve;
        red = (red - other.red) >> halve;

        if (blue > 31)
            blue = 0;
        if (green > 31)
            green = 0;
        if (red > 31)
            red = 0;
    }
};


#endif