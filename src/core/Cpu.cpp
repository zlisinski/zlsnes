#include <sstream>

#include "AddressMode.h"
#include "Cpu.h"
#include "Dma.h"
#include "Interrupt.h"
#include "Memory.h"
#include "Timer.h"

// Use this for opcodes that don't have an AddressMode or data.
#define LogInst(name) do {LogCpu("%02X: %s", opcode, (name)); PrintState();} while (0)

// Opcodes with data but no AddressMode
#define LogInst1(name, param) do {LogCpu("%02X %02X: %s", opcode, (param), (name)); PrintState();} while (0)
#define LogInst2(name, param1, param2) do {LogCpu("%02X %02X %02X: %s", opcode, (param1), (param2), (name)); PrintState();} while (0)

// Opcodes with an AddressMode
#define LogInstM(name, addrmode) do {(addrmode)->Log(name); PrintState();} while (0)


Cpu::Cpu(Memory *memory, Timer *timer, Interrupt *interrupts) :
    reg(),
    opcode(0),
    memory(memory),
    timer(timer),
    interrupts(interrupts),
    dma(memory, timer),
    waiting(false)
{
    addressModes[0x00] = std::make_unique<AddressModeImmediate>(this, memory);
    addressModes[0x01] = std::make_unique<AddressModeDirectIndexedIndirect>(this, memory);
    addressModes[0x02] = std::make_unique<AddressModeImmediate>(this, memory);
    addressModes[0x03] = std::make_unique<AddressModeStackRelative>(this, memory);
    addressModes[0x04] = std::make_unique<AddressModeDirect>(this, memory);
    addressModes[0x05] = std::make_unique<AddressModeDirect>(this, memory);
    addressModes[0x06] = std::make_unique<AddressModeDirect>(this, memory);
    addressModes[0x07] = std::make_unique<AddressModeDirectIndirectLong>(this, memory);
    // No 0x08
    addressModes[0x09] = std::make_unique<AddressModeImmediate>(this, memory);
    addressModes[0x0A] = std::make_unique<AddressModeAccumulator>(this, memory);
    // No 0x0B
    addressModes[0x0C] = std::make_unique<AddressModeAbsolute>(this, memory);
    addressModes[0x0D] = std::make_unique<AddressModeAbsolute>(this, memory);
    addressModes[0x0E] = std::make_unique<AddressModeAbsolute>(this, memory);
    addressModes[0x0F] = std::make_unique<AddressModeAbsoluteLong>(this, memory);
    // No 0x10
    addressModes[0x11] = std::make_unique<AddressModeDirectIndirectIndexed>(this, memory);
    addressModes[0x12] = std::make_unique<AddressModeDirectIndirect>(this, memory);
    addressModes[0x13] = std::make_unique<AddressModeStackRelativeIndirectIndexed>(this, memory);
    addressModes[0x14] = std::make_unique<AddressModeDirectIndexedX>(this, memory);
    addressModes[0x15] = std::make_unique<AddressModeDirectIndexedX>(this, memory);
    addressModes[0x16] = std::make_unique<AddressModeDirectIndexedX>(this, memory);
    addressModes[0x17] = std::make_unique<AddressModeDirectIndirectLongIndexed>(this, memory);
    // No 0x18
    addressModes[0x19] = std::make_unique<AddressModeAbsoluteIndexedY>(this, memory);
    addressModes[0x1A] = std::make_unique<AddressModeAccumulator>(this, memory);
    // No 0x1B
    addressModes[0x1C] = std::make_unique<AddressModeAbsoluteIndexedX>(this, memory);
    addressModes[0x1D] = std::make_unique<AddressModeAbsoluteIndexedX>(this, memory);
    addressModes[0x1E] = std::make_unique<AddressModeAbsoluteIndexedX>(this, memory);
    addressModes[0x1F] = std::make_unique<AddressModeAbsoluteLongIndexedX>(this, memory);

    // Special cases for certain opcodes of LDX, STX, STZ
    addressModeAlternate[0x16] = std::make_unique<AddressModeDirectIndexedY>(this, memory); // LDX, STX
    addressModeAlternate[0x1C] = std::make_unique<AddressModeAbsolute>(this, memory); // STZ
    addressModeAlternate[0x1E] = std::make_unique<AddressModeAbsoluteIndexedY>(this, memory); // LDX

    jmpAddressModes[0x02] = std::make_unique<AddressModeAbsolute>(this, memory); // JSR 20
    jmpAddressModes[0x04] = std::make_unique<AddressModeAbsolute>(this, memory); // JMP 4C
    jmpAddressModes[0x05] = std::make_unique<AddressModeAbsoluteLong>(this, memory); // JMP 5C
    jmpAddressModes[0x06] = std::make_unique<AddressModeAbsoluteIndirect>(this, memory); // JMP 6C
    jmpAddressModes[0x07] = std::make_unique<AddressModeAbsoluteIndexedIndirect>(this, memory); // JMP 7C
    jmpAddressModes[0x0D] = std::make_unique<AddressModeAbsoluteIndirectLong>(this, memory); // JMP DC
    jmpAddressModes[0x0F] = std::make_unique<AddressModeAbsoluteIndexedIndirect>(this, memory); // JSR FC
}

Cpu::~Cpu()
{

}

uint8_t Cpu::ReadPC8Bit()
{
    uint8_t byte = memory->Read8Bit(Bytes::Make24Bit(reg.pb, reg.pc));
    reg.pc++;

    return byte;
}


uint16_t Cpu::ReadPC16Bit()
{
    uint8_t low = memory->Read8Bit(Bytes::Make24Bit(reg.pb, reg.pc));
    reg.pc++;

    uint8_t high = memory->Read8Bit(Bytes::Make24Bit(reg.pb, reg.pc));
    reg.pc++;

    uint16_t word = (high << 8) | low;

    return word;
}


uint32_t Cpu::ReadPC24Bit()
{
    uint8_t low = memory->Read8Bit(Bytes::Make24Bit(reg.pb, reg.pc));
    reg.pc++;

    uint8_t mid = memory->Read8Bit(Bytes::Make24Bit(reg.pb, reg.pc));
    reg.pc++;

    uint8_t high = memory->Read8Bit(Bytes::Make24Bit(reg.pb, reg.pc));
    reg.pc++;

    uint32_t word = (high << 16) | (mid << 8) | low;

    return word;
}


void Cpu::SetEmulationMode(bool value)
{
    reg.emulationMode = value;

    UpdateRegistersAfterFlagChange();
}


void Cpu::UpdateRegistersAfterFlagChange()
{
    if (reg.emulationMode)
    {
        reg.flags.m = 1;
        reg.flags.x = 1;
        reg.sh = 1;
    }

    if (IsIndex8Bit())
    {
        reg.xh = 0;
        reg.yh = 0;
    }
}


