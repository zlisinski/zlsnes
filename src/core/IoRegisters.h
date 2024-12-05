#pragma once

enum EIORegisters
{
// PPU Write-Only
    eRegINIDISP  = 0x2100, // Display Control 1
    eRegOBSEL    = 0x2101, // Object Size and Object Base
    eRegOAMADDL  = 0x2102, // OAM Address (lower 8bit)
    eRegOAMADDH  = 0x2103, // OAM Address (upper 1bit) and Priority Rotation
    eRegOAMDATA  = 0x2104, // OAM Data Write (write-twice)
    eRegBGMODE   = 0x2105, // BG Mode and BG Character Size
    eRegMOSAIC   = 0x2106, // Mosaic Size and Mosaic Enable
    eRegBG1SC    = 0x2107, // BG1 Screen Base and Screen Size
    eRegBG2SC    = 0x2108, // BG2 Screen Base and Screen Size
    eRegBG3SC    = 0x2109, // BG3 Screen Base and Screen Size
    eRegBG4SC    = 0x210A, // BG4 Screen Base and Screen Size
    eRegBG12NBA  = 0x210B, // BG Character Data Area Designation
    eRegBG34NBA  = 0x210C, // BG Character Data Area Designation
    eRegBG1HOFS  = 0x210D, // BG1 Horizontal Scroll (X) (write-twice) / M7HOFS
    eRegBG1VOFS  = 0x210E, // BG1 Vertical Scroll (Y)   (write-twice) / M7VOFS
    eRegBG2HOFS  = 0x210F, // BG2 Horizontal Scroll (X) (write-twice)
    eRegBG2VOFS  = 0x2110, // BG2 Vertical Scroll (Y)   (write-twice)
    eRegBG3HOFS  = 0x2111, // BG3 Horizontal Scroll (X) (write-twice)
    eRegBG3VOFS  = 0x2112, // BG3 Vertical Scroll (Y)   (write-twice)
    eRegBG4HOFS  = 0x2113, // BG4 Horizontal Scroll (X) (write-twice)
    eRegBG4VOFS  = 0x2114, // BG4 Vertical Scroll (Y)   (write-twice)
    eRegVMAIN    = 0x2115, // VRAM Address Increment Mode
    eRegVMADDL   = 0x2116, // VRAM Address (lower 8bit)
    eRegVMADDH   = 0x2117, // VRAM Address (upper 8bit)
    eRegVMDATAL  = 0x2118, // VRAM Data Write (lower 8bit)
    eRegVMDATAH  = 0x2119, // VRAM Data Write (upper 8bit)
    eRegM7SEL    = 0x211A, // Rotation/Scaling Mode Settings
    eRegM7A      = 0x211B, // Rotation/Scaling Parameter A & Maths 16bit operand (write-twice)
    eRegM7B      = 0x211C, // Rotation/Scaling Parameter B & Maths 8bit operand (write-twice)
    eRegM7C      = 0x211D, // Rotation/Scaling Parameter C         (write-twice)
    eRegM7D      = 0x211E, // Rotation/Scaling Parameter D         (write-twice)
    eRegM7X      = 0x211F, // Rotation/Scaling Center Coordinate X (write-twice)
    eRegM7Y      = 0x2120, // Rotation/Scaling Center Coordinate Y (write-twice)
    eRegCGADD    = 0x2121, // Palette CGRAM Address
    eRegCGDATA   = 0x2122, // Palette CGRAM Data Write             (write-twice)
    eRegW12SEL   = 0x2123, // Window BG1/BG2 Mask Settings
    eRegW34SEL   = 0x2124, // Window BG3/BG4 Mask Settings
    eRegWOBJSEL  = 0x2125, // Window OBJ/MATH Mask Settings
    eRegWH0      = 0x2126, // Window 1 Left Position (X1)
    eRegWH1      = 0x2127, // Window 1 Right Position (X2)
    eRegWH2      = 0x2128, // Window 2 Left Position (X1)
    eRegWH3      = 0x2129, // Window 2 Right Position (X2)
    eRegWBGLOG   = 0x212A, // Window 1/2 Mask Logic (BG1-BG4)
    eRegWOBJLOG  = 0x212B, // Window 1/2 Mask Logic (OBJ/MATH)
    eRegTM       = 0x212C, // Main Screen Designation
    eRegTS       = 0x212D, // Sub Screen Designation
    eRegTMW      = 0x212E, // Window Area Main Screen Disable
    eRegTSW      = 0x212F, // Window Area Sub Screen Disable
    eRegCGWSEL   = 0x2130, // Color Math Control Register A
    eRegCGADSUB  = 0x2131, // Color Math Control Register B
    eRegCOLDATA  = 0x2132, // Color Math Sub Screen Backdrop Color
    eRegSETINI   = 0x2133, // Display Control 2

// PPU Read-Only
    eRegMPYL     = 0x2134, //  PPU1 Signed Multiply Result   (lower 8bit)
    eRegMPYM     = 0x2135, //  PPU1 Signed Multiply Result   (middle 8bit)
    eRegMPYH     = 0x2136, //  PPU1 Signed Multiply Result   (upper 8bit)
    eRegSLHV     = 0x2137, //  PPU1 Latch H/V-Counter by Software (Read=Strobe)
    eRegRDOAM    = 0x2138, //  PPU1 OAM Data Read            (read-twice)
    eRegRDVRAML  = 0x2139, //  PPU1 VRAM Data Read           (lower 8bits)
    eRegRDVRAMH  = 0x213A, //  PPU1 VRAM Data Read           (upper 8bits)
    eRegRDCGRAM  = 0x213B, //  PPU2 CGRAM Data Read (Palette)(read-twice)
    eRegOPHCT    = 0x213C, //  PPU2 Horizontal Counter Latch (read-twice)
    eRegOPVCT    = 0x213D, //  PPU2 Vertical Counter Latch   (read-twice)
    eRegSTAT77   = 0x213E, //  PPU1 Status and PPU1 Version Number
    eRegSTAT78   = 0x213F, //  PPU2 Status and PPU2 Version Number

// APU
    eRegAPUI00   = 0x2140, //  Main CPU to Sound CPU Communication Port 0
    eRegAPUI01   = 0x2141, //  Main CPU to Sound CPU Communication Port 1
    eRegAPUI02   = 0x2142, //  Main CPU to Sound CPU Communication Port 2
    eRegAPUI03   = 0x2143, //  Main CPU to Sound CPU Communication Port 3

// WRAM
    eRegWMDATA   = 0x2180, //  WRAM Data Read/Write       (R/W)
    eRegWMADDL   = 0x2181, //  WRAM Address (lower 8bit)  (W)
    eRegWMADDM   = 0x2182, //  WRAM Address (middle 8bit) (W)
    eRegWMADDH   = 0x2183, //  WRAM Address (upper 1bit)  (W)

// Controller
    eRegJOYWR    = 0x4016, //  Joypad Output (W)
    eRegJOYA     = 0x4016, //  Joypad Input Register A (R)
    eRegJOYB     = 0x4017, //  Joypad Input Register B (R)

// CPU Write-Only
    eRegNMITIMEN = 0x4200, //  Interrupt Enable and Joypad Request
    eRegWRIO     = 0x4201, //  Joypad Programmable I/O Port (Open-Collector Output)
    eRegWRMPYA   = 0x4202, //  Set unsigned 8bit Multiplicand
    eRegWRMPYB   = 0x4203, //  Set unsigned 8bit Multiplier and Start Multiplication
    eRegWRDIVL   = 0x4204, //  Set unsigned 16bit Dividend (lower 8bit)
    eRegWRDIVH   = 0x4205, //  Set unsigned 16bit Dividend (upper 8bit)
    eRegWRDIVB   = 0x4206, //  Set unsigned 8bit Divisor and Start Division
    eRegHTIMEL   = 0x4207, //  H-Count Timer Setting (lower 8bits)
    eRegHTIMEH   = 0x4208, //  H-Count Timer Setting (upper 1bit)
    eRegVTIMEL   = 0x4209, //  V-Count Timer Setting (lower 8bits)
    eRegVTIMEH   = 0x420A, //  V-Count Timer Setting (upper 1bit)
    eRegMDMAEN   = 0x420B, //  Select General Purpose DMA Channel(s) and Start Transfer
    eRegHDMAEN   = 0x420C, //  Select H-Blank DMA (H-DMA) Channel(s)
    eRegMEMSEL   = 0x420D, //  Memory-2 Waitstate Control

// CPU Read-Only
    eRegRDNMI    = 0x4210, //  V-Blank NMI Flag and CPU Version Number (Read/Ack)
    eRegTIMEUP   = 0x4211, //  H/V-Timer IRQ Flag (Read/Ack)
    eRegHVBJOY   = 0x4212, //  H/V-Blank flag and Joypad Busy flag (R)
    eRegRDIO     = 0x4213, //  Joypad Programmable I/O Port (Input)
    eRegRDDIVL   = 0x4214, //  Unsigned Division Result (Quotient) (lower 8bit)
    eRegRDDIVH   = 0x4215, //  Unsigned Division Result (Quotient) (upper 8bit)
    eRegRDMPYL   = 0x4216, //  Unsigned Division Remainder / Multiply Product (lower 8bit)
    eRegRDMPYH   = 0x4217, //  Unsigned Division Remainder / Multiply Product (upper 8bit)
    eRegJOY1L    = 0x4218, //  Joypad 1 (gameport 1, pin 4) (lower 8bit)
    eRegJOY1H    = 0x4219, //  Joypad 1 (gameport 1, pin 4) (upper 8bit)
    eRegJOY2L    = 0x421A, //  Joypad 2 (gameport 2, pin 4) (lower 8bit)
    eRegJOY2H    = 0x421B, //  Joypad 2 (gameport 2, pin 4) (upper 8bit)
    eRegJOY3L    = 0x421C, //  Joypad 3 (gameport 1, pin 5) (lower 8bit)
    eRegJOY3H    = 0x421D, //  Joypad 3 (gameport 1, pin 5) (upper 8bit)
    eRegJOY4L    = 0x421E, //  Joypad 4 (gameport 2, pin 5) (lower 8bit)
    eRegJOY4H    = 0x421F, //  Joypad 4 (gameport 2, pin 5) (upper 8bit)

// DMA
    /*
    eRegDMAPx    = 0x43x0, //  DMA/HDMA Parameters
    eRegBBADx    = 0x43x1, //  DMA/HDMA I/O-Bus Address (PPU-Bus aka B-Bus)
    eRegA1TxL    = 0x43x2, //  HDMA Table Start Address (low)  / DMA Curr Addr (low)
    eRegA1TxH    = 0x43x3, //  HDMA Table Start Address (high) / DMA Curr Addr (high)
    eRegA1Bx     = 0x43x4, //  HDMA Table Start Address (bank) / DMA Curr Addr (bank)
    eRegDASxL    = 0x43x5, //  Indirect HDMA Address (low)  / DMA Byte-Counter (low)
    eRegDASxH    = 0x43x6, //  Indirect HDMA Address (high) / DMA Byte-Counter (high)
    eRegDASBx    = 0x43x7, //  Indirect HDMA Address (bank)
    eRegA2AxL    = 0x43x8, //  HDMA Table Current Address (low)
    eRegA2AxH    = 0x43x9, //  HDMA Table Current Address (high)
    eRegNTRLx    = 0x43xA, //  HDMA Line-Counter (from current Table entry)
    */
};