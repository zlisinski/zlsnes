#pragma once

#include <exception>

#include "Address.h"
#include "Cpu.h"
#include "Memory.h"
#include "Zlsnes.h"


class AbsAddressMode
{
public:
    AbsAddressMode(Cpu *cpu, Memory *memory, const char *formatStr, int bytes) :
        cpu(cpu),
        memory(memory),
        address(),
        formatStr(formatStr),
        bytes(bytes)
    {}

    virtual ~AbsAddressMode() {}

    Address GetAddress() const {return address;}
    const char *GetFormatStr() const {return formatStr;}

    // Reads from cpu/memory and advances program counter.
    virtual void LoadAddress() = 0;

    virtual uint8_t Read8Bit() {return memory->Read8Bit(address);}
    virtual uint16_t Read16Bit() {return memory->Read16Bit(address);}

    virtual void Write8Bit(uint8_t value) {memory->Write8Bit(address, value);}
    virtual void Write16Bit(uint16_t value) {memory->Write16Bit(address, value);}

protected:
    Cpu *cpu;
    Memory *memory;
    Address address;
    const char *formatStr;
    int bytes;
};


// a - Absolute
class AddressModeAbsolute : public AbsAddressMode
{
public:
    AddressModeAbsolute(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%04X", 3)
    {}

    virtual void LoadAddress() override
    {
        address = Address(cpu->reg.db, cpu->ReadPC16Bit());
    }
};


// a,x - Absolute,X
class AddressModeAbsoluteIndexedX : public AbsAddressMode
{
public:
    AddressModeAbsoluteIndexedX(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%04X,X", 3)
    {}

    virtual void LoadAddress() override
    {
        address =  Address(cpu->reg.db, cpu->ReadPC16Bit()).AddOffset(cpu->reg.x);
    }
};


// a,y - Absolute,Y
class AddressModeAbsoluteIndexedY : public AbsAddressMode
{
public:
    AddressModeAbsoluteIndexedY(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%04X,Y", 3)
    {}

    virtual void LoadAddress() override
    {
        address = Address(cpu->reg.db, cpu->ReadPC16Bit()).AddOffset(cpu->reg.y);
    }
};


// (a) - (Absolute)
class AddressModeAbsoluteIndirect : public AbsAddressMode
{
public:
    AddressModeAbsoluteIndirect(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "(%04X)", 3)
    {}