void Cpu::ProcessInterrupt()
{
    if (reg.emulationMode)
    {
        const uint32_t vectors[] = {0xFFFE, 0xFFFA}; // IRQ, NMI
        Push16Bit(reg.pc);
        Push8Bit(reg.p);
        reg.pb = 0;
        reg.pc = memory->Read16Bit(vectors[interrupts->IsNmi()]);
        reg.flags.i = 1;
        reg.flags.d = 0;
    }
    else
    {
        const uint32_t vectors[] = {0xFFEE, 0xFFEA}; // IRQ, NMI
        Push8Bit(reg.pb);
        Push16Bit(reg.pc);
        Push8Bit(reg.p);
        reg.pb = 0;
        reg.pc = memory->Read16Bit(vectors[interrupts->IsNmi()]);
        reg.flags.i = 1;
        reg.flags.d = 0;
    }

    LogCpu("Jump to %s interrupt vector %06X", interrupts->IsNmi() ? "NMI" : "IRQ", GetFullPC().ToUint());

    // Irqs must be manually cleared by reading from regTIMEUP.
    if (interrupts->IsNmi())
        interrupts->ClearNmi();
}


void Cpu::Reset()
{
    reg = Registers();
    // Start at the reset vector.
    reg.pc = memory->Read16Bit(0xFFFC);
}


