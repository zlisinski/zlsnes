#pragma once

#include <exception>

#include "Address.h"
#include "Cpu.h"
#include "Memory.h"
#include "Timer.h"
#include "Zlsnes.h"


class AbsAddressMode
{
public:
    AbsAddressMode(Cpu *cpu, Memory *memory, const char *formatStr, int dataLen) :
        cpu(cpu),
        memory(memory),
        address(),
        formatStr(formatStr),
        dataLen(dataLen),
        data24(0)
    {}

    virtual ~AbsAddressMode() {}

    Address GetAddress() const {return address;}

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
                LogCpu(formatStr, cpu->opcode, name);
                break;
            case 1:
                LogCpu(formatStr, cpu->opcode, dataBytes[0], name, data8);
                break;
            case 2:
                LogCpu(formatStr, cpu->opcode, dataBytes[0], dataBytes[1], name, data16);
                break;
            case 3:
                LogCpu(formatStr, cpu->opcode, dataBytes[0], dataBytes[1], dataBytes[2], name, data24);
                break;
        }
    }

protected:
    Cpu *cpu;
    Memory *memory;
    Address address;
    const char *formatStr;
    int dataLen;
    union
    {
        uint8_t data8;
        uint16_t data16;
        uint32_t data24;
        uint8_t dataBytes[4];
    };

    friend class AddressModeTest;
};


