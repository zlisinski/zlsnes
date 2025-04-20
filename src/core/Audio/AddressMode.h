#ifndef ZLSNES_CORE_APU_ADDRESSMODE
#define ZLSNES_CORE_APU_ADDRESSMODE

#include <exception>

#include "Spc700.h"
#include "Memory.h"
#include "../Zlsnes.h"


namespace Audio
{


class AbsAddressMode
{
public:
    AbsAddressMode(Spc700 *cpu, Memory *memory, const char *formatStr, int dataLen) :
        cpu(cpu),
        memory(memory),
        address(0),
        formatStr(formatStr),
        dataLen(dataLen),
        data16(0)
    {}

    virtual ~AbsAddressMode() {}

    uint16_t GetAddress() const {return address;}

    // Reads from cpu/memory and advances program counter.
    virtual void LoadAddress() = 0;

    virtual uint8_t Read8Bit() {return memory->Read8Bit(address);}
    virtual uint16_t Read16Bit() {return memory->Read16Bit(address);}

    virtual void Write8Bit(uint8_t value) {memory->Write8Bit(address, value);}
    virtual void Write16Bit(uint16_t value) {memory->Write16Bit(address, value);}

    void Log(const char *name)
    {
        switch (dataLen)
        {
            case 0:
                LogSpc700(formatStr, cpu->opcode, name);
                break;
            case 1:
                LogSpc700(formatStr, cpu->opcode, dataBytes[0], name, data8);
                break;
            case 2:
                LogSpc700(formatStr, cpu->opcode, dataBytes[0], dataBytes[1], name, data16);
                break;
        }
    }

protected:
    Spc700 *cpu;
    Memory *memory;
    uint16_t address;
    const char *formatStr;
    int dataLen;
    union
    {
        uint8_t data8;
        uint16_t data16;
        uint8_t dataBytes[2];
    };

