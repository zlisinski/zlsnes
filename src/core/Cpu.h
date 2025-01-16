#pragma once

#include <memory>

#include "Zlsnes.h"
#include "Bytes.h"
#include "Dma.h"
#include "Memory.h"

class AbsAddressMode;
class Dma;
class Interrupt;
class Timer;

struct Registers
{
    Registers() :
        a(0),
        x(0),
        y(0),
        d(0),
        sp(0x01FF),
        db(0),
        pb(0),
        pc(0),
        p(0x34),
        breakFlag(false),
        emulationMode(true)
    {}

// Temporarily turn off errors for anonymous structs. This also works on clang, not just gcc.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

    union // Accumulator.
    {
        uint16_t a;
        struct
        {
            uint8_t al;
            uint8_t ah;
        };
    };
    union // X index.
    {
        uint16_t x;
        struct
        {
            uint8_t xl;
            uint8_t xh;
        };
    };
    union // Y index.
    {
        uint16_t y;
        struct
        {
            uint8_t yl;
            uint8_t yh;
        };
    };
    union // Direct page register.
    {
        uint16_t d;
        struct
        {
            uint8_t dl;
            uint8_t dh;
        };
    };
    union // Stack pointer.
    {
        uint16_t sp;
        struct
        {
            uint8_t sl;
            uint8_t sh;
        };
    };

    uint8_t db; // Data bank register.
    uint8_t pb; // Program bank register.
    uint16_t pc; // Program counter.

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

#pragma GCC diagnostic pop

    bool breakFlag;
    bool emulationMode;
};


class Cpu
{
public:
    Cpu(Memory *memory, Timer *timer, Interrupt *interrupts);
    ~Cpu();

    uint8_t ReadPC8Bit();
    uint16_t ReadPC16Bit();
    uint32_t ReadPC24Bit();

    void Reset();
    void ProcessOpCode();

    inline void PrintState() const
    {
        LogCpu("State: a=%04X, x=%04X, y=%04X, d=%04X, db=%02X, pb=%02X, pc=%04X, sp=%04X, p=%02X, flags=c:%X z:%X i:%X d:%X x:%X m:%X v:%X n:%X\n",
               reg.a, reg.x, reg.y, reg.d, reg.db, reg.pb, reg.pc, reg.sp, reg.p,
               reg.flags.c, reg.flags.z, reg.flags.i, reg.flags.d, reg.flags.x, reg.flags.m, reg.flags.v, reg.flags.n);
    }

    inline Address GetFullPC() const {return Address(reg.pb, reg.pc);}
    inline Timer *GetTimer() {return timer;}

    Registers reg;
    uint8_t opcode;

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

    template <typename T>
    inline void SetNFlag(T value)
    {
        static_assert(std::is_integral<T>::value, "value must be integral type.");
        static_assert(sizeof(T) <= 2, "sizeof(T) should be 1 or 2 bytes");

        reg.flags.n = (value & (0x80 << ((sizeof(T) - 1) * 8))) != 0;
    }

    template <typename T>
    inline void SetZFlag(T value)
    {
        static_assert(std::is_integral<T>::value, "value must be integral type.");
        static_assert(sizeof(T) <= 2, "sizeof(T) should be 1 or 2 bytes");

        reg.flags.z = value == 0;
    }

    ///////////////////////////////////////////////////////////////////////////

    template <typename T>
    inline void LoadRegister(T *dest, T value)
    {
        static_assert(std::is_integral<T>::value, "value must be integral type.");
        static_assert(sizeof(T) <= 2, "sizeof(T) should be 1 or 2 bytes");

        *dest = value;
        SetNFlag(*dest);
        SetZFlag(*dest);
    }

    ///////////////////////////////////////////////////////////////////////////

    inline void Push8Bit(uint8_t value)
    {
        memory->Write8Bit(Address(0, reg.sp), value);
        reg.sp--;

        // High byte is always 0x01 in emulation mode, and low byte wraps.
        if (reg.emulationMode == true && reg.sp < 0x0100)
            reg.sp = 0x01FF;
    }

    inline void Push16Bit(uint16_t value)
    {
        Push8Bit(Bytes::GetByte<1>(value));
        Push8Bit(Bytes::GetByte<0>(value));
    }

    inline void Push24Bit(uint32_t value)
    {
        Push8Bit(Bytes::GetByte<2>(value));
        Push8Bit(Bytes::GetByte<1>(value));
        Push8Bit(Bytes::GetByte<0>(value));
    }

    inline uint8_t Pop8Bit()
    {
        reg.sp++;

        // High byte is always 0x01 in emulation mode, and low byte wraps.
        if (reg.emulationMode == true && reg.sp > 0x01FF)
            reg.sp = 0x0100;

        uint8_t value = memory->Read8Bit(Address(0, reg.sp));

        return value;
    }

    inline uint16_t Pop16Bit()
    {
        uint8_t low = Pop8Bit();
        uint8_t high = Pop8Bit();

        return Bytes::Make16Bit(high, low);
    }

    inline uint32_t Pop24Bit()
    {
        uint8_t low = Pop8Bit();
        uint8_t mid = Pop8Bit();
        uint8_t high = Pop8Bit();

        return Bytes::Make24Bit(high, mid, low);
    }

    ///////////////////////////////////////////////////////////////////////////

    template <typename T>
    inline void Compare(T a, T b)
    {
        static_assert(std::is_integral<T>::value, "value must be integral type.");
        static_assert(sizeof(T) <= 2, "sizeof(T) should be 1 or 2 bytes");

        T result = a - b;
        reg.flags.c = a >= b;
        SetNFlag(result);
        SetZFlag(result);
    }

    ///////////////////////////////////////////////////////////////////////////

    void SetEmulationMode(bool value);
    void UpdateRegistersAfterFlagChange();
    void ProcessInterrupt();

    void NotYetImplemented(uint8_t opcode);

    Memory *memory;
    Timer *timer;
    Interrupt *interrupts;
    Dma dma;

    bool waiting;

    using AddressModePtr = std::unique_ptr<AbsAddressMode>;
    AddressModePtr addressModes[32];
    AddressModePtr addressModeAlternate[32];
    AddressModePtr jmpAddressModes[16];

    friend class CpuTest;
};