void Cpu::ProcessOpCode()
{
    if (interrupts->CheckInterrupts())
    {
        // If interrupts are disabled in wait state, just leave wait and continue execution without processing interrupt.
        if (waiting && reg.flags.i)
        {
            waiting = false;
            timer->AddCycle(eClockInternal);
            return;
        }
        else if (interrupts->IsNmi() || (!reg.flags.i && interrupts->IsIrq()))
        {
            ProcessInterrupt();
            return;
        }
    }
    else if (waiting)
    {
        LogCpu("Waiting");
        timer->AddCycle(eClockInternal);
        return;
    }

    opcode = ReadPC8Bit();

    switch (opcode)
    {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Register to register transfer opcodes                                                                             //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        case 0xAA: // TAX - Transfer A to X.
            {
                LogInst("TAX");
                if (IsIndex16Bit())
                    LoadRegister(&reg.x, reg.a);
                else
                    LoadRegister(&reg.xl, reg.al);

                timer->AddCycle(eClockInternal);
            }
            break;

        case 0xA8: // TAY - Transfer A to Y.
            {
                LogInst("TAY");
                if (IsIndex16Bit())
                    LoadRegister(&reg.y, reg.a);
                else
                    LoadRegister(&reg.yl, reg.al);

                timer->AddCycle(eClockInternal);
            }
            break;

        case 0xBA: // TSX - Transfer SP to X.
            {
                LogInst("TSX");
                if (IsIndex16Bit())
                    LoadRegister(&reg.x, reg.sp);
                else
                    LoadRegister(&reg.xl, reg.sl);

                timer->AddCycle(eClockInternal);
            }
            break;

        case 0x8A: // TXA - Transfer X to A.
            {
                LogInst("TXA");
                if (IsAccumulator16Bit())
                    LoadRegister(&reg.a, reg.x);
                else
                    LoadRegister(&reg.al, reg.xl);

                timer->AddCycle(eClockInternal);
            }
            break;

        case 0x9A: // TXS - Transfer X to SP.
            {
                LogInst("TXS");
                // No flags are set. High byte of sp is always 0x01 in emulation mode.
                if (reg.emulationMode)
                    reg.sp = 0x0100 | reg.xl;
                else
                    reg.sp = reg.x;

                timer->AddCycle(eClockInternal);
            }
            break;

        case 0x9B: // TXY - Transfer X to Y.
            {
                LogInst("TXY");
                if (IsIndex16Bit())
                    LoadRegister(&reg.y, reg.x);
                else
                    LoadRegister(&reg.yl, reg.xl);

                timer->AddCycle(eClockInternal);
            }
            break;

        case 0x98: // TYA - Transfer Y to A.
            {
                LogInst("TYA");
                if (IsAccumulator16Bit())
                    LoadRegister(&reg.a, reg.y);
                else
                    LoadRegister(&reg.al, reg.yl);

                timer->AddCycle(eClockInternal);
            }
            break;

        case 0xBB: // TYX - Transfer Y to X.
            {
                LogInst("TYX");
                if (IsIndex16Bit())
                    LoadRegister(&reg.x, reg.y);
                else
                    LoadRegister(&reg.xl, reg.yl);

                timer->AddCycle(eClockInternal);
            }
            break;

        case 0x5B: // TCD/TAD - Transfer A to D.
            {
                LogInst("TCD");
                LoadRegister(&reg.d, reg.a);

                timer->AddCycle(eClockInternal);
            }
            break;

        case 0x1B: // TCS/TAS - Transfer A to SP.
            {
                LogInst("TCS");
                // No flags are set. High byte of sp is always 0x01 in emulation mode.
                if (reg.emulationMode)
                    reg.sp = 0x0100 | reg.al;
                else
                    reg.sp = reg.a;

                timer->AddCycle(eClockInternal);
            }
            break;

        case 0x7B: // TDC/TDA - Transfer D to A.
            {
                LogInst("TDC");
                LoadRegister(&reg.a, reg.d);

                timer->AddCycle(eClockInternal);
            }
            break;

        case 0x3B: // TSC/TSA - Transfer SP to A.
            {
                LogInst("TSC");
                LoadRegister(&reg.a, reg.sp);

                timer->AddCycle(eClockInternal);
            }
            break;

        case 0xEB: // XBA - Swap al and ah
            {
                LogInst("XBA");
                std::swap(reg.ah, reg.al);
                SetNFlag(reg.al);
                SetZFlag(reg.al);

                timer->AddCycle(eClockInternal);
                timer->AddCycle(eClockInternal);
            }
            break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Load opcodes                                                                                                      //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // LDA - Load A
        case 0xA1: // LDA (Direct,X)
        case 0xA3: // LDA Stack,S
        case 0xA5: // LDA Direct
        case 0xA7: // LDA [Direct]
        case 0xA9: // LDA Immediate
        case 0xAD: // LDA Absolute
        case 0xAF: // LDA Long
        case 0xB1: // LDA (Direct),Y
        case 0xB2: // LDA (Direct)
        case 0xB3: // LDA (Stack,S),Y
        case 0xB5: // LDA Direct,X
        case 0xB7: // LDA [Direct],Y
        case 0xB9: // LDA Absolute,Y
        case 0xBD: // LDA Absolute,X
        case 0xBF: // LDA Long,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                LogInstM("LDA", mode);

                if (IsAccumulator16Bit())
                    LoadRegister(&reg.a, mode->Read16Bit());
                else
                    LoadRegister(&reg.al, mode->Read8Bit());
            }
            break;

        // LDX - Load X
        case 0xA2: // LDX Immediate
        case 0xA6: // LDX Direct
        case 0xAE: // LDX Absolute
        case 0xB6: // LDX Direct,Y
        case 0xBE: // LDX Absolute,Y
            {
                // Opcodes 0xB6 and 0xBE are special cases.
                AddressModePtr &mode = (opcode & 0x10 ? addressModeAlternate[opcode & 0x1F] : addressModes[opcode & 0x1F]);
                mode->LoadAddress();
                LogInstM("LDX", mode);

                if (IsIndex16Bit())
                    LoadRegister(&reg.x, mode->Read16Bit());
                else
                    LoadRegister(&reg.xl, mode->Read8Bit());
            }
            break;

        // LDY - Load Y
        case 0xA0: // LDY Immediate
        case 0xA4: // LDY Direct
        case 0xAC: // LDY Absolute
        case 0xB4: // LDY Direct,X
        case 0xBC: // LDY Absolute,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                LogInstM("LDY", mode);

                if (IsIndex16Bit())
                    LoadRegister(&reg.y, mode->Read16Bit());
                else
                    LoadRegister(&reg.yl, mode->Read8Bit());
            }
            break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Store opcodes                                                                                                     //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // STA - Store A
        case 0x81: // STA (Direct,X)
        case 0x83: // STA Stack,S
        case 0x85: // STA Direct
        case 0x87: // STA [Direct]
        case 0x8D: // STA Absolute
        case 0x8F: // STA Long
        case 0x91: // STA (Direct),Y
        case 0x92: // STA (Direct)
        case 0x93: // STA (Stack,S),Y
        case 0x95: // STA Direct,X
        case 0x97: // STA [Direct],Y
        case 0x99: // STA Absolute,Y
        case 0x9D: // STA Absolute,X
        case 0x9F: // STA Long,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                LogInstM("STA", mode);

                if (IsAccumulator16Bit())
                    mode->Write16Bit(reg.a);
                else
                    mode->Write8Bit(reg.al);
            }
            break;

        // STX - Store X
        case 0x86: // STX Direct
        case 0x8E: // STX Absolute
        case 0x96: // STX Direct,Y
            {
                // Opcodes 0x96 is a special case.
                AddressModePtr &mode = (opcode == 0x96 ? addressModeAlternate[opcode & 0x1F] : addressModes[opcode & 0x1F]);
                mode->LoadAddress();
                LogInstM("STX", mode);

                if (IsIndex16Bit())
                    mode->Write16Bit(reg.x);
                else
                    mode->Write8Bit(reg.xl);
            }
            break;

        // STY - Store Y
        case 0x84: // STY Direct
        case 0x8C: // STY Absolute
        case 0x94: // STY Direct,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                LogInstM("STY", mode);

                if (IsIndex16Bit())
                    mode->Write16Bit(reg.y);
                else
                    mode->Write8Bit(reg.yl);
            }
            break;

        // STZ - Store Zero
        case 0x64: // STZ Direct
        case 0x74: // STZ Direct,X
        case 0x9C: // STZ Absolute
        case 0x9E: // STZ Absolute,X
            {
                // Opcodes 0x9C is a special case.
                AddressModePtr &mode = (opcode == 0x9C ? addressModeAlternate[opcode & 0x1F] : addressModes[opcode & 0x1F]);
                mode->LoadAddress();
                LogInstM("STZ", mode);

                if (IsAccumulator16Bit())
                    mode->Write16Bit(0);
                else
                    mode->Write8Bit(0);
            }
            break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Stack opcodes                                                                                                     //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        case 0x48: // PHA - Push A
            {
                LogInst("PHA");
                timer->AddCycle(eClockInternal);

                if (IsAccumulator16Bit())
                    Push16Bit(reg.a);
                else
                    Push8Bit(reg.al);
            }
            break;

        case 0xDA: // PHX - Push X
            {
                LogInst("PHX");
                timer->AddCycle(eClockInternal);

                if (IsIndex16Bit())
                    Push16Bit(reg.x);
                else
                    Push8Bit(reg.xl);
            }
            break;

        case 0x5A: // PHY - Push Y
            {
                LogInst("PHY");
                timer->AddCycle(eClockInternal);

                if (IsIndex16Bit())
                    Push16Bit(reg.y);
                else
                    Push8Bit(reg.yl);
            }
            break;

        case 0x8B: // PHB - Push DB
            {
                LogInst("PHB");
                timer->AddCycle(eClockInternal);

                Push8Bit(reg.db);
            }
            break;

        case 0x0B: // PHD - Push D
            {
                LogInst("PHD");
                timer->AddCycle(eClockInternal);

                Push16Bit(reg.d);
            }
            break;

        case 0x4B: // PHK - Push PB
            {
                LogInst("PHK");
                timer->AddCycle(eClockInternal);

                Push8Bit(reg.pb);
            }
            break;

        case 0x08: // PHP - Push P
            {
                LogInst("PHP");
                timer->AddCycle(eClockInternal);

                Push8Bit(reg.p);
            }
            break;

        case 0xF4: // PEA - Push Effective Address
            {
                uint16_t value = ReadPC16Bit();
                LogInst2("PEA", Bytes::GetByte<1>(value), Bytes::GetByte<0>(value));
                Push16Bit(value);
            }
            break;

        case 0xD4: // PEI - Push Effective Indirect Address
            {
                AddressModeDirect mode(this, memory);
                mode.LoadAddress();
                LogInstM("PEI", &mode);
                Push16Bit(mode.Read16Bit());
            }
            break;

        case 0x62: // PER - Push Effective Relative Address
            {
                int16_t value = static_cast<int16_t>(ReadPC16Bit());
                LogInst2("PER", Bytes::GetByte<1>(value), Bytes::GetByte<0>(value));

                timer->AddCycle(eClockInternal);

                Push16Bit(reg.pc + value);
            }
            break;

        case 0x68: // PLA - Pull/Pop A
            {
                LogInst("PLA");
                timer->AddCycle(2 * eClockInternal);

                if (IsAccumulator16Bit())
                {
                    reg.a = Pop16Bit();
                    SetNFlag(reg.a);
                    SetZFlag(reg.a);
                }
                else
                {
                    reg.al = Pop8Bit();
                    SetNFlag(reg.al);
                    SetZFlag(reg.al);
                }
            }
            break;

        case 0xFA: // PLX - Pull/Pop X
            {
                LogInst("PLX");
                timer->AddCycle(2 * eClockInternal);

                if (IsIndex16Bit())
                {
                    reg.x = Pop16Bit();
                    SetNFlag(reg.x);
                    SetZFlag(reg.x);
                }
                else
                {
                    reg.xl = Pop8Bit();
                    SetNFlag(reg.xl);
                    SetZFlag(reg.xl);
                }
            }
            break;

        case 0x7A: // PLY - Pull/Pop Y
            {
                LogInst("PLY");
                timer->AddCycle(2 * eClockInternal);

                if (IsIndex16Bit())
                {
                    reg.y = Pop16Bit();
                    SetNFlag(reg.y);
                    SetZFlag(reg.y);
                }
                else
                {
                    reg.yl = Pop8Bit();
                    SetNFlag(reg.yl);
                    SetZFlag(reg.yl);
                }
            }
            break;

        case 0xAB: // PLB - Pull/Pop DB
            {
                LogInst("PLB");
                timer->AddCycle(2 * eClockInternal);

                reg.db = Pop8Bit();
                SetNFlag(reg.db);
                SetZFlag(reg.db);
            }
            break;

        case 0x2B: // PLD - Pull/Pop D
            {
                LogInst("PLD");
                timer->AddCycle(2 * eClockInternal);

                reg.d = Pop16Bit();
                SetNFlag(reg.d);
                SetZFlag(reg.d);
            }
            break;

        case 0x28: // PLP - Pull/Pop P
            {
                LogInst("PLP");
                timer->AddCycle(2 * eClockInternal);

                reg.p = Pop8Bit();
                UpdateRegistersAfterFlagChange();
            }
            break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Logical opcodes                                                                                                   //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // AND - Bitwise AND
        case 0x21: // AND (Direct,X)
        case 0x23: // AND Stack,S
        case 0x25: // AND Direct
        case 0x27: // AND [Direct]
        case 0x29: // AND Immediate
        case 0x2D: // AND Absolute
        case 0x2F: // AND Long
        case 0x31: // AND (Direct),Y
        case 0x32: // AND (Direct)
        case 0x33: // AND (Stack,S),Y
        case 0x35: // AND Direct,X
        case 0x37: // AND [Direct],Y
        case 0x39: // AND Absolute,Y
        case 0x3D: // AND Absolute,X
        case 0x3F: // AND Long,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                LogInstM("AND", mode);

                if (IsAccumulator16Bit())
                {
                    uint16_t value = mode->Read16Bit();
                    reg.a &= value;
                    SetNFlag(reg.a);
                    SetZFlag(reg.a);
                }
                else
                {
                    uint8_t value = mode->Read8Bit();
                    reg.al &= value;
                    SetNFlag(reg.al);
                    SetZFlag(reg.al);
                }
            }
            break;

        // EOR - Bitwise Exclusive OR
        case 0x41: // EOR (Direct,X)
        case 0x43: // EOR Stack,S
        case 0x45: // EOR Direct
        case 0x47: // EOR [Direct]
        case 0x49: // EOR Immediate
        case 0x4D: // EOR Absolute
        case 0x4F: // EOR Long
        case 0x51: // EOR (Direct),Y
        case 0x52: // EOR (Direct)
        case 0x53: // EOR (Stack,S),Y
        case 0x55: // EOR Direct,X
        case 0x57: // EOR [Direct],Y
        case 0x59: // EOR Absolute,Y
        case 0x5D: // EOR Absolute,X
        case 0x5F: // EOR Long,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                LogInstM("EOR", mode);

                if (IsAccumulator16Bit())
                {
                    uint16_t value = mode->Read16Bit();
                    reg.a ^= value;
                    SetNFlag(reg.a);
                    SetZFlag(reg.a);
                }
                else
                {
                    uint8_t value = mode->Read8Bit();
                    reg.al ^= value;
                    SetNFlag(reg.al);
                    SetZFlag(reg.al);
                }
            }
            break;

        // ORA - Bitwise OR Accumulator
        case 0x01: // ORA (Direct,X)
        case 0x03: // ORA Stack,S
        case 0x05: // ORA Direct
        case 0x07: // ORA [Direct]
        case 0x09: // ORA Immediate
        case 0x0D: // ORA Absolute
        case 0x0F: // ORA Long
        case 0x11: // ORA (Direct),Y
        case 0x12: // ORA (Direct)
        case 0x13: // ORA (Stack,S),Y
        case 0x15: // ORA Direct,X
        case 0x17: // ORA [Direct],Y
        case 0x19: // ORA Absolute,Y
        case 0x1D: // ORA Absolute,X
        case 0x1F: // ORA Long,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                LogInstM("ORA", mode);

                if (IsAccumulator16Bit())
                {
                    uint16_t value = mode->Read16Bit();
                    reg.a |= value;
                    SetNFlag(reg.a);
                    SetZFlag(reg.a);
                }
                else
                {
                    uint8_t value = mode->Read8Bit();
                    reg.al |= value;
                    SetNFlag(reg.al);
                    SetZFlag(reg.al);
                }
            }
            break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Arithmetic opcodes                                                                                                //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // ADC - Add with Carry
        case 0x61: // ADC (Direct,X)
        case 0x63: // ADC Stack,S
        case 0x65: // ADC Direct
        case 0x67: // ADC [Direct]
        case 0x69: // ADC Immediate
        case 0x6D: // ADC Absolute
        case 0x6F: // ADC Long
        case 0x71: // ADC (Direct),Y
        case 0x72: // ADC (Direct)
        case 0x73: // ADC (Stack,S),Y
        case 0x75: // ADC Direct,X
        case 0x77: // ADC [Direct],Y
        case 0x79: // ADC Absolute,Y
        case 0x7D: // ADC Absolute,X
        case 0x7F: // ADC Long,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                LogInstM("ADC", mode);

                if (IsAccumulator16Bit())
                {
                    uint16_t operand = mode->Read16Bit();
                    uint32_t result;

                    if (reg.flags.d == 0)
                    {
                        result = reg.a + reg.flags.c + operand;
                        reg.flags.v = (((reg.a ^ result) & ~(reg.a ^ operand)) & 0x8000) != 0;
                    }
                    else
                    {
                        result = (reg.a & 0x0F) + (operand & 0x0F) + reg.flags.c;
                        if (result >= 0x0A)
                            result = ((result + 0x06) & 0x0F) + 0x10;
                        result = (reg.a & 0xF0) + (operand & 0xF0) + result;
                        if (result >= 0xA0)
                            result = ((result + 0x60) & 0xFF) + 0x100;
                        result = (reg.a & 0x0F00) + (operand & 0x0F00) + result;
                        if (result >= 0x0A00)
                            result = ((result + 0x600) & 0x0FFF) + 0x1000;
                        result = (reg.a & 0xF000) + (operand & 0xF000) + result;

                        // Set overflow before final adjustment.
                        reg.flags.v = (((reg.a ^ result) & ~(reg.a ^ operand)) & 0x8000) != 0;

                        if (result >= 0xA000)
                            result += 0x6000;
                    }

                    reg.flags.c = result > 0xFFFF;
                    reg.a = result;
                    SetNFlag(reg.a);
                    SetZFlag(reg.a);
                }
                else
                {
                    uint8_t operand = mode->Read8Bit();
                    uint16_t result;

                    if (reg.flags.d == 0)
                    {
                        result = reg.al + reg.flags.c + operand;
                        reg.flags.v = (((reg.al ^ result) & ~(reg.al ^ operand)) & 0x80) != 0;
                    }
                    else
                    {
                        result = (reg.al & 0x0F) + (operand & 0x0F) + reg.flags.c;
                        if (result >= 0x0A)
                            result = ((result + 0x06) & 0x0F) + 0x10;
                        result = (reg.al & 0xF0) + (operand & 0xF0) + result;

                        // Set overflow before final adjustment.
                        reg.flags.v = (((reg.al ^ result) & ~(reg.al ^ operand)) & 0x80) != 0;

                        if (result >= 0xA0)
                            result += 0x60;
                    }

                    reg.flags.c = result > 0xFF;
                    reg.al = result;
                    SetNFlag(reg.al);
                    SetZFlag(reg.al);
                }
            }
            break;

        // SBC - Subtract with Carry
        case 0xE1: // SBC (Direct,X)
        case 0xE3: // SBC Stack,S
        case 0xE5: // SBC Direct
        case 0xE7: // SBC [Direct]
        case 0xE9: // SBC Immediate
        case 0xED: // SBC Absolute
        case 0xEF: // SBC Long
        case 0xF1: // SBC (Direct),Y
        case 0xF2: // SBC (Direct)
        case 0xF3: // SBC (Stack,S),Y
        case 0xF5: // SBC Direct,X
        case 0xF7: // SBC [Direct],Y
        case 0xF9: // SBC Absolute,Y
        case 0xFD: // SBC Absolute,X
        case 0xFF: // SBC Long,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                LogInstM("SBC", mode);

                if (IsAccumulator16Bit())
                {
                    uint16_t operand = ~mode->Read16Bit();
                    uint32_t result;

                    if (reg.flags.d == 0)
                    {
                        result = reg.a + reg.flags.c + operand;
                        reg.flags.v = (((reg.a ^ result) & ~(reg.a ^ operand)) & 0x8000) != 0;
                    }
                    else
                    {
                        result = (reg.a & 0x0F) + (operand & 0x0F) + reg.flags.c;
                        if (result <= 0x0F)
                            result = ((result - 0x06) & 0x0F); //+ 0x10;
                        result = (reg.a & 0xF0) + (operand & 0xF0) + result;
                        if (result <= 0xFF)
                            result = ((result - 0x60) & 0xFF); //+ 0x10;
                        result = (reg.a & 0x0F00) + (operand & 0x0F00) + result;
                        if (result <= 0x0FFF)
                            result = ((result - 0x0600) & 0x0FFF); //+ 0x10;
                        result = (reg.a & 0xF000) + (operand & 0xF000) + result;

                        // Set overflow before final adjustment.
                        reg.flags.v = (((reg.a ^ result) & ~(reg.a ^ operand)) & 0x8000) != 0;

                        if (result <= 0xFFFF)
                            result -= 0x6000;
                    }

                    reg.flags.c = static_cast<int8_t>((result >> 16) & 0xFF) > 0;
                    reg.a = result;
                    SetNFlag(reg.a);
                    SetZFlag(reg.a);
                }
                else
                {
                    uint8_t operand = ~mode->Read8Bit();
                    uint16_t result;

                    if (reg.flags.d == 0)
                    {
                        result = reg.al + reg.flags.c + operand;
                        reg.flags.v = (((reg.al ^ result) & ~(reg.al ^ operand)) & 0x80) != 0;
                    }
                    else
                    {
                        result = (reg.al & 0x0F) + (operand & 0x0F) + reg.flags.c;
                        if (result <= 0x0F)
                            result = ((result - 0x06) & 0x0F); //+ 0x10;
                        result = (reg.al & 0xF0) + (operand & 0xF0) + result;

                        // Set overflow before final adjustment.
                        reg.flags.v = (((reg.al ^ result) & ~(reg.al ^ operand)) & 0x80) != 0;

                        if (result <= 0xFF)
                            result -= 0x60;
                    }

                    reg.flags.c = static_cast<int8_t>(result >> 8) > 0;
                    reg.al = result;
                    SetNFlag(reg.al);
                    SetZFlag(reg.al);
                }
            }
            break;

        case 0x3A: // DEC Accumulator
        case 0xC6: // DEC Direct
        case 0xCE: // DEC Absolute
        case 0xD6: // DEC Direct,X
        case 0xDE: // DEC Absolute,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                LogInstM("DEC", mode);

                if (IsAccumulator16Bit())
                {
                    uint16_t value = mode->Read16Bit();
                    value--;
                    timer->AddCycle(eClockInternal);
                    mode->Write16Bit(value);
                    SetNFlag(value);
                    SetZFlag(value);
                }
                else
                {
                    uint8_t value = mode->Read8Bit();
                    value--;
                    timer->AddCycle(eClockInternal);
                    mode->Write8Bit(value);
                    SetNFlag(value);
                    SetZFlag(value);
                }
            }
            break;

        case 0xCA: // DEX
            {
                LogInst("DEX");

                if (IsIndex16Bit())
                {
                    reg.x--;
                    SetNFlag(reg.x);
                    SetZFlag(reg.x);
                }
                else
                {
                    reg.xl--;
                    SetNFlag(reg.xl);
                    SetZFlag(reg.xl);
                }

                timer->AddCycle(eClockInternal);
            }
            break;

        case 0x88: // DEY
            {
                LogInst("DEY");

                if (IsIndex16Bit())
                {
                    reg.y--;
                    SetNFlag(reg.y);
                    SetZFlag(reg.y);
                }
                else
                {
                    reg.yl--;
                    SetNFlag(reg.yl);
                    SetZFlag(reg.yl);
                }

                timer->AddCycle(eClockInternal);
            }
            break;

        case 0x1A: // INC Accumulator
        case 0xE6: // INC Direct
        case 0xEE: // INC Absolute
        case 0xF6: // INC Direct,X
        case 0xFE: // INC Absolute,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                LogInstM("INC", mode);

                if (IsAccumulator16Bit())
                {
                    uint16_t value = mode->Read16Bit();
                    value++;
                    timer->AddCycle(eClockInternal);
                    mode->Write16Bit(value);
                    SetNFlag(value);
                    SetZFlag(value);
                }
                else
                {
                    uint8_t value = mode->Read8Bit();
                    value++;
                    timer->AddCycle(eClockInternal);
                    mode->Write8Bit(value);
                    SetNFlag(value);
                    SetZFlag(value);
                }
            }
            break;

        case 0xE8: // INX
            {
                LogInst("INX");

                if (IsIndex16Bit())
                {
                    reg.x++;
                    SetNFlag(reg.x);
                    SetZFlag(reg.x);
                }
                else
                {
                    reg.xl++;
                    SetNFlag(reg.xl);
                    SetZFlag(reg.xl);
                }

                timer->AddCycle(eClockInternal);
            }
            break;

        case 0xC8: // INY
            {
                LogInst("INY");

                if (IsIndex16Bit())
                {
                    reg.y++;
                    SetNFlag(reg.y);
                    SetZFlag(reg.y);
                }
                else
                {
                    reg.yl++;
                    SetNFlag(reg.yl);
                    SetZFlag(reg.yl);
                }

                timer->AddCycle(eClockInternal);
            }
            break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Comparison opcodes                                                                                                //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // CMP - Compare to Accumulator
        case 0xC1: // CMP (Direct,X)
        case 0xC3: // CMP Stack,S
        case 0xC5: // CMP Direct
        case 0xC7: // CMP [Direct]
        case 0xC9: // CMP Immediate
        case 0xCD: // CMP Absolute
        case 0xCF: // CMP Long
        case 0xD1: // CMP (Direct),Y
        case 0xD2: // CMP (Direct)
        case 0xD3: // CMP (Stack,S),Y
        case 0xD5: // CMP Direct,X
        case 0xD7: // CMP [Direct],Y
        case 0xD9: // CMP Absolute,Y
        case 0xDD: // CMP Absolute,X
        case 0xDF: // CMP Long,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                LogInstM("CMP", mode);

                if (IsAccumulator16Bit())
                    Compare(reg.a, mode->Read16Bit());
                else
                    Compare(reg.al, mode->Read8Bit());
            }
            break;

        // CPX - Compare to X
        case 0xE0: // CPX Immediate
        case 0xE4: // CPX Direct
        case 0xEC: // CPX Absolute
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                LogInstM("CPX", mode);

                if (IsIndex16Bit())
                    Compare(reg.x, mode->Read16Bit());
                else
                    Compare(reg.xl, mode->Read8Bit());
            }
            break;

        // CPY - Compare to Y
        case 0xC0: // CPY Immediate
        case 0xC4: // CPY Direct
        case 0xCC: // CPY Absolute
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                LogInstM("CPY", mode);

                if (IsIndex16Bit())
                    Compare(reg.y, mode->Read16Bit());
                else
                    Compare(reg.yl, mode->Read8Bit());
            }
            break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Bit test/set/reset opcodes                                                                                        //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // BIT - Test Bit
        case 0x24: // BIT Direct
        case 0x2C: // BIT Absolute
        case 0x34: // BIT Direct,X
        case 0x3C: // BIT Absolute,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                LogInstM("BIT", mode);

                if (IsAccumulator16Bit())
                {
                    uint16_t value = mode->Read16Bit();
                    uint16_t result = reg.a & value;

                    // N and V flag look at data, not result.
                    SetNFlag(value);
                    // V looks at the second highest bit.
                    reg.flags.v = (value & 0x4000) != 0;
                    SetZFlag(result);
                }
                else
                {
                    uint8_t value = mode->Read8Bit();
                    uint8_t result = reg.al & value;

                    // N and V flag look at data, not result.
                    SetNFlag(value);
                    // V looks at the second highest bit.
                    reg.flags.v = (value & 0x40) != 0;
                    SetZFlag(result);
                }
            }
            break;

        case 0x89: // BIT Immediate
            {
                AddressModeImmediate mode(this, memory);
                mode.LoadAddress();
                LogInstM("BIT", &mode);

                if (IsAccumulator16Bit())
                {
                    uint16_t value = mode.Read16Bit();
                    uint16_t result = reg.a & value;

                    // N and V flags are not changed.
                    SetZFlag(result);
                }
                else
                {
                    uint8_t value = mode.Read8Bit();
                    uint8_t result = reg.al & value;

                    // N and V flags are not changed.
                    SetZFlag(result);
                }
            }
            break;

        // TRB - Test and Reset Bit
        case 0x14: // TRB Direct
        case 0x1C: // TRB Absolute
            {
                // TRB is a special case, use 0x0F for masking.
                AddressModePtr &mode = addressModes[opcode & 0x0F];
                mode->LoadAddress();
                LogInstM("TRB", mode);

                if (IsAccumulator16Bit())
                {
                    uint16_t value = mode->Read16Bit();
                    uint16_t result = ~reg.a & value;

                    timer->AddCycle(eClockInternal);

                    // Z flag is based on reg.a AND value.
                    SetZFlag(static_cast<uint16_t>(reg.a & value));
                    mode->Write16Bit(result);
                }
                else
                {
                    uint8_t value = mode->Read8Bit();
                    uint8_t result = ~reg.al & value;

                    timer->AddCycle(eClockInternal);

                    // Z flag is based on reg.al AND value.
                    SetZFlag(static_cast<uint8_t>(reg.al & value));
                    mode->Write8Bit(result);
                }
            }
            break;

        // TSB - Test and Set Bit
        case 0x04: // TSB Direct
        case 0x0C: // TSB Absolute
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                LogInstM("TSB", mode);

                if (IsAccumulator16Bit())
                {
                    uint16_t value = mode->Read16Bit();
                    uint16_t result = reg.a | value;

                    timer->AddCycle(eClockInternal);

                    // Z flag is based on reg.a AND value.
                    SetZFlag(static_cast<uint16_t>(reg.a & value));
                    mode->Write16Bit(result);
                }
                else
                {
                    uint8_t value = mode->Read8Bit();
                    uint8_t result = reg.al | value;

                    timer->AddCycle(eClockInternal);

                    // Z flag is based on reg.al AND value.
                    SetZFlag(static_cast<uint8_t>(reg.al & value));
                    mode->Write8Bit(result);
                }
            }
            break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Shift and Rotate opcodes                                                                                          //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // ASL - Arithmetic Shift Left
        case 0x06: // ASL Direct
        case 0x0A: // ASL Acc
        case 0x0E: // ASL Absolute
        case 0x16: // ASL Direct,X
        case 0x1E: // ASL Absolute,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                LogInstM("ASL", mode);

                if (IsAccumulator16Bit())
                {
                    uint16_t value = mode->Read16Bit();
                    uint16_t result = value << 1;

                    timer->AddCycle(eClockInternal);

                    reg.flags.c = value >> 15;
                    SetNFlag(result);
                    SetZFlag(result);
                    mode->Write16Bit(result);
                }
                else
                {
                    uint8_t value = mode->Read8Bit();
                    uint8_t result = value << 1;

                    timer->AddCycle(eClockInternal);

                    reg.flags.c = value >> 7;
                    SetNFlag(result);
                    SetZFlag(result);
                    mode->Write8Bit(result);
                }
            }
            break;

        // LSR - Logical Shift Right
        case 0x46: // LSR Direct
        case 0x4A: // LSR Acc
        case 0x4E: // LSR Absolute
        case 0x56: // LSR Direct,X
        case 0x5E: // LSR Absolute,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                LogInstM("LSR", mode);

                if (IsAccumulator16Bit())
                {
                    uint16_t value = mode->Read16Bit();
                    uint16_t result = value >> 1;

                    timer->AddCycle(eClockInternal);

                    reg.flags.c = value & 0x01;
                    SetNFlag(result);
                    SetZFlag(result);
                    mode->Write16Bit(result);
                }
                else
                {
                    uint8_t value = mode->Read8Bit();
                    uint8_t result = value >> 1;

                    timer->AddCycle(eClockInternal);

                    reg.flags.c = value & 0x01;
                    SetNFlag(result);
                    SetZFlag(result);
                    mode->Write8Bit(result);
                }
            }
            break;

        // ROL - Rotate Left
        case 0x26: // ROL Direct
        case 0x2A: // ROL Acc
        case 0x2E: // ROL Absolute
        case 0x36: // ROL Direct,X
        case 0x3E: // ROL Absolute,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                LogInstM("ROL", mode);

                if (IsAccumulator16Bit())
                {
                    uint16_t value = mode->Read16Bit();
                    uint16_t result = (value << 1) | reg.flags.c;

                    timer->AddCycle(eClockInternal);

                    reg.flags.c = value >> 15;
                    SetNFlag(result);
                    SetZFlag(result);
                    mode->Write16Bit(result);
                }
                else
                {
                    uint8_t value = mode->Read8Bit();
                    uint8_t result = (value << 1) | reg.flags.c;

                    timer->AddCycle(eClockInternal);

                    reg.flags.c = value >> 7;
                    SetNFlag(result);
                    SetZFlag(result);
                    mode->Write8Bit(result);
                }
            }
            break;

        // ROR - Rotate Right
        case 0x66: // ROR Direct
        case 0x6A: // ROR Acc
        case 0x6E: // ROR Absolute
        case 0x76: // ROR Direct,X
        case 0x7E: // ROR Absolute,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                LogInstM("ROR", mode);

                if (IsAccumulator16Bit())
                {
                    uint16_t value = mode->Read16Bit();
                    uint16_t result = (value >> 1) | (reg.flags.c << 15);

                    timer->AddCycle(eClockInternal);

                    reg.flags.c = value & 0x01;
                    SetNFlag(result);
                    SetZFlag(result);
                    mode->Write16Bit(result);
                }
                else
                {
                    uint8_t value = mode->Read8Bit();
                    uint8_t result = (value >> 1) | (reg.flags.c << 7);

                    timer->AddCycle(eClockInternal);

                    reg.flags.c = value & 0x01;
                    SetNFlag(result);
                    SetZFlag(result);
                    mode->Write8Bit(result);
                }
            }
            break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Branch opcodes                                                                                                    //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        case 0x80: // BRA - Branch
            {
                int8_t offset = static_cast<int8_t>(ReadPC8Bit());
                LogCpu("%02X %02X: BRA %d", opcode, (uint8_t)offset, offset);
                PrintState();
                timer->AddCycle(eClockInternal);

#ifndef TESTING
                // Detect an infinite loop and stop execution. Allow when running unit tests.
                if (offset == -2)
                    throw InfiniteLoopException();
#endif

                reg.pc += offset;
            }
            break;

        case 0x82: // BRL - Branch Long
            {
                int16_t offset = static_cast<int16_t>(ReadPC16Bit());
                LogCpu("%02X %02X %02X: BRL %d", opcode, Bytes::GetByte<0>(offset), Bytes::GetByte<1>(offset), offset);
                PrintState();
                timer->AddCycle(eClockInternal);

#ifndef TESTING
                // Detect an infinite loop and stop execution. Allow when running unit tests.
                if (offset == -3)
                    throw InfiniteLoopException();
#endif

                reg.pc += offset;
            }
            break;

        case 0x10: // BPL - Branch if Plus (n flag clear)
        case 0x30: // BMI - Branch if Minus (n flag set)
        case 0x50: // BVC - Branch if Overflow Clear
        case 0x70: // BVS - Branch if Overflow Set
        case 0x90: // BCC - Branch if Carry Clear
        case 0xB0: // BCS - Branch if Carry Set
        case 0xD0: // BNE - Branch if Not Equal (z flag clear)
        case 0xF0: // BEQ - Branch if Equal (z flag set)
            {
                int8_t offset = static_cast<int8_t>(ReadPC8Bit());
                const char *names[] = {"BPL", "BMI", "BVC", "BVS", "BCC", "BCS", "BNE", "BEQ"};
                LogCpu("%02X %02X: %s %d", opcode, (uint8_t)offset, names[opcode >> 5], offset);
                PrintState();

                // Since you can't take the address of bitfield, a lookup table with pointers to the flags can't be used.
                // Instead, shift the p register until the desired bit is lsb. This is n, v, c, and z.
                uint8_t flagShift[] = {7, 6, 0, 1};

                // Bit 6 of the opcode says which flag to check, bit 5 is whether the flag should be set or cleared.
                if (((reg.p >> flagShift[opcode >> 6]) & 0x01) == ((opcode >> 5) & 0x01))
                {
                    timer->AddCycle(eClockInternal);
                    reg.pc += offset;
                }
            }
            break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Jump opcodes                                                                                                      //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // Short Jumps
        case 0x4C: // JMP Absolute
        case 0x6C: // JMP (Absolute)
        case 0x7C: // JMP (Absolute,X)
            {
                AddressModePtr &mode = jmpAddressModes[opcode >> 4];
                mode->LoadAddress();
                LogInstM("JMP", mode);

                uint16_t newPc = mode->GetAddress().GetOffset();

#ifndef TESTING
                // Detect an infinite loop and stop execution. Allow when running unit tests.
                if (reg.pc - newPc == 3)
                    throw InfiniteLoopException();
#endif

                reg.pc = newPc;
            }
            break;

        // Long Jumps
        case 0x5C: // JMP AbsoluteLong
        case 0xDC: // JMP/JML [Absolute]
            {
                AddressModePtr &mode = jmpAddressModes[opcode >> 4];
                mode->LoadAddress();
                LogInstM("JMP", mode);

                uint8_t newPb = mode->GetAddress().GetBank();
                uint16_t newPc = mode->GetAddress().GetOffset();

#ifndef TESTING
                // Detect an infinite loop and stop execution. Allow when running unit tests.
                if (Bytes::Make24Bit(newPb, newPc) - Bytes::Make24Bit(reg.pb, reg.pc) == 4)
                    throw InfiniteLoopException();
#endif

                reg.pb = newPb;
                reg.pc = newPc;
            }
            break;

        // Jump to Subroutine
        case 0x20: // JSR Absolute
        case 0xFC: // JSR (Absolute,X)
            {
                AddressModePtr &mode = jmpAddressModes[opcode >> 4];
                mode->LoadAddress();
                LogInstM("JSR", mode);

                timer->AddCycle(eClockInternal);

                Push16Bit(reg.pc - 1);
                reg.pc = mode->GetAddress().GetOffset();
            }
            break;

        // Jump to Subroutine Long
        case 0x22: // JSL AbsoluteLong
            {
                AddressModeAbsoluteLong mode(this, memory);
                mode.LoadAddress();
                LogInstM("JSL", &mode);

                timer->AddCycle(eClockInternal);

                Push8Bit(reg.pb);
                Push16Bit(reg.pc - 1);
                reg.pb = mode.GetAddress().GetBank();
                reg.pc = mode.GetAddress().GetOffset();
            }
            break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Return opcodes                                                                                                    //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // RTS - Return from subroutine
        case 0x60:
            {
                LogInst("RTS");
                timer->AddCycle(2 * eClockInternal);

                reg.pc = Pop16Bit() + 1;

                timer->AddCycle(eClockInternal);
            }
            break;

        // RTL - Return from subroutine long
        case 0x6B:
            {
                LogInst("RTL");
                timer->AddCycle(2 * eClockInternal);

                reg.pc = Pop16Bit() + 1;
                reg.pb = Pop8Bit();
            }
            break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Software interrupts                                                                                               //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        case 0x00: // BRK - Breakpoint
        case 0x02: // COP - Coprocessor
            {
                const char *names[] = {"BRK", "COP"};
                uint8_t signature = ReadPC8Bit();
                LogCpu("%02X %02X: %s %02X", opcode, signature, names[opcode >> 1], signature);
                PrintState();

                if (reg.emulationMode)
                {
                    const uint32_t vectors[] = {0xFFFE, 0xFFF4};
                    Push16Bit(reg.pc);
                    Push8Bit(reg.p | 0x10);
                    reg.pb = 0;
                    reg.pc = memory->Read16Bit(vectors[opcode >> 1]);
                    reg.flags.i = 1;
                    reg.flags.d = 0;
                }
                else
                {
                    const uint32_t vectors[] = {0xFFE6, 0xFFE4};
                    Push8Bit(reg.pb);
                    Push16Bit(reg.pc);
                    Push8Bit(reg.p);
                    reg.pb = 0;
                    reg.pc = memory->Read16Bit(vectors[opcode >> 1]);
                    reg.flags.i = 1;
                    reg.flags.d = 0;
                }
            }
            break;

        case 0x40: // RTI - Return from interrupt
            {
                LogInst("RTI");
                timer->AddCycle(2 * eClockInternal);

                reg.p = Pop8Bit();
                UpdateRegistersAfterFlagChange();
                reg.pc = Pop16Bit();
                if (!reg.emulationMode)
                    reg.pb = Pop8Bit();
            }
            break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Flag Set/Clear Opcodes                                                                                            //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // CLC - Clear Carry
        case 0x18:
            {
                LogInst("CLC");
                reg.flags.c = 0;
                timer->AddCycle(eClockInternal);
            }
            break;

        // SEC - Set Carry
        case 0x38:
            {
                LogInst("SEC");
                reg.flags.c = 1;
                timer->AddCycle(eClockInternal);
            }
            break;

        // CLI - Clear Interrupt Disable
        case 0x58:
            {
                LogInst("CLI");
                reg.flags.i = 0;
                timer->AddCycle(eClockInternal);
            }
            break;

        // SEI - Set Interrupt Disable
        case 0x78:
            {
                LogInst("SEI");
                reg.flags.i = 1;
                timer->AddCycle(eClockInternal);
            }
            break;

        // CLV - Clear Overflow
        case 0xB8:
            {
                LogInst("CLV");
                reg.flags.v = 0;
                timer->AddCycle(eClockInternal);
            }
            break;

        // CLD - Clear Decimal
        case 0xD8:
            {
                LogInst("CLD");
                reg.flags.d = 0;
                timer->AddCycle(eClockInternal);
            }
            break;

        // SED - Set Decimal
        case 0xF8:
            {
                LogInst("SED");
                reg.flags.d = 1;
                timer->AddCycle(eClockInternal);
            }
            break;

        // REP - Reset P flag
        case 0xC2:
            {
                AddressModeImmediate mode(this, memory);
                mode.LoadAddress();
                LogInstM("REP", &mode);

                reg.p &= ~mode.Read8Bit();
                UpdateRegistersAfterFlagChange();

                timer->AddCycle(eClockInternal);
            }
            break;

        // SEP - Set P flag
        case 0xE2:
            {
                AddressModeImmediate mode(this, memory);
                mode.LoadAddress();
                LogInstM("SEP", &mode);

                reg.p |= mode.Read8Bit();
                UpdateRegistersAfterFlagChange();

                timer->AddCycle(eClockInternal);
            }
            break;

        // XCE - Exchange c and e
        case 0xFB:
            {
                LogInst("XCE");

                uint8_t carry = reg.flags.c;
                reg.flags.c = reg.emulationMode;
                reg.emulationMode = carry;
                UpdateRegistersAfterFlagChange();

                timer->AddCycle(eClockInternal);
            }
            break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Memory Move Opcodes                                                                                               //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // MVP - Move Memory Positive
        case 0x44:
            {
                uint8_t dstBank = ReadPC8Bit();
                uint8_t srcBank = ReadPC8Bit();
                LogInst2("MVP", dstBank, srcBank);

                Address dst(dstBank, reg.y);
                Address src(srcBank, reg.x);

                memory->Write8Bit(dst, memory->Read8Bit(src));

                reg.db = dstBank;
                reg.a--;
                reg.x--;
                reg.y--;
                if (IsIndex8Bit())
                {
                    reg.x &= 0x00FF;
                    reg.y &= 0x00FF;
                }

                timer->AddCycle(2 * eClockInternal);

                // Loop until reg.a underflows.
                if (reg.a != 0xFFFF)
                    reg.pc -= 3;
            }
            break;

        // MVN - Move Memory Negative
        case 0x54:
            {
                uint8_t dstBank = ReadPC8Bit();
                uint8_t srcBank = ReadPC8Bit();
                LogInst2("MVN", dstBank, srcBank);

                Address dst(dstBank, reg.y);
                Address src(srcBank, reg.x);

                memory->Write8Bit(dst, memory->Read8Bit(src));

                reg.db = dstBank;
                reg.a--;
                reg.x++;
                reg.y++;
                if (IsIndex8Bit())
                {
                    reg.x &= 0x00FF;
                    reg.y &= 0x00FF;
                }

                timer->AddCycle(2 * eClockInternal);

                // Loop until reg.a underflows.
                if (reg.a != 0xFFFF)
                    reg.pc -= 3;
            }
            break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Nop Opcodes                                                                                                       //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // NOP
        case 0xEA:
            {
                LogInst("NOP");
                timer->AddCycle(eClockInternal);
            }
            break;

        // WDM - 2 byte NOP
        case 0x42:
            {
                AddressModeImmediate mode(this, memory);
                mode.LoadAddress();
                LogInstM("WDM", &mode);

                uint8_t nopData = mode.Read8Bit();
                (void)nopData;
            }
            break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Stop/Wait Opcodes                                                                                                 //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // WAI - Wait
        case 0xCB:
            {
                LogInst("WAI");
                timer->AddCycle(3 * eClockInternal);
                waiting = true;
            }
            break;

        // STP - Stop
        case 0xDB:
            {
                // Documentation says that only a reset should wake up stop, but test roms and other emulators seem to treat it the same as WAI.
                LogInst("STP");
                timer->AddCycle(3 * eClockInternal);
                waiting = true;
            }
            break;
    }
}


void Cpu::NotYetImplemented(uint8_t opcode)
{
    // reg.pc is advanced in ReadPC8Bit, so subtract 1 to get the real address of the error.
    uint32_t addr = GetFullPC(reg.pc - 1);

    std::stringstream ss;
    ss << "NYI opcode 0x" << std::hex << std::uppercase << (int)opcode << " at 0x" << (int)addr;
    throw NotYetImplementedException(ss.str());
}