// a - Absolute
class AddressModeAbsolute : public AbsAddressMode
{
public:
    AddressModeAbsolute(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X %02X: %s %04X", 2)
    {}

    virtual void LoadAddress() override
    {
        data16 = cpu->ReadPC16Bit();
        address = Address(cpu->reg.db, data16);
    }
};


// a,x - Absolute,X
class AddressModeAbsoluteIndexedX : public AbsAddressMode
{
public:
    AddressModeAbsoluteIndexedX(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X %02X: %s %04X,X", 2)
    {}

    virtual void LoadAddress() override
    {
        data16 = cpu->ReadPC16Bit();
        address = Address(cpu->reg.db, data16).AddOffset(cpu->reg.x);

        if ((((cpu->opcode & 0x1F) == 0x1E) && cpu->opcode != 0xBE) || cpu->opcode == 0x9D || // 0x[13579DE]E or 0x9D
            cpu->reg.flags.x == 0 ||
            Bytes::GetByte<1>(data16) != Bytes::GetByte<1>(address.GetOffset()))
        {
            // Add an extra cycle for write opcodes, when x is 0, or when a page boundary is crossed.
            cpu->GetTimer()->AddCycle(EClockSpeed::eClockInternal);
        }
    }
};


// a,y - Absolute,Y
class AddressModeAbsoluteIndexedY : public AbsAddressMode
{
public:
    AddressModeAbsoluteIndexedY(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X %02X: %s %04X,Y", 2)
    {}

    virtual void LoadAddress() override
    {
        data16 = cpu->ReadPC16Bit();
        address = Address(cpu->reg.db, data16).AddOffset(cpu->reg.y);

        if (cpu->opcode == 0x99 || cpu->reg.flags.x == 0 ||
            Bytes::GetByte<1>(data16) != Bytes::GetByte<1>(address.GetOffset()))
        {
            // Add an extra cycle for write opcodes, when x is 0, or when a page boundary is crossed.
            cpu->GetTimer()->AddCycle(EClockSpeed::eClockInternal);
        }
    }
};


// (a) - (Absolute)
class AddressModeAbsoluteIndirect : public AbsAddressMode
{
public:
    AddressModeAbsoluteIndirect(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X %02X: %s (%04X)", 2)
    {}

    virtual void LoadAddress() override
    {
        data16 = cpu->ReadPC16Bit();
        uint16_t addr = memory->Read16BitWrapBank(0, data16);
        address = Address(0, addr);
    }

    virtual uint8_t Read8Bit() override {throw std::logic_error("Can't read from AddressModeAbsoluteIndirect");}
    virtual uint16_t Read16Bit() override {throw std::logic_error("Can't read from AddressModeAbsoluteIndirect");}
    virtual void Write8Bit(uint8_t value) override {(void)value; throw std::logic_error("Can't write to AddressModeAbsoluteIndirect");}
    virtual void Write16Bit(uint16_t value) override {(void)value; throw std::logic_error("Can't write to AddressModeAbsoluteIndirect");}
};


// [a] - [Absolute]
class AddressModeAbsoluteIndirectLong : public AbsAddressMode
{
public:
    AddressModeAbsoluteIndirectLong(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X %02X: %s [%04X]", 2)
    {}

    virtual void LoadAddress() override
    {
        data16 = cpu->ReadPC16Bit();
        uint32_t addr = memory->Read24BitWrapBank(0, data16);
        address = Address(addr);
    }

    virtual uint8_t Read8Bit() override {throw std::logic_error("Can't read from AddressModeAbsoluteIndirect");}
    virtual uint16_t Read16Bit() override {throw std::logic_error("Can't read from AddressModeAbsoluteIndirect");}
    virtual void Write8Bit(uint8_t value) override {(void)value; throw std::logic_error("Can't write to AddressModeAbsoluteIndirect");}
    virtual void Write16Bit(uint16_t value) override {(void)value; throw std::logic_error("Can't write to AddressModeAbsoluteIndirect");}
};


// (a,x) - (Absolute,X)
class AddressModeAbsoluteIndexedIndirect : public AbsAddressMode
{
public:
    AddressModeAbsoluteIndexedIndirect(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X %02X: %s (%04X,X)", 2)
    {}

    virtual void LoadAddress() override
    {
        data16 = cpu->ReadPC16Bit();
        uint16_t addr = memory->Read16BitWrapBank(cpu->reg.pb, data16 + cpu->reg.x);
        address = Address(cpu->reg.pb, addr);
    }

    virtual uint8_t Read8Bit() override {throw std::logic_error("Can't read from AddressModeAbsoluteIndexedIndirect");}
    virtual uint16_t Read16Bit() override {throw std::logic_error("Can't read from AddressModeAbsoluteIndexedIndirect");}
    virtual void Write8Bit(uint8_t value) override {(void)value; throw std::logic_error("Can't write to AddressModeAbsoluteIndexedIndirect");}
    virtual void Write16Bit(uint16_t value) override {(void)value; throw std::logic_error("Can't write to AddressModeAbsoluteIndexedIndirect");}
};


// al - Long
class AddressModeAbsoluteLong : public AbsAddressMode
{
public:
    AddressModeAbsoluteLong(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X %02X %02X: %s %06X", 3)
    {}

    virtual void LoadAddress() override
    {
        data24 = cpu->ReadPC24Bit();
        address = Address(data24);
    }
};


// al,x - Long,X
class AddressModeAbsoluteLongIndexedX : public AbsAddressMode
{
public:
    AddressModeAbsoluteLongIndexedX(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X %02X %02X: %s %06X,X", 3)
    {}

    virtual void LoadAddress() override
    {
        data24 = cpu->ReadPC24Bit();
        address = Address(data24 + cpu->reg.x);
    }
};


// A - Accumulator
class AddressModeAccumulator : public AbsAddressMode
{
public:
    AddressModeAccumulator(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X: %s Acc", 0)
    {}

    virtual void LoadAddress() override
    {
        address = Address();
    }

    virtual uint8_t Read8Bit() override {return cpu->reg.al;}
    virtual uint16_t Read16Bit() override {return cpu->reg.a;}
    virtual void Write8Bit(uint8_t value) override {cpu->reg.al = value;}
    virtual void Write16Bit(uint16_t value) override {cpu->reg.a = value;}
};


// d - Direct
class AddressModeDirect : public AbsAddressMode
{
public:
    AddressModeDirect(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X: %s %02X", 1)
    {}

    virtual void LoadAddress() override
    {
        // TODO: Handle emulation mode special case.
        data8 = cpu->ReadPC8Bit();
        address = Address(0, data8 + cpu->reg.d);

        // Add a extra cycle if dl is not 0.
        if (cpu->reg.dl != 0)
            cpu->GetTimer()->AddCycle(EClockSpeed::eClockInternal);
    }

    virtual uint16_t Read16Bit() override {return memory->Read16BitWrapBank(address);}
    virtual void Write16Bit(uint16_t value) override {memory->Write16BitWrapBank(address, value);}
};


// d,x - Direct,X
class AddressModeDirectIndexedX : public AbsAddressMode
{
public:
    AddressModeDirectIndexedX(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X: %s %02X,X", 1)
    {}

    virtual void LoadAddress() override
    {
        data8 = cpu->ReadPC8Bit();
        if (cpu->reg.emulationMode && cpu->reg.dl == 0)
        {
            // Low byte wraps, only when dl is 0.
            address = Address(0, Bytes::Make16Bit(cpu->reg.dh, static_cast<uint8_t>(data8 + cpu->reg.x)));
        }
        else
        {
            address = Address(0, data8 + cpu->reg.d + cpu->reg.x);
        }

        // Add a extra cycle if dl is not 0.
        if (cpu->reg.dl != 0)
            cpu->GetTimer()->AddCycle(EClockSpeed::eClockInternal);
        cpu->GetTimer()->AddCycle(EClockSpeed::eClockInternal);
    }

    virtual uint16_t Read16Bit() override {return memory->Read16BitWrapBank(address);}
    virtual void Write16Bit(uint16_t value) override {memory->Write16BitWrapBank(address, value);}
};


// d,y - Direct,Y
class AddressModeDirectIndexedY : public AbsAddressMode
{
public:
    AddressModeDirectIndexedY(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X: %s %02X,Y", 1)
    {}

    virtual void LoadAddress() override
    {
        data8 = cpu->ReadPC8Bit();
        if (cpu->reg.emulationMode && cpu->reg.dl == 0)
        {
            // Low byte wraps, only when dl is 0.
            address = Address(0, Bytes::Make16Bit(cpu->reg.dh, static_cast<uint8_t>(data8 + cpu->reg.y)));
        }
        else
        {
            address = Address(0, data8 + cpu->reg.d + cpu->reg.y);
        }

        // Add a extra cycle if dl is not 0.
        if (cpu->reg.dl != 0)
            cpu->GetTimer()->AddCycle(EClockSpeed::eClockInternal);
        cpu->GetTimer()->AddCycle(EClockSpeed::eClockInternal);
    }

    virtual uint16_t Read16Bit() override {return memory->Read16BitWrapBank(address);}
    virtual void Write16Bit(uint16_t value) override {memory->Write16BitWrapBank(address, value);}
};


// (d) - (Direct)
class AddressModeDirectIndirect : public AbsAddressMode
{
public:
    AddressModeDirectIndirect(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X: %s (%02X)", 1)
    {}

    virtual void LoadAddress() override
    {
        // TODO: Handle emulation mode special case.
        data8 = cpu->ReadPC8Bit();

        // Add a extra cycle if dl is not 0.
        if (cpu->reg.dl != 0)
            cpu->GetTimer()->AddCycle(EClockSpeed::eClockInternal);

        uint16_t addr = memory->Read16BitWrapBank(0, data8 + cpu->reg.d);
        address = Address(cpu->reg.db, addr);
    }
};


// [d] - [Direct]
class AddressModeDirectIndirectLong : public AbsAddressMode
{
public:
    AddressModeDirectIndirectLong(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X: %s [%02X]", 1)
    {}

    virtual void LoadAddress() override
    {
        data8 = cpu->ReadPC8Bit();
        address = Address(memory->Read24BitWrapBank(0, data8 + cpu->reg.d));

        // Add a extra cycle if dl is not 0.
        if (cpu->reg.dl != 0)
            cpu->GetTimer()->AddCycle(EClockSpeed::eClockInternal);
    }
};


// (d,x) - (Direct,X)
class AddressModeDirectIndexedIndirect : public AbsAddressMode
{
public:
    AddressModeDirectIndexedIndirect(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X: %s (%02X,X)", 1)
    {}

    virtual void LoadAddress() override
    {
        uint16_t addr;
        data8 = cpu->ReadPC8Bit();

        // Add a extra cycle if dl is not 0.
        if (cpu->reg.dl != 0)
            cpu->GetTimer()->AddCycle(EClockSpeed::eClockInternal);
        cpu->GetTimer()->AddCycle(EClockSpeed::eClockInternal);

        if (cpu->reg.emulationMode && cpu->reg.dl == 0)
        {
            // Low byte wraps, only when dl is 0.
            addr = memory->Read16BitWrapBank(0, Bytes::Make16Bit(cpu->reg.dh, static_cast<uint8_t>(data8 + cpu->reg.x)));
        }
        else
        {
            addr = memory->Read16BitWrapBank(0, data8 + cpu->reg.d + cpu->reg.x);
        }

        address = Address(cpu->reg.db, addr);
    }
};


// (d),y - (Direct),Y
class AddressModeDirectIndirectIndexed : public AbsAddressMode
{
public:
    AddressModeDirectIndirectIndexed(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X: %s (%02X),Y", 1)
    {}

    virtual void LoadAddress() override
    {
        // TODO: Handle emulation mode special case.
        data8 = cpu->ReadPC8Bit();

        // Add a extra cycle if dl is not 0.
        if (cpu->reg.dl != 0)
            cpu->GetTimer()->AddCycle(EClockSpeed::eClockInternal);

        uint16_t addr = memory->Read16BitWrapBank(0, data8 + cpu->reg.d);
        address = Address(cpu->reg.db, addr).AddOffset(cpu->reg.y);

        if (cpu->opcode == 0x91 || cpu->reg.flags.x == 0 ||
            Bytes::GetByte<1>(addr) != Bytes::GetByte<1>(address.GetOffset()))
        {
            // Add an extra cycle for write opcodes, when x is 0, or when a page boundary is crossed.
            cpu->GetTimer()->AddCycle(EClockSpeed::eClockInternal);
        }
    }
};


// [d],y - [Direct],Y
class AddressModeDirectIndirectLongIndexed : public AbsAddressMode
{
public:
    AddressModeDirectIndirectLongIndexed(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X: %s [%02X],Y", 1)
    {}

    virtual void LoadAddress() override
    {
        data8 = cpu->ReadPC8Bit();

        // Add a extra cycle if dl is not 0.
        if (cpu->reg.dl != 0)
            cpu->GetTimer()->AddCycle(EClockSpeed::eClockInternal);

        uint32_t addr = memory->Read24BitWrapBank(0, data8 + cpu->reg.d);
        address = Address(addr).AddOffset(cpu->reg.y);
    }
};


// Immediate
class AddressModeImmediate : public AbsAddressMode
{
public:
    AddressModeImmediate(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X %02X: %s #%04X", 2)
    {}

    // Does nothing.
    virtual void LoadAddress() override
    {
        // Immediate mode reads different amount of bytes depending on accumulator/index size.
        if ((((cpu->opcode & 0x1F) == 0x09) && cpu->reg.flags.m == 0) || // A, opcode == 0x[02468ACE]9
            ((((cpu->opcode & 0x9F) == 0x80) || (cpu->opcode == 0xA2)) && cpu->reg.flags.x == 0)) // X/Y, opcode == 0xA2 or 0x[ACE]0
        {
            dataLen = 2;
            formatStr = "%02X %02X %02X: %s #%04X";
            data16 = cpu->ReadPC16Bit();
        }
        else
        {
            dataLen = 1;
            formatStr = "%02X %02X: %s #%02X";
            data8 = cpu->ReadPC8Bit();
        }
    }

    virtual uint8_t Read8Bit() override
    {
        return data8;
    }
    virtual uint16_t Read16Bit() override
    {
        return data16;
    }
    virtual void Write8Bit(uint8_t value) override {(void)value; throw std::logic_error("Can't write to AddressModeImmediate");}
    virtual void Write16Bit(uint16_t value) override {(void)value; throw std::logic_error("Can't write to AddressModeImmediate");}
};


// d,s - Stack,S
class AddressModeStackRelative : public AbsAddressMode
{
public:
    AddressModeStackRelative(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X: %s %02X,S", 1)
    {}

    virtual void LoadAddress() override
    {
        data8 = cpu->ReadPC8Bit();
        address = Address(0, data8 + cpu->reg.sp);

        cpu->GetTimer()->AddCycle(EClockSpeed::eClockInternal);
    }

    virtual uint16_t Read16Bit() override {return memory->Read16BitWrapBank(address);}
    virtual void Write16Bit(uint16_t value) override {memory->Write16BitWrapBank(address, value);}
};


// (d,s),y - (Stack,S),Y
class AddressModeStackRelativeIndirectIndexed : public AbsAddressMode
{
public:
    AddressModeStackRelativeIndirectIndexed(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X %02X: %s (%02X,S),Y", 1)
    {}

    virtual void LoadAddress() override
    {
        data8 = cpu->ReadPC8Bit();

        cpu->GetTimer()->AddCycle(EClockSpeed::eClockInternal);

        uint32_t addr = memory->Read16BitWrapBank(0, data8 + cpu->reg.sp);
        address = Address(cpu->reg.db, addr).AddOffset(cpu->reg.y);

        cpu->GetTimer()->AddCycle(EClockSpeed::eClockInternal);
    }
};


// ProgramCounterRelative // r - Relative8
// ProgramCounterRelativeLong // rl - Relative16
// Stack // s