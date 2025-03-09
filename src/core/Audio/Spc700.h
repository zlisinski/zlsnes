#ifndef ZLSNES_CORE_APU_SPC700
#define ZLSNES_CORE_APU_SPC700


#include "../Zlsnes.h"


namespace Audio
{


class AbsAddressMode;
class Memory;
class Timer;


class Spc700
{
public:
    struct Registers
    {
// Temporarily turn off errors for anonymous structs. This also works on clang, not just gcc.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

        union
        {
            uint16_t ya = 0;
            struct
            {
                uint8_t a; // Accumulator.
                uint8_t y; // Y index.
            };
        };

        uint8_t x = 0; // X index.
        uint8_t sp = 0xFF; // Stack pointer.
        uint16_t pc = 0xFFC0; // Program counter.

        union
        {
            uint8_t p = 0;
            struct
            {
                uint8_t c:1; // Carry, bit 0.
                uint8_t z:1; // Zero, bit 1.
                uint8_t i:1; // IRQ Disable, bit 2.
                uint8_t h:1; // Half-carry, bit 3.
                uint8_t b:1; // Break, bit 4.
                uint8_t p:1; // Zero page location, bit 5.
                uint8_t v:1; // Overflow, bit 6.
                uint8_t n:1; // Negative, bit 7.
            } flags;
        };
#pragma GCC diagnostic pop

    } reg;

    Spc700(Memory *memory, Timer *timer);
    virtual ~Spc700();

    uint8_t ReadPC8Bit();
    uint16_t ReadPC16Bit();

    void Step(int clocksToRun);
    void ProcessOpCode();

private:

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

    template <typename T>
    inline void LoadRegister(T &dest, T value)
    {
        static_assert(std::is_integral<T>::value, "value must be integral type.");
        static_assert(sizeof(T) <= 2, "sizeof(T) should be 1 or 2 bytes");

        dest = value;
        SetNFlag(dest);
        SetZFlag(dest);
    }

    uint8_t Add8Bit(uint8_t x, uint8_t y);
    uint8_t Sub8Bit(uint8_t x, uint8_t y);
    void Compare(uint8_t x, uint8_t y);
    void Push(uint8_t value);
    uint8_t Pop();

    Memory *memory = nullptr;
    Timer *timer = nullptr;

    bool waiting = false;
    int clocksAhead = 0;

    using AddressModePtr = std::unique_ptr<AbsAddressMode>;
    AddressModePtr addressModes[32];
    AddressModePtr addressModesMovXY[32];

    friend class Spc700Test;
};

} // end namespace

#endif