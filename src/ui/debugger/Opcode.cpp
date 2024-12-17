#include <QtCore/QSet>
#include "../UiUtils.h"
#include "core/Bytes.h"
#include "core/Cpu.h"
#include "Opcode.h"

struct AddrModeInfo
{
    QString name;
    QString argFormat;

    AddrModeInfo(const QString &name, const QString &argFormat) : name(name), argFormat(argFormat) {}
};

static AddrModeInfo addrModes[] = {
    {"Absolute", "%04X"},          // Absolute
    {"Absolute,X", "%04X,X"},      // AbsoluteIndexedX
    {"Absolute,Y", "%04X,Y"},      // AbsoluteIndexedY
    {"(Absolute)", "(%04X)"},      // AbsoluteIndirect
    {"[Absolute]", "[%04X]"},      // AbsoluteIndirectLong
    {"(Absolute,X)", "(%04X,X)"},  // AbsoluteIndexedIndirect
    {"Long", "%06X"},              // AbsoluteLong
    {"Long,X", "%06X,X"},          // AbsoluteLongIndexedX
    {"Accumulator", "Acc"},        // Accumulator
    {"Direct", "%02X"},            // Direct
    {"Direct,X", "%02X,X"},        // DirectIndexedX
    {"Direct,Y", "%02X,Y"},        // DirectIndexedY
    {"(Direct)", "(%02X)"},        // DirectIndirect
    {"[Direct]", "[%02X]"},        // DirectIndirectLong
    {"(Direct,X)", "(%02X,X)"},    // DirectIndexedIndirect
    {"(Direct),Y", "(%02X),Y"},    // DirectIndirectIndexed
    {"[Direct],Y", "[%02X],Y"},    // DirectIndirectLongIndexed
    {"Imm", "#%02X"},              // Immediate
    {"Stack,S", "%02X,S"},         // StackRelative
    {"(Stack,S),Y", "(%02X,S),Y"}, // StackRelativeIndirectIndexed
    {"Relative8", "%d"},           // Relative8
    {"Relative16", "%d"},          // Relative16
    {"", ""}                       // None/other
};


