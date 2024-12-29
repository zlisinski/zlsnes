#ifndef ZLSNES_CORE_PPUCONSTANTS_H
#define ZLSNES_CORE_PPUCONSTANTS_H

#include "Zlsnes.h"

// Bits-per-pixel for each background layer for each mode.
static const uint8_t BG_BPP_LOOKUP[8][4] = {
    {2, 2, 2, 2}, // mode 0
    {4, 4, 2, 0}, // mode 1
    {4, 4, 0, 0}, // mode 2
    {8, 4, 0, 0}, // mode 3
    {8, 2, 0, 0}, // mode 4
    {4, 2, 0, 0}, // mode 5
    {4, 0, 0, 0}, // mode 6
    {8, 0, 0, 0}, // mode 7
};
static const uint8_t OBJ_BPP = 4;

static const uint8_t OBJ_H_SIZE_LOOKUP[8][2] = {
    {8, 16}, // 0
    {8, 32}, // 1
    {8, 64}, // 2
    {16, 32}, // 3
    {16, 64}, // 4
    {32, 64}, // 5
    {16, 32}, // 6
    {16, 32}, // 7
};

static const uint8_t OBJ_V_SIZE_LOOKUP[8][2] = {
    {8, 16}, // 0
    {8, 32}, // 1
    {8, 64}, // 2
    {16, 32}, // 3
    {16, 64}, // 4
    {32, 64}, // 5
    {32, 64}, // 6
    {32, 32}, // 7
};

#endif