    friend class AddressModeTest;
};


// a - Absolute
class AddressModeAbsolute : public AbsAddressMode
{
public:
    AddressModeAbsolute(Spc700 *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X %02X: %s !%04X", 2)
    {}

    void LoadAddress() override
    {
        data16 = cpu->ReadPC16Bit();
        address = data16;
    }
};


// a,x - Absolute,X
class AddressModeAbsoluteIndexedX : public AbsAddressMode
{
public:
    AddressModeAbsoluteIndexedX(Spc700 *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X %02X: %s !%04X,X", 2)
    {}

    void LoadAddress() override
    {
        data16 = cpu->ReadPC16Bit();
        address = data16 + cpu->reg.x;
    }
};


// a,y - Absolute,Y
class AddressModeAbsoluteIndexedY : public AbsAddressMode
{
public:
    AddressModeAbsoluteIndexedY(Spc700 *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X %02X: %s !%04X,Y", 2)
    {}

    void LoadAddress() override
    {
        data16 = cpu->ReadPC16Bit();
        address = data16 + cpu->reg.y;
    }
};


// A - Accumulator
class AddressModeAccumulator : public AbsAddressMode
{
public:
    AddressModeAccumulator(Spc700 *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X: %s A", 0)
    {}

    void LoadAddress() override {}

    uint8_t Read8Bit() override {return cpu->reg.a;}
    uint16_t Read16Bit() override {throw std::logic_error("Can't read 16bit from AddressModeAccumulator");}
    void Write8Bit(uint8_t value) override {cpu->reg.a = value;}
    void Write16Bit(uint16_t value) override {(void)value; throw std::logic_error("Can't write 16bit to AddressModeAccumulator");}
};


// d - Direct
class AddressModeDirect : public AbsAddressMode
{
public:
    AddressModeDirect(Spc700 *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X: %s %02X", 1)
    {}

    void LoadAddress() override
    {
        data8 = cpu->ReadPC8Bit();
        address = Bytes::Make16Bit(cpu->reg.flags.p, data8);
    }

    uint16_t Read16Bit() override
    {
        return memory->Read16BitWrapPage(cpu->reg.flags.p, data8);
    }
    void Write16Bit(uint16_t value) override
    {
        memory->Write16BitWrapPage(cpu->reg.flags.p, data8, value);
    }
};


// d,x - Direct,X
class AddressModeDirectIndexedX : public AbsAddressMode
{
public:
    AddressModeDirectIndexedX(Spc700 *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X: %s %02X,X", 1)
    {}

    void LoadAddress() override
    {
        data8 = cpu->ReadPC8Bit();
        address = Bytes::Make16Bit(cpu->reg.flags.p, static_cast<uint8_t>(data8 + cpu->reg.x));
    }

    uint16_t Read16Bit() override
    {
        return memory->Read16BitWrapPage(cpu->reg.flags.p, data8 + cpu->reg.x);
    }
    void Write16Bit(uint16_t value) override
    {
        memory->Write16BitWrapPage(cpu->reg.flags.p, data8 + cpu->reg.x, value);
    }
};


// d,x - Direct,Y
class AddressModeDirectIndexedY : public AbsAddressMode
{
public:
    AddressModeDirectIndexedY(Spc700 *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X: %s %02X,X", 1)
    {}

    void LoadAddress() override
    {
        data8 = cpu->ReadPC8Bit();
        address = Bytes::Make16Bit(cpu->reg.flags.p, static_cast<uint8_t>(data8 + cpu->reg.y));
    }

    uint16_t Read16Bit() override
    {
        return memory->Read16BitWrapPage(cpu->reg.flags.p, data8 + cpu->reg.y);
    }
    void Write16Bit(uint16_t value) override
    {
        memory->Write16BitWrapPage(cpu->reg.flags.p, data8 + cpu->reg.y, value);
    }
};


// Immediate
class AddressModeImmediate : public AbsAddressMode
{
public:
    AddressModeImmediate(Spc700 *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X: %s #%02X", 1)
    {}

    // Does nothing.
    void LoadAddress() override
    {
        data8 = cpu->ReadPC8Bit();
    }

    uint8_t Read8Bit() override
    {
        return data8;
    }
    uint16_t Read16Bit() override {throw std::logic_error("Can't read 16bits in AddressModeImmediate");}
    void Write8Bit(uint8_t value) override {(void)value; throw std::logic_error("Can't write to AddressModeImmediate");}
    void Write16Bit(uint16_t value) override {(void)value; throw std::logic_error("Can't write to AddressModeImmediate");}
};


// (X) - Indirect
class AddressModeIndirectX : public AbsAddressMode
{
public:
    AddressModeIndirectX(Spc700 *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X: %s (X)", 0)
    {}

    void LoadAddress() override
    {
        address = Bytes::Make16Bit(cpu->reg.flags.p, cpu->reg.x);
    }
};


// (Y) - Indirect
class AddressModeIndirectY : public AbsAddressMode
{
public:
    AddressModeIndirectY(Spc700 *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X: %s (X)", 0)
    {}

    void LoadAddress() override
    {
        address = Bytes::Make16Bit(cpu->reg.flags.p, cpu->reg.y);
    }
};


// [d+X] - IndirectIndexedX
class AddressModeIndirectIndexedX : public AbsAddressMode
{
public:
    AddressModeIndirectIndexedX(Spc700 *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X: %s [%02X+X]", 1)
    {}

    void LoadAddress() override
    {
        data8 = cpu->ReadPC8Bit();
        uint16_t ptrAddr = Bytes::Make16Bit(cpu->reg.flags.p, data8 + cpu->reg.x);
        address = memory->Read16BitWrapPage(ptrAddr);
    }
};


// [d+X] - IndirectIndexedY
class AddressModeIndirectIndexedY : public AbsAddressMode
{
public:
    AddressModeIndirectIndexedY(Spc700 *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X: %s [%02X]+Y", 1)
    {}

    void LoadAddress() override
    {
        data8 = cpu->ReadPC8Bit();
        uint16_t ptrAddr = Bytes::Make16Bit(cpu->reg.flags.p, data8);
        address = memory->Read16BitWrapPage(ptrAddr) + cpu->reg.y;
    }
};


} // end namespace

#endif