const Opcode Opcode::opcodes[256] = {
    {2, "BRK", EAddrModes::None},                           // 00 BRK 
    {2, "ORA", EAddrModes::DirectIndexedIndirect},          // 01 ORA (Direct,X)
    {2, "COP", EAddrModes::None},                           // 02 COP 
    {2, "ORA", EAddrModes::StackRelative},                  // 03 ORA Stack,S
    {2, "TSB", EAddrModes::Direct},                         // 04 TSB Direct
    {2, "ORA", EAddrModes::Direct},                         // 05 ORA Direct
    {2, "ASL", EAddrModes::Direct},                         // 06 ASL Direct
    {2, "ORA", EAddrModes::DirectIndirectLong},             // 07 ORA [Direct]
    {1, "PHP", EAddrModes::None},                           // 08 PHP 
    {2, "ORA", EAddrModes::Immediate},                      // 09 ORA Immediate
    {1, "ASL", EAddrModes::Accumulator},                    // 0A ASL Acc
    {1, "PHD", EAddrModes::None},                           // 0B PHD 
    {3, "TSB", EAddrModes::Absolute},                       // 0C TSB Absolute
    {3, "ORA", EAddrModes::Absolute},                       // 0D ORA Absolute
    {3, "ASL", EAddrModes::Absolute},                       // 0E ASL Absolute
    {4, "ORA", EAddrModes::AbsoluteLong},                   // 0F ORA Long
    {2, "BPL", EAddrModes::Relative8},                      // 10 BPL 
    {2, "ORA", EAddrModes::DirectIndirectIndexed},          // 11 ORA (Direct),Y
    {2, "ORA", EAddrModes::DirectIndirect},                 // 12 ORA (Direct)
    {2, "ORA", EAddrModes::StackRelativeIndirectIndexed},   // 13 ORA (Stack,S),Y
    {2, "TRB", EAddrModes::Direct},                         // 14 TRB Direct
    {2, "ORA", EAddrModes::DirectIndexedX},                 // 15 ORA Direct,X
    {2, "ASL", EAddrModes::DirectIndexedX},                 // 16 ASL Direct,X
    {2, "ORA", EAddrModes::DirectIndirectLongIndexed},      // 17 ORA [Direct],Y
    {1, "CLC", EAddrModes::None},                           // 18 CLC 
    {3, "ORA", EAddrModes::AbsoluteIndexedY},               // 19 ORA Absolute,Y
    {1, "INC", EAddrModes::Accumulator},                    // 1A INC Accumulator
    {1, "TCS", EAddrModes::None},                           // 1B TCS 
    {3, "TRB", EAddrModes::Absolute},                       // 1C TRB Absolute
    {3, "ORA", EAddrModes::AbsoluteIndexedX},               // 1D ORA Absolute,X
    {3, "ASL", EAddrModes::AbsoluteIndexedX},               // 1E ASL Absolute,X
    {4, "ORA", EAddrModes::AbsoluteLongIndexedX},           // 1F ORA Long,X
    {3, "JSR", EAddrModes::Absolute},                       // 20 JSR Absolute
    {2, "AND", EAddrModes::DirectIndexedIndirect},          // 21 AND (Direct,X)
    {4, "JSL", EAddrModes::AbsoluteLong},                   // 22 JSL AbsoluteLong
    {2, "AND", EAddrModes::StackRelative},                  // 23 AND Stack,S
    {2, "BIT", EAddrModes::Direct},                         // 24 BIT Direct
    {2, "AND", EAddrModes::Direct},                         // 25 AND Direct
    {2, "ROL", EAddrModes::Direct},                         // 26 ROL Direct
    {2, "AND", EAddrModes::DirectIndirectLong},             // 27 AND [Direct]
    {1, "PLP", EAddrModes::None},                           // 28 PLP 
    {2, "AND", EAddrModes::Immediate},                      // 29 AND Immediate
    {1, "ROL", EAddrModes::Accumulator},                    // 2A ROL Acc
    {1, "PLD", EAddrModes::None},                           // 2B PLD 
    {3, "BIT", EAddrModes::Absolute},                       // 2C BIT Absolute
    {3, "AND", EAddrModes::Absolute},                       // 2D AND Absolute
    {3, "ROL", EAddrModes::Absolute},                       // 2E ROL Absolute
    {4, "AND", EAddrModes::AbsoluteLong},                   // 2F AND Long
    {2, "BMI", EAddrModes::Relative8},                      // 30 BMI 
    {2, "AND", EAddrModes::DirectIndirectIndexed},          // 31 AND (Direct),Y
    {2, "AND", EAddrModes::DirectIndirect},                 // 32 AND (Direct)
    {2, "AND", EAddrModes::StackRelativeIndirectIndexed},   // 33 AND (Stack,S),Y
    {2, "BIT", EAddrModes::DirectIndexedX},                 // 34 BIT Direct,X
    {2, "AND", EAddrModes::DirectIndexedX},                 // 35 AND Direct,X
    {2, "ROL", EAddrModes::DirectIndexedX},                 // 36 ROL Direct,X
    {2, "AND", EAddrModes::DirectIndirectLongIndexed},      // 37 AND [Direct],Y
    {1, "SEC", EAddrModes::None},                           // 38 SEC 
    {3, "AND", EAddrModes::AbsoluteIndexedY},               // 39 AND Absolute,Y
    {1, "DEC", EAddrModes::Accumulator},                    // 3A DEC Accumulator
    {1, "TSC", EAddrModes::None},                           // 3B TSC 
    {3, "BIT", EAddrModes::AbsoluteIndexedX},               // 3C BIT Absolute,X
    {3, "AND", EAddrModes::AbsoluteIndexedX},               // 3D AND Absolute,X
    {3, "ROL", EAddrModes::AbsoluteIndexedX},               // 3E ROL Absolute,X
    {4, "AND", EAddrModes::AbsoluteLongIndexedX},           // 3F AND Long,X
    {1, "RTI", EAddrModes::None},                           // 40 RTI 
    {2, "EOR", EAddrModes::DirectIndexedIndirect},          // 41 EOR (Direct,X)
    {2, "WDM", EAddrModes::None},                           // 42 WDM 
    {2, "EOR", EAddrModes::StackRelative},                  // 43 EOR Stack,S
    {3, "MVP", EAddrModes::None},                           // 44 MVP 
    {2, "EOR", EAddrModes::Direct},                         // 45 EOR Direct
    {2, "LSR", EAddrModes::Direct},                         // 46 LSR Direct
    {2, "EOR", EAddrModes::DirectIndirectLong},             // 47 EOR [Direct]
    {1, "PHA", EAddrModes::None},                           // 48 PHA 
    {2, "EOR", EAddrModes::Immediate},                      // 49 EOR Immediate
    {1, "LSR", EAddrModes::Accumulator},                    // 4A LSR Acc
    {1, "PHK", EAddrModes::None},                           // 4B PHK 
    {3, "JMP", EAddrModes::Absolute},                       // 4C JMP Absolute
    {3, "EOR", EAddrModes::Absolute},                       // 4D EOR Absolute
    {3, "LSR", EAddrModes::Absolute},                       // 4E LSR Absolute
    {4, "EOR", EAddrModes::AbsoluteLong},                   // 4F EOR Long
    {2, "BVC", EAddrModes::Relative8},                      // 50 BVC 
    {2, "EOR", EAddrModes::DirectIndirectIndexed},          // 51 EOR (Direct),Y
    {2, "EOR", EAddrModes::DirectIndirect},                 // 52 EOR (Direct)
    {2, "EOR", EAddrModes::StackRelativeIndirectIndexed},   // 53 EOR (Stack,S),Y
    {3, "MVN", EAddrModes::None},                           // 54 MVN 
    {2, "EOR", EAddrModes::DirectIndexedX},                 // 55 EOR Direct,X
    {2, "LSR", EAddrModes::DirectIndexedX},                 // 56 LSR Direct,X
    {2, "EOR", EAddrModes::DirectIndirectLongIndexed},      // 57 EOR [Direct],Y
    {1, "CLI", EAddrModes::None},                           // 58 CLI 
    {3, "EOR", EAddrModes::AbsoluteIndexedY},               // 59 EOR Absolute,Y
    {1, "PHY", EAddrModes::None},                           // 5A PHY 
    {1, "TCD", EAddrModes::None},                           // 5B TCD 
    {4, "JMP", EAddrModes::AbsoluteLong},                   // 5C JMP AbsoluteLong
    {3, "EOR", EAddrModes::AbsoluteIndexedX},               // 5D EOR Absolute,X
    {3, "LSR", EAddrModes::AbsoluteIndexedX},               // 5E LSR Absolute,X
    {4, "EOR", EAddrModes::AbsoluteLongIndexedX},           // 5F EOR Long,X
    {1, "RTS", EAddrModes::None},                           // 60 RTS 
    {2, "ADC", EAddrModes::DirectIndexedIndirect},          // 61 ADC (Direct,X)
    {3, "PER", EAddrModes::Relative16},                     // 62 PER 
    {2, "ADC", EAddrModes::StackRelative},                  // 63 ADC Stack,S
    {2, "STZ", EAddrModes::Direct},                         // 64 STZ Direct
    {2, "ADC", EAddrModes::Direct},                         // 65 ADC Direct
    {2, "ROR", EAddrModes::Direct},                         // 66 ROR Direct
    {2, "ADC", EAddrModes::DirectIndirectLong},             // 67 ADC [Direct]
    {1, "PLA", EAddrModes::None},                           // 68 PLA 
    {2, "ADC", EAddrModes::Immediate},                      // 69 ADC Immediate
    {1, "ROR", EAddrModes::Accumulator},                    // 6A ROR Acc
    {1, "RTL", EAddrModes::None},                           // 6B RTL 
    {3, "JMP", EAddrModes::AbsoluteIndirect},               // 6C JMP (Absolute)
    {2, "ADC", EAddrModes::Absolute},                       // 6D ADC Absolute
    {2, "ROR", EAddrModes::Absolute},                       // 6E ROR Absolute
    {4, "ADC", EAddrModes::AbsoluteLong},                   // 6F ADC Long
    {2, "BVS", EAddrModes::Relative8},                      // 70 BVS 
    {2, "ADC", EAddrModes::DirectIndirectIndexed},          // 71 ADC (Direct),Y
    {2, "ADC", EAddrModes::DirectIndirect},                 // 72 ADC (Direct)
    {2, "ADC", EAddrModes::StackRelativeIndirectIndexed},   // 73 ADC (Stack,S),Y
    {2, "STZ", EAddrModes::DirectIndexedX},                 // 74 STZ Direct,X
    {2, "ADC", EAddrModes::DirectIndexedX},                 // 75 ADC Direct,X
    {2, "ROR", EAddrModes::DirectIndexedX},                 // 76 ROR Direct,X
    {2, "ADC", EAddrModes::DirectIndirectLongIndexed},      // 77 ADC [Direct],Y
    {1, "SEI", EAddrModes::None},                           // 78 SEI 
    {3, "ADC", EAddrModes::AbsoluteIndexedY},               // 79 ADC Absolute,Y
    {1, "PLY", EAddrModes::None},                           // 7A PLY 
    {1, "TDC", EAddrModes::None},                           // 7B TDC 
    {3, "JMP", EAddrModes::AbsoluteIndexedIndirect},        // 7C JMP (Absolute,X)
    {3, "ADC", EAddrModes::AbsoluteIndexedX},               // 7D ADC Absolute,X
    {3, "ROR", EAddrModes::AbsoluteIndexedX},               // 7E ROR Absolute,X
    {4, "ADC", EAddrModes::AbsoluteLongIndexedX},           // 7F ADC Long,X
    {2, "BRA", EAddrModes::Relative8},                      // 80 BRA 
    {2, "STA", EAddrModes::DirectIndexedIndirect},          // 81 STA (Direct,X)
    {3, "BRL", EAddrModes::Relative16},                     // 82 BRL 
    {2, "STA", EAddrModes::StackRelative},                  // 83 STA Stack,S
    {2, "STY", EAddrModes::Direct},                         // 84 STY Direct
    {2, "STA", EAddrModes::Direct},                         // 85 STA Direct
    {2, "STX", EAddrModes::Direct},                         // 86 STX Direct
    {2, "STA", EAddrModes::DirectIndirectLong},             // 87 STA [Direct]
    {1, "DEY", EAddrModes::None},                           // 88 DEY 
    {2, "BIT", EAddrModes::Immediate},                      // 89 BIT Immediate
    {1, "TXA", EAddrModes::None},                           // 8A TXA 
    {1, "PHB", EAddrModes::None},                           // 8B PHB 
    {3, "STY", EAddrModes::Absolute},                       // 8C STY Absolute
    {3, "STA", EAddrModes::Absolute},                       // 8D STA Absolute
    {3, "STX", EAddrModes::Absolute},                       // 8E STX Absolute
    {4, "STA", EAddrModes::AbsoluteLong},                   // 8F STA Long
    {2, "BCC", EAddrModes::Relative8},                      // 90 BCC 
    {2, "STA", EAddrModes::DirectIndirectIndexed},          // 91 STA (Direct),Y
    {2, "STA", EAddrModes::DirectIndirect},                 // 92 STA (Direct)
    {2, "STA", EAddrModes::StackRelativeIndirectIndexed},   // 93 STA (Stack,S),Y
    {2, "STY", EAddrModes::DirectIndexedX},                 // 94 STY Direct,X
    {2, "STA", EAddrModes::DirectIndexedX},                 // 95 STA Direct,X
    {2, "STX", EAddrModes::DirectIndexedY},                 // 96 STX Direct,Y
    {2, "STA", EAddrModes::DirectIndirectLongIndexed},      // 97 STA [Direct],Y
    {1, "TYA", EAddrModes::None},                           // 98 TYA 
    {3, "STA", EAddrModes::AbsoluteIndexedY},               // 99 STA Absolute,Y
    {1, "TXS", EAddrModes::None},                           // 9A TXS 
    {1, "TXY", EAddrModes::None},                           // 9B TXY 
    {3, "STZ", EAddrModes::Absolute},                       // 9C STZ Absolute
    {3, "STA", EAddrModes::AbsoluteIndexedX},               // 9D STA Absolute,X
    {3, "STZ", EAddrModes::AbsoluteIndexedX},               // 9E STZ Absolute,X
    {4, "STA", EAddrModes::AbsoluteLongIndexedX},           // 9F STA Long,X
    {2, "LDY", EAddrModes::Immediate},                      // A0 LDY Immediate
    {2, "LDA", EAddrModes::DirectIndexedIndirect},          // A1 LDA (Direct,X)
    {2, "LDX", EAddrModes::Immediate},                      // A2 LDX Immediate
    {2, "LDA", EAddrModes::StackRelative},                  // A3 LDA Stack,S
    {2, "LDY", EAddrModes::Direct},                         // A4 LDY Direct
    {2, "LDA", EAddrModes::Direct},                         // A5 LDA Direct
    {2, "LDX", EAddrModes::Direct},                         // A6 LDX Direct
    {2, "LDA", EAddrModes::DirectIndirectLong},             // A7 LDA [Direct]
    {1, "TAY", EAddrModes::None},                           // A8 TAY 
    {2, "LDA", EAddrModes::Immediate},                      // A9 LDA Immediate
    {1, "TAX", EAddrModes::None},                           // AA TAX 
    {1, "PLB", EAddrModes::None},                           // AB PLB 
    {3, "LDY", EAddrModes::Absolute},                       // AC LDY Absolute
    {3, "LDA", EAddrModes::Absolute},                       // AD LDA Absolute
    {3, "LDX", EAddrModes::Absolute},                       // AE LDX Absolute
    {4, "LDA", EAddrModes::AbsoluteLong},                   // AF LDA Long
    {2, "BCS", EAddrModes::Relative8},                      // B0 BCS 
    {2, "LDA", EAddrModes::DirectIndirectIndexed},          // B1 LDA (Direct),Y
    {2, "LDA", EAddrModes::DirectIndirect},                 // B2 LDA (Direct)
    {2, "LDA", EAddrModes::StackRelativeIndirectIndexed},   // B3 LDA (Stack,S),Y
    {2, "LDY", EAddrModes::DirectIndexedX},                 // B4 LDY Direct,X
    {2, "LDA", EAddrModes::DirectIndexedX},                 // B5 LDA Direct,X
    {2, "LDX", EAddrModes::DirectIndexedY},                 // B6 LDX Direct,Y
    {2, "LDA", EAddrModes::DirectIndirectLongIndexed},      // B7 LDA [Direct],Y
    {1, "CLV", EAddrModes::None},                           // B8 CLV 
    {3, "LDA", EAddrModes::AbsoluteIndexedY},               // B9 LDA Absolute,Y
    {1, "TSX", EAddrModes::None},                           // BA TSX 
    {1, "TYX", EAddrModes::None},                           // BB TYX 
    {3, "LDY", EAddrModes::AbsoluteIndexedX},               // BC LDY Absolute,X
    {3, "LDA", EAddrModes::AbsoluteIndexedX},               // BD LDA Absolute,X
    {3, "LDX", EAddrModes::AbsoluteIndexedY},               // BE LDX Absolute,Y
    {4, "LDA", EAddrModes::AbsoluteLongIndexedX},           // BF LDA Long,X
    {2, "CPY", EAddrModes::Immediate},                      // C0 CPY Immediate
    {2, "CMP", EAddrModes::DirectIndexedIndirect},          // C1 CMP (Direct,X)
    {2, "REP", EAddrModes::None},                           // C2 REP 
    {2, "CMP", EAddrModes::StackRelative},                  // C3 CMP Stack,S
    {2, "CPY", EAddrModes::Direct},                         // C4 CPY Direct
    {2, "CMP", EAddrModes::Direct},                         // C5 CMP Direct
    {2, "DEC", EAddrModes::Direct},                         // C6 DEC Direct
    {2, "CMP", EAddrModes::DirectIndirectLong},             // C7 CMP [Direct]
    {1, "INY", EAddrModes::None},                           // C8 INY 
    {2, "CMP", EAddrModes::Immediate},                      // C9 CMP Immediate
    {1, "DEX", EAddrModes::None},                           // CA DEX 
    {1, "WAI", EAddrModes::None},                           // CB WAI 
    {3, "CPY", EAddrModes::Absolute},                       // CC CPY Absolute
    {3, "CMP", EAddrModes::Absolute},                       // CD CMP Absolute
    {3, "DEC", EAddrModes::Absolute},                       // CE DEC Absolute
    {4, "CMP", EAddrModes::AbsoluteLong},                   // CF CMP Long
    {2, "BNE", EAddrModes::Relative8},                      // D0 BNE 
    {2, "CMP", EAddrModes::DirectIndirectIndexed},          // D1 CMP (Direct),Y
    {2, "CMP", EAddrModes::DirectIndirect},                 // D2 CMP (Direct)
    {2, "CMP", EAddrModes::StackRelativeIndirectIndexed},   // D3 CMP (Stack,S),Y
    {2, "PEI", EAddrModes::Direct},                         // D4 PEI 
    {2, "CMP", EAddrModes::DirectIndexedX},                 // D5 CMP Direct,X
    {2, "DEC", EAddrModes::DirectIndexedX},                 // D6 DEC Direct,X
    {2, "CMP", EAddrModes::DirectIndirectLongIndexed},      // D7 CMP [Direct],Y
    {1, "CLD", EAddrModes::None},                           // D8 CLD 
    {3, "CMP", EAddrModes::AbsoluteIndexedY},               // D9 CMP Absolute,Y
    {1, "PHX", EAddrModes::None},                           // DA PHX 
    {1, "STP", EAddrModes::None},                           // DB STP 
    {3, "JMP", EAddrModes::AbsoluteIndirectLong},           // DC JMP [Absolute]
    {3, "CMP", EAddrModes::AbsoluteIndexedX},               // DD CMP Absolute,X
    {3, "DEC", EAddrModes::AbsoluteIndexedX},               // DE DEC Absolute,X
    {4, "CMP", EAddrModes::AbsoluteLongIndexedX},           // DF CMP Long,X
    {2, "CPX", EAddrModes::Immediate},                      // E0 CPX Immediate
    {2, "SBC", EAddrModes::DirectIndexedIndirect},          // E1 SBC (Direct,X)
    {2, "SEP", EAddrModes::None},                           // E2 SEP 
    {2, "SBC", EAddrModes::StackRelative},                  // E3 SBC Stack,S
    {2, "CPX", EAddrModes::Direct},                         // E4 CPX Direct
    {2, "SBC", EAddrModes::Direct},                         // E5 SBC Direct
    {2, "INC", EAddrModes::Direct},                         // E6 INC Direct
    {2, "SBC", EAddrModes::DirectIndirectLong},             // E7 SBC [Direct]
    {1, "INX", EAddrModes::None},                           // E8 INX 
    {2, "SBC", EAddrModes::Immediate},                      // E9 SBC Immediate
    {1, "NOP", EAddrModes::None},                           // EA NOP 
    {1, "XBA", EAddrModes::None},                           // EB XBA 
    {3, "CPX", EAddrModes::Absolute},                       // EC CPX Absolute
    {3, "SBC", EAddrModes::Absolute},                       // ED SBC Absolute
    {3, "INC", EAddrModes::Absolute},                       // EE INC Absolute
    {4, "SBC", EAddrModes::AbsoluteLong},                   // EF SBC Long
    {2, "BEQ", EAddrModes::Relative8},                      // F0 BEQ 
    {2, "SBC", EAddrModes::DirectIndirectIndexed},          // F1 SBC (Direct),Y
    {2, "SBC", EAddrModes::DirectIndirect},                 // F2 SBC (Direct)
    {2, "SBC", EAddrModes::StackRelativeIndirectIndexed},   // F3 SBC (Stack,S),Y
    {3, "PEA", EAddrModes::None},                           // F4 PEA // This addressmode may be incorrect
    {2, "SBC", EAddrModes::DirectIndexedX},                 // F5 SBC Direct,X
    {2, "INC", EAddrModes::DirectIndexedX},                 // F6 INC Direct,X
    {2, "SBC", EAddrModes::DirectIndirectLongIndexed},      // F7 SBC [Direct],Y
    {1, "SED", EAddrModes::None},                           // F8 SED 
    {3, "SBC", EAddrModes::AbsoluteIndexedY},               // F9 SBC Absolute,Y
    {1, "PLX", EAddrModes::None},                           // FA PLX 
    {1, "XCE", EAddrModes::None},                           // FB XCE 
    {3, "JSR", EAddrModes::AbsoluteIndexedIndirect},        // FC JSR (Absolute,X)
    {3, "SBC", EAddrModes::AbsoluteIndexedX},               // FD SBC Absolute,X
    {3, "INC", EAddrModes::AbsoluteIndexedX},               // FE INC Absolute,X
    {4, "SBC", EAddrModes::AbsoluteLongIndexedX},           // FF SBC Long,X
};


