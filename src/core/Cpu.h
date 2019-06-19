#pragma once

#include "Zlsnes.h"
#include "Memory.h"


struct Registers
{
    Registers() :
        a(0),
        x(0),
        y(0),
        d(0),
        db(0),
        pb(0),
        pc(0),
        sp(0x0100),
        p(0x34),
        breakFlag(false),
        emulationMode(true)
    {}

    uint16_t a; // Accumulator.
    uint16_t x; // X index.
    uint16_t y; // Y index.
    uint16_t d; // Direct page register.
    uint8_t db; // Data bank register.
    uint8_t pb; // Program bank register.
    uint16_t pc; // Program counter.
    uint16_t sp; // Stack pointer.

    union
    {
        uint8_t p;
        struct
        {
            uint8_t c:1; // Carry, bit 0.
            uint8_t z:1; // Zero, bit 1.
            uint8_t i:1; // IRQ Disable, bit 2.
            uint8_t d:1; // Decimal, bit 3.
            uint8_t x:1; // Index size (native mode), break (emulation mode), bit 4.
            uint8_t m:1; // Accumulator size, bit 5.
            uint8_t v:1; // Overflow, bit 6.
            uint8_t n:1; // Negative, bit 7.
        } flags;
    };

    bool breakFlag;
    bool emulationMode;
};


class Cpu
{
public:
    Cpu(Memory *memory);

    uint8_t ReadPC8Bit();
    uint16_t ReadPC16Bit();
    uint32_t ReadPC24Bit();

    void ProcessOpCode();

    inline void PrintState() const
    {
        LogInstruction("State: a=%04X, x=%04X, y=%04X, d=%04X, db=%02X, pb=%02X, pc=%04X, sp=%04X, p=%02X, flags=c:%X z:%X i:%X d:%X x:%X m:%X v:%X n:%X\n",
               reg.a, reg.x, reg.y, reg.d, reg.db, reg.pb, reg.pc, reg.sp, reg.p,
               reg.flags.c, reg.flags.z, reg.flags.i, reg.flags.d, reg.flags.x, reg.flags.m, reg.flags.v, reg.flags.n);
    }

    Registers reg;

private:

    inline bool IsAccumulator8Bit()
    {
        return reg.emulationMode == true || reg.flags.m == 1;
    }

    inline bool IsAccumulator16Bit()
    {
        return reg.emulationMode == false && reg.flags.m == 0;
    }

    inline bool IsIndex8Bit()
    {
        return reg.emulationMode == true || reg.flags.x == 1;
    }

    inline bool IsIndex16Bit()
    {
        return reg.emulationMode == false && reg.flags.x == 0;
    }

    inline uint32_t GetFullPC(uint16_t pc) const
    {
        return (reg.pb << 16) | pc;
    }

    ///////////////////////////////////////////////////////////////////////////

    inline void LoadRegister8Bit(uint16_t *dest, uint16_t value)
    {
        *dest = (*dest & 0xFF00) | (value & 0x00FF);
        reg.flags.n = (*dest & 0x80) != 0;
        reg.flags.z = (*dest & 0x00FF) == 0;
    }

    inline void LoadRegister16Bit(uint16_t *dest, uint16_t value)
    {
        *dest = value;
        reg.flags.n = (*dest & 0x8000) != 0;
        reg.flags.z = *dest == 0;
    }

    void NotYetImplemented(uint8_t opcode);

    Memory *memory;

    friend class CpuTest;
};