    virtual void LoadAddress() override
    {
        uint16_t addr = memory->Read16BitWrapBank(0, cpu->ReadPC16Bit());
        address = Address(cpu->reg.pb, addr);
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
        AbsAddressMode(cpu, memory, "(%04X,X)", 3)
    {}

    virtual void LoadAddress() override
    {
        uint16_t addr = memory->Read16BitWrapBank(cpu->reg.pb, cpu->ReadPC16Bit() + cpu->reg.x);
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
        AbsAddressMode(cpu, memory, "%06X", 4)
    {}

    virtual void LoadAddress() override
    {
        address = Address(cpu->ReadPC24Bit());
    }
};


// al,x - Long,X
class AddressModeAbsoluteLongIndexedX : public AbsAddressMode
{
public:
    AddressModeAbsoluteLongIndexedX(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%06X,X", 4)
    {}

    virtual void LoadAddress() override
    {
        address = Address(cpu->ReadPC24Bit() + cpu->reg.x);
    }
};


// d - Direct
class AddressModeDirect : public AbsAddressMode
{
public:
    AddressModeDirect(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X", 2)
    {}

    virtual void LoadAddress() override
    {
        // TODO: Handle emulation mode special case.
        address = Address(0, cpu->ReadPC8Bit() + cpu->reg.d);
    }

    virtual uint16_t Read16Bit() override {return memory->Read16BitWrapBank(address);}
    virtual void Write16Bit(uint16_t value) override {memory->Write16BitWrapBank(address, value);}
};


// d,x - Direct,X
class AddressModeDirectIndexedX : public AbsAddressMode
{
public:
    AddressModeDirectIndexedX(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X,X", 2)
    {}

    virtual void LoadAddress() override
    {
        // TODO: Handle emulation mode special case.
        address = Address(0, cpu->ReadPC8Bit() + cpu->reg.d + cpu->reg.x);
    }

    virtual uint16_t Read16Bit() override {return memory->Read16BitWrapBank(address);}
    virtual void Write16Bit(uint16_t value) override {memory->Write16BitWrapBank(address, value);}
};


// d,y - Direct,Y
class AddressModeDirectIndexedY : public AbsAddressMode
{
public:
    AddressModeDirectIndexedY(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X,X", 2)
    {}

    virtual void LoadAddress() override
    {
        // TODO: Handle emulation mode special case.
        address = Address(0, cpu->ReadPC8Bit() + cpu->reg.d + cpu->reg.y);
    }

    virtual uint16_t Read16Bit() override {return memory->Read16BitWrapBank(address);}
    virtual void Write16Bit(uint16_t value) override {memory->Write16BitWrapBank(address, value);}
};


// (d) - (Direct)
class AddressModeDirectIndirect : public AbsAddressMode
{
public:
    AddressModeDirectIndirect(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "(%02X)", 2)
    {}

    virtual void LoadAddress() override
    {
        // TODO: Handle emulation mode special case.
        uint16_t addr = memory->Read16BitWrapBank(0, cpu->ReadPC8Bit() + cpu->reg.d);
        address = Address(cpu->reg.db, addr);
    }
};


// [d] - [Direct]
class AddressModeDirectIndirectLong : public AbsAddressMode
{
public:
    AddressModeDirectIndirectLong(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "[%02X]", 2)
    {}

    virtual void LoadAddress() override
    {
        address = Address(memory->Read24BitWrapBank(0, cpu->ReadPC8Bit() + cpu->reg.d));
    }
};


// (d,x) - (Direct,X)
class AddressModeDirectIndexedIndirect : public AbsAddressMode
{
public:
    AddressModeDirectIndexedIndirect(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "(%02X,X)", 2)
    {}

    virtual void LoadAddress() override
    {
        // TODO: Handle emulation mode special case.
        uint16_t addr = memory->Read16BitWrapBank(0, cpu->ReadPC8Bit() + cpu->reg.d + cpu->reg.x);
        address = Address(cpu->reg.db, addr);
    }
};


// (d),y - (Direct),Y
class AddressModeDirectIndirectIndexed : public AbsAddressMode
{
public:
    AddressModeDirectIndirectIndexed(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "(%02X),Y", 2)
    {}

    virtual void LoadAddress() override
    {
        // TODO: Handle emulation mode special case.
        uint16_t addr = memory->Read16BitWrapBank(0, cpu->ReadPC8Bit() + cpu->reg.d);
        address = Address(cpu->reg.db, addr).AddOffset(cpu->reg.y);
    }
};


// [d],y - [Direct],Y
class AddressModeDirectIndirectLongIndexed : public AbsAddressMode
{
public:
    AddressModeDirectIndirectLongIndexed(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "[%02X],Y", 2)
    {}

    virtual void LoadAddress() override
    {
        uint32_t addr = memory->Read24BitWrapBank(0, cpu->ReadPC8Bit() + cpu->reg.d);
        address = Address(cpu->reg.db, addr).AddOffset(cpu->reg.y);
    }
};


// Immediate
class AddressModeImmediate : public AbsAddressMode
{
public:
    AddressModeImmediate(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "(%04X,X)", 3)
    {}

    // Does nothing.
    virtual void LoadAddress() override {}

    virtual uint8_t Read8Bit() override {return cpu->ReadPC8Bit();}
    virtual uint16_t Read16Bit() override {return cpu->ReadPC16Bit();}
    virtual void Write8Bit(uint8_t value) override {(void)value; throw std::logic_error("Can't write to AddressModeImmediate");}
    virtual void Write16Bit(uint16_t value) override {(void)value; throw std::logic_error("Can't write to AddressModeImmediate");}
};

// d,s - Stack,S
class AddressModeStackRelative : public AbsAddressMode
{
public:
    AddressModeStackRelative(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "%02X,S", 2)
    {}

    virtual void LoadAddress() override
    {
        address = Address(0, cpu->ReadPC8Bit() + cpu->reg.sp);
    }

    virtual uint16_t Read16Bit() override {return memory->Read16BitWrapBank(address);}
    virtual void Write16Bit(uint16_t value) override {memory->Write16BitWrapBank(address, value);}
};


// (d,s),y - (Stack,S),Y
class AddressModeStackRelativeIndirectIndexed : public AbsAddressMode
{
public:
    AddressModeStackRelativeIndirectIndexed(Cpu *cpu, Memory *memory) :
        AbsAddressMode(cpu, memory, "(%02X,S),Y", 2)
    {}

    virtual void LoadAddress() override
    {
        uint32_t addr = memory->Read16BitWrapBank(0, cpu->ReadPC8Bit() + cpu->reg.sp);
        address = Address(cpu->reg.db, addr).AddOffset(cpu->reg.y);
    }
};


// ProgramCounterRelative // r - Relative8
// ProgramCounterRelativeLong // rl - Relative16
// Stack // s