static const QSet<uint8_t> ImmediateA = {
    0x09, // ORA
    0x29, // AND
    0x49, // EOR
    0x69, // ADC
    0x89, // BIT
    0xA9, // LDA
    0xC9, // CMP
    0xE9, // SBC
};
static const QSet<uint8_t> ImmediateXY = {
    0xA0, // LDY
    0xA2, // LDX
    0xC0, // CPY
    0xE0, // CPX
};


Opcode Opcode::GetOpcode(Address pc, const uint8_t *memory, const Registers *reg)
{
    Opcode op = opcodes[memory[0]];

    op.address = pc;
    op.addrModeStr = addrModes[(int)op.addrMode].name;
    op.operandStr = addrModes[(int)op.addrMode].argFormat;

    // Handle size of Immediate mode based off the Accumulator/Index size.
    if (op.addrMode == EAddrModes::Immediate && 
        ((ImmediateA.contains(memory[0]) && reg->flags.m == 0) || 
         (ImmediateXY.contains(memory[0]) && reg->flags.x == 0)))
    {
        op.byteCount = 3;
        op.operandStr = "#%04X";
    }
    
    for (int i = 0; i < op.byteCount; i++)
    {
        op.bytesStr += UiUtils::FormatHexByte(memory[i]) + " ";
    }

    if (op.addrMode == EAddrModes::Relative8)
    {
        int8_t signedArg = (int8_t)memory[1];
        op.operandStr.replace("%d", QString::number(signedArg));
    }
    else if (op.addrMode == EAddrModes::Relative16)
    {
        int16_t signedArg = (int16_t)Bytes::Make16Bit(memory[2], memory[1]);
        op.operandStr.replace("%d", QString::number(signedArg));
    }
    else if (op.addrMode == EAddrModes::None)
    {
        // None is also used for some misc instructions with data that don't fit well in the other modes.
        // Just print the bytes if they exist.
        for (int i = 1; i < op.byteCount; i++)
        {
            op.operandStr += UiUtils::FormatHexByte(memory[i]) + " ";
        }
    }
    else
    {
        op.operandStr.replace("%02X", UiUtils::FormatHexByte(memory[1]));
        op.operandStr.replace("%04X", UiUtils::FormatHexWord(memory[2] << 8 | memory[1]));
        op.operandStr.replace("%06X", UiUtils::FormatHexByte(memory[3]) + UiUtils::FormatHexByte(memory[2]) + UiUtils::FormatHexByte(memory[1]));
    }

    return op; // return a copy.
}


Opcode::Opcode(int byteCount, const QString &opcodeStr, EAddrModes addrMode) :
    byteCount(byteCount),
    opcodeStr(opcodeStr),
    addrMode(addrMode)
{

}


QString Opcode::ToString() const
{
    QString ret = UiUtils::FormatHexByte(address.GetBank()) + ":" + UiUtils::FormatHexWord(address.GetOffset()) + " " + opcodeStr + " " + operandStr;    
    return ret;
}