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

    // Addressing modes ///////////////////////////////////////////////////////

    inline uint16_t GetOpAbsolute() // a - Absolute
    {
        uint32_t addr = Make24Bit(reg.db, ReadPC16Bit());
        return memory->Read16Bit(addr);
    }

    inline uint16_t GetOpAbsoluteIndexedX() // a,x - Absolute,X
    {
        uint32_t addr = Make24Bit(reg.db, ReadPC16Bit()) + reg.x;
        return memory->Read16Bit(addr);
    }

    inline uint16_t GetOpAbsoluteIndexedY() // a,y - Absolute,Y
    {
        uint32_t addr = Make24Bit(reg.db, ReadPC16Bit()) + reg.y;
        return memory->Read16Bit(addr);
    }

    inline uint32_t GetOpAbsoluteIndirect() // (a) - (Absolute)
    {
        uint16_t addr = memory->Read16BitWrapBank(0, ReadPC16Bit());
        return Make24Bit(reg.pb, addr);
    }

    inline uint32_t GetOpAbsoluteIndexedIndirect() // (a,x) - (Absolute,X)
    {
        uint16_t addr = memory->Read16BitWrapBank(reg.pb, ReadPC16Bit() + reg.x);
        return Make24Bit(reg.pb, addr);
    }

    inline uint16_t GetOpAccumulator() // A - Accumulator
    {
        return reg.a;
    }

    inline uint16_t GetOpDirect() // d - Direct
    {
        // TODO: Handle emulation mode special case.
        uint16_t addr = ReadPC8Bit() + reg.d;
        return memory->Read16BitWrapBank(0, addr);
    }

    inline uint16_t GetOpDirectIndexedX() // d,x - Direct,X
    {
        // TODO: Handle emulation mode special case.
        uint16_t addr = ReadPC8Bit() + reg.d + reg.x;
        return memory->Read16BitWrapBank(0, addr);
    }

    inline uint16_t GetOpDirectIndexedY() // d,y - Direct,Y
    {
        // TODO: Handle emulation mode special case.
        uint16_t addr = ReadPC8Bit() + reg.d + reg.y;
        return memory->Read16BitWrapBank(0, addr);
    }

    inline uint16_t GetOpDirectIndirect() // (d) - (Direct)
    {
        // TODO: Handle emulation mode special case.
        uint16_t addr = memory->Read16BitWrapBank(0, ReadPC8Bit() + reg.d);
        return memory->Read16Bit(Make24Bit(reg.db, addr));
    }

    inline uint16_t GetOpDirectIndirectLong() // [d] - [Direct]
    {
        uint32_t addr = memory->Read24BitWrapBank(0, ReadPC8Bit() + reg.d);
        return memory->Read16Bit(addr);
    }

    inline uint16_t GetOpDirectIndexedIndirect() // (d,x) - (Direct,X)
    {
        // TODO: Handle emulation mode special case.
        uint16_t addr = memory->Read16BitWrapBank(0, ReadPC8Bit() + reg.d + reg.x);
        return memory->Read16Bit(Make24Bit(reg.db, addr));
    }

    inline uint16_t GetOpDirectIndirectIndexed() // (d),y - (Direct),Y
    {
        // TODO: Handle emulation mode special case.
        uint16_t addr = memory->Read16BitWrapBank(0, ReadPC8Bit() + reg.d);
        return memory->Read16Bit(Make24Bit(reg.db, addr) + reg.y);
    }

    inline uint16_t GetOpDirectIndirectLongIndexed() // [d],y - [Direct],Y
    {
        uint32_t addr = memory->Read24BitWrapBank(0, ReadPC8Bit() + reg.d);
        return memory->Read16Bit(Make24Bit(reg.db, addr) + reg.y);
    }

    // The size of read depends on the instruction.
    /*inline uint32_t GetOpImmediate() // # - Immediate
    {
        return ReadPC16Bit();
    }*/

    inline uint16_t GetOpAbsoluteLong() // al - Long
    {
        return memory->Read16Bit(ReadPC24Bit());
    }

    inline uint16_t GetOpAbsoluteLongIndexedX() // al,x - Long,X
    {
        return memory->Read16Bit(ReadPC24Bit() + reg.x);
    }

    // ProgramCounterRelative // r - Relative8
    // ProgramCounterRelativeLong // rl - Relative16
    // Stack // s

    inline uint16_t GetOpStackRelative() // d,s - Stack,S
    {
        return memory->Read16BitWrapBank(0, ReadPC8Bit() + reg.sp);
    }

    inline uint16_t GetOpStackRelativeIndirectIndexed() // (d,s),y - (Stack,S),Y
    {
        uint32_t addr = memory->Read16BitWrapBank(0, ReadPC8Bit() + reg.sp);
        return memory->Read16Bit(Make24Bit(reg.db, addr) + reg.y);
    }

    Registers reg;

private:
    void NotYetImplemented(uint8_t opcode);

    Memory *memory;

};