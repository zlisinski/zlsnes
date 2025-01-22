#pragma once

#include <vector>
#include "Zlsnes.h"
#include "Address.h"


class Cartridge
{
public:
    enum ERomType
    {
        eLoROM = 0x00,
        eHiROM = 0x01,
        eLoROMSDD1 = 0x02,
        eLoROMSA1 = 0x03,
        eExHiROM = 0x05,
        //eHiRomSPC7110 = 0x0A
    };

#pragma pack(1)
    struct StandardHeader
    {
        char title[21] = {0};
        uint8_t mode = 0;
        uint8_t chipset = 0;
        uint8_t romSize = 0;
        uint8_t ramSize = 0;
        uint8_t country = 0;
        uint8_t devId = 0;
        uint8_t romVersion = 0;
        uint16_t checksumComplement = 0;
        uint16_t checksum = 0;
    };

    struct ExtendedHeader
    {
        char makerCode[2] = {0};
        char gameCode[4] = {0};
        char reserved[6] = {0};
        uint8_t expansionFlashSize = 0;
        uint8_t expansionRamSize = 0;
        uint8_t specialVersion = 0;
        uint8_t chipsetSubtype = 0;
    };
#pragma pack()

    Cartridge();

    bool LoadRom(const std::string &filename);
    bool SaveSram();
    void Reset();

    uint32_t MapAddress(uint32_t addr, std::vector<uint8_t> **mem);
    uint8_t ReadByte(uint32_t addr);
    void WriteByte(uint32_t addr, uint8_t byte);

    uint8_t *GetBytePtr(uint32_t addr);
    const StandardHeader &GetStandardHeader() const {return standardHeader;}
    const ExtendedHeader &GetExtendedHeader() const {return extendedHeader;}
    ERomType GetRomType() const {return romType;}
    bool IsLoRom() const {return isLoRom;}
    bool IsFastSpeed() const {return isFastSpeed;}
    bool IsInterleaved() const {return isInterleaved;}

protected:
    bool Validate();
    bool FindHeader(size_t headerOffset);

    std::vector<uint8_t> rom;
    std::vector<uint8_t> sram;
    std::string sramFilename;

    StandardHeader standardHeader;
    ExtendedHeader extendedHeader;

    ERomType romType = ERomType::eLoROM;
    bool isLoRom = true;
    bool isFastSpeed = false;
    bool isInterleaved = false;
};