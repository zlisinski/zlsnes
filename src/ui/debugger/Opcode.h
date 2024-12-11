#pragma once

#include <QtCore/QString>

#include "core/Zlsnes.h"
#include "core/Address.h"

struct Registers;

class Opcode
{
public:
    static Opcode GetOpcode(Address pc, const uint8_t *memory, const Registers *reg);

    QString ToString() const;
    QString GetBytesStr() const {return bytesStr;}
    QString GetOpcodeStr() const {return opcodeStr;}
    QString GetOperandStr() const {return operandStr;}
    QString GetAddrModeStr() const {return addrModeStr;}
    Address GetAddress() const {return address;}
    int GetByteCount() const {return byteCount;}

private:
    enum class EAddrModes
    {
        Absolute,
        AbsoluteIndexedX,
        AbsoluteIndexedY,
        AbsoluteIndirect,
        AbsoluteIndirectLong,
        AbsoluteIndexedIndirect,
        AbsoluteLong,
        AbsoluteLongIndexedX,
        Accumulator,
        Direct,
        DirectIndexedX,
        DirectIndexedY,
        DirectIndirect,
        DirectIndirectLong,
        DirectIndexedIndirect,
        DirectIndirectIndexed,
        DirectIndirectLongIndexed,
        Immediate,
        StackRelative,
        StackRelativeIndirectIndexed,
        Relative8,
        Relative16,
        None
    };

    Opcode(int byteCount, const QString &opcodeStr, EAddrModes addrMode);

    int byteCount;
    QString opcodeStr;
    EAddrModes addrMode;
    QString bytesStr;
    QString operandStr;
    QString addrModeStr;
    Address address;
    
    static const Opcode opcodes[256];
};
